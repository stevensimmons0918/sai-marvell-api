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
#ifndef MTDINITIALIZATION_H
#define MTDINITIALIZATION_H

#if C_LINKAGE
#ifdef __cplusplus
extern "C" {
#endif
#endif

#define MTD_ERR_WAIT_NOT_IMPLEMENTED     0xF7    /* mtdWait() is not implemented */
#define MTD_ERR_GET_REV_FAIL             0xF8
#define MTD_ERR_READ_REG_FAIL            0xF9    /* reading from phy reg failed. */
#define MTD_ERR_UNLOAD_DRIVER_FAIL       0xFA    /* unload driver failed */
#define MTD_ERR_LOAD_DRIVER_NULL         0xFB    /* driver structure is NULL. */
#define MTD_ERR_INCORRECT_MDIO           0xFC    /* invalid MDIO version or NULL pointer */
#define MTD_ERR_LOAD_DRIVER_EXISTED      0xFD    /* Device Driver already loaded. */
#define MTD_ERR_UNDEFINED                0xFE    /* Specific in Error message or undefined */

/*******************************************************************************
  mtdLoadDriver

  DESCRIPTION:
        Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 and 88E21X0 Driver Initialization Routine. 
        This is the first routine that needs be called by system software. 
        It takes parameters from system software, and returns a pointer (*dev) 
        to a data structure which includes information related to this Marvell Phy
        device. This pointer (*dev) is then used for all the API functions. 
        The following is the job performed by this routine:
            1. store MDIO read/write function into the given MTD_DEV structure
            2. run any device specific initialization routine
            3. Initialize the deviceId
            

  INPUTS:
      readMdio - pointer to host's function to do MDIO read
      writeMdio - pointer to host's function to do MDIO write
      waitFunc - pointer to host's function to implement system specific wait
      anyPort - port address of any port for this device
      forceLoad - MTD_TRUE to ignore the failure to fully recognize the device ID
                  MTD_FALSE to require full recognition of the device   
                  WARNING: Don't use MTD_TRUE  unless checking all APIs that will
                  be called to make sure they will function properly with incomplete device
                  information. MTD_FALSE should be used in the vast majority of situations.

  OUTPUTS:
      dev  - pointer to holds device information to be used for each API call.
      errCode - if function returns MTD_FAIL, more information is contained in errCode
              as follows:
            MTD_ERR_WAIT_NOT_IMPLEMENTED
            MTD_ERR_GET_REV_FAIL
            MTD_ERR_READ_REG_FAIL
            MTD_ERR_UNLOAD_DRIVER_FAIL
            MTD_ERR_LOAD_DRIVER_NULL
            MTD_ERR_INCORRECT_MDIO
            MTD_ERR_LOAD_DRIVER_EXISTED
            MTD_ERR_UNDEFINED

  RETURNS:
        MTD_OK               - on success
        MTD_FAIL             - on error

  COMMENTS:
        mtdUnloadDriver is also provided to do driver cleanup.

        An MTD_DEV is required for each type of PHY device in the system. For
        example, if there are 16 ports of X3240 and 4 ports of X3220, 
        two MTD_DEV are required, and one call to mtdLoadDriver() must
        be made with one of the X3240 ports, and one with one of the X3220
        ports.

        Host may either pass pointers to their MDIO read/write functions here
        for run-time calling of their MDIO read/write, or for a compile-time
        solution, programmers may modify mtdHwXmdioRead() and mtdHwXmdioWrite()
        in mtdHwCntl.c directly to call their functions and pass NULL here.

        Depending on MTD_CLAUSE_22_MDIO compile switch, readMdio() and
        writeMdio() prototypes are different.

        Depending if Serdes package is selected or not in the mtdFeature.h, this
        function will also call mtdInitSerdesDev() to initialize Serdes.

        If the device ID is not fully recognized this function will fail unless
        forceLoad is set to MTD_TRUE. This allows the API to be used on 
        future devices that have not had full support added yet. Suggest to use
        MTD_FALSE unless using a small subset of the API which is known to be
        ok with just the family and a few other device features being recognized.
*******************************************************************************/
MTD_STATUS mtdLoadDriver
(
    IN FMTD_READ_MDIO     readMdio,    
    IN FMTD_WRITE_MDIO    writeMdio,   
    IN FMTD_WAIT_FUNC     waitFunc,
    IN MTD_U16            anyPort,
    IN MTD_BOOL           forceLoad,
    OUT MTD_DEV           *dev,
    OUT MTD_U16           *errCode
);

/*******************************************************************************
* mtdUnloadDriver
*
* DESCRIPTION:
*       This function disables Device interrupt, and clears MTD_DEV structure.
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
);

#if C_LINKAGE
#ifdef __cplusplus
}
#endif
#endif

#endif /* MTDINITIALIZATION_H */




