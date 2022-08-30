/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalPort.h

#ifndef _cpssHalPort_h_
#define _cpssHalPort_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>


/**
 * \file cpssHalPort.h
 * \brief This file contains API prototypes and type definitions
 *        for the cpssHalLink Mac Manager(Wrappers)
 */
#define CPSSHAL_PORT_ADVERT_SPEED_NONE            0x00000000
#define CPSSHAL_PORT_ADVERT_SPEED_10MB            0x00000001
#define CPSSHAL_PORT_ADVERT_SPEED_100MB           0x00000002
#define CPSSHAL_PORT_ADVERT_SPEED_1000MB          0x00000004
#define CPSSHAL_PORT_ADVERT_SPEED_10G             0x00000008
#define CPSSHAL_PORT_ADVERT_SPEED_25G             0x00000010
#define CPSSHAL_PORT_ADVERT_SPEED_40G             0x00000020
#define CPSSHAL_PORT_ADVERT_SPEED_50G             0x00000040
#define CPSSHAL_PORT_ADVERT_SPEED_100G            0x00000080
#define CPSSHAL_PORT_ADVERT_SPEED_200G            0x00000100  //doubt
#define CPSSHAL_PORT_ADVERT_SPEED_400G            0x00000200  //doubt


/**
 * \brief Get physical port number of cpu
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] portNum CPU Port number.
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

XP_STATUS cpssHalGlobalSwitchControlGetCpuPhysicalPortNum(int deviceId,
                                                          uint32_t* portNum);

/**
 * \brief Get the link status for a specific port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 1o 127.
 * \param [out] status Link status
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
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

XP_STATUS cpssHalMacGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                                 xpSpeed *speed);

/**
 * \brief Get operational speed for a specific port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 1o 127.
 * \param [out] speed - Enum specifies the port speed
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalGetPortOperSpeed(xpsDevice_t devId, uint32_t portNum,
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

XP_STATUS cpssHalMacPortEnable(xpsDevice_t devId, uint32_t portNum,
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

XP_STATUS cpssHalMacFecModeSet(int devId, uint32_t portNum, xpFecMode fecMode);


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

XP_STATUS cpssHalMacFecModeGet(xpsDevice_t devId, uint32_t portNum,
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
XP_STATUS cpssHalPortPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                   uint32_t portNum, uint32_t* rxSerdesLaneArr, uint32_t* txSerdesLaneArr);

/**
 * \brief Get Mac Config Mode
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] xpMacConfigMode
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacConfigModeGet(int devId, uint32_t portNum,
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
XP_STATUS cpssHalMacGetPortStatus(xpsDevice_t devId, uint32_t portNum,
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
XP_STATUS cpssHalMacPortInitWithLinkStatusControl(xpsDevice_t devId,
                                                  uint32_t portNum, xpMacConfigMode macConfigMode,
                                                  CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType, bool initSerdes,
                                                  bool prbsTestMode, bool firmwareUpload, xpFecMode fecMode, uint8_t enableFEC,
                                                  uint8_t keepPortDown, uint8_t initPort);


/**
 * \brief API to Get the MRU size for port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] frameSize, holds MRU size
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 */
XP_STATUS cpssHalPortGetMru(xpsDevice_t devId, uint32_t portNum,
                            uint16_t *frameSize);

/**
 * \brief API to Print  mac loopback status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [out] status
 */
XP_STATUS cpssHalMacLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                xpMacLoopbackLevel loopback,  uint8_t *status);

/**
 * \brief API to set mac loopback.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [out] status
 */
XP_STATUS cpssHalMacLoopback(xpsDevice_t devId, uint32_t portNum,
                             xpMacLoopbackLevel loopback, uint8_t enable);

/**
* \brief API to set rxMaxfrm len.
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] portNum Valid values are 0-127 and 255
* \param [in] framesize
* \param [out] status
*/
XP_STATUS cpssHalPortSetMru(xpsDevice_t devId, uint32_t portNum,
                            uint16_t frameSize);

/**
 * \brief Checking the given port is valid or not
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS cpssHalPortIsPortNumValid(int devId, uint32_t portNum);

/**
 * \brief To get max channel of mac.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] macMaxChanNum mac max channel number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS cpssHalPortGlobalSwitchControlGetMacMaxChannelNumber(int devId,
                                                               uint8_t* macMaxChanNum);

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

XP_STATUS cpssHalMacPortSerdesTune(int devId, xpsPort_t *portList,
                                   uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force);
/**
 * \brief DFE running status.
 * \param [in] devId device Id. Valid values are 0-63.
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 * \param [out] isDfeRunning DFE running status.
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalMacPortSerdesIsDfeRunning(xpsDevice_t devId, xpsPort_t portNum,
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

XP_STATUS cpssHalMacPortSerdesIsDfeRunningDebug(xpsDevice_t devId,
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

XP_STATUS cpssHalMacPortSerdesTxTune(int devId, xpsPort_t *portList,
                                     uint32_t numOfPort, uint8_t force);
/**
 * \brief API to Print  serdes loopback status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] status
 */

XP_STATUS cpssHalSerdesLoopbackGet(xpsDevice_t devId, uint32_t portNum,
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

XP_STATUS cpssHalSerdesLoopback(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable);

/**
 * \brief API to deInit a port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalMacPortDeInit(xpsDevice_t devId, uint32_t portNum);

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
XP_STATUS cpssHalMacPortInit(xpsDevice_t devId, uint32_t portNum,
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
XP_STATUS cpssHalPortAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t portANAbility, uint16_t portANCtrl, uint8_t keepPortDown);

/**
 * \brief API to set AN on a port
 *
 * \param [in] devId device Id
 * \param [in] portNum
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalPortAutoNegDisable(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief API to enable/disable port mirrioring
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] analyzerId
 * \param [in] ingress
 * \param [in] enable
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalPortMirrorEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint32_t analyzerId, bool ingress, bool enable);

XP_STATUS cpssHalPortMacSaBaseSet(int devId, GT_ETHERADDR *mac);
XP_STATUS cpssHalPortMacSaLsbSet(xpsDevice_t devId, uint32_t port,
                                 uint8_t macLsb);
/**
 *\brief: returns the AN state for HAL port
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *
 *\return [in] enable - gives state enable or disable
 */

int cpssHalPortMacAnEnableGet(xpsDevice_t devId, uint32_t portNum);

/**
 *\brief: sets the AN state for HAL port
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] enable - set state enable or disable
 *
*/
void cpssHalPortMacAnEnable(xpsDevice_t devId, uint32_t portNum, int enable);

/**
 *\brief: Gives Remote ANL ability
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] portRemoteANAbility - remote port abilities
 *\param [in] portRemoteANCtrl - remote port AN control
 *
*/
XP_STATUS cpssHalMacPortANLtRemoteAbilityGet(xpDevice_t devId, uint32_t port,
                                             uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl);
/**
 *\brief: sets the cable Type
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] cable type
 * 0 : long cable
 * 1 : short cable
 * 2 : optic cable
*/
void cpssHalPortMacCableLenSet(xpsDevice_t devId, uint32_t portNum,
                               int cableType);
/**
 *\brief: return the cable type
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [out] cable type
 * 0 : long cable
 * 1 : short cable
 * 2 : optic cable
 *
*/
uint32_t cpssHalMacGetPortCableLen(xpsDevice_t devId, uint32_t portNum);

void cpssHalDeletPort(GT_U8 cpssDevId, uint32_t cpssPortNum, int retries);

GT_STATUS cpssHalLinkstatuschangeEvent(GT_U8 devNum, uint32_t portNum);
/**
 *\brief: Get fec mode
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *
*/
CPSS_PORT_FEC_MODE_ENT cpssHalPortFecParamGet(xpsDevice_t devId,
                                              uint32_t portNum);
void cpssHalPortFecParamSet(xpsDevice_t devId, uint32_t portNum,
                            xpFecMode fecMode, bool valid);

/**
 * \brief API to get port mirrioring
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [in] ingress
 * \param [out] analyzerId
 *
 ** \return XP_STATUS
 */
XP_STATUS cpssHalPortMirrorGet(xpsDevice_t devId, uint32_t portNum,
                               bool ingress, uint32_t *analyzerId);

#ifdef __cplusplus
}
#endif

#endif
