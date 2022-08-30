/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_28G_PIPE4_RPLL_X4_1P2V
********************************************************************/
#ifndef MCESD_C28GP4X4_API_H
#define MCESD_C28GP4X4_API_H

#ifdef C28GP4X4

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
MCESD_STATUS API_C28GP4X4_GetFirmwareRev
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
MCESD_STATUS API_C28GP4X4_GetPLLLock
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
MCESD_STATUS API_C28GP4X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
);

/**
@brief  Returns the state of CDR Lock for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] state - MCESD_TRUE when CDR is locked, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Initializes Receiver.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Should only be used when a valid signal is present on the receiver port

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_RxInit
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
MCESD_STATUS API_C28GP4X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TXEQ_PARAM param,
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
MCESD_STATUS API_C28GP4X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TXEQ_PARAM param,
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
MCESD_STATUS API_C28GP4X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PARAM param,
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
MCESD_STATUS API_C28GP4X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PARAM param,
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
MCESD_STATUS API_C28GP4X4_SetDfeEnable
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
MCESD_STATUS API_C28GP4X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Gets the value of the DFE tap on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  eTap - enum representing a tap

@param[out] tapValue - signed value of the specified tap

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DFE_TAP eTap,
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
MCESD_STATUS API_C28GP4X4_SetMcuEnable
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
MCESD_STATUS API_C28GP4X4_GetMcuEnable
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
MCESD_STATUS API_C28GP4X4_SetLaneEnable
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
MCESD_STATUS API_C28GP4X4_GetLaneEnable
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
MCESD_STATUS API_C28GP4X4_SetMcuBroadcast
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
MCESD_STATUS API_C28GP4X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
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
MCESD_STATUS API_C28GP4X4_SetPowerTx
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
MCESD_STATUS API_C28GP4X4_GetPowerTx
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
MCESD_STATUS API_C28GP4X4_SetPowerRx
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
MCESD_STATUS API_C28GP4X4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  state - MCESD_TRUE to enable TX output, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTxOutputEnable
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
MCESD_STATUS API_C28GP4X4_GetTxOutputEnable
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
MCESD_STATUS API_C28GP4X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Power on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetPowerIvRef
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
MCESD_STATUS API_C28GP4X4_SetPowerPLL
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
MCESD_STATUS API_C28GP4X4_GetPowerPLL
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
MCESD_STATUS API_C28GP4X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_PHYMODE mode
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] mode - enum represents SATA, SAS, SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_PHYMODE *mode
);

/**
@brief  Sets the reference frequency and reference clock selection group

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  refFreq - enum that represents the reference frequency
@param[in]  refClkSel - enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_REFFREQ refFreq,
    IN E_C28GP4X4_REFCLK_SEL refClkSel
);

/**
@brief  Gets the reference frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] refFreq - enum that represents the reference frequency
@param[out] refClkSel - enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_REFFREQ *refFreq,
    OUT E_C28GP4X4_REFCLK_SEL *refClkSel
);

/**
@brief  Changes TX and RX bitrate to the specified speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  speed - E_C28GP4X4_SERDES_SPEED enum that represents the TX/RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SERDES_SPEED speed
);

/**
@brief  Gets the current TX/RX bit rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] speed - E_C28GP4X4_SERDES_SPEED that represents the TX/RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_SERDES_SPEED *speed
);

/**
@brief  Sets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txWidth - E_C28GP4X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[in]  rxWidth - E_C28GP4X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DATABUS_WIDTH txWidth,
    IN E_C28GP4X4_DATABUS_WIDTH rxWidth
);

/**
@brief  Gets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txWidth - E_C28GP4X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[out] rxWidth - E_C28GP4X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_DATABUS_WIDTH *txWidth,
    OUT E_C28GP4X4_DATABUS_WIDTH *rxWidth
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
);

/**
@brief  Set align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
);

/**
@brief  Get align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90
);

/**
@brief  Perform TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
);

/**
@brief  Start TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
);

/**
@brief  Checks TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX

@param[out] completed - true if completed
@param[out] failed - true if failed


@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
);

/**
@brief  Stops TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
);

/**
@brief  Sets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX
@param[in]  training - S_C28GP4X4_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    IN S_C28GP4X4_TRAINING_TIMEOUT *training
);

/**
@brief  Gets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - C28GP4X4_TRAINING_TRX or C28GP4X4_TRAINING_RX

@param[out] training - S_C28GP4X4_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    OUT S_C28GP4X4_TRAINING_TIMEOUT *training
);

/**
@brief  Gets the eye height values after training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] trainedEyeHeight - pointer to S_C28GP4X4_TRAINED_EYE_HEIGHT

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_TRAINED_EYE_HEIGHT *trainedEyeHeight
);

/**
@brief  Sets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CDR parameter (E_C28GP4X4_CDR_PARAM)
@param[in]  paramValue - value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - CDR parameter (E_C28GP4X4_CDR_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable slew rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  slewRateEn - enum represents the slewrate enable state

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_EN slewRateEn
);

/**
@brief  Determines if slew rate is enabled or not

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] slewRateEn - enum represents the slewrate enable state

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_SLEWRATE_EN *slewRateEn
);

/**
@brief  Sets the value of the slew rate parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - slew rate parameter (E_C28GP4X4_SLEWRATE_PARAM)
@param[in]  paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the slew rate parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  param - slew rate parameter (E_C28GP4X4_SLEWRATE_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Determines if signal is squelched

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] squelched - MCESD_TRUE if no signal is detected, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
);

/**
@brief  Sets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  threshold - the squelch threshold level (range: C28GP4X4_SQ_THRESH_MIN ~ C28GP4X4_SQ_THRESH_MAX)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
);

/**
@brief  Gets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
);

/**
@brief  Sets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  path - C28GP4X4_PATH_LOCAL_ANALOG_LB, C28GP4X4_PATH_EXTERNAL or C28GP4X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DATAPATH path
);

/**
@brief  Gets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] path - C28GP4X4_PATH_LOCAL_ANALOG_LB, C28GP4X4_PATH_EXTERNAL or C28GP4X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_DATAPATH *path
);

/**
@brief  Gets the temperature in degrees milli-Celsius

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] temperature - value in degrees milli-Celsius

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
);

/**
@brief  Sets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txPolarity - set transmitter to C28GP4X4_POLARITY_NORMAL or C28GP4X4_POLARITY_INVERTED
@param[in]  rxPolarity - set receiver to C28GP4X4_POLARITY_NORMAL or C28GP4X4_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_POLARITY txPolarity,
    IN E_C28GP4X4_POLARITY rxPolarity
);

/**
@brief  Gets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPolarity - C28GP4X4_POLARITY_NORMAL or C28GP4X4_POLARITY_INVERTED
@param[out] rxPolarity - C28GP4X4_POLARITY_NORMAL or C28GP4X4_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_POLARITY *txPolarity,
    OUT E_C28GP4X4_POLARITY *rxPolarity
);

/**
@brief  Inject x errors on transmitter data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  errors - inject 1..8 errors

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
);

/**
@brief  Sets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txPattern - pattern structure for transmitter
@param[in]  rxPattern - pattern structure for receiver
@param[in]  sataLongShort - C28GP4X4_SATA_SHORT, C28GP4X4_SATA_LONG or C28GP4X4_SATA_NOT_USED
@param[in]  sataInitialDisparity - C28GP4X4_DISPARITY_NEGATIVE, C28GP4X4_DISPARITY_POSITIVE or C28GP4X4_DISPARITY_NOT_USED
@param[in]  userPattern - string of hexadecimal characters (max 20 characters); valid when tx/rx pattern = C28GP4X4_PAT_USER
@param[in]  userK - K character for 8/10 encoder with 64-bit User Pattern

@note Use this function to configure TX and RX pattern before calling API_C28GP4X4_StartPhyTest()

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN S_C28GP4X4_PATTERN *txPattern,
    IN S_C28GP4X4_PATTERN *rxPattern,
    IN E_C28GP4X4_SATA_LONGSHORT sataLongShort,
    IN E_C28GP4X4_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
);

/**
@brief  Gets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPattern - pattern structure for transmitter
@param[out] rxPattern - pattern structure for receiver
@param[out] sataLongShort - C28GP4X4_SATA_SHORT, C28GP4X4_SATA_LONG or C28GP4X4_SATA_NOT_USED
@param[out] sataInitialDisparity - C28GP4X4_DISPARITY_NEGATIVE, C28GP4X4_DISPARITY_POSITIVE or C28GP4X4_DISPARITY_NOT_USED
@param[out] userPattern - string of hexadecimal characters representing the user pattern
@param[out] userK - K character for 8/10 encoder with 64-bit User Pattern

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_PATTERN *txPattern,
    OUT S_C28GP4X4_PATTERN *rxPattern,
    OUT E_C28GP4X4_SATA_LONGSHORT *sataLongShort,
    OUT E_C28GP4X4_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
);

/**
@brief  Gets the pattern comparator statistics

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] statistics - pointer to S_C28GP4X4_PATTERN_STATISTICS which will be populated by this function

@note API_C28GP4X4_StartPhyTest() must be called first to start the pattern checker.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_PATTERN_STATISTICS *statistics
);

/**
@brief  Resets pattern comparator statistics. Zeroes out total bits and errors

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Starts the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_StartPhyTest
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
MCESD_STATUS API_C28GP4X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Initializes EOM

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Called by API_C28GP4X4_EOMGetWidthHeight to initialize EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Disables EOM Circuit

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@note Called by API_C28GP4X4_EOMGetWidthHeight to disable EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Get measurement data at phase, voltage

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  phase - phase to measure
@param[in]  voltage - voltage to measure (offset from center; both upper and lower voltage are measured)

@param[out] measurement - pointer to S_C28GP4X4_EOM_DATA which will hold the results

@note Called by API_C28GP4X4_EOMGetWidthHeight to measure a specific point

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_C28GP4X4_EOM_DATA *measurement
);

/**
@brief  Returns EYE width and height

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] width - EYE width
@param[out] height - EYE height

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
);

/**
@brief  Get Eye Data

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@param[out] eyeRawDataPtr - pointer to S_C28GP4X4_EYE_RAW_PTR which store eye raw data

@note Call API_C28GP4X4_EOMGetWidthHeight before to check if eye is centered

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C28GP4X4_EYE_RAW_PTR eyeRawDataPtr
);

/**
@brief  Plot Eye Data

@param[in]  eyeRawDataPtr - pointer to S_C28GP4X4_EYE_RAW_PTR which store eye raw data
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@note Call API_C28GP4X4_EOMGetEyeData before to populate eyeRawDataPtr
@note Outputs plot through MCESD_DBG_INFO

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_EOMPlotEyeData
(
    IN S_C28GP4X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
);

/**
@brief  Gets DRO

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] freq - Pointer to S_C28GP4X4_DRO_DATA that holds 10 freq results

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetDRO
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X4_DRO_DATA *freq
);

/**
@brief  Set Tx Local Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  value - value to set Local Tx Coefficient Preset Index

@note Values greaters than 0xB are reserved

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 value
);

/**
@brief  Get Tx Local Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] value - value of Local Tx Coefficient Preset Index

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *value
);

/**
@brief  Set Remote Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  value - value to set Remote Preset

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 value
);

/**
@brief  Get Remote Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] value - value of Remote Preset

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *value
);

/**
@brief  Set CTLE Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  parameter - CTLE Preset Parameter
@param[in]  paramValue - value to set parameter

@note Setting any preset parameter will cause FW to use register values instead of default values
@note If preset parameter is set, then all other preset parameters should be set as well
@note Exception is CTLE_PRESET_DEFAULT which will make FW use default preset values

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_SetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PRESET parameter,
    IN MCESD_U32 paramValue
);

/**
@brief  Get Remote Preset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  parameter - CTLE Preset Parameter

@param[out] paramValue - value to set parameter

@note CTLE_PRESET_DEFAULT 0 is FW default, 1 is override from register

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_GetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PRESET parameter,
    OUT MCESD_U32 *paramValue
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C28GP4X4 */

#endif /* defined MCESD_C28GP4X4_API_H */
