/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalLed.h

#ifndef _cpssHalLed_h_
#define _cpssHalLed_h_

#include "xpTypes.h"
//#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsMac.h"
#include "gtGenTypes.h"
#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include "cpssHalProfile.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \file cpssHalLed.h
 * \brief This file contains API to enable Port LED
 */


/**
 * \brief config Led stream
 *
 * \param [in] devNum device Number. Valid values are 0-63
 * \param [in] devType  devive type
 * \param [in] ledProfile Led Profile

 * \return [GT_STATUS] On success GT_OK.
 */
GT_STATUS cpssHalLedInit(GT_U8 devNum, XP_DEV_TYPE_T devType,
                         LED_PROFILE_TYPE_E ledProfile);

/**
 * \brief config Led stream
 *
 * \param [in] devNum device Number. Valid values are 0-63
 * \param [in] ledInterfaceNum Led interface number
 * \param [in] ledProfile Led Profile

 * \return [GT_STATUS] On success GT_OK.
 */
GT_STATUS cpssHalLedConfig(GT_U8 devNum, GT_U32 ledInterfaceNum,
                           XP_DEV_TYPE_T devType, LED_PROFILE_TYPE_E ledProfile);
/**
 * \brief config specific port led
 *
 * \param [in] devNum device Number. Valid values are 0-63
 * \param [in] portNum  Port number.
 * \param [in] position led position.

 * \return [GT_STATUS] On success GT_OK.
 */

GT_STATUS cpssHalLedPortConfig(GT_U8 devNum, XP_DEV_TYPE_T devType,
                               GT_PHYSICAL_PORT_NUM portNum, GT_U32 position, LED_PROFILE_TYPE_E ledProfile);

/**
 * \brief config specific port led
 *
 * \param [in] cpssDevNum device Number. Valid values are 0-63
 * \param [in] portNum  Port number.
 * \param [in] cpssSpeeed
 * \param [in] linkState

 * \return [GT_STATUS] On success GT_OK.
 */
GT_STATUS cpssHalLedPortForcedStatus(GT_U8 cpssDevNum,
                                     GT_PHYSICAL_PORT_NUM portNum, CPSS_PORT_SPEED_ENT cpssSpeed, int linkState);
#ifdef __cplusplus
}
#endif

#endif
