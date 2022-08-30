/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalPortMgr.h

#ifndef _cpssHalPortMgr_h_
#define _cpssHalPortMgr_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/cpssTypes.h>


/**
 * \file cpssHalPortMgr.h
 * \brief This file contains API prototypes and type definitions
 *        for the cpssHalLink Mac Manager(Wrappers)
 */

/**
 * \brief Get the link status for a specific port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 1o 127.
 * \param [out] status Link status
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *status);

/**
 * \brief Get the speed for a specific port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 1o 127.
 * \param [out] speed - Enum specifies the port speed
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                                    xpSpeed *speed);

/**
 * \brief Enable or Disable a given range of ports
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 1o 127.
 * \param [in] enable true to enable the port, false to disable the port
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrPortEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable);

/**
 * \brief It will set the fec mode for given port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] fecMode fec mode of the port. RS and FC are the availbale fec modes
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */

XP_STATUS cpssHalMacMgrFecModeSet(int devId, uint32_t portNum,
                                  xpFecMode fecMode);


/**
 * \brief If fec is enable it will give the fec mode else MAX_FEC_MODE enum value 2 will come
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] fecMode fec mode of the port. RS and FC are the availbale fec modes
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
 */

XP_STATUS cpssHalMacMgrFecModeGet(xpsDevice_t devId, uint32_t portNum,
                                  xpFecMode *fecMode);

/**
 * \brief Get Lane Swap Info
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] cpssRxSerdesLaneSwapInfo
 * \param [in] cpssTxSerdesLaneSwapInfo
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalPortMgrPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                      uint32_t portNum, uint32_t* rxSerdesLaneArr, uint32_t* txSerdesLaneArr);

/**
 * \brief Get Mac Config Mode
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] xpMacConfigMode
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacMgrConfigModeGet(int devId, uint32_t portNum,
                                     xpMacConfigMode* macConfigMode);

/**
 * \brief Get Port Status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] linkStatus
 * \param [out] faultStatus
 * \param [out] serdesStatus
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacMgrGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                                     bool *linkStatus, bool *faultStatus, bool *serdesStatus);

/**
 * \brief Port Init
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] xpMacConfigMode macConfigMode
 * \param [in] bool initSerdes
 * \param [in] bool prbsTestMode
 * \param [in] bool firmwareUpload
 * \param [in] xpFecMode fecMode
 * \param [in] enableFEC
 * \param [in] keepPortDown
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacMgrPortInitWithLinkStatusControl(xpsDevice_t devId,
                                                     uint32_t portNum, xpMacConfigMode macConfigMode,
                                                     CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType, bool initSerdes,
                                                     bool prbsTestMode, bool firmwareUpload, xpFecMode fecMode, uint8_t enableFEC,
                                                     uint8_t keepPortDown, uint8_t initPort);



/**
 * \brief API to Print  mac loopback status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [out] status
 */
XP_STATUS cpssHalMacMgrLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                   xpMacLoopbackLevel loopback,  uint8_t *status);

/**
 * \brief API to set mac loopback.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [out] status
 */
XP_STATUS cpssHalMacMgrLoopback(xpsDevice_t devId, uint32_t portNum,
                                xpMacLoopbackLevel loopback, uint8_t enable);


/**
 * \brief Serdes Tuning
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portList List of ports associated with the serdes to tune
 * \param [in] numOfPort Number of ports to tune
 * \param [in] tuneMode DFE serdes tuning type
 * \param [in] force Force serdes tuning
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrPortSerdesTune(int devId, xpsPort_t *portList,
                                      uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force);
/**
 * \brief DFE running status.
 * \param [in] devId device Id. Valid values are 0-63.
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 * \param [out] isDfeRunning DFE running status.
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrPortSerdesIsDfeRunning(xpsDevice_t devId,
                                              xpsPort_t portNum,
                                              uint8_t *isDfeRunning);

/**
 * \brief DFE running Debug.
 * \param [in] devId device Id. Valid values are 0-63.
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 * \param [out] txTuneStatusPtr DFE running status.
 *
 * \param [out] rxTuneStatusPtr DFE running status
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrPortSerdesIsDfeRunningDebug(xpsDevice_t devId,
                                                   xpsPort_t portNum, uint8_t *rxTuneStatusPtr, uint8_t *txTuneStatusPtr);


/**
 * \brief Tx Tuning
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portList List of ports associated with the serdes to tune
 * \param [in] numOfPort Number of ports to tune
 * \param [in] force Force serdes tuning
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacMgrPortSerdesTxTune(int devId, xpsPort_t *portList,
                                        uint32_t numOfPort, uint8_t force);
/**
 * \brief API to Print  serdes loopback status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] status
 */

XP_STATUS cpssHalMgrSerdesLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *status);

/**
 * \brief API to set mac loopback.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] enable
 *
 ** \return XP_STATUS
 */

XP_STATUS cpssHalMgrSerdesLoopback(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 * \brief API to deInit a port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalMacMgrPortDeInit(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief API to init a port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum
 * \param [in] macConfigMode
 * \param [in] fecMode
 * \param [in] enableFEC
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalMacMgrPortInit(xpsDevice_t devId, uint32_t portNum,
                                xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC);

/**
 * \brief API to set AN on a port
 *
 * \param [in] devId device Id
 * \param [in] portNum
 * \param [in] portANAbility defines the abilities of port to be advertised
 * \param [in] portANCtrl defines fecMode to be used
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalPortMgrAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t portANAbility, uint16_t portANCtrl, uint8_t keepPortDown);

/**
 * \brief API to set AN on a port
 *
 * \param [in] devId device Id
 * \param [in] portNum
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalPortMgrAutoNegDisable(xpsDevice_t devId, uint32_t portNum);



/**
 *\brief: Gives Remote ANL ability
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] portRemoteANAbility - remote port abilities
 *\param [in] portRemoteANCtrl - remote port AN control
 *
*/
XP_STATUS cpssHalMacMgrPortANLtRemoteAbilityGet(xpDevice_t devId, uint32_t port,
                                                uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl);

/**
 *\brief: Gives FD ability
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] portAdvertFdAbility - port abilities
 *
*/
XP_STATUS cpssHalMacMgrGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                        uint32_t *portAdvertFdAbility);
#ifdef __cplusplus
}
#endif

#endif
