/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
#include "mtdApiRegs.h"
#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdFwDownload.h"
#include "mtdAPIInternal.h"
#include "mtdUtils.h"
#include "mtdFwImage.h"
#include "mtdFwDlSlave.h"


#if MTD_INCLUDE_SLAVE_IMAGE

/* See prototype for mtdUpdateFlashImage() in mtdFwDownlaoad.h for details */
MTD_STATUS mtdUpdateFlash
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    OUT MTD_U16 *errCode
)
{
    return mtdUpdateFlashImage(devPtr, 
                               port, 
                               appData, 
                               appSize, 
                               MTD_slave_image,
                               MTD_SIZEOF_SLAVE_IMAGE,
                               errCode);
}

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
)
{
    return mtdParallelUpdateFlashImage(devPtr, 
                                       ports, 
                                       appData, 
                                       appSize, 
                                       MTD_slave_image, 
                                       MTD_SIZEOF_SLAVE_IMAGE, 
                                       numPorts, 
                                       erroredPorts, 
                                       errCode);
}


#endif
