// xpsMac.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#ifndef _xpsMac_h_
#define _xpsMac_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsMac.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpsMac.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Mac Manager
 */


/**
 * \brief To reconfigure MAC mode for debug purpose
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127 and 255.
 * \param [in] macConfig enum specifying mac configuration mode
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_VALUE_OUT_OF_RANGE/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetMacConfigModeDebug(xpsDevice_t devId, uint32_t portNum,
                                      xpMacConfigMode
                                      macConfig); //For debug purpose only, which will write the mac mode in HW.;

/**
 * \brief delete the instance of the physical port.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number of range from 0 to 127 and 255.
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_ERR_NULL_POINTER/XP_ERR_REG_READ
*/
XP_STATUS xpsMacPortDeInit(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief initializing port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum number of the port to init
 * \param [in] macconfig configuration of the MAC mode
 * \param [in] fecMode - FEC mode selection: RS/FC FEC
 * \param [in] enableFEC - enable FEC module between PCS and serdes Intreface
 * \return XP_STATUS
*/

XP_STATUS xpsMacPortInit(xpDevice_t devId, uint32_t portNum,
                         xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC);

/**
 * \brief DeRegister event handler previously registered with
 * eventHandlerDeRegister
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] macNum
 * \return [XP_STATUS]  status On success XP_NO_ERR
 *          status On error XP_ERR_REG_WRITE/XP_ERR_NULL_POINTER/XP_ERR_REG_READ/XP_ERR_MAC_NOT_INITED
*/
XP_STATUS xpsMacEventHandler(xpsDevice_t devId, uint8_t macNum);
/**
 *\brief
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] enable - enable/disable loopback
 *\return [XP_STATUS] status On success XP_NO_ERR
 *                    status On err XP_INVALID_MAC_MODE/XP_ERR_MODE_NOT_SUPPORTED
*/

XP_STATUS xpsMacPortLoopbackEnable(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 *\brief: Get the loopback status for port
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [out] status - Loopback enable/disable status
 *\return [XP_STATUS] status On success XP_NO_ERR
*/

XP_STATUS xpsMacPortLoopbackEnableGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *status);


/**
 *\brief
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] enable - enable/disable Far end loopback
 *\return [XP_STATUS] status On success XP_NO_ERR
*/

XP_STATUS xpsMacPortFarEndLoopbackEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable);

/**
 * \brief To enable/disable RX crc check
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   Enable/disable rxCrcCheck
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacRxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable);

/**
 * \brief To enable/disable strip of RX frame FCS field before frame is transferred to the application logic.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   Enable/disable FCS strip
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacRxStripFcsEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable);

/**
 * \brief To enable/disable rx flow control decoder
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   Enable/disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacRxFlowControlDecodeEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable);

/**
 * \brief To Set length of START/PRE/SFD length of received frame
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] length   To set Preamble Length bytes.
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetRxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpRxPreambleLenBytes length);

/**
 * \brief  Configure receiver frame jabber size
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. allowed range 0 to 127 and 255
 * \param [in] jabberFrmSize size of jabber frame.This value has to be multiple of data path width(in bytes) in side MAC which is as follows.
 *      100G:Multiple of 16.
 *      40G :Multiple of 8.
 *      10G :Multiple 0f 4.
 *      1G  :Multiple 0f 1.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetRxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t jabberFrmSize);

/**
 * \brief  Set priority for monitoring the PFC transmission. Each bit in value determines whether priority is enabled or disabled
 *         This field is also trasmitted in the priority vector filed in the PFC pause frame.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. allowed range 0 to 127 and 255
 * \param [in] txPriPauseVector Transmit priority pause vector. Allowed range 0 to 0XFF
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetTxPriorityPauseVector(xpsDevice_t devId, uint32_t portNum,
                                         uint16_t txPriPauseVector);

#if 0
/**
 * \brief  Set VLAN tag 3, VLAN tag3 is used to identify the third VLAN tag in incoming frame on given channel.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] vlanTag Vlan tag
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetVlanTag3(xpsDevice_t devId, uint32_t portNum,
                            uint16_t vlanTag);

/**
 * \brief  Set VLAN tag 1, VLAN tag1 is used to identify the first VLAN tag in incoming frame on given channel.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] vlanTag Vlan tag
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetVlanTag1(xpsDevice_t devId, uint32_t portNum,
                            uint16_t vlanTag);

/**
 * \brief  Set VLAN tag 2, VLAN tag2 is used to identify the second VLAN tag in incoming frame on given channel.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] vlanTag Vlan tag
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetVlanTag2(xpsDevice_t devId, uint32_t portNum,
                            uint16_t vlanTag);
#endif

/**
 * \brief Get pause frame bit status for transmit the pause control frame
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] status status of bit
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPauseFrameGen(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *status);

/**
 * \brief Set pause frame bit status for transmit the pause control frame
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] status status of bit
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetPauseFrameGen(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t status);

/**
 * \brief  Configure lower,middle and higher destination MAC address field of port for flow control.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] macAddr MAC address
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetMacFlowCtrlFrmDestAddr(xpsDevice_t devId, uint32_t portNum,
                                          macAddr_t macAddr);

/**
 * \brief  Configure lower,middle and higher source MAC address field of port for flow control.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] macAddr MAC address
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetMacFlowCtrlFrmSrcAddr(xpsDevice_t devId, uint32_t portNum,
                                         macAddr_t macAddr);

/**
 * \brief To set XON pause time, XON pause time is used in the generated pause control frame
 * when the XOFF PAUSE frame is to be transmitted.
 * This filed is used both in the regular PAUSE flow control frame and priority PAUSE flow control frm.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] pauseTime pause time
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetXonPauseTime(xpsDevice_t devId, uint32_t portNum,
                                uint16_t pauseTime);

/**
 * \brief To set XOFF pause time, XOFF pause time is used in the generated pause control frame
 * when the XOFF PAUSE frame is to be transmitted.
 * This field is used both in the regular PAUSE flow control frame and priority PAUSE flow control frame.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] pauseTime pause time
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetXoffPauseTime(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t pauseTime);

/**
 * \brief  getting TX timestamp value.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [out] timeStampVal Value of multicast hash filter table
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxTimeStampValue(xpsDevice_t devId, uint32_t portNum,
                                    uint64_t *timeStampVal);
/**
 * \brief To check whether time stamp value register has valid value
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [out] valid status
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTimeStampValidStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t *valid);

/**
 * \brief To get time stamp ID associated with the time stamp
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [out] *timeStampId time stamp ID
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTimeStampId(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *timeStampId);

/**
 * \brief To set pause time value that is used in the generated pause control frame under software control
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] pauseTime pause time
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetFlowCtrlPauseTime(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t pauseTime);

/**
 * \brief Adjust Rx auto frame length
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 255.
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
//XP_STATUS xpsMacSetRxAutoFrmLenAdjst(xpsDevice_t devId, uint32_t portNum, xpVlanCheckMode config);

/**
 * \brief Adjust Tx inter frame gap length
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum      port number. Valid values are 0 to 127 and 255.
 * \param [in] ifgLength    inter frame gap length
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetTxIfgLen(xpsDevice_t devId, uint32_t portNum,
                            uint8_t ifgLength);

/**
 * \brief  Configure transmit frame jabber size
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 *
 * \param [in] jabberFrmSize size of jabber frame.This value has to be multiple of data path width(in bytes) in side MAC which is as follows.
 *      100G:Multiple of 16.
 *      40G :Multiple of 8.
 *      10G :Multiple 0f 4.
 *      1G  :Multiple 0f 1.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetTxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t jabberFrmSize);

/**
 * \brief Enable/disable pacing of frames using the ff_txnextfrmifg value.
 * When pacing is enable, the value from the ff_txnextfrmifg is used as the IFG between frames.
 * When pacing is not enable, the value programmed in IFG length is used for all frames.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   true to enable, false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxIfgCtrlPerFrameEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable);

/**
 * \brief Enable/disable Tx crc check
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   true to enable, false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t enable);

/**
 * \brief Invert FCS field that is being inserted into outgoing frame
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   true to enable invert FCS field, false to disable invert FCS disable field
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxFcsInvertEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable);

/**
 * \brief Enable/disable flow control frame generation.
 *
 * When flow control frame generation is enabled for transmission of pause
 * control frames.The PAUSE control frame are transmitted either on software
 * control or based on application logic's controls.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   true to enable transmission of pause control frame, false to disable it
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable);

/**
 * \brief Enable/disable transmission of priority flow control frames.
 * The PFC control frames are transmitted based on application logic's control.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable   true to enable priority flow control generation, false to disable it
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxPriorityFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                            uint8_t enable);

/**
 * \brief Set the Tx preamble length
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] bytes    size of the Tx preamble
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetTxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpPreambleLenBytes bytes);

/**
 * \brief Put reference channel of given port in soft reset.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 255.
 * \param [in] enable   enable = true to put channel in soft reset, enable = false to get out of soft reset.
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPortSoftReset(xpsDevice_t devId, uint32_t portNum,
                              uint8_t enable);

/**
 * \brief Enable/Disable given port RX/TX channel
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 255.
 * \param [in] enable   enum specifies rx/tx or both path enable/disable
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacRxtxPortEnable(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable);

/**
 * \brief xpsMacTxPortEnable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 255.
 * \param [in] enable   enum specifies rx/tx or both path enable/disable
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacTxPortEnable(xpsDevice_t devId, uint32_t portNum,
                             uint8_t enable);

/**
 * \brief xpsMacRxPortEnable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 255.
 * \param [in] enable   enum specifies rx/tx or both path enable/disable
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacRxPortEnable(xpsDevice_t devId, uint32_t portNum,
                             uint8_t enable);

/**
 * \brief reset statistic of reference channel of given port.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 255
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacStatCounterReset(xpsDevice_t devId, uint32_t portNum);


XP_STATUS xpsMacStatCounterCreateAclRuleForV4V6(xpsDevice_t devId,
                                                uint32_t portNum,
                                                uint32_t *counterId_Ingress_v4, uint32_t *counterId_Ingress_v6,
                                                uint32_t *counterId_Egress_v4,  uint32_t *counterId_Egress_v6);


XP_STATUS xpsMacStatCounterRemoveAclRuleForV4V6(xpsDevice_t devId,
                                                uint32_t portNum,
                                                uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                                uint32_t counterId_Egress_v4,  uint32_t counterId_Egress_v6);


XP_STATUS xpsMacGetCounterV4V6Stats(xpsDevice_t devId, uint32_t portNum,
                                    uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                    uint32_t counterId_Egress_v4, uint32_t counterId_Egress_v6,
                                    uint64_t *v4TxPkts,  uint64_t *v4TxBytes,  uint64_t *v4RxPkts,
                                    uint64_t *v4RxBytes,
                                    uint64_t *v6TxPkts,  uint64_t *v6TxBytes,  uint64_t *v6RxPkts,
                                    uint64_t *v6RxBytes);


/**
 * \brief Enable/Disable a given range of ports
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] fromPortNum  starting value of port range
 * \param [in] toPortNum        ending value of port range
 * \param [in] enable       enable = true to enable the port, enable = false to diable the port
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_ERR_VALUE_OUT_OF_RANGE/XP_ERR_INVALID_ARG/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPortEnableRange(xpsDevice_t devId, uint8_t fromPortNum,
                                uint8_t toPortNum, xpPortEnable enable);

/**
 * \brief Reset devices depending on the value of enum expPortLevel
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] enable   1: reset on the desired level
 *          0: do not reset on desired level
 * \param [in] portNum      Port number. Valid values are 0-127
 * \param [in] reset    an enum value which specifies whether we have to reset MAC, SERDES or ALL
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPortReset(xpsDevice_t devId, uint8_t enable, uint32_t portNum,
                          xpResetPortLevel reset);

/**
 * \brief enable/disable loopback mode
 *
 * Loopback can be enabled on the TxToRx SGMII SERDES, RxToTx GMII,
 * PCS, MAC PCS, MAC FIFO interface
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0-127
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [in] enable - 1: loopback enable, 0: loopback disable
 * \return [XP_STATUS]  On success XP_NO_ERR.
 *          On failure XP_ERR_INVALID_VALUE/XP_ERR_INVALID_ARG/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacMacLoopback(xpsDevice_t devId, uint32_t portNum,
                            xpMacLoopbackLevel loopback, uint8_t enable);

/**
 * \brief get loopback mode status enable/disable
 *
 * Loopback can be on the TxToRx SGMII SERDES, RxToTx GMII,
 * PCS, MAC PCS, MAC FIFO interface
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0-127
 * \param [in] loopback - An enum value which specifies loopback level
 * \param [out] status
 * \return [XP_STATUS]  On success XP_NO_ERR.
 *          On failure XP_ERR_INVALID_VALUE/XP_ERR_INVALID_ARG/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacMacLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                               xpMacLoopbackLevel loopback, uint8_t *status);

/**
 * \brief get loopback status enable/disable  on RxToTx GMII
 *
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0-127
 * \param [out] enable
 * \return [XP_STATUS]  On success XP_NO_ERR.
 *          On failure XP_ERR_INVALID_VALUE/XP_ERR_INVALID_ARG/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGmiiLoopbackEnableGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable);

/**
 * \brief Set this Port as Ext PHY connected to it
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127.
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_PORT_NOT_INITED/XP_ERR_INVALID_VALUE
 */
XP_STATUS xpsMacConfigMdioMaster(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief SDeconfigure EXT PHY from this port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127.
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_PORT_NOT_INITED/XP_ERR_INVALID_VALUE
 */
XP_STATUS xpsMacDeconfigMdioMaster(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief Read value from an external PHY register
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127.
 * \param [in] phyAddr Address of PHY
 * \param [in] phyRegAddr Address of PHY register
 * \param [out] value value read from the PHY register
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_ERR_VALUE_OUT_OF_RANGE/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacExtPHYRead(xpsDevice_t devId, uint32_t portNum, uint8_t phyAddr,
                           uint8_t phyRegAddr, uint16_t *value);

/**
 * \brief Write value to an external PHY register
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127
 * \param [in] phyAddr Address of PHY
 * \param [in] phyRegAddr Address of PHY register
 * \param [in] value value to be written
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_PORT_NOT_INITED/XP_ERR_VALUE_OUT_OF_RANGE/XP_ERR_INVALID_VALUE
*/
XP_STATUS xpsMacExtPHYWrite(xpsDevice_t devId, uint32_t portNum,
                            uint8_t phyAddr,
                            uint8_t phyRegAddr, uint16_t value);

/**
 * \brief Used to retrieve default speed for port.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum      Port number. Valid values are 0-127
 * \param [out] speed       Default port speed
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPortDefaultSpeed(xpsDevice_t devId, uint32_t portNum,
                                    xpSpeed *speed);

/**
 * \brief Used to enable/disable Auto Negotiation for the Port.
 * AN only allowed if relevant MAC of port is configured in SGMII,QSGMII and MIX mode.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum      Port number. Valid values are 0-127
 * \param [in] enable   enable/Disable the auto negotiation
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetPortAutoNeg(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable);

/**
 * \brief Used to restart Auto Negotiation for the Port.
 * Restart of AN only allowed if relevant MAC of port is configured in SGMII,QSGMII and MIX mode.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum      Port number. Valid values are 0-127
 * \param [in] restart      restart the auto negotiation
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPortAutoNegRestart(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t restart);

/**
 * \brief Get the Auto-negotiation Done for a port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0-127
 * \param [out] status - get the status of auto-negotiation
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPortAutoNegDone(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *status);

/**
 * \brief Get the Auto-negotiation Sync Status for a port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0-127
 * \param [out] status - get the status of auto-negotiation
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPortAutoNegSyncStatus(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *status);

/**
 * \brief Initialise the PHY
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127.
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_PORT_NOT_INITED/XP_ERR_INVALID_VALUE
 */
XP_STATUS xpsMacIsMdioMasterConf(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief To set MDIO clock division ratio between the apd_clk and the generated MDC.Only even values are supported with a maximum value of 8'hFE
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] divCtrlRatio division ration between the apd_clk and the generated MDC.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetMdioClkDivisonCtrl(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t divCtrlRatio);

/**
 * \brief Register event handler for Ports
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0 to 127
 * \param [in] eventType - Type of event
 * \return [XP_STATUS]  On success XP_NO_ERR,
 *          On error XP_ERR_REG_WRITE/XP_ERR_NULL_POINTER/XP_ERR_REG_READ/XP_ERR_MAC_NOT_INITED
*/
//XP_STATUS xpsMacEventHandlerRegister(xpsDevice_t devId, uint32_t portNum, xpEventType eventType, xpEventHandler eventHandler);

/**
 * \brief Enable/disable interrupts
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum - Port number. Valid values are 0 to 127
 * \param [in] eventType - Type of event
 * \param [in] enable - State indicating enabling or disabling the interrupt
 * \return [XP_STATUS]  On success XP_NO_ERR,
 *          On error XP_ERR_REG_WRITE/XP_ERR_NULL_POINTER/XP_ERR_REG_READ/XP_ERR_MAC_NOT_INITED
*/
XP_STATUS xpsMacInterruptEnable(xpsDevice_t devId, uint32_t portNum,
                                xpEventType eventType, bool enable);

/**
 * \brief Get various statistics about Port in xpStatistics structure directly
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127.
 * \param [in] fromStatNum Start collecting statistics from this number. Valid values are 0-31
 * \param [in] toStatNum Collect statistics till this number. Valid values are 0-31
 * \param [out] stat A structure which holds various statistics of a port. Statistics of counters which
 * falls in range of fromStatNum to toStatNum will only be updated.
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_ERR_INVALID_ARG/XP_ERR_VALUE_OUT_OF_RANGE/XP_ERR_INVALID_ARG
*/
XP_STATUS xpsMacGetCounterStatsDirect(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat);

/**
 * \brief Get various statistics about Port in xpStatistics structure and claer out the stats in mac
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number
 * \param [in] fromStatNum Start collecting statistics from this number. Valid values are 0-31
 * \param [in] toStatNum Collect statistics till this number. Valid values are 0-31
 * \param [out] stat A structure which holds various statistics of a port. Statistics of counters which
 * falls in range of fromStatNum to toStatNum will only be updated.
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_ERR_INVALID_ARG/XP_ERR_VALUE_OUT_OF_RANGE/XP_ERR_INVALID_ARG
*/
XP_STATUS xpsMacGetCounterStatsClearOnRead(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat);

/**
 * \brief xpsMacCounterStatsRxPkts
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] rxTotalPkts
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacCounterStatsRxPkts(xpDevice_t devId, uint32_t portNum,
                                   uint64_t *rxTotalPkts);
/**
 * \brief xpsMacCounterStatsTxPkts
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] txTotalPkts
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacCounterStatsTxPkts(xpDevice_t devId, uint32_t portNum,
                                   uint64_t *txTotalPkts);

/**
 * \brief xpsMacCounterStatsRxOctets
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] rxTotalOctets
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacCounterStatsRxOctets(xpDevice_t devId, uint32_t portNum,
                                     uint64_t *rxTotalOctets);
/**
 * \brief xpsMacCounterStatsTxOctets
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] txTotalOctets
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacCounterStatsTxOctets(xpDevice_t devId, uint32_t portNum,
                                     uint64_t *txTotalOctets);

/**
 * \brief To get PCS fault status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] status status = true when fault condition detected
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPcsFaultStatus(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *status);

/**
 * \brief To set double clock frequency of serdes or core clock
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacPcsDoubleSpeedEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t enable);

/**
 * \brief To enable 10g encoding/decoding format
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacEncoding10gEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable);

/**
 * \brief To stop PCS interrupts from triggering
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacPcsInterruptMaskEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To get whether pcs interrupt mask enable or disable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 176
 * \param [out] *enable - true for enable, false for disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
 */
XP_STATUS xpsMacGetPcsInterruptMaskEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);
/**
 * \brief To trigger PCS associated interrupts
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacPcsInterruptTestEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To get whether pcs interrupt test enable or disable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 176
 * \param [out] *enable - true for enable, false for disable
 * \return [XP_STATUS] status On success XP_NO_ERR
 */
XP_STATUS xpsMacGetPcsInterruptTestEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);
/**
 * \brief Set value in seed-A register for generate pseudo random test pattern.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127.
 * \param [in] seed seed value.
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsMacSetPcsTestPatternSeedA(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t seed);

/**
 * \brief Set value in seed-B register for generate pseudo random test pattern.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127
 * \param [in] seed seed value.
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetPcsTestPatternSeedB(xpsDevice_t devId, uint32_t portNum,
                                       uint64_t seed);

/**
 * \brief Select the PCS data pattern
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127.
 * \param [in] dataPattern enum specifying the data patten
 * \return [XP_STATUS] On success XP_NO_ERR. On error XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsDataPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                     xpPCSDataPattern dataPattern);

/**
 * \brief Select the PCS test pattern
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127.
 * \param [in] testPattern enum specifying the test pattern
 * \return [XP_STATUS] On success XP_NO_ERR. On error XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsTestPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                     xpPCSTestPattern testPattern);

/**
 * \brief Enable/disable RX/TX test mode
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port Number. Valid values are 0 to 127.
 * \param [in] testMode enum specifying the test mode and enable/disable status*
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetPcsRxTxTestModeEnable(xpsDevice_t devId, uint32_t portNum,
                                         xpPCSEnableTestMode testMode);

/**
 * \brief  Enable the PCS scrambled test pattern
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127.
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacSetPcsScrambledIdealTestPattern(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable);

/**
 * \brief Get PCS lock status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] pcsLock *pcslock=true if pcs lock enabled. *pclock=false if pcs is disabled
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsLockStatus(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *pcsLock);

/**
 * \brief Get higher bit rate error
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] *pcsHiBer true of false depending on the status
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsHiBerStatus(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *pcsHiBer);

/**
 * \brief Get the PCS Alignment status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] blockLock true or false depending on the PCS Alignment
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsBlockLockStatus(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t *blockLock);

/**
 * \brief Get the PCS Alignment lock status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] alignmentLock
 * \param [out] status
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsAlignmentLock(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *status, uint32_t *alignmentLock);

/**
 * \brief Get the PCS Debug Deskew OverFlow
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] deskewOverFlowErr
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsDebugDeskewOverFlow(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *deskewOverFlowErr);

/**
 * \brief Get the PCS Debug Tx Gearbox Fifo Err
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] fifoErr
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsDebugTxGearboxFifoErr(xpsDevice_t devId, uint32_t portNum,
                                            uint8_t* fifoErr);

/**
 * \brief Get the Rx Preamble Length
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] length
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetRxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpRxPreambleLenBytes *length);

/**
 * \brief Get the PCS Alignment status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] maxEntries
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsFullThreshold(xpDevice_t devId, uint32_t portNum,
                                    uint8_t *maxEntries);

/**
 * \brief Get the PCS Tx Test Mode Enable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] testMode
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsTxTestModeEnable(xpDevice_t devId, uint32_t portNum,
                                       xpPCSEnableTestMode *testMode);

/**
 * \brief Get the PCS Rx Test Mode Enable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] testMode
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsRxTestModeEnable(xpDevice_t devId, uint32_t portNum,
                                       xpPCSEnableTestMode *testMode);
/**
 * \brief Get the PCS Test Pattern SeedA
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] seed
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsTestPatternSeedA(xpDevice_t devId, uint32_t portNum,
                                       uint64_t *seed);

/**
 * \brief Get the PCS Test Pattern SeedB
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] seed
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetPcsTestPatternSeedB(xpDevice_t devId, uint32_t portNum,
                                       uint64_t *seed);

/**
 * \brief Get the Encoding 10g Enable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [out] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 *             On failure XP_ERR_NULL_POINTER/XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacGetEncoding10gEnable(xpDevice_t devId, uint32_t portNum,
                                     uint8_t *enable);
/**
 * \brief Get the PCS lane mapping status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127
 * \param [in] laneNum lane number. Valid values are 0-19
 * \param [out] laneMapping
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPcsLaneMappingStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t laneNum, uint8_t *laneMapping);

/**
 * \brief Enable/disable PCS
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127
 * \param [in] enable on=true to enable, on=false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable);

/**
 * \brief Enable/disable bypass scrambler
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127
 * \param [in] enable on=true to enable, on=false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsBypassScramblerEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable);

/**
 * \brief Enable/disable use of PCS short timer
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127
 * \param [in] enable on=true to enable, on=false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On failure XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsUseShortTimerEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief Enable/disable PCS Ignore signal OK
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable on=true to enable, on=false to disable
 *
 * \return [XP_STATUS] On success XP_NO_ERR. On error XP_PORT_NOT_INITED
*/
XP_STATUS xpsMacPcsIgnoreSignalOkEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable);

/**
 * \brief Set for bypass TX buffer
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacPcsBypassTxBufEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t enable);

/**
 * \brief To Set maximum number of entries that may be stored in the TX gearbox a sync fifo at one time.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] maxEntries maximum number of entries. Allowed range 0-8
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetPcsFullThreshold(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t maxEntries);

/**
 * \brief To configure that assume data to be error free in order to improve latency (violates IEEE).
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacPcsLowLatencyModeEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable);

/**
 * \brief To get whether pcs low latency mode enable or disable
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 176
 * \param [out] *enable - true for enable, false for disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPcsLowLatencyModeEnable(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable);
/**
 * \brief To enable FEC.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacFecEnable(xpsDevice_t devId, uint32_t portNum, uint8_t enable);

/**
 * \brief To get status of FC FEC ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] status status = Indicates FEC ability for each of the 10GBASE-R PHY type if FEC_ability = 1
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFcFecAbility(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *status);

/**
 * \brief To get status of FC FEC error indication ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] status Indicates FEC ability for each of the 10GBASE-R PHY type
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFcFecErrIndicationAbility(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *status);

/**
 * \brief To enable/disable FC FEC.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacFcFecEnable(xpsDevice_t devId, uint32_t portNum,
                            uint8_t enable);

/**
 * \brief To enable/disable FC FEC enable error to PCS.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacFcFecErrToPcsEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable);

/**
 * \brief To get counter of each corrected FC FEC blocks processed when FEC_SIGNAL indication is OK.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] value  counter of each corrected FEC blocks processed
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetFcFecCorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *value);

/**
 * \brief To get counter of each uncorrected FC FEC blocks processed when FEC_SIGNAL indication is OK.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] value counter of each uncorrected FEC blocks processed
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetFcFecUncorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value);

/**
 * \brief To enable/disable soft reset in FC FEC.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacFcFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable);

/**
 * \brief To get whether enable/disable soft reset in FC FEC
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127
 * \param [out] *enable - enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFcFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable);

/**
 * \brief To enable RS FEC bypass.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacRsFecBypassEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable);

/**
 * \brief To get status of RS FEC enable.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] error   true if FEC decode indicates errors to PCS layer,
 *                      false if FEC decode does not indicates errors.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecEnaErrorIndication(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *error);

/**
 * \brief To get RS FEC ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] status  true if sub-layer has RS_FEC ability
 *                      false if sub-layer does not have RS_FEC ability
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecAbility(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *status);

/**
 * \brief To get RS FEC bypass correction ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] status  true if FEC decoder has ability to bypass error correction
 *                      false if FEC decoder does not have correction bypass ability.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecBypassCorrAbility(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *status);

/**
 * \brief To get RS FEC error indication ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [out] error   true if FEC decoder has ability to indicate error.
 *                      false if FEC decoder does not have ability to indicate error.
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecErrIndicationAbi(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *error);

/**
 * \brief To get counter of each corrected RS FEC blocks processed.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] value  counter of each corrected FEC blocks processed
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetRsFecCorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                          uint32_t *value);

/**
 * \brief To get counter of each uncorrected RS FEC blocks processed.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] value  counter of each uncorrected FEC blocks processed
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetRsFecUncorrectedBlockCnt(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value);

/**
 * \brief To get symbol error counter of FEC lane.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [in] laneNum  lane Number
 * \param [out] value  counter of symbol error
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetRsFecSymbolErrCnt(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint32_t *value);

/**
 * \brief To enable/disable soft reset in 100GB RS-FEC.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] enable enable=true to enable, enable=false to disable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacRsFecSoftReset(xpsDevice_t devId, uint32_t portNum,
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
XP_STATUS xpsMacFecModeSet(xpDevice_t devId, uint32_t portNum,
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
XP_STATUS xpsMacFecModeGet(xpDevice_t devId, uint32_t portNum,
                           xpFecMode *fecMode);

/**
 * \brief To get Pcs Use Short Timer enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsUseShortTimerEnable(xpDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);

/**
 * \brief To get Pcs enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsEnable(xpDevice_t devId, uint32_t portNum,
                             uint8_t *enable);

/**
 * \brief To get Pcs Double Speed enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsDoubleSpeedEnable(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *enable);

/**
 * \brief To get Pcs Bypass Tx Buf enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsBypassTxBufEnable(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *enable);

/**
 * \brief To get Pcs Bypass Scrambler enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsBypassScramblerEnable(xpDevice_t devId, uint32_t portNum,
                                            uint8_t *enable);

/**
 * \brief To get Pcs Ignore Signal Ok enable status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] enable
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsIgnoreSignalOkEnable(xpDevice_t devId, uint32_t portNum,
                                           uint8_t *enable);

/**
 * \brief To get Pcs Select Data Pattern
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Valid values are 0-127 and 255
 * \param [out] dataPattern
 *
 * \return [XP_STATUS]  status On success XP_NO_ERR.
 *          status On error XP_ERR_NULL_POINTER/XP_ERR_REG_READ/
 *                  XP_ERR_OP_NOT_SUPPORTED
*/
XP_STATUS xpsMacGetPcsDataPatternSelect(xpDevice_t devId, uint32_t portNum,
                                        xpPCSDataPattern *dataPattern);

/**
 * \brief To set BPAN ability.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] backPlaneAbilityMode Back Plane ability mode
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetBackPlaneAbility(xpsDevice_t devId, uint32_t portNum,
                                    xpBackPlaneAbilityModes backPlaneAbilityMode);

/**
 * \brief To get the configured back plane ability mode
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0 to 127 and 176
 * \param [out] *backPlaneAbilityMode - mode of back plane ability
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
 */
XP_STATUS xpsMacGetBackPlaneAbility(xpsDevice_t devId, uint32_t portNum,
                                    xpBackPlaneAbilityModes *backPlaneAbilityMode);














/**
 * \brief To reset all channels
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetResetAllChannel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 * \brief To get status of all channel reset bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetResetAllChannel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable);

/**
 * \brief To set full thresold in 4 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxFifoThreshold4ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold);

/**
 * \brief To get value of full thresold in 4 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFifoThreshold4ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold);

/**
 * \brief To set full thresold in 2 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxFifoThreshold2ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold);

/**
 * \brief To get value of full thresold in 2 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFifoThreshold2ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold);

/**
 * \brief To set full thresold in 1 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxFifoThreshold1ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t thresold);

/**
 * \brief To get value of full thresold in 1 channel mode of TX FIFO
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] thresold
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFifoThreshold1ch(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *thresold);

/**
 * \brief To set transmit fifo interface type(must be set 1(FIFO Almost Full Protocol))
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 * \brief To get transmit fifo interface type
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable);

/**
 * \brief To set receive fifo interface type(must be set 1(FIFO Almost Full Protocol))
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetRxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 * \brief To get receive fifo interface type
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxInterfaceType(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable);

/**
 * \brief To get IEEE 802.3 Clause 73.6.1 Received Selector field, S[4:0]
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] rxSelector
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetAnRxSelector(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *rxSelector);

/**
 * \brief To get IEEE 802.3 Clause 73.6.3 Received Nonce field, N[4:0]
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] rxNonce
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetAnRxNonce(xpsDevice_t devId, uint32_t portNum,
                             uint8_t *rxNonce);

/**
 * \brief To read status of transmitted a frame since the register last read
 1:  Frame transmitted
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxActive(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable);

/**
 * \brief When set, indicates transmitted an Error Frame (EOF_ERROR,UNDERRUN, JABBER) since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxError(xpsDevice_t devId, uint32_t portNum,
                           uint8_t *enable);

/**
 * \brief When set, indicates underflow happened since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxUnderrun(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable);

/**
 * \brief When set, indicates trasnsmitted a Pause Frame since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxPause(xpsDevice_t devId, uint32_t portNum,
                           uint8_t *enable);

/**
 * \brief When set, indicates Received a frame since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxActive(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable);

/**
 * \brief When set, indicates Received a frame with CRC Error since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxCrcError(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable);

/**
 * \brief When set, indicates Received a frame with error since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxError(xpsDevice_t devId, uint32_t portNum,
                           uint8_t *enable);

/**
 * \brief When set, indicates overflow happend since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxOverflow(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *enable);

/**
 * \brief When set, indicates Received a pause frame since the register last read
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxPause(xpsDevice_t devId, uint32_t portNum,
                           uint8_t *enable);

/**
 * \brief To set register bit of interrupt  on fifo overflow condition on corresponding channel
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxFifoOverFlowErrorInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable);

/**
 * \brief To read status of interrupt on fifo overflow condition on corresponding channel
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFifoOverFlowErrorInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);

/**
 * \brief To enable/disable Tx FIFO Overflow Error Interrupt on corresponding channel
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxFifoOverFlowErrorIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable);

/**
 * \brief To get status(enable/disable) of Tx FIFO Overflow Error Interrupt on corresponding channel
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFifoOverFlowErrorIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable);

/**
 * \brief when set, An errored block was received
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecErrorBlockInt(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable);

/**
 * \brief To get status of Uncorrectable Block Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecUncorrectedBlockInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);

/**
 * \brief To get status of Block Lock Achieved Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockInt(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable);

/**
 * \brief To get status of Block Lock Lost Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockLostInt(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);

/**
 * \brief To get status of Block Lock Changed Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockChangedInt(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);

/**
 * \brief To enable error block Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecErrorBlockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t enable);

/**
 * \brief To get status of  error block Interrupt enable bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecErrorBlockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                          uint8_t *enable);

/**
 * \brief To enable uncorrected block Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecUncorrectedBlockIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable);

/**
 * \brief To get status of   uncorrected block Interrupt enable bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecUncorrectedBlockIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable);

/**
 * \brief To enable block lock Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t enable);

/**
 * \brief To get status of block lock interrupt enable bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockIntEnable(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *enable);

/**
 * \brief To enable block lock lost Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockLostIntEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t enable);

/**
 * \brief To get status of block lock lost Interrupt enable bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockLostIntEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable);

/**
 * \brief To enable block lock changed interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockChangedIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable);

/**
 * \brief To get status of block lock changed interrupt enable bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFecBlockLockChangedIntEnable(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable);

/**
 * \brief To test error block Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecErrorBlockIntTest(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable);

/**
 * \brief To test uncorrected block Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecUncorrectedBlockIntTest(xpsDevice_t devId,
                                              uint32_t portNum, uint8_t enable);

/**
 * \brief To test block lock Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockIntTest(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To test block lock lost Interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockLostIntTest(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable);

/**
 * \brief To test block lock changed interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetFecBlockLockChangedIntTest(xpsDevice_t devId,
                                              uint32_t portNum, uint8_t enable);

/**
 * \brief To enable AN Done interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetSgmiiIntEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable);

/**
 * \brief To get status(enable/disable) of AN Done interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetSgmiiIntEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable);

/**
 * \brief To test AN done interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetSgmiiIntTest(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable);

/**
 * \brief To get status of test bit of AN done interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetSgmiiIntTest(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable);
/**
 * \brief To get status of test bit of AN done interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSgmiiSerdesLoopbackEnableGet(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable);
/**
 * \brief To enable/disable read on counte clean
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable);

/**
 * \brief To get status of read on clean enable bit of MIB counter
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable);

/**
 * \brief When set, the read access to the MIB Counters Memory has Priority over the normal MIB Counters Memory Background Operation(Counters updates due to packets reception and Transmission).
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetReadCounterPriority(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To get status of priority read bit of MIB counter
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetReadCounterPriority(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);

/**
 * \brief To enable this, causes transmit path to enter into Drain mode where all the data from the TXFIFO is drained out.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetTxDrainMode(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable);

/**
 * \brief To get status of TX drain
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxDrainMode(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *enable);

/**
 * \brief To enables the detection of Early EOF indication from PCS which helps in reducing the latency.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetRxEarlyEofDetection(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To get staus of enable early EOF Detection
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxEarlyEofDetection(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);
/**
 * \brief Set this bit while use short alignment marker
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetRsFecDebugShortAMP(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable);

/**
 * \brief To get status of short aligment marker
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecDebugShortAMP(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable);

/**
 * \brief When set, FEC Interrupt is asserted
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetRsFecDebugTestInit(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable);

/**
 * \brief To get status of FEC interrupt test bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecDebugTestInit(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable);

/**
 * \brief To enable/disable stop Alignment State Machine when CW Test has failed
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetRsFecCWTestStopASM(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t enable);

/**
 * \brief NA
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecCWTestStopASM(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *enable);

/**
 * \brief Selector for SERDES Tx.SERDES must be configured to 40b mode.
 0: SERDES Tx data is from PCS (No Auto Negotiation)
 1: SERDES Tx Data is from BPAN Block.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetBpanSerdesTxSel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable);

/**
 * \brief To get bit status of SERDES Tx selector.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetBpanSerdesTxSel(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *enable);

/**
 * \brief To set auto negotiation mode
 0: CPU controlled
 1: Automatic
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetBpanMode(xpsDevice_t devId, uint32_t portNum,
                            uint8_t enable);

/**
 * \brief To get auto negotiaion mode.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetBpanMode(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *enable);

/**
 * \brief To Enables AN Rx Gearbox Re Sync
 0: Disable
 1: Enable
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
*/
XP_STATUS xpsMacSetBpanRxGearboxReSync(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);

/**
 * \brief To get bit status of AN Rx Gearbox Re Sync
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetBpanRxGearboxReSync(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);



/**
 * \brief To get status of logical OR of all interrupt
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecInterruptStatus(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t *enable);

/**
 * \brief To get Alignment marker lock status per each lane
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] amcLock
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecAmcLock(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *amcLock);

/**
 * \brief To get lane 0 internal mapped number
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] lanMap
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecLane0InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap);

/**
 * \brief To get lane 1 internal mapped number
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] lanMap
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecLane1InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap);

/**
 * \brief To get lane 2 internal mapped number
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] lanMap
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecLane2InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap);

/**
 * \brief To get lane 3 internal mapped number
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] lanMap
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecLane3InternalMap(xpsDevice_t devId, uint32_t portNum,
                                         uint8_t *lanMap);

/**
 * \brief To get link status interrupt enable
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] status
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacLinkStatusInterruptEnableGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t *status);

/**
 * \brief To set link status interrupt enable/disble
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] status
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacLinkStatusInterruptEnableSet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t status);


/**
 * @brief print all port status
 *
 * @param [in] devId device id. Valid values are 0-63
 *
 * @return [XP_STATUS] status On success XP_NO_ERR
*/
void xpsMacPrintAllPortStatus(xpsDevice_t devId);

/**
 * \brief To get device specific port linkStatus,faultStatus and serdesStatus
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] linkStatus
 * \param [out] serdesStatus
 * \param [out] faultStatus
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                              uint8_t *linkStatus, uint8_t *serdesStatus, uint8_t *faultStatus);

/**
 * \brief To set device specific port linkStatus
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] status
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                              uint8_t status);

/**
 * \brief To get Rx Crc Check Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable);

/**
 * \brief To get Rx Strip Fcs Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxStripFcsEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable);

/**
 * \brief To get Rx Flow Control Decode Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxFlowControlDecodeEnable(xpsDevice_t devId,
                                             uint32_t portNum,
                                             uint8_t *enable);

/**
 * \brief To get Tx Priority PauseVector
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] txPriPauseVector
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxPriorityPauseVector(xpsDevice_t devId, uint32_t portNum,
                                         uint16_t *txPriPauseVector);

/**
 * \brief To get Tx Ifg Len
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] ifgLength
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxIfgLen(xpsDevice_t devId, uint32_t portNum,
                            uint8_t *ifgLength);

/**
 * \brief To get Tx Jabber Frame Size
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] jabberFrmSize
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t *jabberFrmSize);

/**
 * \brief To get Rx Jabber Frame Size
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] jabberFrmSize
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxJabberFrmSize(xpsDevice_t devId, uint32_t portNum,
                                   uint16_t *jabberFrmSize);

/**
 * \brief To get Tx Ifg Ctrl Per Frame Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxIfgCtrlPerFrameEnable(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable);

/**
 * \brief To get Flow Ctrl PauseTime
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] pauseTime
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFlowCtrlPauseTime(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t *pauseTime);

/**
 * \brief To get Xoff PauseTime
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] pauseTime
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetXoffPauseTime(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t *pauseTime);

/**
 * \brief To get Xon PauseTime
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] pauseTime
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetXonPauseTime(xpsDevice_t devId, uint32_t portNum,
                                uint16_t *pauseTime);

/**
 * \brief To set TimeStampId
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] timeStampId
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetTimeStampId(xpsDevice_t devId, uint32_t portNum,
                               uint8_t timeStampId);

/**
 * \brief To get Tx Fcs Invert Enable
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] *enable - 0 - not enable, 1 - enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFcsInvertEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *enable);


/**
 * \brief To get Mac FlowCtrl Frame Src Addr
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] macAddr
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetMacFlowCtrlFrmSrcAddr(xpsDevice_t devId, uint32_t portNum,
                                         macAddr_t macAddr);

/**
 * \brief To get Mac FlowCtrl Frame Dest Addr
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] macAddr
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetMacFlowCtrlFrmDestAddr(xpsDevice_t devId, uint32_t portNum,
                                          macAddr_t macAddr);

/**
 * \brief To get Tx Flow Control Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxFlowControlEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);

/**
 * \brief To get Tx Priority Flow Control Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxPriorityFlowControlEnable(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t *enable);

/**
 * \brief To get Tx Crc Check Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxCrcCheckEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t *enable);

/**
 * \brief To get Tx Preamble Length
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] bytes
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxPreambleLen(xpsDevice_t devId, uint32_t portNum,
                                 xpPreambleLenBytes *bytes);

/**
 * \brief To get Rx Tx PortEnable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxtxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable);

/**
 * \brief To get Tx Port Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetTxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable);

/**
 * \brief To get Rx Port Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRxPortEnable(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable);

/**
 * \brief To get Rx Port Soft Reset status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPortSoftReset(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t *enable);

//XP_STATUS xpsMacGetStatCounterReset(xpsDevice_t devId, uint32_t portNum);

//XP_STATUS xpsMacGetPortEnableRange(xpsDevice_t devId, uint8_t fromPortNum, uint8_t toPortNum, xpPortEnable *enable);

/**
 * \brief To get Port Reset
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] reset
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPortReset(xpsDevice_t devId, uint8_t *enable,
                             uint32_t portNum, xpResetPortLevel reset);

/**
 * \brief To get Mdio Clk Divison Ctrl
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] divCtrlRatio
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetMdioClkDivisonCtrl(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *divCtrlRatio);

/**
 * \brief To get Pcs Test Pattern Select
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] testPattern
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPcsTestPatternSelect(xpsDevice_t devId, uint32_t portNum,
                                        xpPCSTestPattern *testPattern);

/**
 * \brief To get Pcs Scrambled Ideal Test Pattern
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetPcsScrambledIdealTestPattern(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable);

//XP_STATUS xpsMacGetPcsRxTxTestModeEnable(xpsDevice_t devId, uint32_t portNum, xpPCSEnableTestMode testMode);

/**
 * \brief To get FC FEC Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFcFecEnable(xpsDevice_t devId, uint32_t portNum,
                               uint8_t *enable);

/**
 * \brief To get FC FEC Err To Pcs Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetFcFecErrToPcsEnable(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);

/**
 * \brief To get RS FEC Bypass Enable status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecBypassEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *enable);

/**
 * \brief To set RS FEC Ena Error Indication
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [in] error
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacSetRsFecEnaErrorIndication(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t error);

/**
 * \brief To get Rs Fec Soft Reset status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum port number  Valid values are 0 to 127 and 255.
 * \param [out] enable
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacGetRsFecSoftReset(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *enable);






















//XP_STATUS xpsMacGetBpanRemoteFecRequestEnable(xpsDevice_t devId, uint32_t portNum, bool *enable);



/**
 * \brief To write xp Mac
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] regAddr
 * \param [in] regValue
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacXpMacWrite(xpsDevice_t devId, uint32_t portNum,
                           uint32_t regAddr,
                           uint16_t regValue);

/**
 * \brief To read xp Mac
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid values are 0-127 and 255
 * \param [in] regAddr
 * \param [out] regValue
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacXpMacRead(xpsDevice_t devId, uint32_t portNum, uint32_t regAddr,
                          uint16_t *regValue);

/**
 * \brief xpsMacRegisterPlatformSerdesInitFunc
 *
 * \param [out] func
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacRegisterPlatformSerdesInitFunc(platformSerdesInitFunc func);

/**
 * \brief xpsMacRegisterGetChassisPortInfoFunc
 *
 * \param [out] func
 *
 * \return [XP_STATUS] status On success XP_NO_ERR
*/
XP_STATUS xpsMacRegisterGetChassisPortInfoFunc(getChassisPortInfoFunc func);

/**
 * \brief This API will enable or disable receive pause frame filtering
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number - Valid values are 0-127 and 176
 * \param [in] enable True: Enable False: Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRxFlowControlFilterEnable(xpDevice_t devId, uint32_t portNum,
                                          uint8_t enable);

/**
 * \brief This API will get enable or disable status of receive filter pause frame feature
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number - Valid values are 0-127 and 176
 * \param [out] enable True: Enable False: Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRxFlowControlFilterEnableGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t *enable);

/**
 * \brief To read the pcs operational status from HW.
 * PCS will be in fully functional state when the status is true.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number. Valid value are 0-127 and 176.
 * \param [out] pcsOperation pcs operational status. True indicated that pcs is in fully functional mode
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacPcsOperationalStatusGet(xpDevice_t devId, uint32_t portNum,
                                        uint8_t *pcsOperation);

/**
 * \brief Reset all interrupt register stats
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] macNum mac/ptg num
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacDisableMacInterrupts(xpDevice_t devId, uint8_t macNum);

/**
 * \public
 * \brief To enable/disable the transmission of Local/Remote fault to link partner
 * 0 - disable
 * 1 - enable
 *
 * \param[in] devId device Id. Valid values are 0-63
 * \param[in] portNum port number. allowed range 0 to 127 and 176
 * \param[in] faultType - Type of fault to transmit Local/Remote
 * \param[in] value -  1 -> enable and 0 -> disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacTxFaultGenSet(xpDevice_t devId, uint32_t portNum,
                              xpMacTxFaultGen faultType, uint16_t value);

/**
 * \public
 * \brief To get the status(enable/disable) of the transmission of Local/Remote fault to link partner
 * 0 - disable
 * 1 - enable
 *
 * \param[in] devId device Id. Valid values are 0-63
 * \param[in] portNum port number. allowed range 0 to 127 and 176
 * \param[in] faultType - Type of fault to transmit Local/Remote
 * \param[out] value -  1 -> enable and 0 -> disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacTxFaultGenGet(xpDevice_t devId, uint32_t portNum,
                              xpMacTxFaultGen faultType, uint16_t *value);

/**
 * \brief Trap specific decoded value.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [out] trap TBD
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacPcsDecodeTrapGet(xpDevice_t devId, uint32_t portNum,
                                 xpDecodeTrap *trap);


/**
 * \brief Trap specific decoded value.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] trap TBD
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacPcsDecodeTrapSet(xpDevice_t devId, uint32_t portNum,
                                 xpDecodeTrap trap);

/**
 * \brief override the fault status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] value 1 for overriding fault status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacFaultStatusOverride(xpDevice_t devId, uint32_t portNum,
                                    uint8_t value);

/**
 * \brief override the link status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] value 1 for overriding link status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacLinkStatusOverride(xpDevice_t devId, uint32_t portNum,
                                   uint8_t value);

/**
 * \brief This API will Set the BackPlane AutoNeg Process for Base Page configure
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] ptgNum Port group number - Valid values are 0-31
 * \param [in] ANability
 * \param [in] ANctrl
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegProcessSet(xpDevice_t devId, uint32_t portNum,
                                           uint16_t ANability, uint16_t ANctrl, int is25GModeIEEE);

/**
 * \brief This API will Set the BackPlane AutoNeg Process for Next Page configure
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] ptgNum Port group number - Valid values are 0-31
 * \param [in] ANability
 * \param [in] ANctrl
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegSendConsortiumNextPage(xpDevice_t devId,
                                                       uint32_t portNum, uint16_t ANability, uint16_t ANctrl);
XP_STATUS xpsMacBackPlaneAutoNegConfigureNullNextPage(xpDevice_t devId,
                                                      uint32_t portNum);

XP_STATUS xpsMacBackPlaneAutoNegIsDone(xpDevice_t devId, uint32_t portNum,
                                       uint16_t *AN_status, uint16_t *AN_next_page);

/**
 * \brief This API will Reset the BackPlane AutoNeg Process
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] ptgNum Port group number - Valid values are 0-31
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegProcessReset(xpDevice_t devId,
                                             uint32_t portNum);

/**
 * \brief This API will Get the BackPlane AutoNeg Status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] ptgNum Port group number - Valid values are 0-31
 * \param [out] pointer to AN_status
 * \param [out] pointer to AN Ability
 * \param [out] pointer to Remote AN Ability
 * \param [out] pointer to AN Ctrl
 * \param [out] pointer to Remote AN Ctrl
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegStatusGet(xpDevice_t devId, uint32_t portNum,
                                          uint16_t *AN_status, uint16_t *ANability, uint16_t *remoteANability,
                                          uint16_t *ANctrl, uint16_t *remoteANctrl);

/**
 * \brief This API will do AN Success Action
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] ptgNum Port group number - Valid values are 0-31
 * \param [in] AN Ability
 * \param [in] Remote AN Ability
 * \param [in] AN Ctrl
 * \param [in] Remote AN Ctrl
 * \param [in] DFE Tune Enable
 * \param [in] serdes_init
 * \param [in] Link Training Enable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegSuccessAction(xpDevice_t devId,
                                              uint32_t portNum,
                                              uint16_t ANability, uint16_t remoteANability, uint16_t ANctrl,
                                              uint16_t remoteANctrl, uint8_t dfeTuneEn, uint8_t serdes_init,
                                              uint8_t LT_enable, uint8_t LT_timeoutDis);

/**
 * \brief This will invoke AACS server
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [in] tcpPort. TCP port of AACS server to connect on
 *
 * \return int32_t On error returns 0
 */
int32_t xpsMacSerdesAacsServer(xpDevice_t devId, uint8_t serdesId, int tcpPort);

/**
 * \brief This API will Set the Mac Training Process
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number - Valid values are 0-127
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSerdesLinkTrainingProcessSet(xpDevice_t devId, uint32_t portNum,
                                             xpMacConfigMode macConfigMode, bool fixedSpeed_an, uint8_t LT_timeoutDis);

/**
 * \brief Check if port serdes Dfe is running
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127.
 * \param [out] isDfeRunning 1 if Dfe is running, 0 if Dfe is in idle state
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMacPortSerdesDfeRunning(xpsDevice_t devId, xpsPort_t portNum,
                                     uint8_t *isDfeRunning);

/**
 * \brief Read port serdes signalOk value
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127.
 * \param [out] signalOk signal value
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMacPortSerdesGetSignalOk(xpsDevice_t devId, xpsPort_t portNum,
                                      uint8_t *signalOk);

/**
 * \brief This API will get the Mac Training status in a Loop with timeout
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number - Valid values are 0-127
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSerdesLinkTrainingStatusGetLoop(xpDevice_t devId,
                                                uint32_t portNum, xpMacConfigMode macConfigMode);
XP_STATUS xpsMacSerdesLinkTrainingClear(xpDevice_t devId, uint32_t portNum,
                                        xpMacConfigMode macConfigMode);
XP_STATUS xpsMacPortSerdesTuneConditionGet(xpsDevice_t devId,
                                           xpsPort_t portNum);
XP_STATUS xpsMacpcsRErrBlckAndPcsBerCntGet(xpDevice_t devId, uint32_t portNum,
                                           uint32_t *pcsRErrBlckCnt, uint32_t *pcsBerCnt);

/**
 * \brief Change the MAC mode
 *
 * This method assumes that all existing ports are deleted for given MAC,
 * and performs the mode change sequence. User can keep the port status Down upon switching the PTG speed if keepPortStatus argument given as 1. If it is 0 then it will keep the normal status of the port.
 * User can bring up the ports afterward when they switch the speed with keepPortDown as 1
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] macNum Mac number. Valid values are 0 to 32.
 * \param [in] newMode enum specifying mac configuration mode
 * \param [in] fecMode enum specifying the FEC mode, if enabled
 * \param [in] enableFEC enable FEC mode
 * \param [in] keepPortDown 1: for keeping the port down upon switching the speed
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMacSwitchMacConfigModeWithLinkStatusControl(xpsDevice_t devId,
                                                         uint8_t macNum, xpMacConfigMode newMode, xpFecMode fecMode, uint8_t enableFEC,
                                                         uint8_t keepPortDown);

/**
 * \brief  Initialise the PTG with the provided speed mode. User can keep the port status Down upon switching the PTG speed if keepPortStatus argument given as 1. If it is 0 then it will keep the normal status of the port.
 * User can bring up the ports afterward when they switch the speed with keepPortDown as 1
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] ptgNum       ptg number. Valid values are 0 to 32
 * \param [in] macConfig    mac mode
 * \param [in] speed        speed mode in MIXED_MODE
 * \param [in] initSerdes Flag to determine whether serdes needs to be inirialized or not
 * \param [in] prbsTestMode Flag to determine if serdes needs to be initialized in PRBS mode or CORE mode
 * \param [in] firmwareUpload Flag to determine if serdes rom file (firmware) needs to be uploaded or not
 * \param [in] fecMode  FEC mode. Available FEC modes are: RS_FEC_MODE and FC_FEC_MODE
 * \param [in] enableFEC    enable FEC mode
 * \param [in] keepPortDown 1: for keeping the port down upon switching the speed
 *
 * \return [XP_STATUS]  On success XP_NO_ERR.
 *          On failure appropriate error code.
*/
XP_STATUS xpsMacPortGroupInitWithLinkStatusControl(xpDevice_t devId,
                                                   uint8_t ptgNum, xpMacConfigMode macConfig, xpSpeed speed, uint8_t initSerdes,
                                                   uint8_t prbsTestMode, uint8_t firmwareUpload, xpFecMode fecMode,
                                                   uint8_t enableFEC, uint8_t keepPortDown);

/**
 * \brief  Initialise the port with the provided speed mode. User can keep the port status Down upon switching the port speed if keepPortStatus argument given as 1. If it is 0 then it will keep the normal status of the port.
 * User can bring up the ports afterward when they switch the speed with keepPortDown as 1
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum      port number. Valid values are 0 to 127
 * \param [in] macConfigMode    mac mode
 * \param [in] initSerdes Flag to determine whether serdes needs to be inirialized or not
 * \param [in] prbsTestMode Flag to determine if serdes needs to be initialized in PRBS mode or CORE mode
 * \param [in] firmwareUpload Flag to determine if serdes rom file (firmware) needs to be uploaded or not
 * \param [in] fecMode  FEC mode. Available FEC modes are: RS_FEC_MODE and FC_FEC_MODE
 * \param [in] enableFEC    enable FEC mode
 * \param [in] keepPortDown 1: for keeping the port down upon switching the speed
 *
 * \return [XP_STATUS]  On success XP_NO_ERR.
 *          On failure appropriate error code.
*/
XP_STATUS xpsMacPortInitWithLinkStatusControl(xpDevice_t devId,
                                              uint32_t portNum,
                                              xpMacConfigMode macConfigMode, uint8_t cpssIntfType,
                                              uint8_t initSerdes, uint8_t prbsTestMode,
                                              uint8_t firmwareUpload, xpFecMode fecMode, uint8_t enableFEC,
                                              uint8_t keepPortDown, uint8_t initPort);

/**
 * \public
 * \brief xpsMacBackPlaneAutoNegSendConsortiumExtNextPage
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127.
 * \param [in] ANability
 * \param [in] ANctrl
 *
 * \return XP_STATUS status on success XP_NO_ERR
 */
XP_STATUS xpsMacBackPlaneAutoNegSendConsortiumExtNextPage(xpDevice_t devId,
                                                          uint32_t portNum, uint16_t ANability, uint16_t ANctrl);

/**
 * \public
 * \brief API to print statistics
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127.
 * \param [in] stat A structure which holds various statistics of a port.
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsMacPrintPortStat(xpDevice_t devId, uint32_t portNum,
                              xp_Statistics *stat);
/**
 * \public
 * \brief set auto negotion in SGMII.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] enable configure sgmii auto negotiation
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSgmiiAutoNegSet(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable);
/**
 * \public
 * \brief 1: Enable Auto Negotiation
 *   0: Disable Auto Negotiation
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [out] enable sgmii auto negotiation enable/disable status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSgmiiAutoNegGet(xpsDevice_t devId, uint32_t portNum,
                                uint8_t *enable);
/**
 * \public
 * \brief Restart Auto Negotiation
 * Setting this bit to 1, restarts Auto negotiation,
 * This bit is automatically cleared once Auto negotiation is done
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] enable configure auto negotiation restart bit
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSgmiiAutoNegRestartSet(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t enable);
/**
 * \brief Get auto negotiation restart of SGMII.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [out] enable auto negotiation restart enable/disable status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacSgmiiAutoNegRestartGet(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *enable);
XP_STATUS xpsMacSgmiiDisparityErrCntGet(xpDevice_t devId, uint32_t portNum,
                                        uint16_t *count);
XP_STATUS xpsMacSgmiiInvalidCodeCntGet(xpDevice_t devId, uint32_t portNum,
                                       uint16_t *count);
/**
 * \public
 * \brief To change programmable AM lengh total of 18-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [in] value to be configure
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacCustomAlignmentMarkerLenSet(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t value);
/**
 * \public
 * \brief To get programmable AM lengh total of 18-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [out] value configured value
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacCustomAlignmentMarkerLenGet(xpsDevice_t devId, uint32_t portNum,
                                            uint32_t *value);
/**
 * \public
 * \brief To change programmable AM lengh total of 18-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [in] value to be configured
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecProgAmpLockLenSet(xpsDevice_t devId, uint32_t portNum,
                                       uint32_t value);
/**
 * \public
 * \brief To change rsfec PAM lengh total of 18-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [out] value configured value
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsMacRsFecProgAmpLockLenGet(xpsDevice_t devId, uint32_t portNum,
                                       uint32_t *value);
/**
 * \public
 * \brief To change rsfec Programmable CodeWord AM lengh total of 16-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [in] value to be configure
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsMacRsFecProgCwAmpLenSet(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t value);
/**
 * \public
 * \brief To change rsfec Programmable CodeWord AM lengh total of 16-bit
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127
 * \param [out] value configured value
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsMacRsFecProgCwAmpLenGet(xpsDevice_t devId, uint32_t portNum,
                                     uint16_t *value);
/**
* \public
* \brief  Mapping of PCS Rx Lane with serdes Lane set
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] portNum umac port numbers. valid port is 0 to 127
* \param [in] laneNum PCS Lane number 0-4
* \param [in] serdesLaneNum Serdes Lane Number 0-7
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsMacSerdesRxLaneRemapSet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t serdesLaneNum);
/**
* \public
* \brief  Mapping of PCS Rx Lane with serdes Lane get
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] portNum umac port numbers. valid port is 0 to 127
* \param [in] laneNum PCS Lane number 0-4
* \param [out] serdesLaneNum Serdes Lane Number 0-7
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsMacSerdesRxLaneRemapGet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t *serdesLaneNum);
/**
* \public
* \brief  Mapping of PCS Tx Lane with serdes Lane set
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] portNum umac port numbers. valid port is 0 to 127
* \param [in] laneNum PCS Lane number 0-4
* \param [in] serdesLaneNum Serdes Lane Number 0-7
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsMacSerdesTxLaneRemapSet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t serdesLaneNum);
/**
* \public
* \brief  Mapping of PCS Tx Lane with serdes Lane get
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] portNum umac port numbers. valid port is 0 to 127
* \param [in] laneNum PCS Lane number 0-4
* \param [out] serdesLaneNum Serdes Lane Number 0-7
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsMacSerdesTxLaneRemapGet(xpDevice_t devId, uint32_t portNum,
                                     uint8_t laneNum, uint8_t *serdesLaneNum);
/**
 * \public
 * \brief When set to 1, always allow clause 49 block types
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecAlwaysUseClause49EnableSet(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t enable);
/**
 * \public
 * \brief When set to 1, never allow clause 49 block types
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecNeverUseClause49EnableSet(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t enable);
/**
 * \public
 * \brief When set, enables pcs scrambler decoding
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecPcsScramblerEnableSet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable);
/**
 * \public
 * \brief When set, enables mlg scrambler decoding
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecMlgScramblerEnableSet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t enable);
/**
 * \public
 * \brief get always allow clause 49 block types status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [out] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecAlwaysUseClause49EnableGet(xpsDevice_t devId,
                                                uint32_t portNum, uint8_t *enable);
/**
 * \public
 * \brief get never allow clause 49 block types status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecNeverUseClause49EnableGet(xpsDevice_t devId,
                                               uint32_t portNum, uint8_t *enable);
/**
 * \public
 * \brief get pcs scrambler decoding status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecPcsScramblerEnableGet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable);
/**
 * \public
 * \brief get enables mlg scrambler decoding status
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] enable Enable/Disable
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsMacRsFecMlgScramblerEnableGet(xpsDevice_t devId, uint32_t portNum,
                                           uint8_t *enable);
/**
 * \public
 * \brief Set app fifo ports mapping
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [in] value port value which is mapped to channelNum
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsMacAppFifoPortMapSet(xpDevice_t devId, uint32_t portNum,
                                  uint8_t value);
/**
 * \public
 * \brief get app fifo ports mapping
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 176.
 * \param [out] value port value which is mapped to channelNum
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsMacAppFifoPortMapGet(xpDevice_t devId, uint32_t portNum,
                                  uint8_t *value);
/**
 * \public
 * \brief  get the fault status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum umac port numbers. valid port is 0 to 127
 * \param [out] status fault status.
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsMacFaultStatusOverrideGet(xpDevice_t devId, uint32_t portNum,
                                       uint8_t *status);
/**
 * \public
 * \brief  get the link status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum umac port numbers. valid port is 0 to 127
 * \param [out] status link status. Link is UP or DOWN
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsMacLinkStatusOverrideGet(xpDevice_t devId, uint32_t portNum,
                                      uint8_t *status);
/**
 * \public
 * \brief  Load Z80 Firmware
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] z80FwPath
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsMacTv80RunFirmware(xpDevice_t devId, const char *z80FwPath);

/**
 * \brief  check if port stats available on given port
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number
 * \param [out] isAvailable True if stats available
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsMacIsPortStatsAvailable(xpDevice_t devId, uint32_t portNum,
                                     uint8_t *isAvailable);

/**
 *\brief: Re-init the serdes
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *
 *\return [XP_STATUS] status On success XP_NO_ERR
*/

XP_STATUS xpsMacSerdesReInit(xpsDevice_t devId, uint32_t portNum);

/**
 *\brief: sets the AN state for XP port
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in] enable - set state enable or disable
 *
 *\return [XP_STATUS] status On success XP_NO_ERR
*/


XP_STATUS xpsMacSetPortAnEnable(xpsDevice_t devId, uint32_t portNum,
                                int enable);


/**
 *\brief: gives the AN state for XP port
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *
 *\return [in] enable - state enable or disable
 *\return [XP_STATUS] status On success XP_NO_ERR
*/

int xpsMacGetPortAnEnable(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief Retruns speed capability for AP port.
 *
 * \param [in] devId device id. Valid values are 0-
 * \param [in] portNum  port number. .
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

XP_STATUS xpsMacGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                 uint32_t *portAdvertFdAbility);

/**
 * \brief Retruns speed and FEC capability for AP remote port .
 *
 * \param [in] devId device id. Valid values are 0-
 * \param [in] portNum  port number. .
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */
XP_STATUS xpsMacPortANLtRemoteAbilityGet(xpDevice_t devId, uint32_t port,
                                         uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl);

/**
 *\brief: sets the cable type.
 *
 *\param [in] devId device Id. Valid values are 0-63
 *\param [in] portNum - Port number
 *\param [in]  type - set state enable or disable
type 0 : long cable
type 1 : short cable
type 3 : optic cable
 *
 *\return [XP_STATUS] status On success XP_NO_ERR
*/


XP_STATUS xpsMacSetPortCableLen(xpsDevice_t devId, uint32_t portNum,
                                int enable);

/**
 * \brief Sets Platform config file
 *
 * \param [in]Filename
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */
XP_STATUS xpsMacSetPlatformConfigFileName(char *fileName);

#ifdef __cplusplus
}
#endif

#endif //_XPSLINK_H_
