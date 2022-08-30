/* adapter_alloc.h
 *
 * User-space implementation of the Adapter buffer allocation functionality.
 */

/*****************************************************************************
* Copyright (c) 2011-2016 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef ADAPTER_ALLOC_H_
#define ADAPTER_ALLOC_H_
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*----------------------------------------------------------------------------
  Adapter_Alloc
*/
static inline void *
Adapter_Alloc(unsigned int size)
{
    if (size > 0) /* Many callers do not check if requested size is non-zero */
        return cpssOsMalloc(size); /* malloc() may return non-NULL for size=0 */
    else
        return NULL; /* ... but only extreme optimists do not check the result! */
}


/*----------------------------------------------------------------------------
  Adapter_Free
*/
static inline void
Adapter_Free(void *p)
{
    if (p != NULL)
    {
        cpssOsFree(p); /* free() may take a NULL pointer but why bother */
        p = NULL;
    }
}


#endif /* ADAPTER_ALLOC_H_ */


/* end of file adapter_alloc.h */
