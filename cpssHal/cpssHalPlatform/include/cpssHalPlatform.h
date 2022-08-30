/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalPlatform.h

#ifndef _cpssHalPlatform_h_
#define _cpssHalPlatform_h_

#include "xpTypes.h"
//#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsMac.h"
#include "gtGenTypes.h"
#include <cpss/generic/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \file cpssHalPlatform.h
 * \brief This file contains API prototypes and type definitions
 *  for the platform
 */


/**
 * \brief to write value for GPIO pin.
 *
 * \param [in] devNum device Number. Valid values are 0-63
 * \param [in] gpioPinPosition Valid pin positions are 0-31.
 * \param [in] GT_BOOL to set or clear the value for GPIO pin corresponding to gpioPinPosition

 * \return [GT_STATUS] On success GT_OK.
 */

GT_STATUS cpssHalGpioRegisterWrite(GT_U8 devNum, GT_U8 gpioPinPosition,
                                   GT_BOOL set);

/**
 * \brief platform specific init
 *                   *
 * \param [in] devNum device Number. Valid values are 0-63
 * \param [in] devType device type.

 * * \return [GT_STATUS] On success GT_OK.
 *                                                                */

GT_STATUS cpssHalPlatformInit(GT_U8 devNum, XP_DEV_TYPE_T devType);

/**
 * \brief platform specific deinit
 *                   *
 * \param [in] cpssDevNum device Number. Valid values are 0-63

 * * \return [GT_STATUS] On success GT_OK.
 *                                                                */

GT_STATUS cpssHalPlatformDeInit(GT_U8 cpssDevNum);

#ifdef __cplusplus
}
#endif

#endif
