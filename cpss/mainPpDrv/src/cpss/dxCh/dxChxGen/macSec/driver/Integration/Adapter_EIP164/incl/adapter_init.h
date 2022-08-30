/* adapter_init.h
 *
 * Data types and Interfaces
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

#ifndef INCLUDE_GUARD_ADAPTER_INIT_H
#define INCLUDE_GUARD_ADAPTER_INIT_H

/* Driver Framework Basic Defs API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/*----------------------------------------------------------------------------
 *                           Adapter initialization
 *----------------------------------------------------------------------------
 */



GT_BOOL
Adapter_Init(GT_U8 devNum, GT_U32 unitBmp);

void
Adapter_UnInit(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice);

void
Adapter_Report_Build_Params(void);


/** Data path IDs */
enum
{
    DP_ID0,
    DP_ID1,
    DP_ID2,
    DP_ID3
};

/** MACSec devices types */
enum
{
    EIP163_TYPE,
    EIP164_TYPE
};

/** MACSec device direction */
enum
{
    INGRESS_DIR,
    EGRESS_DIR
};


#endif /* Include Guard */

/* end of file adapter_init.h */
