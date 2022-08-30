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
* @file linuxLmsgqLib.h
*
* @brief Used for linux/pthreads/lmsgqLib.c
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsLinuxLmsgqLibh
#define __gtOsLinuxLmsgqLibh

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _osMsgQSTC
{
    OS_OBJECT_HEADER_STC    header;
    pthread_mutex_t         mtx;
    pthread_cond_t          rxSem;
    pthread_cond_t          txSem;
    int                     maxMsgs;
    int                     maxMsgSize;
    int                     messages;
    char                    *buffer;
    int                     head;
    int                     tail;
    int                     waitRx;
    int                     waitTx;
} OS_MSGQ_STC;

/*******************************************************************************
* V2L_ts_malloc
*
* DESCRIPTION:
*       thread-safe malloc
*
* INPUTS:
*       blksize    - number of bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to allocated buffer or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
void* V2L_ts_malloc
(
    IN  size_t blksize
);
    
/*******************************************************************************
* V2L_ts_free
*
* DESCRIPTION:
*       thread-safe free
*
* INPUTS:
*       blkaddr    - address previosly allocated buffer None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
void V2L_ts_free
(
    IN  void *blkaddr
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsLinuxSimStubss */
/* Do Not Add Anything Below This Line */



