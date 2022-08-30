/* cs_driver_ext.h
 *
 * Top-level Product Configuration Settings specific for FPGA.
 */

/*****************************************************************************
* Copyright (c) 2015-2017 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_CS_DRIVER_EXT_H
#define INCLUDE_GUARD_CS_DRIVER_EXT_H

/* EIP-164 HAWK board specific parameters */
#define ADAPTER_HAWK_HW_RESET_ENABLE

/* Maximum number of optional devices in device list */
#if defined(ARCH_X86)||defined(ARCH_POWERPC)
#define DRIVER_OPTIONAL_DEVICE_COUNT  3
#else
#define DRIVER_OPTIONAL_DEVICE_COUNT  0
#endif


#endif /* Include Guard */


/* end of file cs_driver_ext.h */
