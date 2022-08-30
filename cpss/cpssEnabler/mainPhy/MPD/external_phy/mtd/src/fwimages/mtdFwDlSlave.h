/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
#ifndef MTDSLSLAVE_H
#define MTDSLSLAVE_H
#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

#if MTD_INCLUDE_SLAVE_IMAGE

/* The functions in mtdFwDlSlave are provided to allow using the slave
   hdr file compiled and linked as part of the API instead of
   as a file which must be opened and read in by the host software
   and passed to mtdUpdateFlashImage() and mtdParallelUpdateFlashImage()

   The functions below call the corresponding function and pass the slave
   hdr data.
*/


/* See prototype for mtdUpdateFlashImage() in mtdFwDownlaoad.h for details */
MTD_STATUS mtdUpdateFlash
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    OUT MTD_U16 *errCode
);

/* See prototype for mtdParallelUpdateFlashImage() in mtdFwDownlaoad.h for details */
MTD_STATUS mtdParallelUpdateFlash
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 ports[],
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    IN MTD_U16 numPorts, 
    OUT MTD_U16 erroredPorts[],
    OUT MTD_U16 *errCode
);


#endif

#if C_LINKAGE
#if defined __cplusplus
}
#endif 
#endif

#endif /* defined MTDSLSLAVE_H */

