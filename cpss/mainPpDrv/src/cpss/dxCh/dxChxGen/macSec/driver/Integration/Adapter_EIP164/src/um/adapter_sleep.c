/* adapter_sleep.c
 *
 * Linux user-space specific Adapter module
 * responsible for adapter-wide time management.
 */

/*****************************************************************************
* Copyright (c) 2008-2016 INSIDE Secure B.V. All Rights Reserved.
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

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>

/* Driver Framework Basic Defs API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* get the OS , extDrv functions*/
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*----------------------------------------------------------------------------
 * Adapter_SleepMS
 */
void
Adapter_SleepMS(
        const unsigned int Duration_ms)
{
    cpssOsTimerWkAfter(Duration_ms);
}


/* end of file adapter_sleep.c */
