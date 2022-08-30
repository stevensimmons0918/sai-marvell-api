/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
5FFP_COMPHY_56G_PIPE5_X4_4PLL
********************************************************************/
#ifndef MCESD_N5XC56GP5X4_API_H
#define MCESD_N5XC56GP5X4_API_H

#ifdef N5XC56GP5X4

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
MCESD_STATUS API_N5XC56GP5X4_GetFirmwareRev
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
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] tsLocked - True when locked or False when not locked
@param[out] rsLocked - True when locked or False when not locked

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL* tsLocked,
    OUT MCESD_BOOL* rsLocked
);

/**
@brief  Returns the state of readiness for TX and RX

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txReady - True when TX is ready, otherwise False
@param[out] rxReady - True when RX is ready, otherwise False

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
);

/**
@brief  Initializes Receiver.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  timeout - poll timoeut in ms

@note Should only be used when a valid signal is present on the receiver port

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 timeout
);

/**
@brief  Sets the TX equalization parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - TX equalization parameter
@param[in]  paramValue - the value to set

@note PAM2 Valid Taps: PRE, MAIN, POST
@note PAM4 Valid Taps: PRE2, PRE, MAIN, POST

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the TX equalization parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - TX equalization parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
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
MCESD_STATUS API_N5XC56GP5X4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN E_N5XC56GP5X4_POLARITY polarity
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
MCESD_STATUS API_N5XC56GP5X4_GetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    OUT E_N5XC56GP5X4_POLARITY *polarity
);

/**
@brief  Sets the CTLE parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CTLE parameter
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CTLE parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CTLE parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable DFE on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to enable DFE on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get DFE Enable state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE indicates DFE is enabled on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable freezing DFE updates on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to freeze DFE updates on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Freeze DFE Updates state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE indicates DFE updates is frozen on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Gets the value of the DFE tap on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTmb - top, middle or bottom
@param[in]  tap - DFE tap

@param[out] tapValue - signed value of the specified tap

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTmb,
    IN E_N5XC56GP5X4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
);

/**
@brief  Enable or Disable MCU broadcast. When broadcast is enabled, register writes applies to all lanes.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to enable MCU broadcast mode; MCESD_FALSE to disable MCU broadcast mode

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuBroadcast
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
MCESD_STATUS API_N5XC56GP5X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Transmitter

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power on the transmitter, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Transmitter power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE means transmitter has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Receiver

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power on the receiver, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Receiver power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE means receiver has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to enable TX output, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE when TX Output is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxOutputEnable
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
MCESD_STATUS API_N5XC56GP5X4_SetPowerIvRef
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
MCESD_STATUS API_N5XC56GP5X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off PLL for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power up lane PLL, otherwise MCESD_FALSE to power down lane PLL

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of PLL power for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE when lane PLL is powered up, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  mode - E_N5XC56GP5X4_PHYMODE enum to represent SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5XC56GP5X4_PHYMODE mode
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] mode - N5XC56GP5X4_PHYMODE_SERDES.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_N5XC56GP5X4_PHYMODE *mode
);

/**
@brief  Sets the reference frequency and reference clock selection group

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[in]  rxFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[in]  txClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group
@param[in]  rxClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_REFFREQ txFreq,
    IN E_N5XC56GP5X4_REFFREQ rxFreq,
    IN E_N5XC56GP5X4_REFCLK_SEL txClkSel,
    IN E_N5XC56GP5X4_REFCLK_SEL rxClkSel
);

/**
@brief  Gets the reference frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[out] rxFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[out] txClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group
@param[out] rxClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_REFFREQ *txFreq,
    OUT E_N5XC56GP5X4_REFFREQ *rxFreq,
    OUT E_N5XC56GP5X4_REFCLK_SEL *txClkSel,
    OUT E_N5XC56GP5X4_REFCLK_SEL *rxClkSel
);

/**
@brief  Changes TX and RX bitrate to the specified speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txSpeed - E_N5XC56GP5X4_SERDES_SPEED enum that represents the TX bitrate
@param[in]  rxSpeed - E_N5XC56GP5X4_SERDES_SPEED enum that represents the RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SERDES_SPEED txSpeed,
    IN E_N5XC56GP5X4_SERDES_SPEED rxSpeed
);

/**
@brief  Gets the current TX/RX bit rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txSpeed - E_N5XC56GP5X4_SERDES_SPEED that represents the TX bitrate
@param[out] rxSpeed - E_N5XC56GP5X4_SERDES_SPEED that represents the RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SERDES_SPEED *txSpeed,
    OUT E_N5XC56GP5X4_SERDES_SPEED *rxSpeed
);

/**
@brief  Sets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[in]  rxWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATABUS_WIDTH txWidth,
    IN E_N5XC56GP5X4_DATABUS_WIDTH rxWidth
);

/**
@brief  Gets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[out] rxWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *txWidth,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *rxWidth
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuClockFreq
(
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
MCESD_STATUS API_N5XC56GP5X4_GetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
);

/**
@brief  Set align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
);

/**
@brief  Get align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90
);

/**
@brief  Perform TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Start TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@note If running PHY Test, then reset PHY Test by calling StopPhyTest() followed by StartPhyTest().

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Checks TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@param[out] completed - true if completed
@param[out] failed - true if failed


@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
);

/**
@brief  Stops TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@note If running PHY Test, then reset PHY Test by calling StopPhyTest() followed by StartPhyTest().

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Sets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX
@param[in]  training - S_N5XC56GP5X4_TRAIN_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    IN S_N5XC56GP5X4_TRAIN_TIMEOUT *training
);

/**
@brief  Gets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@param[out] training - S_N5XC56GP5X4_TRAIN_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT S_N5XC56GP5X4_TRAIN_TIMEOUT *training
);

/**
@brief  Gets the eye height values after training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] trainedEyeHeight - pointer to S_N5XC56GP5X4_TRAINED_EYE_H

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_TRAINED_EYE_H *trainedEyeHeight
);

/**
@brief  Sets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CDR parameter (E_N5XC56GP5X4_CDR_PARAM)
@param[in]  paramValue - value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CDR parameter (E_N5XC56GP5X4_CDR_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Determines if signal is squelched

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] squelched - MCESD_TRUE if no signal is detected, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
);

/**
@brief  Sets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  threshold - the squelch threshold level (range: N5XC56GP5X4_SQ_THRESH_MIN ~ N5XC56GP5X4_SQ_THRESH_MAX)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
);

/**
@brief  Gets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
);

/**
@brief  Sets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  path - N5XC56GP5X4_PATH_EXTERNAL or N5XC56GP5X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATAPATH path
);

/**
@brief  Gets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] path - N5XC56GP5X4_PATH_EXTERNAL or N5XC56GP5X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATAPATH *path
);

/**
@brief  Gets the temperature in degrees 100 micro-Celsius (factor of 10,000)

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] temperature - value in degrees 100 micro-Celsius (factor of 10,000)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
);

/**
@brief  Sets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txPolarity - set transmitter to N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV
@param[in]  rxPolarity - set receiver to N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_POLARITY txPolarity,
    IN E_N5XC56GP5X4_POLARITY rxPolarity
);

/**
@brief  Gets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPolarity - N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV
@param[out] rxPolarity - N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_POLARITY *txPolarity,
    OUT E_N5XC56GP5X4_POLARITY *rxPolarity
);

/**
@brief  Inject x errors on transmitter data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  errors - inject 1..8 errors

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
);

/**
@brief  Sets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txPattern - E_N5XC56GP5X4_PATTERN for transmitter
@param[in]  rxPattern - E_N5XC56GP5X4_PATTERN for receiver
@param[in]  txUserPattern - string of hexadecimal characters (max 20 characters); valid when tx pattern = N5XC56GP5X4_PAT_USER
@param[in]  rxUserPattern - string of hexadecimal characters (max 20 characters); valid when rx pattern = N5XC56GP5X4_PAT_USER

@note Use this function to configure TX and RX pattern before calling API_N5XC56GP5X4_StartPhyTest()
@note For patterns JITTER_8T and JITTER_4T, databus is adjusted to 32 or 64 bits
@note For patterns JITTER_10T and JITTER_5T, databus is adjusted to 40 or 80 bits

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_PATTERN txPattern,
    IN E_N5XC56GP5X4_PATTERN rxPattern,
    IN const char *txUserPattern,
    IN const char *rxUserPattern
);

/**
@brief  Gets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txPattern - pattern structure for transmitter
@param[out] rxPattern - pattern structure for receiver
@param[out] txUserPattern - string of hexadecimal characters representing the user pattern
@param[out] rxUserPattern - string of hexadecimal characters representing the user pattern

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_PATTERN *txPattern,
    OUT E_N5XC56GP5X4_PATTERN *rxPattern,
    OUT char *txUserPattern,
    OUT char *rxUserPattern
);

/**
@brief  Sets MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for transmitter
@param[in]  rxSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB txSwapMsbLsb,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB rxSwapMsbLsb
);

/**
@brief  Get MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out]  txSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for transmitter
@param[out]  rxSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *txSwapMsbLsb,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *rxSwapMsbLsb
);

/**
@brief  Sets GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txGrayCode - E_N5XC56GP5X4_GRAY_CODE for transmitter
@param[in]  rxGrayCode - E_N5XC56GP5X4_GRAY_CODE for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_GRAY_CODE txGrayCode,
    IN E_N5XC56GP5X4_GRAY_CODE rxGrayCode
);

/**
@brief  Get GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out]  txGrayCode - E_N5XC56GP5X4_GRAY_CODE for transmitter
@param[out]  rxGrayCode - E_N5XC56GP5X4_GRAY_CODE for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_GRAY_CODE *txGrayCode,
    OUT E_N5XC56GP5X4_GRAY_CODE *rxGrayCode
);

/**
@brief  Sets PreCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txState - MCESD_BOOL for transmitter
@param[in]  rxState - MCESD_BOOL for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPreCode
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

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
);

/**
@brief  Gets the pattern comparator statistics

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] statistics - pointer to S_N5XC56GP5X4_PATTERN_STATS which will be populated by this function

@note API_N5XC56GP5X4_StartPhyTest() must be called first to start the pattern checker.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_PATTERN_STATS *statistics
);

/**
@brief  Resets pattern comparator statistics. Zeroes out total bits and errors

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Starts the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Stops the pattern generator & comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Initializes EOM

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to initialize EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Disables EOM Circuit

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to disable EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Get measurement data at phase, voltage

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTMB - N5XC56GP5X4_EYE_TOP, N5XC56GP5X4_EYE_MID or N5XC56GP5X4_EYE_BOT
@param[in]  phase - phase to measure
@param[in]  voltage - voltage to measure (offset from center; both upper and lower voltage are measured)

@param[out] measurement - pointer to S_N5XC56GP5X4_EOM_DATA which will hold the results

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to measure a specific point

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_N5XC56GP5X4_EOM_DATA *measurement
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
MCESD_STATUS API_N5XC56GP5X4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
);

/**
@brief  Returns EYE width and height

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTMB - N5XC56GP5X4_EYE_TOP, N5XC56GP5X4_EYE_MID or N5XC56GP5X4_EYE_BOT

@param[out] width - EYE width
@param[out] height - EYE height

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
);

/**
@brief  Perform CDS

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ExecuteCDS(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* N5XC56GP5X4 */

#endif /* defined MCESD_N5XC56GP5X4_API_H */
