/*******************************************************************************
*
*         Copyright 2003, MARVELL SEMICONDUCTOR ISRAEL, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED AS IS. MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL SEMICONDUCTOR ISRAEL. (MSIL),  MARVELL TAIWAN, LTD. AND      *
* SYSKONNECT GMBH.                                                             *
********************************************************************************
* microinit_main.c
*
* DESCRIPTION:
*       This file implements microinit entry point
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <gtOs/gtGenTypes.h>
#include <cm3FileOps.h>
#include <cm3BootChannel.h>
#include <stdio.h>      /* printf, NULL */
#include <cm3NetPort.h>
#include <asicSimulation/SCIB/scib.h>



/*******************************************************************************
* unq_pollingPerProjectUserFuntion
* DESCRIPTION:
*       This function can be used for any special function
*       that is needed per project and subject to polling mechanisem.
* INPUTS:
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS microInitCm3Unq_pollingPerProjectUserFuntion()
{
    /* In Falcon MI we use the Port task to check bootchannel messages instead of supervisor */
    if (cm3SimShmIpcBootChannelReady())
    {
        SIM_CM3_LOG("\n==> BC start processing message.\n");

        if(cm3SimBootChannelRecv())
        {
            return GT_FAIL;
        }

        SIM_CM3_LOG("\n==>BC end processing message.\n");
    }

    return GT_OK;
}

