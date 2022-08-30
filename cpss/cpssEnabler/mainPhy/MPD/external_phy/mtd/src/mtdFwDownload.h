/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions using MDIO access to download firmware
into the internal RAM or attached flash memory of the Marvell 88X32X0, 
88X33X0, 88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#ifndef MTDDL_H
#define MTDDL_H

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

#if DB_TEST_ENVIRONMENT
#define DB_TEST_FLASHCODE   1 /* set to 0 for host environment*/
                              /* (removes many extra print statements) */
                              /* set to 1 to build sample code */
                              /* to interface to Marvell DB via USB2SMI board */
                              /* (extra debug statements during flash/ram download) */
#else
#define DB_TEST_FLASHCODE   0
#endif

/* Parallel Download - RAM & FLASH */
#define MTD_MAX_PORTS_TO_DOWNLOAD 32  /* this is the maximum number of ports that can be downloaded */
                                      /* in parallel ram or parallel flash download functions */
                                      /* which happens to be the same as the maximum number of ports on one MDIO bus */

#define MTD_MAX_SLAVE_SIZE (50*1024UL)

#define MTD_HEADER_SIZE 32

/* Parts prior to X3540 */
#define MTD_MAX_HEADERS 2
#define MTD_MAX_APP_SIZE (216*1024UL) 
#define MTD_MAX_FLASH_IMAGE_SIZE (MTD_MAX_APP_SIZE + (MTD_MAX_HEADERS*MTD_HEADER_SIZE))

/* Parts after X35X0 */
#define MTD_MAX_X35X0_HEADERS 8
#define MTD_MAX_X35X0_APP_SIZE (256*1024UL)
#define MTD_MAX_X35X0_MAX_SERDES_IMG_SZ ((64+4+4)*1024UL)
#define MTD_MAX_X35X0_FLASH_IMAGE_SIZE (MTD_MAX_X35X0_APP_SIZE+ MTD_MAX_X35X0_MAX_SERDES_IMG_SZ + (MTD_MAX_X35X0_HEADERS*MTD_HEADER_SIZE))

#define MTD_CKSUM_OFFSET 16 /* Header checksum location in header */

#define MTD_EXECUTABLE_TO (3000) /* max time in ms that intermediate executables may take in multi-stage boot process */

#define MTD_IMAGE_VERSION_OFFSET 0x100UL /* In an executable section, this is the location of the version */

/* On X3540/E2540, firmware download must be done together on grouped ports(P0-P3) */
#define MTD_NON_GROUPED_RAM_DOWNLOAD         0xFFD5
/* Firmware download is required on both of the grouped port for X3540/E2540 devices, but grouped not found in the list */
#define MTD_GROUPED_PORTS_NOT_FOUND           0xFFD6
/* Max sections number reached without end-of-file indication */
#define MTD_TOO_MANY_SECTIONS               0xFFD7
/* Intermediate executable in a multi-stage boot took too long to finish execution (see MTD_EXECUTABLE_TO) */
#define MTD_EXECECUTABLE_TIMED_OUT          0xFFD8
/* Image passed in and size do not match each other, image is probably corrupted or wrong size passed */
#define MTD_ERR_PROCESSING_IMAGE_FILE       0xFFD9
/* Image header is corrupted */
#define MTD_BAD_HEADER                      0xFFDA
/* Slave command timed out */
#define MTD_SLAVE_CMD_TIMEOUT               0xFFDC
/* Header file download to RAM checksum doesn't match expected checksum in header */
#define MTD_HDR_RAM_CHECKSUM_BAD            0xFFDD
/* At least one port during a parallel RAM or flash download failed */
#define MTD_PAR_DOWNLOAD_FAILED             0xFFDE
/* Image larger than max image size to store to flash or the slave code is too large */
#define MTD_IMAGE_TOO_LARGE_TO_DOWNLOAD     0xFFDF
/* Slave code did not start. - Slave code failed to download properly. */
#define MTD_SLAVE_CODE_DID_NOT_START        0xFFE0
/* Flash verified FAILED! Flash probably corrupted */
#define MTD_VERIFY_ERR                      0xFFE1
/* Unknown error, downloading the flash failed! */
#define MTD_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL  0xFFE2
/* App code did not start. - App code failed to download properly in to the RAM. */
#define MTD_APP_CODE_DID_NOT_START          0xFFE3
/* App code failed. - Number of ports to download is greater than maximum ports */
#define MTD_NUM_PORTS_TOO_LARGE             0xFFE4

/* Error reading or writing MDIO register */
#define MTD_IO_ERROR                        0xFFE5
/*size must be an even number of bytes*/
#define MTD_SIZE_NOT_EVEN                   0xFFE6
/* Slave encountered error while erasing flash */
#define MTD_ERR_ERASING_FLASH               0xFFE7
/* unexpected value read back from download code */
#define MTD_ERR_VALUE_READ_BACK             0xFFE8
/* Did not get MTD_SLAVE_OK for writing the data */
#define MTD_ERR_START_WRITE_DATA            0xFFE9
/* Slave failed to get all the data correctly*/
#define MTD_WRITE_VERIFY_DATA                0xFFEA
/* Some kind of error occurred on Slave */
#define MTD_ERR_ON_SLAVE                    0xFFEB
/* Checksum error */
#define MTD_ERR_CHECKSUM                    0xFFEC
/* Slave didn't write enough words to flash. Some kind of error occurred*/
#define MTD_ERR_SLAVE_WRITE_FULL            0xFFED
/* last transfer failed */
#define MTD_ERR_LAST_TRANSFER               0xFFEE
/* RAM checksum register (present only in SFT910X RevB or later) had wrong checksum */
#define MTD_RAM_HW_CHECKSUM_ERR             0xFFEF
/* PHY wasn't waiting in download mode */
#define MTD_PHY_NOT_IN_DOWNLOAD_MODE        0xFFFF


/******************************************************************************
 MTD_STATUS mtdUpdateFlashImage(
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U8 appData[],
     IN MTD_U32 appSize, 
     IN MTD_U8 slaveData[],
     IN MTD_U32 slaveSize,
     OUT MTD_U16 *errCode);
                                  
 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    appData[] - application code to be downloaded into the flash
    appSize - application code size in bytes (must be even)
    slaveData[] - slave code to be downloaded into the RAM
    slaveSize - size of the slave code in bytes (must be even)

 Outputs:
    errCode - if function returns MTD_FAIL, more information is contained in errCode
    as follows:

         MTD_TOO_MANY_SECTIONS
         MTD_EXECECUTABLE_TIMED_OUT
         MTD_ERR_PROCESSING_IMAGE_FILE
         MTD_BAD_HEADER
         MTD_SLAVE_CMD_TIMEOUT 
         MTD_HDR_RAM_CHECKSUM_BAD
         MTD_IMAGE_TOO_LARGE_TO_DOWNLOAD
         MTD_SLAVE_CODE_DID_NOT_START
         MTD_VERIFY_ERR
         MTD_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL
         MTD_APP_CODE_DID_NOT_START
         MTD_IO_ERROR
         MTD_SIZE_NOT_EVEN              
         MTD_ERR_ERASING_FLASH      
         MTD_ERR_VALUE_READ_BACK   
         MTD_ERR_START_WRITE_DATA  
         MTD_ERR_ON_SLAVE  
         MTD_ERR_CHECKSUM 
         MTD_ERR_SLAVE_WRITE_FULL 
         MTD_ERR_LAST_TRANSFER                    
         MTD_RAM_HW_CHECKSUM_ERR
         MTD_PHY_NOT_IN_DOWNLOAD_MODE

 Returns:
    MTD_OK if successful. MTD_FAIL if not.

 Description:
    The function first checks if the app code size is greater than what it 
    should be and returns an error if that was the case (or if the image
    sizes are not even, an error will also be returned).
    
    Then it calls mtdPutPhyInMdioDownloadMode() to put the PHY in download mode 
    and downloads the slave code in to the RAM and starts executing. 
    
    Once the flash is written with the help of the slave, slave verifies
    the flash program and returns errors if the image is corrupted (the slave
    reads back the image from flash and verifies the checksum matches the expected checksum).

    Any error occurs returns MTD_FAIL and additional information may be in "errCode".

 Side effects:
    None.

 Notes/Warnings:
    This function calls two main functions ...
    1. mtdMdioRamDownload() to download the slave helper code
    2. mtdMdioFlashDownload() to update the flash using the slave helper code

    The function asserts the SPI_CONFIG (by calling mtdPutPhyInMdioDownloadMode()) when it begins the code download, 
    but it does not remove the download mode. In order to execute the newly downloaded 
    code, SPI_CONFIG must be lowered by calling mtdRemovePhyMdioDownloadMode() (or changing the strap config) and the 
    PHY must be reset to use the newly updated flash image. Or the PHY chip hardware reset can
    be issued to both remove the download mode and reload the code from flash (assuming the PHY is strapped
    for flash upload and not MDIO loading).

 Example:
    FILE *hFile, *sFile;                        
    MTD_U8 appData[MTD_MAX_X35X0_FLASH_IMAGE_SIZE];  // Could use MTD_MAX_FLASH_IMAGE_SIZE instead
                                                     // if using PHYs before X35X0
    MTD_U8 slaveCode[MTD_MAX_SLAVE_SIZE];    // slave code image
    MTD_STATUS retStatus;
    MTD_U16 error;

    fileSize = mtdOpenReadFile(devPtr,port, &hFile, "image_name.hdr", appData,
                                    MTD_MAX_X35X0_FLASH_IMAGE_SIZE); 
                                    // Could use MTD_MAX_FLASH_IMAGE_SIZE instead
                                    // if using PHYs before X35X0
    slaveFileSize = mtdOpenReadFile(devPtr,port, &sFile, 
                                    "x3240flashdlslave_0_6_6_0_10673.hdr", slaveCode,
                                    MTD_MAX_SLAVE_SIZE);
    retStatus = mtdUpdateFlashImage(devPtr,port, appData, fileSize, slaveCode, 
                                 slaveFileSize, &error);
    if (retStatus == MTD_FAIL)
        ...check "error" for what happened/type of error that occurred

    mtdChipHardwareReset(devPtr,port); // Removes download mode from all ports and reloads all
                                       // ports with the new code from flash
    
    fclose(hFile);
    fclose(sFile);

******************************************************************************/ 
MTD_STATUS mtdUpdateFlashImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    IN MTD_U8 slaveData[],
    IN MTD_U32 slaveSize,
    OUT MTD_U16 *errCode
);

/******************************************************************************
 MTD_STATUS mtdUpdateRamImage(
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U8 appData[],
     IN MTD_U32 appSize
     OUT MTD_U16 *errCode);
                                  
 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    appData[] - application code to be downloaded into the RAM
    appSize - application code size in bytes (must be even)

 Outputs:
    errCode - if return status is MTD_FAIL, errCode may contain additional
    information about the error. If it does, it will be one of the following:

         MTD_NON_GROUPED_RAM_DOWNLOAD
         MTD_TOO_MANY_SECTIONS
         MTD_EXECECUTABLE_TIMED_OUT
         MTD_ERR_PROCESSING_IMAGE_FILE
         MTD_BAD_HEADER
         MTD_HDR_RAM_CHECKSUM_BAD
         MTD_APP_CODE_DID_NOT_START
         MTD_IO_ERROR
         MTD_SIZE_NOT_EVEN
         MTD_RAM_HW_CHECKSUM_ERR
         MTD_PHY_NOT_IN_DOWNLOAD_MODE

 Returns:
    MTD_OK if successful, MTD_FAIL if an error was detected.

 Description:
    The function first checks if the app code size is greater than what it 
    should be and returns an error if that was the case (or if the image size
    is not even, and error will also be returned).

    It calls mtdPutPhyInMdioDownloadMode() to put the port in download mode by
    raising the SPI_CONFIG bit (required to execute from RAM) and resets the T unit.

    Then it downloads the app code to the RAM and starts it executing. Returns
    MTD_FAIL if any error is encountered, with possibly more information in "errCode".
    
 Side effects:
    None

 Notes/Warnings:
    This function is not applicable for X3540/E2540 devices. Call mtdParallelUpdateRamImage() instead.
    For X3540/E2540 one-port-at-a-time RAM download is not supported.

Example:
    FILE *hFile;
    MTD_U8 appData[MTD_MAX_X35X0_FLASH_IMAGE_SIZE];  // Could use MTD_MAX_FLASH_IMAGE_SIZE instead
                                                     // if using PHYs before X35X0
    MTD_U16 error;
    MTD_STATUS retStatus;
    
    fileSize = mtdOpenReadFile(devPtr,port, &hFile, "image_name.hdr", appData,
                                    MTD_MAX_X35X0_FLASH_IMAGE_SIZE); 
                                    // Could use MTD_MAX_FLASH_IMAGE_SIZE instead
                                    // if using PHYs before X35X0
    retStatus = mtdUpdateRamImage(devPtr,port, appData, fileSize, &error);
    if (retStatus == MTD_FAIL)...check "error" for possible error codes    

    fclose(hFile);
******************************************************************************/
MTD_STATUS mtdUpdateRamImage(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize,
    OUT MTD_U16 *errCode);

/******************************************************************************
 MTD_STATUS mtdParallelUpdateRamImage(
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 ports[],
     IN MTD_U8 appData[],
     IN MTD_U32 appSize, 
     IN MTD_U16 numPorts,
     OUT MTD_U16 erroredPorts[]
     OUT MTD_U16 *errCode);
                                  

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    ports[] - list of port numbers to which the application code is to be downloaded
              On X3540/E2540 devices, both port in the port group(P0-1, P2-3)
              must be passed in.
    appData[] - application code to be downloaded into RAM
    appSize - application code size in bytes (must be even)
    numPorts - number of ports to download (any number from 0-31)

 Outputs:
    errCode - If return status is MTD_FAIL, errCode may contain one of the following:

        MTD_GROUPED_PORTS_NOT_FOUND
        MTD_TOO_MANY_SECTIONS
        MTD_EXECECUTABLE_TIMED_OUT
        MTD_ERR_PROCESSING_IMAGE_FILE
        MTD_BAD_HEADER
        MTD_PAR_DOWNLOAD_FAILED (one or more ports failed)
        MTD_HDR_RAM_CHECKSUM_BAD
        MTD_APP_CODE_DID_NOT_START
        MTD_NUM_PORTS_TOO_LARGE
        MTD_IO_ERROR
        MTD_SIZE_NOT_EVEN
        MTD_PHY_NOT_IN_DOWNLOAD_MODE


    erroredPorts[] - Failure code on ports that had error during download.

        The following are the failure codes stored in the array erroredPorts[]

        MTD_EXECECUTABLE_TIMED_OUT
        MTD_RAM_HW_CHECKSUM_ERR
        MTD_PHY_NOT_IN_DOWNLOAD_MODE


 Returns:
    Download successful on all ports - MTD_OK
    Download not successful on at least one port - MTD_FAIL

    
 Description:
    The function first checks if the number of ports to download is greater
    than maximum ports to download and if true returns an error.
    
    The code then checks if the app code size is greater than what it 
    should be and returns an error if that was the case (or code size is
    not even will also return an error).

    Then it puts all the ports into the download mode by calling
    mtdPutPhyInMdioDownloadMode() which will overwrite the SPI_CFG strap to
    disable flash loading and do a T unit hardware reset. 
    
    Then it downloads the app code to the RAM in parallel and starts it executing. 

    Makes sure code started.

 Side effects:
    None.

 Notes/Warnings:    
    If this function returns an error in errCode, then it should be assumed
    possibly no ports were downloaded successfully.

    If this function returns MTD_FAIL, the chip(s) should be hardware reset to 
    recover, because one or more ports may be left in parallel download mode.

    If MTD_PAR_DOWNLOAD_FAILED is returned, the erroredPorts[] list will
    contain the ports that had an error (non-zero value) and the ports
    that have a 0 value were loaded successfully.

    On X3540/E2540 devices, the new firmware must be downloaded to paired
    ports first/second, or third/fourth (e.g. P0/P1 or P2/P3) because these
    ports share the same serdes.

Example:
    FILE *hFile;
    MTD_U8 appData[MTD_MAX_APP_SIZE+MTD_HEADER_SIZE];        // hdr file image
    MTD_U16 error;
    MTD_STATUS retStatus;
    const MTD_U16 ports[4] = {0,1,2,3}; // Will download MDIO ports 0...3 in parallel
    
    fileSize = mtdOpenReadFile(devPtr,port, &hFile, "image_name.hdr", appData,
                                    (MTD_MAX_APP_SIZE+MTD_HEADER_SIZE));
                                    
    retStatus = mtdParallelUpdateRamImage(devPtr,ports, 
                                          appData, fileSize, 4,
                                          erroredPorts, &error);
    if (retStatus == MTD_FAIL)
    {
        ...check error, and/or all ports in erroredPorts for failure codes
    }
    fclose(hFile);

******************************************************************************/
MTD_STATUS mtdParallelUpdateRamImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 ports[],
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    IN MTD_U16 numPorts,
    OUT MTD_U16 erroredPorts[],
    OUT MTD_U16 *errCode
);

/******************************************************************************       
  MTD_STATUS mtdParallelUpdateFlashImage
  (
      IN MTD_DEV_PTR devPtr,
      IN MTD_U16 ports[],
      IN MTD_U8 appData[],
      IN MTD_U32 appSize, 
      IN MTD_U8 slaveData[],
      IN MTD_U32 slaveSize,
      IN MTD_U16 numPorts, 
      OUT MTD_U16 erroredPorts[],
      OUT MTD_U16 *errCode
  );
                                  

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    ports[] - list of port numbers to which the application code needs to be downloaded
              important: only one port per chip
    appData[] - application code to be downloaded into the flash
    appSize - application code size in bytes (must be even)
    slaveData[] - slave code to be downloaded in to the RAM
    slaveSize - file size of the slave code un bytes (must be even)
    numPorts - number of ports to download (any number from 0-31), should be
               one per chip (as multiple ports per chip share the same flash)

 Outputs:
     errCode - if return is MTD_FAIL, errCode may contain additional information:
         MTD_TOO_MANY_SECTIONS
         MTD_EXECECUTABLE_TIMED_OUT
         MTD_ERR_PROCESSING_IMAGE_FILE
         MTD_BAD_HEADER
         MTD_HDR_RAM_CHECKSUM_BAD
         MTD_PAR_DOWNLOAD_FAILED (one or more ports failed to download successfully,
                                 failed ports listed in erroredPorts with error
                                 code there)
         MTD_IMAGE_TOO_LARGE_TO_DOWNLOAD
         MTD_APP_CODE_DID_NOT_START
         MTD_NUM_PORTS_TOO_LARGE
         MTD_IO_ERROR
         MTD_SIZE_NOT_EVEN
         MTD_PHY_NOT_IN_DOWNLOAD_MODE


    erroredPorts[] - Failure code on ports that had error during download.

        The following are the failure codes stored in the array erroredPorts[]
            
        MTD_EXECECUTABLE_TIMED_OUT
        MTD_SLAVE_CMD_TIMEOUT
        MTD_HDR_RAM_CHECKSUM_BAD
        MTD_PAR_DOWNLOAD_FAILED
        MTD_SLAVE_CODE_DID_NOT_START
        MTD_VERIFY_ERR
        MTD_APP_CODE_DID_NOT_START
        MTD_ERR_ERASING_FLASH
        MTD_ERR_VALUE_READ_BACK
        MTD_ERR_START_WRITE_DATA
        MTD_WRITE_VERIFY_DATA
        MTD_ERR_ON_SLAVE
        MTD_RAM_HW_CHECKSUM_ERR
        MTD_PHY_NOT_IN_DOWNLOAD_MODE


 Returns:
    Download successful on all ports - MTD_OK
    Download not successful on at least one port - MTD_FAIL

 Description:
    This function parallelizes the flash download to speed up downloading a new
    image by using the MDIO parallel write feature in the PHY to allow writing
    multiple PHYs at the same time during the write cycles. It does this by
    enabling the parallel write feature for writes, disabling it for reads.

    The function first checks if the number of ports to download is greater
    than maximum ports to download and if true returns an error.

    The code then checks if the app code size is greater than what it 
    should be (or not even) and returns an error if that was the case.

    Then it puts the PHYs in download mode using mtdPutPhyInMdioDownloadMode() and 
    downloads the slave code into the RAM and starts it executing. 

    The slave code helps program the app code into the flash and verifies 
    the image after downloading by reading back and matching against the checksum.

 Side effects:
    None.

 Notes/Warnings:
    The function puts the PHY into download mode but it does
    not remove the download mode after the download. In order to execute the downloaded 
    code, do a chip reset, mtdChipHardwareReset(), which will remove download
    mode (if it was enabled with the overwrite function, if not, then the strap
    pin must be changed before calling the chip reset) and have all ports load
    the new image from flash.

    On chips with multiple ports per chip, only download on one port. All ports
    on a chip share the same flash part. For example X3220, has 2 ports, only
    download the first port. On X3240, it has 4 ports, so only download the first
    port (if multiple X3240, download every fourth port, e.g. 0,4,8, etc.).

    If this function returns MTD_FAIL, the chip(s) should be hardware reset
    because one or more ports may be left in parallel download mode.

 Example:
    FILE *hFile, *sFile;                        
    MTD_U8 appData[MTD_MAX_APP_SIZE+MTD_HEADER_SIZE];        // hdr file image
    MTD_U8 slaveCode[MTD_MAX_SLAVE_SIZE];    // slave code image
    MTD_U16 error;
    MTD_STATUS retStatus;
    const MTD_U16 ports = {0,4,8}; // Will download 3 flashes in parallel, in this
                                   // case these are 3 x 4-port chips
    
    fileSize = mtdOpenReadFile(devPtr,port, &hFile, 
                               "image_name.hdr", appData, 
                               (MTD_MAX_APP_SIZE + MTD_HEADER_SIZE)); 
    slaveFileSize = mtdOpenReadFile(devPtr,port, &sFile, 
                                    "flashdownloadslave.bin", slaveCode, 
                                    MTD_MAX_SLAVE_SIZE);
    retStatus = mtdParallelUpdateFlashImage(devPtr,ports, appData, 
                                            fileSize, slaveCode, 
                                            slaveFileSize, 3,
                                            erroredPorts,&error);
    if (retStatus == MTD_FAIL)
    {
        ...check error, and/or all ports in erroredPorts for failure codes
    }                                
    
    mtdChipHardwareReset(devPtr,0); // Removes download mode from all ports and reloads all
                                    // ports with the new code from flash
    mtdChipHardwareReset(devPtr,4); // Removes download mode from all ports and reloads all
                                    // ports with the new code from flash
    mtdChipHardwareReset(devPtr,8); // Removes download mode from all ports and reloads all
                                    // ports with the new code from flash    fclose(hFile);
    fclose(sFile);

******************************************************************************/ 
MTD_STATUS mtdParallelUpdateFlashImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 ports[],
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    IN MTD_U8 slaveData[],
    IN MTD_U32 slaveSize,
    IN MTD_U16 numPorts, 
    OUT MTD_U16 erroredPorts[],
    OUT MTD_U16 *errCode
);


/******************************************************************************
MTD_STATUS mtdGetMinMaxFlashLoadTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize,
    OUT MTD_U16 minMs,
    OUT MTD_U16 maxMs
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   appData[] - HDR/IHDR image which was loaded into flash
   appSize - image size in bytes (must be even)

Outputs:
   minMs - minimum time to wait for the flash on a device to load
   maxMs - worst case time to wait for the flash to load on a device

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

   Will return MTD_FAIL on an unsupported PHY (but will attempt to
   return correct version). See below for a list of unsupported PHYs.

Description:
    Looks at the flash image header and calculates the minimum and maximum
    load times based on the flash interface speed selected in the flash
    header. Assumes all of the headers have the same SPI speed (only examines
    the first header).

    Calculates the minimum load time based on one image load per port. Maximum
    is 3 times this (worst case number of retries per port). Minimum delay
    is calculated using the actual image size, SPI speed selected and one try
    per port

    This utility can be used to calculate a minimum delay needed for the flash
    to load across devices, and a timeout on a worst case delay.   

Side effects:
    None.

Notes/Warnings:
    None.

******************************************************************************/
MTD_STATUS mtdGetMinMaxFlashLoadTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize,
    OUT MTD_U16 minMs,
    OUT MTD_U16 maxMs
);

/******************************************************************************
MTD_STATUS mtdGetTunitFwVersionFromImage
(
   IN MTD_U8 appData[],
   IN MTD_U32 appSize,
   OUT MTD_U8 *major, 
   OUT MTD_U8 *minor, 
   OUT MTD_U8 *inc, 
   OUT MTD_U8 *test

);


Inputs:
    appData[] - an HDR or IHDR image for the PHY
    appSize - image size in bytes (must be even)

Outputs:
    major - major version, X.Y.Z.W, the X
    minor - minor version, X.Y.Z.W, the Y
    inc   - incremental version, X.Y.Z.W, the Z
    test  - test version, X.Y.Z.W, the W, should be 0 for released code, 
            non-zero indicates this is a non-released code


Returns:
    MTD_OK if query was successful, MTD_FAIL if not.

Description:
    Locates and returns the T unit firmware version from an HDR or IHDR
    image.

Side effects:
    None.

Notes/Warnings:
    This function may be called before the mtdLoadDriver() function, so 
    no device structure is required. 

    There's minimal error checking on the image itself. Assumes there is
    at least one section passed in. The header checksums are verified
    as valid at least.

******************************************************************************/
MTD_STATUS mtdGetTunitFwVersionFromImage
(
   IN MTD_U8 appData[],
   IN MTD_U32 appSize,
   OUT MTD_U8 *major, 
   OUT MTD_U8 *minor, 
   OUT MTD_U8 *inc, 
   OUT MTD_U8 *test
);


#if C_LINKAGE
#if defined __cplusplus 
}
#endif 
#endif

#endif /* defined MTDDL_H */
