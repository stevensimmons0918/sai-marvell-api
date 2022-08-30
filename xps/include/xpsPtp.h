// xpsPtp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsPtp.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS PTP Manager
 */

#ifndef _XPSPTP_H_
#define _XPSPTP_H_

#include "xpsInit.h"
#include "xpsEnums.h"
//#include "xpPtpMgr.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize the XPS Ptp Manager
 *
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpInit();

/**
 * \brief API will verfiy if new time-stamp value is available.
 *
 * \param [in] devId
 * \param [out] timeStamp
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpFetchTimeStamp(xpsDevice_t devId, uint64_t *timeStamp);

/**
 * \brief API to initialize and add device speicific primitives
 *        in primitive manager(s).
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief API to de-initialize and remove device speicific primitives
 *        in primitive manager(s).
 *
 * \param [in] devId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpRemoveDevice(xpsDevice_t devId);

/**
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpDeInit();

/**
 * \brief API to load the current time to the clock.
 *
 * \warning This API isn't accurate.  Please use increment APIs to change the clock
 *
 * \param [in] devId
 * \param [in] currentTime
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpLoadCurrentTime(xpsDevice_t devId, int64_t currentTime);

/**
 * \brief API to get the current time which was loaded.
 *
 * \param [in] devId
 * \param [out] currentTime
 * \return XP_STATUS
*/
XP_STATUS xpsPtpGetCurrentTime(xpsDevice_t devId, uint64_t *currentTime);

/**
 * \brief API to adjust the  current time to the clock.
 *
 * \param [in] devId
 * \param [in] currentTime
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpAdjustCurrentTime(xpsDevice_t devId, int64_t deltaTime);

/**
 * \brief API to adjust the  current time to the clock.
 *
 * \param [in] devId
 * \param [out] currentTime
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpGetAdjustCurrentTime(xpsDevice_t devId, uint64_t *deltaTime);

/**
 * \param [in] devId
 * \param[in] nsInc The amount of integer nanoseconds (xpsDevice_t devId, 0-15) to
 increment on each clock
 * \param[in] fns_inc The amount of fractional nanoseconds to
 inrement on each clock. The units of this
 value are in 1/pow(xpsDevice_t devId, 2,32)
 * \brief Load the provided time value into the device clock
 * \return [XP_STATUS] on success XP_NO_ERR
*/
XP_STATUS xpsPtpLoadClockInc(xpsDevice_t devId, uint32_t nsInc,
                             uint32_t fnsInc);

/**
 * \param [in] devId
 * \param [out] nsInc The amount of integer nanoseconds (xpsDevice_t devId, 0-15) to
 increment on each clock
 * \param [out] fns_inc The amount of fractional nanoseconds to
 inrement on each clock. The units of this
 value are in 1/pow(xpsDevice_t devId, 2,32)
 * \brief Load the provided time value into the device clock
 * \return [XP_STATUS] on success XP_NO_ERR
*/
XP_STATUS xpsPtpGetClockInc(xpsDevice_t devId, uint32_t *nsInc,
                            uint32_t* fnsInc);

/**
 * \param [in] devId
 * \param [in] outputNum which ptp sync pin is in use
 * \param [in] startTime Time(xpsDevice_t devId, in ns) at which to start the clock generation
 * \param [in] pulseWidth Width(xpsDevice_t devId, in ns) of the high portion of the clock
 * \param [in] nsInc Nanosecond portion of the clock period
 * \param [in] fsnInc Fractional nanosecond of the clock period
 * \brief Setup the device to send a recurring output clock
 *        Setting a start_time which is lower than the current
 *        time value may result in a glitch on the output.  The
 *        duty cycle of the generated clock is approximately
 *            pulse_width / ns_inc.
 *        This functionality is intended for generating low-speed
 *            clocks, jitter and slew rates may be unacceptable at
 *            speeds above 1 Mhz.
*/
XP_STATUS xpsPtpSetOutputClock(xpsDevice_t devId, uint32_t outputNum,
                               uint64_t startTime, int pulseWidth, uint32_t  nsInc, uint32_t fsnInc);

/**
 * \param [in] devId
 * \param[in] outputNum which ptp sync pin is in use
 * \param[out] startTime Time(xpsDevice_t devId, in ns) at which to start the clock generation
 * \param[out] pulseWidth Width(xpsDevice_t devId, in ns) of the high portion of the clock
 * \param[out] nsInc Nanosecond portion of the clock period
 * \param[out] fsnInc Fractional nanosecond of the clock period
 * \brief Setup the device to send a recurring output clock
 *        Setting a start_time which is lower than the current
 *        time value may result in a glitch on the output.  The
 *        duty cycle of the generated clock is approximately
 *            pulse_width / ns_inc.
 *        This functionality is intended for generating low-speed
 *            clocks, jitter and slew rates may be unacceptable at
 *            speeds above 1 Mhz.
*/
XP_STATUS xpsPtpGetOutputClock(xpsDevice_t devId, uint64_t *startTime,
                               uint32_t* ns, uint32_t* fsn);

/**
 * \param [in] devId
 * \param [in] pinNum  which ptp_sync pin to use(xpsDevice_t devId, 0-1)
 * \param [in] direction configure pin as input or output
 * \brief  Set direction (xpsDevice_t devId, input or output) of sync pin
 * \return XP_STATUS
*/
XP_STATUS xpsPtpSetSyncDir(xpsDevice_t devId, uint32_t pinNum,
                           uint32_t direction);

/**
 * \param [in] devId
 * \param [in] pinNum
 * \param [out] direction
 * \return XP_STATUS
*/
XP_STATUS xpsPtpGetSyncDir(xpsDevice_t devId, uint32_t pinNum,
                           uint32_t *direction);

/**
 * \param [in] inputNum which ptp_sync pin to use (xpsDevice_t devId, 0-1)
 * \param [in] action set action to take when a pulse arrives in this pin
 * \brief Set registers to capture an incoming pulse
 * Action Encoding | Description
 * ----------------|-------------
 *        0        | No action (xpsDevice_t devId, disabled)
 *        1        | Save current time to capture_time register
 *        2        | Load capture_time register to current time
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPtpSetupPulseAction(xpsDevice_t devId, uint32_t inputNum,
                                 uint32_t action);

/**
 * \param [in] devId
 * \param [in] inputNum
 * \param [out] action
 * \return XP_STATUS
*/
XP_STATUS xpsPtpGetPulseAction(xpsDevice_t devId, uint32_t inputNum,
                               uint32_t *action);

/**
 * \param [in] devId
 * \param [in] nsTime time in ns to be captured
 * \brief Set the load pulse value
 *                Set the value which will be loaded into the current time when
 *                a pulse arrives on one of the two ptp_sync pins.
 * \return XP_STATUS
*/
XP_STATUS xpsPtpSetCaptureTime(xpsDevice_t devId, uint64_t nsTime);

/**
 * \param [in] devId
 * \param [in] inputNum which sync_pin to use (xpsDevice_t devId, 0/1)
 * \param [out] arrivalTime location to store the retirved time
 * \brief Get arrival time of an expected incoming pulse.
 * \return XP_STATUS
*/
XP_STATUS xpsPtpGetPulseTime(xpsDevice_t devId, uint32_t inputNum,
                             uint64_t *arrivalTime);

/**
 * \param [in] devId
 * \param [in] portNum port number. Valid values are 0 to 127 and 176.
 * \param [out] egressTs Egress Time Stamp
 * \brief Get timestamp of egress PTP frame.
 * \return XP_STATUS
 */
XP_STATUS xpsPtpGetEgressTimeStamp(xpsDevice_t devId, uint32_t portNum,
                                   uint64_t *egressTs);

#ifdef __cplusplus
}
#endif

#endif //_XPSPTP_H_
