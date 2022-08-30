/* adapter_lock_ext.h
 *
 * Adapter concurrency (locking) management
 * extensions for Linux user-space
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

#ifndef INCLUDE_GUARD_ADAPTER_LOCK_EXT_H
#define INCLUDE_GUARD_ADAPTER_LOCK_EXT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#if 0
/* Linux User API */
#include <pthread.h>        /* pthread_mutex_* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define ADAPTER_LOCK_DEFINE(Lock)   \
                pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER

/* Lock structure, so it can be part of another structure or array */
typedef pthread_mutex_t Adapter_Lock_Struct_t;

/* Initializer for elements of Adapter_Lock_Struct_t */
#define ADAPTER_LOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#else

typedef struct
{
    uint32_t lock;
} Adapter_Lock_Struct_t;
#endif


#endif /* INCLUDE_GUARD_ADAPTER_LOCK_EXT_H */

/* end of file adapter_lock_ext.h */
