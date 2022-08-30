/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions for initializing the driver and setting 
up the user-provide MDIO access functions for the Marvell 88X32X0, 
88X33X0, 88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdAPIInternal.h>
#include <mtdInitialization.h>
#include <mtdHwCntl.h>
#include <mtdHwSerdesCntl.h>
#include <mtdDiagnostics.h>

/*******************************************************************************
*
********************************************************************************/

MTD_STATUS mtdLoadDriver
(
    IN FMTD_READ_MDIO     readMdio,    
    IN FMTD_WRITE_MDIO    writeMdio,   
    IN FMTD_WAIT_FUNC     waitFunc,
    IN MTD_U16            anyPort,
    IN MTD_BOOL           forceLoad,
    OUT MTD_DEV          *dev,
    OUT MTD_U16           *errCode
)
{
    MTD_U16 data;
    *errCode = MTD_ERR_UNDEFINED;

/*    MTD_DBG_INFO("mtdLoadDriver Called.\n"); */

    /* Check for parameters validity        */
#if 0 /* allow to be called with NULL for IO functions, in case host wants to implement their own and not use the */
      /* ones inside the MTD_DEV struct */
    if(readMdio == NULL || writeMdio == NULL )
    {
        MTD_DBG_ERROR("MDIO read or write pointers are NULL.\n");
        *errCode = MTD_ERR_INCORRECT_MDIO;
        return MTD_FAIL;
    }
#endif


    /* Check for parameters validity        */
    if(dev == NULL)
    {
        MTD_DBG_ERROR("MTD_DEV pointer is NULL.\n");
        *errCode = MTD_ERR_LOAD_DRIVER_NULL;
        return MTD_FAIL;
    }
    
    /* The initialization was already done. */
    if(dev->devEnabled)
    {
        MTD_DBG_ERROR("Device Driver already loaded.\n");
        *errCode = MTD_ERR_LOAD_DRIVER_EXISTED;
        return MTD_FAIL;
    }

    dev->fmtdReadMdio =  readMdio;
    dev->fmtdWriteMdio = writeMdio;
    dev->fmtdWaitFunc = waitFunc;

    if (dev->fmtdWaitFunc == NULL)
    {
        /* Make sure mtdWait() was implemented */
        if (mtdWait(dev,1) == MTD_FAIL)
        {
            MTD_DBG_ERROR("mtdWait() not implemented.\n");
            *errCode = MTD_ERR_WAIT_NOT_IMPLEMENTED;
            return MTD_FAIL;
        }
    }

    if((mtdHwXmdioRead(dev, anyPort,1,0 ,&data))!= MTD_OK) /* try to read 1.0 */
    {
        MTD_DBG_ERROR("Reading to reg %x failed.\n",0);
        mtdUnloadDriver(dev);
        *errCode = MTD_ERR_READ_REG_FAIL;
        return MTD_FAIL;
    }

    if (data == 0x0000 || data == 0xFFFF)
    {
        MTD_DBG_ERROR("Reading to reg %x failed.\n",0);
        mtdUnloadDriver(dev);
        *errCode = MTD_ERR_READ_REG_FAIL;
        return MTD_FAIL;
    }
  
    /* MTD_DBG_INFO("mtdLoadDriver successful.\n"); */

    if (dev->msec_ctrl.msec_rev == MTD_MSEC_REV_FPGA)
    {
        dev->deviceId = MTD_REV_3310P_A0; /* verification: change if needed */
        dev->numPortsPerDevice = 1; /* verification: change if needed */
        dev->thisPort = 0;
    } 
    else
    {
        /* After everything else is done, can fill in the device id */
        if ((mtdGetPhyRevision(dev, anyPort, 
                               &(dev->deviceId), 
                               &(dev->numPortsPerDevice), 
                               &(dev->thisPort))) != MTD_OK)
        {
            if (forceLoad == MTD_FALSE)
            {
                MTD_DBG_ERROR("mtdGetPhyRevision Failed.\n");
                mtdUnloadDriver(dev);
                *errCode = MTD_ERR_GET_REV_FAIL;
                return MTD_FAIL;
            }
            else
            {
                /* Do some sanity checks to make sure at least enough data is filled in to have */
                /* most of the API work, numPorts if not correct might cause a problem somewhere */
                /* but let them do it if they force it. */
                if (deviceFamilyValid(dev->deviceId) == MTD_FAIL)
                {
                    MTD_DBG_ERROR("mtdGetPhyRevision Failed and forceLoad Failed.\n");
                    mtdUnloadDriver(dev);
                    *errCode = MTD_ERR_GET_REV_FAIL;
                    return MTD_FAIL;
                }
                else
                {
                    MTD_DBG_INFO("mtdGetPhyRevision Failed and forceLoad succeeded.\n");
                    MTD_DBG_INFO("WARNING: Unrecognized device!\n");
                    MTD_DBG_INFO("WARNING: Using partial device info to proceed!\n");
                }
            }
        }
    }
    
    if (MTD_IS_X32X0_BASE(dev->deviceId) && (dev->deviceId != MTD_REV_3240_A1) && (dev->deviceId != MTD_REV_3240P_A1))
    {
        /* 88X32X0 devices A0 and earlier required indirect Macsec access through T unit processor */
        dev->devInfo |= MTD_MACSEC_INDIRECT_ACCESS;
    }
    else
    {
        /* 88X32X0 A1 and 88X33X0 and later force direct access */
        dev->devInfo &= ~MTD_MACSEC_INDIRECT_ACCESS;
    }

#if (MTD_CLAUSE_22_MDIO == 1)
    if (MTD_IS_X32X0_BASE(dev->deviceId))
    {
        mtdUnloadDriver(dev);
        *errCode = MTD_ERR_INCORRECT_MDIO;
        return MTD_FAIL; /* Clause 45 using Clause 22 is not available in 88X32X0 phys */
    }
#endif

    if (MTD_IS_X35X0_BASE(dev->deviceId))
    {
#if MTD_PKG_CE_SERDES
        /* Initialize serdes device structure if any of COMPHY Serdes code selected */
        if(mtdInitSerdesDev(dev, anyPort) != MTD_OK)
        {
            MTD_DBG_ERROR("mtdLoadDriver: Serdes initialization failed.\n");
            return MTD_FAIL;
        }
#endif  /* MTD_PKG_CE_SERDES */
    }

    dev->devEnabled = MTD_TRUE;

/*    MTD_DBG_INFO("mtdLoadDriver successful.\n"); */

    return MTD_OK;
}

/*******************************************************************************
* mtdUnloadDriver
*
* DESCRIPTION:
*       This function clears MTD_DEV structure.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MTD_OK           - on success
*       MTD_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           mtdLoadDriver().
*
*******************************************************************************/
MTD_STATUS mtdUnloadDriver
(
    IN MTD_DEV* dev
)
{
/*    MTD_DBG_INFO("mtdUnloadDriver Called.\n"); */

    dev->fmtdReadMdio =  NULL;
    dev->fmtdWriteMdio = NULL;
    dev->fmtdWaitFunc = NULL;

    if (MTD_IS_X35X0_BASE(dev->deviceId))
    {
#if MTD_PKG_CE_SERDES
        /* This is a placeholder, the mtdUnloadSerdesDev() is calling mcesdUnloadDriver(), */
        /* which is actually doing nothing right now */
        /* if there is any Serdes-specific operations included in the future */
        /* this need to be changed */
        if(mtdUnloadSerdesDev(dev, 0) != MTD_OK)
        {
            MTD_DBG_ERROR("mtdUnloadDriver: Serdes device struct reset failed.\n");
            return MTD_FAIL;
        }
#endif  /* MTD_PKG_CE_SERDES */
    }

    dev->devEnabled = MTD_FALSE;

    return MTD_OK;
}

