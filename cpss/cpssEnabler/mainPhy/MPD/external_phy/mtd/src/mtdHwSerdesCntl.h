/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions to define hardware control operations to
the selected Serdes code.

Including Serdes Read/Write and Set/Get Pin Config, etc.
********************************************************************/
#ifndef MTD_HWSERDES_CNTL_H
#define MTD_HWSERDES_CNTL_H

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

#if MTD_PKG_CE_SERDES

/* COMPHY Serdes Indirect Access Commands */
#define MTD_COMPHY_READ_CMD          0x0000
#define MTD_COMPHY_WRITE_CMD         0x0001
#define MTD_COMPHY_WRITE_INC_CMD     0xC001     /* post increment on writes only */
#define MTD_COMPHY_READ_INC_CMD      0x8000     /* post increment on reads only */

#define MTD_COMPHY28X2_MajorRev      2
#define MTD_COMPHY28X2_MinorRev      0

/******************************************************************************
 API Functions shared among different devices and Serdes
*******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdInitSerdesDev
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 anyPort
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    anyPort - port address of any port for this device
 Outputs:
    None.
 Returns:
    MTD_OK if successful
    MTD_FAIL if not
 Description:
    This function load and initialize serdes device structure in the given devPtr
 Side effects:
    None.
 Notes/Warnings:
    This function has been already called in the mtdLoadDriver().
    This is a non-reentrant function.
*******************************************************************************/
MTD_STATUS mtdInitSerdesDev
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 anyPort
);

/******************************************************************************
 MTD_STATUS mtdUnloadSerdesDev
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 anyPort
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    anyPort - port address of any port for this device
 Outputs:
    None.
 Returns:
    MTD_OK if successful
    MTD_FAIL if not
 Description:
    This function clear the serdes device structure in the given devPtr.
    Set all function pointer to NULL.
 Side effects:
    None.
 Notes/Warnings:
    This function should only be called after successful execution of
    mtdInitSerdesDev().
    This is a non-reentrant function.
*******************************************************************************/
MTD_STATUS mtdUnloadSerdesDev
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 anyPort
);

/******************************************************************************
 MTD_STATUS mtdSetSerdesPortGetDevPtr
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 mdioPort,
     OUT MCESD_DEV_PTR *pSerdesDev
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - the MDIO port address that host want to perform serdes operations
               on, 0-31

 Outputs:
    pSerdesDev - a pointer to serdes device control structure pointer.
                 i.e. a double pointer to the serdes device control structure
                 that used on current device.

 Returns:
        MTD_OK if successful
        MTD_FAIL if not

Description:
    Serdes control port and device MDIO port has a certain mappings. This function
    set the mapped serdes control port in the device structure based on the input
    MDIO port. Also it returns a pointer(MCESD_DEV_PTR *) to the Serdes device control
    structure pointer based on current device type.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdSetSerdesPortGetDevPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    OUT MCESD_DEV_PTR *pSerdesDev
);

/******************************************************************************
 MTD_STATUS mtdGetSerdesDevPtr
 (
     IN MTD_DEV_PTR devPtr,
     OUT MCESD_DEV_PTR *pSerdesDev
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call

 Outputs:
    pSerdesDev - a pointer to serdes device control structure pointer.
                 i.e. a double pointer to the serdes device control structure
                 that used on current device.

 Returns:
        MTD_OK if successful
        MTD_FAIL if not

Description:
    Get a pointer(MCESD_DEV_PTR *) to the Serdes device control structure pointer
    based on current device type.

 Side effects:
    None

 Notes/Warnings:
    Recommend to call mtdSetSerdesPortGetDevPtr() directly with serdes port setting
*******************************************************************************/
MTD_STATUS mtdGetSerdesDevPtr
(
    IN MTD_DEV_PTR devPtr,
    OUT MCESD_DEV_PTR *pSerdesDev
);

/******************************************************************************
 MTD_STATUS mtdSetSerdesPort
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 mdioPort
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - the MDIO port address that host want to perform serdes operations
               on, 0-31

 Outputs:
    None.

 Returns:
    MTD_OK if successful
    MTD_FAIL if not

 Description:
    Serdes control port and device MDIO port has a certain mappings. This function
    set the mapped serdes control port in the device structure based on the input
    MDIO port.

  Side effects:
    None

  Notes/Warnings:
    This function MUST be called before performing any port related Serdes operations.
*******************************************************************************/
MTD_STATUS mtdSetSerdesPort
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
);

/******************************************************************************
 MTD_STATUS mtdHwSerdesRead
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 mdioPort,
     IN MTD_U32 reg,
     OUT MTD_U32 *value
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    reg - Serdes register address

 Outputs:
    value - 32 bits read data from the MDIO

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads a 32 bits data from the Serdes register.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_STATUS mtdHwSerdesRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U32 reg,
    OUT MTD_U32 *value
);

/******************************************************************************
 MTD_STATUS mtdHwSerdesWrite
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 mdioPort,
     IN MTD_U32 reg,
     IN MTD_U32 value
 )

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    reg - Serdes register address
    value - 32 bits data to write

 Outputs:
    None.

 Returns:
    MTD_OK always

 Description:
    This function write a 32 bits data to the Serdes register.

 Side effects:
    None.

 Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_STATUS mtdHwSerdesWrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U32 reg,
    IN MTD_U32 value
);

/******************************************************************************
 API Functions for specific devices and Serdes
*******************************************************************************/

#if MTD_CE_SERDES28X2
/******************************************************************************
MTD_STATUS mtdSerdes28GX2GetFirmwareVersion
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort, 
    OUT MTD_U8 *major, 
    OUT MTD_U8 *minor, 
    OUT MTD_U8 *patch, 
    OUT MTD_U8 *build
);


  Inputs:  
        devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
        mdioPort - MDIO port address, 0-31

  Outputs:
        major - first 8-bits of serdes firmware version
        minor - second 8-bits of serdes firmware version
        patch - third 8-bits of serdes firmware version
        build - last 8-bits of serdes firmware version

  Returns:
        MTD_OK when success; otherwise MTD_FAIL

  Description:
        This function gets the version of the serdes firmware and returns it. 
        The version is in the format W.X.Y.Z, where each letter is a number
        in the range 0...255.

  Side effects:
        None

  Notes/Warnings:
        None
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2GetFirmwareVersion
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort, 
    OUT MTD_U8 *major, 
    OUT MTD_U8 *minor, 
    OUT MTD_U8 *patch, 
    OUT MTD_U8 *build
);

/* Definitions and data structure used for the eye measurements */
#define MTD_EYE_MAX_VOLT_STEPS 63
#define MTD_EYE_MAX_PHASE_LEVEL 127

typedef struct _MTD_SERDES_28GX2_EYE_RAW
{
    MTD_32 eyeRawData[(MTD_EYE_MAX_PHASE_LEVEL * 2) + 1][(MTD_EYE_MAX_VOLT_STEPS * 2) + 1];
} MTD_SERDES_28GX2_EYE_RAW, *PMTD_SERDES_28GX2_EYE_RAW;

/******************************************************************************
MTD_STATUS mtdSerdes28GX2EyeRawData
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U8 serdesLane,
    OUT PMTD_SERDES_28GX2_EYE_RAW pEyeRawData
);

  Inputs:  
        devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
        mdioPort - MDIO port address, 0-31
        serdesLane - Serdes lane; 1 or 2

  Outputs:
        pEyeRawData - 2 dimensional array defined in MTD_SERDES_28GX2_EYE_RAW structure 

  Returns:
        MTD_OK when success; otherwise MTD_FAIL

  Description:
        The function retrieves the eye measurement raw data and put it into a 2 dimensional 
        array defined in MTD_SERDES_28GX2_EYE_RAW structure. The 2 dimensional array stores 
        the phase (horizontal) and the amplitude(vertical) data of a single SerDes lane 
        provided as the input parameter.   

        The mtdSerdes28GX2EyePlotStats() API will process the raw eye 2 dimensional array 
        and plot the eye diagram with the debug logging enabled.

  Side effects:
        None

  Notes/Warnings:
        The link must be up before calling the mtdSerdes28GX2EyeRawData() API. Otherwise,
        the return raw data will not be meaningful.

        Depending on the link speed, this eye API may take up to a minute to complete. 
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2EyeRawData
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U8 serdesLane,
    OUT PMTD_SERDES_28GX2_EYE_RAW pEyeRawData
);

/******************************************************************************
MTD_STATUS mtdSerdes28GX2EyePlotStats
(
    IN MTD_DEV_PTR pDev,
    IN PMTD_SERDES_28GX2_EYE_RAW pEyeRawData,
    OUT MTD_U16 *eyeWidth,
    OUT MTD_U16 *eyeHeight,
    OUT MTD_U16 *eyeArea
);

  Inputs:  
        devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
        pEyeRawData - 2 dimensional array defined in MTD_SERDES_28GX2_EYE_RAW structure 
                      with valid raw eye data

  Outputs:
        eyeWidth - width of the eye opening 
        eyeHeight - height of the eye opening
        eyeArea - number of open samples

  Returns:
        MTD_OK when success; otherwise MTD_FAIL

  Description:
        The function will process the raw data of an eye 2 dimensional array 
        and plot the eye diagram with the debug logging enabled. It will compute 
        the width at the 0 voltage amplitude and the height at the center of the 
        phase.

  Side effects:
        None

  Notes/Warnings:
        Need to provide valid raw eye 2 dimensional array as defined in the 
        MTD_SERDES_28GX2_EYE_RAW structure 
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2EyePlotStats
(
    IN MTD_DEV_PTR pDev,
    IN PMTD_SERDES_28GX2_EYE_RAW pEyeRawData,
    OUT MTD_U16 *eyeWidth,
    OUT MTD_U16 *eyeHeight,
    OUT MTD_U16 *eyeArea
);

#endif  /* MTD_CE_SERDES28X2 */

#endif  /* MTD_PKG_CE_SERDES */

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* defined MTD_HWSERDES_CNTL_H */

