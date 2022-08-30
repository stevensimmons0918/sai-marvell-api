/* adapter_lock.c
 *
 * Adapter concurrency (locking) management
 * Linux user-space implementation
 *
 */

/*****************************************************************************
* Copyright (c) 2013-2017 INSIDE Secure B.V. All Rights Reserved.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* Adapter locking API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>     /* IDENTIFIER_NOT_USED */

/* Adapter Lock Internal API */
#include <Integration/Adapter_EIP164/incl/adapter_lock_internal.h>

/* Adapter Memory Allocation API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>

/* Logging API */
#undef LOG_SEVERITY_MAX
#ifndef CPSS_LOG_ENABLE
#define LOG_SEVERITY_MAX    LOG_SEVERITY_NO_OUTPUT
#else
#define LOG_SEVERITY_MAX    LOG_SEVERITY_WARN
#endif /*CPSS_LOG_ENABLE*/

/* Logging API */
#include <Kit/Log/incl/log.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Adapter_Lock_Alloc
 */
Adapter_Lock_t
Adapter_Lock_Alloc(void)
{
    return Adapter_Lock_NULL;
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Free
 */
void
Adapter_Lock_Free(Adapter_Lock_t Lock)
{
    IDENTIFIER_NOT_USED(Lock);
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Acquire
 */
void
Adapter_Lock_Acquire(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    IDENTIFIER_NOT_USED(Lock);
    IDENTIFIER_NOT_USED(Flags);
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Release
 */
void
Adapter_Lock_Release(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    IDENTIFIER_NOT_USED(Lock);
    IDENTIFIER_NOT_USED(Flags);
}


/* end of file adapter_lock.c */
