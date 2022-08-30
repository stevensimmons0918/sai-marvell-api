/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_12G_PIPE4_1P2V
********************************************************************/
#ifndef MCESD_C12GP41P2V_API_H
#define MCESD_C12GP41P2V_API_H

#ifdef C12GP41P2V

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

/**
@brief  Returns the state of PLL lock

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] locked - True when locked or False when not locked

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *locked
);

/**
@brief  Returns the state of readiness for TX and RX

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] txReady - True when TX is ready, otherwise False
@param[out] rxReady - True when RX is ready, otherwise False

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
);

/**
@brief  Returns the state of CDR Lock for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE when CDR is locked, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Initializes Receiver.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@note Should only be used when a valid signal is present on the receiver port

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_RxInit
(
    IN MCESD_DEV_PTR devPtr
);

/**
@brief  Sets the TX equalization parameter to a value.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - TX equalization parameter
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the TX equalization parameter.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - TX equalization parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Sets the CTLE parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - CTLE parameter (E_C12GP41P2V_CTLE_PARAM)
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CTLE parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - CTLE parameter (E_C12GP41P2V_CTLE_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable DFE on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to enable DFE on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Get DFE Enable state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE indicates DFE is enabled on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable freezing DFE updates on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to freeze DFE updates on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Get Freeze DFE Updates state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE indicates DFE updates is frozen on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Gets the value of the DFE tap on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  eTap - E_C12GP41P2V_DFE_TAP enum representing a tap

@param[out] tapValue - signed value of the specified tap

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DFE_TAP eTap,
    OUT MCESD_32 *tapValue
);

/**
@brief  Set power on/off to Transmitter

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power on the transmitter, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Get Transmitter power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE means transmitter has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Receiver

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power on the receiver, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Get Receiver power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE means receiver has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to enable TX output, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE when TX Output is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetPowerIvRef
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
MCESD_STATUS API_C12GP41P2V_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off PLL for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up lane PLL, otherwise MCESD_FALSE to power down lane PLL

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of PLL power for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE when lane PLL is powered up, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  mode - E_C12GP41P2V_PHYMODE enum to represent SATA, SAS, SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PHYMODE mode
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] mode - C12GP41P2V_PHYMODE_SERDES or C12GP41P2V_PHYMODE_SATA, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_PHYMODE *mode
);

/**
@brief  Sets the reference frequency and reference clock selection group

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  refFreq - E_C12GP41P2V_REFFREQ enum that represents the reference frequency
@param[in]  refClkSel - E_C12GP41P2V_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_REFFREQ refFreq,
    IN E_C12GP41P2V_REFCLK_SEL refClkSel
);

/**
@brief  Gets the reference frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] refFreq - E_C12GP41P2V_REFFREQ enum that represents the reference frequency
@param[out] refClkSel - E_C12GP41P2V_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_REFFREQ *refFreq,
    OUT E_C12GP41P2V_REFCLK_SEL *refClkSel
);

/**
@brief  Changes TX and RX bitrate to the specified speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  speed - E_C12GP41P2V_SERDES_SPEED enum that represents the TX/RX bitrate

@note For 11.5625G, SetRefFreq before calling and call this function before power on.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SERDES_SPEED speed
);

/**
@brief  Gets the current TX/RX bit rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] speed - E_C12GP41P2V_SERDES_SPEED that represents the TX/RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SERDES_SPEED *speed
);

/**
@brief  Sets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  width - E_C12GP41P2V_DATABUS_WIDTH enum that represents the number of bits for TX/RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATABUS_WIDTH width
);

/**
@brief  Gets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] width - E_C28GP4X2_DATABUS_WIDTH enum that represents the number of bits for TX/RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATABUS_WIDTH *width
);

/**
@brief  Set align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 align90
);

/**
@brief  Get align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *align90
);

/**
@brief  Perform TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type - C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
);

/**
@brief  Start TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type - C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
);

/**
@brief  Check TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type - C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX

@param[out] completed - true if completed
@param[out] failed - true if failed

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
);

/**
@brief  Stops TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type - C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
);

/**
@brief  Sets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type - C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX
@param[in]  training - S_C12GP41P2V_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    IN S_C12GP41P2V_TRAINING_TIMEOUT *training
);

/**
@brief  Gets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  type = C12GP41P2V_TRAINING_TRX or C12GP41P2V_TRAINING_RX

@param[out] training - S_C12GP41P2V_TRAINING_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT S_C12GP41P2V_TRAINING_TIMEOUT *training
);

/**
@brief  Sets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - CDR parameter (E_C12GP41P2V_CDR_PARAM)
@param[in]  paramValue - value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - CDR parameter (E_C12GP41P2V_CDR_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable slew rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  slewRateEn - enum represents the slewrate enable state

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_EN slewRateEn
);

/**
@brief  Determines if slew rate is enabled or not

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] slewRateEn - enum represents the slewrate enable state

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SLEWRATE_EN *slewRateEn
);

/**
@brief  Sets the value of the slew rate parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - slew rate parameter (E_C28GP4X2_SLEWRATE_PARAM)
@param[in]  paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the slew rate parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  param - slew rate parameter (E_C12GP41P2V_SLEWRATE_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Determines if signal is squelched

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] squelched - MCESD_TRUE if no signal is detected, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *squelched
);

/**
@brief  Sets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  threshold - the squelch threshold level (range: C12GP41P2V_SQ_THRESH_MIN ~ C12GP41P2V_SQ_THRESH_MAX)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_16 threshold
);

/**
@brief  Gets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_16 *threshold
);

/**
@brief  Sets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  path - C12GP41P2V_PATH_LOCAL_ANALOG_LB, C12GP41P2V_PATH_EXTERNAL or C12GP41P2V_PATH_FAR_END_LB

@note Must be in PHY isolation to set C12GP41P2V_PATH_LOCAL_ANALOG_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATAPATH path
);

/**
@brief  Gets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] path - C12GP41P2V_PATH_LOCAL_ANALOG_LB, C12GP41P2V_PATH_EXTERNAL or C12GP41P2V_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATAPATH *path
);

/**
@brief  Sets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  txPolarity - set transmitter to C12GP41P2V_POLARITY_NORMAL or C12GP41P2V_POLARITY_INVERTED
@param[in]  rxPolarity - set receiver to C12GP41P2V_POLARITY_NORMAL or C12GP41P2V_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_POLARITY txPolarity,
    IN E_C12GP41P2V_POLARITY rxPolarity
);

/**
@brief  Gets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] txPolarity - C12GP41P2V_POLARITY_NORMAL or C12GP41P2V_POLARITY_INVERTED
@param[out] rxPolarity - C12GP41P2V_POLARITY_NORMAL or C12GP41P2V_POLARITY_INVERTED

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_POLARITY *txPolarity,
    OUT E_C12GP41P2V_POLARITY *rxPolarity
);

/**
@brief  Inject x errors on transmitter data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  errors - inject 1..8 errors

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 errors
);

/**
@brief  Sets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  txPattern - pattern structure for transmitter
@param[in]  rxPattern - pattern structure for receiver
@param[in]  sataLongShort - C12GP41P2V_SATA_SHORT, C12GP41P2V_SATA_LONG or C12GP41P2V_SATA_NOT_USED
@param[in]  sataInitialDisparity - C12GP41P2V_DISPARITY_NEGATIVE, C12GP41P2V_DISPARITY_POSITIVE or C12GP41P2V_DISPARITY_NOT_USED
@param[in]  userPattern - string of hexadecimal characters (max 20 characters); valid when tx/rx pattern = C12GP41P2V_PAT_USER
@param[in]  userK - K character for 8/10 encoder

@note Use this function to configure TX and RX pattern before calling API_C12GP41P2V_StartPhyTest()

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PATTERN *txPattern,
    IN S_C12GP41P2V_PATTERN *rxPattern,
    IN E_C12GP41P2V_SATA_LONGSHORT sataLongShort,
    IN E_C12GP41P2V_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
);

/**
@brief  Gets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] txPattern - pattern structure for transmitter
@param[out] rxPattern - pattern structure for receiver
@param[out] sataLongShort - C12GP41P2V_SATA_SHORT, C12GP41P2V_SATA_LONG or C12GP41P2V_SATA_NOT_USED
@param[out] sataInitialDisparity - C12GP41P2V_DISPARITY_NEGATIVE, C12GP41P2V_DISPARITY_POSITIVE or C12GP41P2V_DISPARITY_NOT_USED
@param[out] userPattern - string of hexadecimal characters representing the user pattern
@param[out] userK - K character for 8/10 encoder with 64-bit User Pattern

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN *txPattern,
    OUT S_C12GP41P2V_PATTERN *rxPattern,
    OUT E_C12GP41P2V_SATA_LONGSHORT *sataLongShort,
    OUT E_C12GP41P2V_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
);

/**
@brief  Gets the pattern comparator statistics

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] statistics - pointer to S_C12GP41P2V_PATTERN_STATISTICS which will be populated by this function

@note API_C12GP41P2V_StartPhyTest() must be called first to start the pattern checker.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN_STATISTICS *statistics
);

/**
@brief  Resets pattern comparator statistics. Zeroes out total bits and errors

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr
);

/**
@brief  Starts the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr
);

/**
@brief  Stops the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr
);

/**
@brief  Initializes EOM

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] savedFields - pointer to S_C12GP41P2V_EOM_SAVED which will be populated by this function
@param[out] phaseOffset - permanent phase offset

@note Called by API_C12GP41P2V_EOMGetWidthHeight to initialize EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_EOM_SAVED *savedFields,
    OUT MCESD_16 *phaseOffset
);

/**
@brief  Disables EOM Circuit

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  savedFields - saved field values in S_C12GP41P2V_EOM_SAVED
@param[in]  softwarePhase - current phase value kept track by software
@param[in]  phaseOffset - permanent phase offset

@note Called by API_C12GP41P2V_EOMGetWidthHeight to disable EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_EOM_SAVED savedFields,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset
);

/**
@brief  Get measurement data at phase, voltage

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  phase - phase to measure
@param[in]  voltage - voltage to measure (offset from center; both upper and lower voltage are measured)
@param[in]  softwarePhase - current phase value kept track by software
@param[in]  phaseOffset - permanent phase offset

@param[out] measurement - pointer to S_C28GP4X2_EOM_DATA which will hold the results

@note Called by API_C12GP41P2V_EOMGetWidthHeight to measure a specific point

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset,
    OUT S_C12GP41P2V_EOM_DATA *measurement
);

/**
@brief  Returns EYE width and height

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] width - EYE width
@param[out] height - EYE height

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
);

/**
@brief  Get Eye Data

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@param[out] eyeRawDataPtr - pointer to S_C12GP41P2V_EYE_RAW_PTR which store eye raw data

@note Call API_C12GP41P2V_EOMGetWidthHeight before to check if eye is centered

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr
);

/**
@brief  Plot Eye Data

@param[in]  eyeRawDataPtr - pointer to S_C12GP41P2V_EYE_RAW_PTR which store eye raw data
@param[in]  voltageSteps - voltage range to sweep (0 for DEFAULT)
@param[in]  phaseLevels - phase range to sweep (0 for DEFAULT)

@note Call API_C12GP41P2V_EOMGetEyeData before to populate eyeRawDataPtr
@note Outputs plot through MCESD_DBG_INFO

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_EOMPlotEyeData
(
    IN S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C12GP41P2V */

#endif /* defined MCESD_C12GP41P2V_API_H */
