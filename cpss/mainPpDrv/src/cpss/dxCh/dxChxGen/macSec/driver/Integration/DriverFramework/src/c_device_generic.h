/* c_device_generic.h
 *
 * This is the default configuration file for the generic Driver Framework
 * implementation.
 */

/*****************************************************************************
* Copyright (c) 2017 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/* Top-level Hardware platform configuration */
#include <Kit/DriverFramework/incl/cs_hwpal.h>

/* Enables strict argument checking for input parameters */
/*#define HWPAL_STRICT_ARGS_CHECK */

/* choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
#ifndef LOG_SEVERITY_MAX
#ifndef CPSS_LOG_ENABLE
#define LOG_SEVERITY_MAX    LOG_SEVERITY_NO_OUTPUT
#else
#define LOG_SEVERITY_MAX    LOG_SEVERITY_INFO
#endif /*CPSS_LOG_ENABLE*/
#endif
/* Some magic number for device data validation */
/*#define HWPAL_DEVICE_MAGIC   0xBABADEDAUL */


/* end of file c_device_generic.h */
