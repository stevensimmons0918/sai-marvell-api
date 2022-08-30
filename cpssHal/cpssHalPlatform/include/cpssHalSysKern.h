/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalSysKern.h
*
* @brief header which defines functions to load/unload Kernel modules
*
* @version   01
********************************************************************************
*/
#ifndef _cpssHalSysKern_h_
#define _cpssHalSysKern_h_


#include <cpss/generic/cpssTypes.h>


#define PLATFORM_GPIO_DRIVER    "/lib/modules/mvGpioDrv.ko"
#define PLATFORM_GPIO_MODNAME   "mvGpioDrv"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \file cpssHalSysKern.h
 * \brief This file contains API prototypes and type definitions
 *  for the platform
 */


/**
 * \brief Load kernel driver
 *
 * \param [in] kernel driver module name
 * \param [in] kernel params

 * \return [GT_STATUS] On success GT_OK.
 */

GT_STATUS load_kernel_module(char *mod_file_name, char *params);

/**
 * \brief Unload kernel driver
 *
 * \param [in] kernel driver module name

 * \return [GT_STATUS] On success GT_OK.
 */

GT_STATUS remove_kernel_module(char *mod_name);

#ifdef __cplusplus
}
#endif

#endif

