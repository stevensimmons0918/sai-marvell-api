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
* @file linuxLtaskLib.h
*
* @brief Used for linux/pthreads/ltaskLib.c
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsLinuxLtaskLibh
#define __gtOsLinuxLtaskLibh

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _V2L_taskSTC
{
    OS_OBJECT_HEADER_STC    header;
    pthread_t               pthrid;
    int                     vxw_priority;
    unsigned                (__TASKCONV *entry_point)(GT_VOID*);
    GT_VOID                 *param;
} _V2L_taskSTC;

typedef struct
{
    /* pthread ID of task */
    pthread_t               pthrid;
    /* Task Id, values > 0, means valid values */
    GT_32                   tid;
} _V2L_pthread_hash_STC;

/* hash table size - simulation of complex systems with multiple devices may
   create 100 tasks approximately. Use hash size 512 to minimize collisions. */
#define V2L_PTHREAD_HASH_TABLE_SIZE_CNS 512

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsLinuxSimStubss */
/* Do Not Add Anything Below This Line */



