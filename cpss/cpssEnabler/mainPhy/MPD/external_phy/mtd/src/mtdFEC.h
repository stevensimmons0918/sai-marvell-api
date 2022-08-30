/*******************************************************************************
Copyright (C) 2014 - 2021, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/************************************************************************
This file contains function prototypes to configure RS-FEC
and read RS-FEC status for the Marvell X35X0/E25X0 PHY
************************************************************************/
#ifndef MTD_RSFEC_H
#define MTD_RSFEC_H

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif


/******************************************************************************
MTD_STATUS mtdRsFECEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL readToClear,
    IN MTD_U16 enable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    readToClear - MTD_TRUE to have the counters cleared when they
                  are read, or MTD_FALSE the counters will not
                  clear when you read them (they are only 
                  cleared by mtdRsFECReset() that reset the RS-FEC block)
    enable - MTD_ENABLE to enable RS-FEC
             MTD_DISABLE to disable RS-FEC

 Outputs:
    None.

 Returns:
    MTD_OK or MTD_FAIL, if action was successful or not

 Description:
    Enables or disable the RS-FEC.

 Side effects:
    None

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

******************************************************************************/
MTD_STATUS mtdRsFECEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL readToClear,
    IN MTD_U16 enable
);

/*******************************************************************************
MTD_STATUS mtdSetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U16 host_or_line,
    IN MTD_U16 laneOffset,
    IN MTD_U16 bypassIndicationEnable,
    IN MTD_U16 bypassCorrectionEnable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    bypassIndicationEnable -
                 MTD_ENABLE:  FEC decoder does not indicate errors to the PCS;
                 MTD_DISABLE: FEC decoder indicates errors to the PCS layer
    bypassCorrectionEnable -
                 MTD_ENABLE:  FEC decoder performs detection without correction;
                 MTD_DISABLE: FEC decoder performs detection and correction

 Outputs:
    None

 Returns:
    MTD_OK if RS-FEC configuration is successful, MTD_FAIL if not

 Description:
    Used to modify the FEC Bypass Indication Enable and/or FEC Bypass Correction
    Enable in the RS-FEC Control Register.

 Side effects:
    None.

 Notes/Warnings:
    If both are set to MTD_ENABLE, an error will be returned.

    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdSetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 bypassIndicationEnable,
    IN MTD_U16 bypassCorrectionEnable
);

/*******************************************************************************
MTD_STATUS mtdGetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *bypassIndicationEnable,
    OUT MTD_U16 *bypassCorrectionEnable
);


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    bypassIndicationEnable -
                 MTD_ENABLE:  FEC decoder does not indicate errors to the PCS;
                 MTD_DISABLE: FEC decoder indicates errors to the PCS

    bypassCorrectionEnable -
                 MTD_ENABLE:  FEC decoder performs detection without correction;
                 MTD_DISABLE: FEC decoder performs detection and correction

 Returns:
    MTD_OK if RS-FEC configuration is successful, MTD_FAIL if not

 Description:
    Used to read the FEC Bypass Indication Enable and/or FEC Bypass Correction
    Enable in the RS-FEC Control Register in the RS-FEC Control register.

 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdGetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *bypassIndicationEnable,
    OUT MTD_U16 *bypassCorrectionEnable
);


/*******************************************************************************
MTD_STATUS mtdGetRsFecStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *pcsLaneAlignment,
    OUT MTD_U16 *fecLaneAlignment,
    OUT MTD_U16 *latchedRsFecHighErr,
    OUT MTD_U16 *currRsFecHighErr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    pcsLaneAlignment - PCS lane alignment status of FEC encoder
                       1 = aligned
                       0 = not-aligned

    fecLaneAlignment - FEC receiver lane alignment status (locked and aligned)
                       1 = aligned
                       0 = not-aligned
    latchedRsFecHighErr - first read of the LH bit for the RS-FEC Error Indication
    currRsFecHighErr - second read of the LH bit for the RS-FEC Error Indication (current)

 Returns:
    MTD_OK if RS-FEC status is successful, MTD_FAIL if not

 Description:
    Reads and returns the status above from the correct RS-FEC status
    registers based on the mode/speed.

 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

 *******************************************************************************/
MTD_STATUS mtdGetRsFecStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *pcsLaneAlignment,
    OUT MTD_U16 *fecLaneAlignment,
    OUT MTD_U16 *latchedRsFecHighErr,
    OUT MTD_U16 *currRsFecHighErr
);

/*******************************************************************************
MTD_STATUS mtdRsFECReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);


 Inputs:
    pDev - pointer to MTD_DEV initialized by mtdInitDriver() call
    mdioPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK if successful, MTD_FAIL if there was an error

 Description:
    Reset the RS-FEC block, including RS-FEC counters.

 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdRsFECReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

/*******************************************************************************
MTD_STATUS mtdGetRsFecCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    codeWordCounter - RS-FEC corrected codewords counter

 Returns:
    MTD_OK if read RS-FEC corrected codewords counter is successful, MTD_FAIL if not

 Description:
    Reads the RS-FEC corrected codewords counter

 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdGetRsFecCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
);

/*******************************************************************************
MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    codeWordCounter - RS-FEC uncorrected codewords counter

 Returns:
    MTD_OK if read RS-FEC uncorrected codewords counter is successful, MTD_FAIL if not

 Description:
    Reads the RS-FEC uncorrected codewords counter


 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
);

/*******************************************************************************
MTD_STATUS mtdGetRsFecSymbolErrorCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *errorCounter
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    errorCounter - RS-FEC Symbol error counter

 Returns:
    MTD_OK if read symbol error counter is successful, MTD_FAIL if not

 Description:
    Read RS-FEC symbol error counter per FEC lane

 Side effects:
    None.

 Notes/Warnings:
    This is for the 20GBASE-R modes in the X35X0/E25X0 devices.

*******************************************************************************/
MTD_STATUS mtdGetRsFecSymbolErrorCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *errorCounter
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* defined MTD_RSFEC_H */
