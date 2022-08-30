/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_112G_X4
********************************************************************/
#ifndef MCESD_C112GX4_API_H
#define MCESD_C112GX4_API_H

#ifdef C112GX4

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/**
@brief  Returns the version number of SERDES MCU firmware

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] major - major version number
@param[out] minor - minor version number
@param[out] patch - patch version number
@param[out] build - build version number

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
);

/**
@brief  Returns the state of PLL lock

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] locked - True when locked or False when not locked

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *locked
);

/**
@brief  Returns the state of readiness for TX and RX

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txReady - True when TX is ready, otherwise False
@param[out] rxReady - True when RX is ready, otherwise False

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
);


/**
@brief  Initializes Receiver.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Should only be used when a valid signal is present on the receiver port

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Sets the TX equalization parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - TX equalization parameter
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the TX equalization parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - TX equalization parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Sets the CTLE parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CTLE parameter
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CTLE parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CTLE parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable DFE on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to enable DFE on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get DFE Enable state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE indicates DFE is enabled on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable freezing DFE updates on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to freeze DFE updates on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Freeze DFE Updates state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE indicates DFE updates is frozen on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Get DFE Enable state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] acqRate - data acquisition rate (e.g. C112GX4_RATE_QUARTER)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDataAcquisitionRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATA_ACQ_RATE *acqRate
);

/**
@brief  Gets the value of the DFE tap on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  eyeTmb - C112GX4_EYE_TOP, C112GX4_EYE_MID or C112GX4_EYE_BOT
@param[in]  tap - DFE tap

@param[out] tapValue - signed value of the specified tap (milli-codes)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTmb,
    IN E_C112GX4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
);

/**
@brief  Enable or Disable MCU on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to enable MCU, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Gets whether MCU is enabled or disabled on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE indcates MCU is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable or disable a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to enable lane, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Gets whether lane is enabled or disabled on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE indicates lane is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable or Disable MCU broadcast. When broadcast is enabled, register writes applies to all lanes.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to enable MCU broadcast mode; MCESD_FALSE to disable MCU broadcast mode

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Returns the state of MCU broadcast

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE when MCU broadcast is enabled; otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off PLL for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to power up lane PLL, otherwise MCESD_FALSE to power down lane PLL

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of PLL power for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE when lane PLL is powered up, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Transmitter

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to power on the transmitter, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Transmitter power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE means transmitter has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Receiver

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to power on the receiver, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Receiver power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE means receiver has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  mode - enum represents SATA, SAS, SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PHYMODE mode
);

/**
@brief  Gets PHYMODE

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] mode - enum represents SATA, SAS, SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_PHYMODE *mode
);

/**
@brief  Sets the reference frequency and reference clock selection group

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  freq - enum that represents the reference frequency
@param[in]  clkSel - enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_REFFREQ freq,
    IN E_C112GX4_REFCLK_SEL clkSel
);

/**
@brief  Gets the reference frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] refFreq - enum that represents the reference frequency
@param[out] refClkSel - enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_REFFREQ *refFreq,
    OUT E_C112GX4_REFCLK_SEL *refClkSel
);

/**
@brief  Changes TX and RX bitrate to the specified speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  speed - enum that represents the TX/RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_SERDES_SPEED speed
);

/**
@brief  Gets the current TX/RX bit rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] speed - enum that represents the TX/RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_SERDES_SPEED *speed
);

/**
@brief  Sets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txWidth - enum that represents the number of bits for TX databus
@param[in]  rxWidth - enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_DATABUS_WIDTH txWidth,
    IN E_C112GX4_DATABUS_WIDTH rxWidth
);

/**
@brief  Gets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txWidth - enum that represents the number of bits for TX databus
@param[out] rxWidth - enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATABUS_WIDTH *txWidth,
    OUT E_C112GX4_DATABUS_WIDTH *rxWidth
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
);

/**
@brief  Gets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
);

/**
@brief  Enable/Disable TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to enable TX output, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE when TX Output is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Get on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Perform TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
);

/**
@brief  Start TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
);

/**
@brief  Checks TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX

@param[out] completed - true if completed
@param[out] failed - true if failed


@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
);

/**
@brief  Stops TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
);

/**
@brief  Sets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX
@param[in]  training - S_C56GX4_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    IN S_C112GX4_TRAINING_TIMEOUT *training
);

/**
@brief  Gets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C112GX4_TRAINING_TRX or C112GX4_TRAINING_RX

@param[out] training - S_C56GX4_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    OUT S_C112GX4_TRAINING_TIMEOUT *training
);

/**
@brief  Initializes EOM

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Called by API_C112GX4_EOMGetWidthHeight to initialize EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Disables EOM Circuit

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Called by API_C112GX4_EOMGetWidthHeight to disable EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Get measurement data at phase, voltage

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  eyeTMB - C112GX4_EYE_TOP, C112GX4_EYE_MID or C112GX4_EYE_BOT
@param[in]  phase - phase to measure
@param[in]  voltage - voltage to measure (offset from center; both upper and lwoer voltage are measured)

@param[out] measurement - pointer to S_C112GX4_EOM_DATA which will hold the results

@note Called by API_C112GX4_EOMGetWidthHeight to measure a specific point
@note Requires a valid signal at the receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_C112GX4_EOM_DATA *measurement
);

/**
@brief  Returns the number of phase steps for 1 UI at the current speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] phaseStepCount - step count of phase
@param[out] voltageStepCount - step count of voltage

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
);

/**
@brief  Performs eye opening measurement and returns width and height

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  eyeTMB - C112GX4_EYE_TOP, C112GX4_EYE_MID or C112GX4_EYE_BOT

@param[out] width - EYE width
@param[out] heightUpper - upper EYE height
@param[out] heightLower - lower EYE height

@note Requires a valid signal at the receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *heightUpper,
    OUT MCESD_U16 *heightLower
);

/**
@brief  Gets the eye height values after training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] trainedEyeHeight - pointer to S_C112GX4_TRAINED_EYE_HEIGHT

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C112GX4_TRAINED_EYE_HEIGHT *trainedEyeHeight
);

/**
@brief  Sets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txPattern - E_C112GX4_PATTERN for transmitter
@param[in]  rxPattern - E_C112GX4_PATTERN for receiver
@param[in]  userPattern - string of hexadecimal characters (max 20 characters); valid when tx/rx pattern = C112GX4_PAT_USER

@note Use this function to configure TX and RX pattern before calling API_C56GX4_StartPhyTest()
@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_PATTERN txPattern,
    IN E_C112GX4_PATTERN rxPattern,
    IN const char *userPattern
);

/**
@brief  Gets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPattern - E_C112GX4_PATTERN for transmitter
@param[out] rxPattern - E_C112GX4_PATTERN for receiver
@param[out] userPattern - string of hexadecimal characters

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_PATTERN *txPattern,
    OUT E_C112GX4_PATTERN *rxPattern,
    OUT char *userPattern
);

/**
@brief  Sets MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txSwapMsbLsb - E_C112GX4_SWAP_MSB_LSB for transmitter
@param[in]  rxSwapMsbLsb - E_C112GX4_SWAP_MSB_LSB for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_SWAP_MSB_LSB txSwapMsbLsb,
    IN E_C112GX4_SWAP_MSB_LSB rxSwapMsbLsb
);

/**
@brief  Get MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out]  txSwapMsbLsb - E_C112GX4_SWAP_MSB_LSB for transmitter
@param[out]  rxSwapMsbLsb - E_C112GX4_SWAP_MSB_LSB for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_SWAP_MSB_LSB *txSwapMsbLsb,
    OUT E_C112GX4_SWAP_MSB_LSB *rxSwapMsbLsb
);

/**
@brief  Sets GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txGrayCode - E_C112GX4_GRAY_CODE for transmitter
@param[in]  rxGrayCode - E_C112GX4_GRAY_CODE for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_GRAY_CODE txGrayCode,
    IN E_C112GX4_GRAY_CODE rxGrayCode
);

/**
@brief  Get GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out]  txGrayCode - E_C112GX4_GRAY_CODE for transmitter
@param[out]  rxGrayCode - E_C112GX4_GRAY_CODE for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_GRAY_CODE *txGrayCode,
    OUT E_C112GX4_GRAY_CODE *rxGrayCode
);

/**
@brief  Sets PreCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txState - MCESD_BOOL for transmitter
@param[in]  rxState - MCESD_BOOL for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txState,
    IN MCESD_BOOL rxState
);

/**
@brief  Get PreCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out]  txState - MCESD_BOOL for transmitter
@param[out]  rxState - MCESD_BOOL for receiver

@note 112G doesn't support SATA patterns
@note 112G doesn't support 8B10B encoding

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
);

/**
@brief  Starts the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Stops the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Executing Control DFE Scheme sequence

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_ExecuteCDS
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Gets the pattern comparator statistics

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] statistics - pointer to S_C112GX4_PATTERN_STATISTICS which will be populated by this function

@note API_C112GX4_StartPhyTest() must be called first to start the pattern checker.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C112GX4_PATTERN_STATISTICS *statistics
);

/**
@brief  Resets pattern comparator statistics. Zeroes out total bits and errors

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Determines if signal is squelched

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] squelched - MCESD_TRUE if no signal is detected, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
);

/**
@brief  Sets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 threshold
);

/**
@brief  Gets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U8 *threshold
);

/**
@brief  Set align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 align90
);

/**
@brief  Get align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] align90 - align90 value*
@param[out] trainingResult - training result in two's complement *
 *

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *align90,
    OUT MCESD_16 *trainingResult
);

/**
@brief  Inject X errors on transmitter data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  errors - inject 1 - 8 errors

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
);

/**
@brief  Sets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CDR parameter (E_C112GX4_CDR_PARAM)
@param[in]  paramValue - value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CDR_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CDR parameter (E_C112GX4_CDR_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Gets the temperature in degrees milli-Celsius

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] temperature - value in degrees milli-Celsius

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
);

/**
@brief  Sets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txPolarity - set transmitter to C56GX4_POLARITY_NORMAL or C56GX4_POLARITY_INVERTED
@param[in]  rxPolarity - set receiver to C56GX4_POLARITY_NORMAL or C56GX4_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_POLARITY txPolarity,
    IN E_C112GX4_POLARITY rxPolarity
);

/**
@brief  Gets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPolarity - C56GX4_POLARITY_NORMAL or C56GX4_POLARITY_INVERTED
@param[out] rxPolarity - C56GX4_POLARITY_NORMAL or C56GX4_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_POLARITY *txPolarity,
    OUT E_C112GX4_POLARITY *rxPolarity
);

/**
@brief  Sets the PHYTEST Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  path - C112GX4_PATH_EXTERNAL or C112GX4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_DATAPATH path
);

/**
@brief  Gets the PHYTEST Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] path - C112GX4_PATH_EXTERNAL or C112GX4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATAPATH *path
);

/**
@brief  Looks up DFE_F0 in table

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  value - DFE_F0 value

@param[out] value100uV - value converted to 100uV

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDfeF0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 value,
    OUT MCESD_U16 *value100uV
);

/**
@brief  Converts width and height code to ps and 0.1mv

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  width - width value in code
@param[in]  heightUpper - upper height value in code
@param[in]  heightLower - lower height value in code

@param[out] widthmUI - width converted to mUI
@param[out] height100uV - height converted to 100uV

@note Converts width to mUI by code * 1000 / maxUI
@note Converts upper and lower height code by calling API_C112GX4_GetDfeF0( code ) and taking the sum

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMConvertWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 width,
    IN MCESD_U16 heightUpper,
    IN MCESD_U16 heightLower,
    OUT MCESD_U16 *widthmUI,
    OUT MCESD_U16 *height100uV
);

/**
@brief  Get Eye Data

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  eyeTMB - C112GX4_EYE_TOP, C112GX4_EYE_MID or C112GX4_EYE_BOT
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@param[out] eyeRawDataPtr - pointer to S_C112GX4_EYE_RAW_PTR which store eye raw data

@note Call API_C112GX4_EOMGetWidthHeight before to check if eye is centered

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C112GX4_EYE_RAW_PTR eyeRawDataPtr
);

/**
@brief  Plot Eye Data

@param[in]  eyeRawDataPtr - pointer to S_C112GX4_EYE_RAW_PTR which store eye raw data
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@note Call API_C112GX4_EOMGetEyeData before to populate eyeRawDataPtr
@note Outputs plot through MCESD_DBG_INFO

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_EOMPlotEyeData
(
    IN S_C112GX4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
);

/**
@brief  Display's Tx Training Log

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  logArrayDataPtr - pointer to array of S_C112GX4_TRX_TRAINING_LOGENTRY
@param[in]  logArraySizeEntries - size of array

@param[out] validEntries - number of valid entries in provided array

@note Outputs log through MCESD_DBG_INFO
@note Maximum number of entries is 23

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_DisplayTrainingLog
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN S_C112GX4_TRX_TRAINING_LOGENTRY logArrayDataPtr[],
    IN MCESD_U32 logArraySizeEntries,
    OUT MCESD_U32* validEntries
);

/**
@brief  Sets the TX equalization parameter's polarity.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - TX equalization parameter
@param[in]  polarity - 0 for normal and 1 for inverted

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN E_C112GX4_POLARITY polarity
);

/**
@brief  Gets the polarity of the TX equalization parameter.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - TX equalization parameter

@param[out] polarity - 0 for normal and 1 for inverted

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT E_C112GX4_POLARITY* polarity
);

/**
@brief  Gets DRO

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] freq - Pointer to S_C112GX4_DRO_DATA that holds 10 freq results

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C112GX4_GetDRO
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C112GX4_DRO_DATA *freq
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C112GX4 */

#endif /* defined MCESD_C112GX4_API_H */
