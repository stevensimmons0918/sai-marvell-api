/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file linuxKeLsemLib.h
*
* @brief Used for linux/kernelExt/ke_lsemLib.c
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsLinuxKeLsemLibh
#define __gtOsLinuxKeLsemLibh

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _osMutex
{
    OS_OBJECT_HEADER_STC    header;
    pthread_mutex_t         mtx;
#ifdef  OS_USERSPACE_MUTEX_STAT
    int                     lockcount;
    int                     unlockcount;
#endif
}GT_LINUX_MUTEX;

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsLinuxSimStubss */
/* Do Not Add Anything Below This Line */



