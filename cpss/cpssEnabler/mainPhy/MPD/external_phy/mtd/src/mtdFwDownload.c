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
#include <mtdApiRegs.h>
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdFwDownload.h>
#include <mtdAPIInternal.h>
#include <mtdUtils.h>
#include <mpdTypes.h>

#if DB_TEST_FLASHCODE
void amuse_user(void);
#endif

MTD_STATUS mtdUpdateFlashImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize, 
    IN MTD_U8 slaveData[],
    IN MTD_U32 slaveSize,
    OUT MTD_U16 *errCode
)
{    
    MTD_BOOL appStarted;
    MTD_U16 slaveReply;
#if DB_TEST_FLASHCODE
    MTD_U16 flashType;
    MTD_U16 numSections;
#endif

    *errCode = 0;
    
    /******************************************************************************* 
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/
    if (appSize > mtdGetDevMaxImageSize(devPtr) || slaveSize > MTD_MAX_SLAVE_SIZE)
    {
        /* Image must not be larger than the maximum image that can be stored in flash */
        /* Slave image must be small enough  */
        MTD_DBG_ERROR("Image is larger than max flash image or slave code is too large!\n");
        *errCode = MTD_IMAGE_TOO_LARGE_TO_DOWNLOAD; 
        return MTD_FAIL;
    }
    
    /*******************************************************************************
             Download slave code to phy's RAM and start it,
             this code assists in programming the flash connected by SPI
    *******************************************************************************/    
#if DB_TEST_FLASHCODE
    if (!(slaveData == (MTD_U8 *)0 && slaveSize == 0))
    {
        /* In test code allow skipping slave download by passing NULL/0 this is a debug feature */
        MTD_ATTEMPT(mtdMdioRamDownload(devPtr,slaveData,slaveSize,port,errCode));  
    }
#else
    MTD_ATTEMPT(mtdMdioRamDownload(devPtr,slaveData,slaveSize,port,errCode));  
#endif

    /* make sure the slave code started/is running */
    TRY_IO(mtdDidPhyAppCodeStart(devPtr,port,&appStarted));
   
    if (appStarted == MTD_FALSE)
    {
        MTD_DBG_ERROR("Slave code did not start.\n");
        MTD_DBG_ERROR("Slave download failed. Exiting...\n");
        *errCode = MTD_SLAVE_CODE_DID_NOT_START;        
        return MTD_FAIL;
    }

    /*******************************************************************************
       Write the image to flash with slave's help
    *******************************************************************************/
    MTD_ATTEMPT(mtdMdioFlashDownload(devPtr,port, appData, appSize, errCode));
    
    /*******************************************************************************
       Let slave verify image
    *******************************************************************************/

    /* Using slave code to verify image.
       This commands slave to read in entire flash image and calculate checksum and make sure
       checksum matches the checksum in the header. A failure means flash was corrupted.
      
       Another method would be to reset the phy (with it strapped to execute from flash) 
       and see that the new code starts successfully, since a bad checksum will result in 
       the code not being started */

#if DB_TEST_FLASHCODE
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, FLASH_MFG_TYPE, &flashType));
    MTD_DBG_INFO("Flash type is 0x%04X\n",(int)flashType);
    MTD_DBG_INFO("Flash programming complete. Verifying image via slave.\n");    
#endif

    if (mtdTrySlaveCommand(devPtr, port, VERIFY_FLASH, &slaveReply, 15000) == MTD_FAIL)
    {
        *errCode = MTD_SLAVE_CMD_TIMEOUT;
        return MTD_FAIL;
    }

    if (slaveReply == MTD_SLAVE_OK)
    {
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\nFlash image verified. Call mtdRemovePhyDownloadMode() or change strap and reboot to execute new code\n");
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, NUM_SECTIONS, &numSections));
        MTD_DBG_INFO("Flash image had %d sections\n",(int)(numSections+1));
#endif
        return MTD_OK;   /*Flash download complete */
    }
    else
    {
        if (slaveReply == MTD_SLAVE_VERIFY_ERR)
        {
            MTD_DBG_ERROR("\nFlash verified FAILED! Flash probably corrupted. Re-try download.\n");
#if DB_TEST_FLASHCODE
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, NUM_SECTIONS, &numSections));
            MTD_DBG_ERROR("Section number %d had a verification error.\n",(int)(numSections));
#endif
            *errCode = MTD_VERIFY_ERR;                
            return MTD_FAIL;
        }
        else
        {
            MTD_DBG_ERROR("\nExpected 0x%04X from slave got 0x%04X. Exiting...\n", MTD_SLAVE_OK, (int)slaveReply);
            *errCode = MTD_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL;
            return MTD_FAIL;
        }
    }

    return MTD_OK; /* Should never come here */
}

MTD_STATUS mtdUpdateRamImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize,
    OUT MTD_U16 *errCode
)
{

    *errCode = 0;

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("X35X0/E2540 devices must call mtdParallelUpdateRamImage() to download firmware on port pairs.\n");
        *errCode = MTD_NON_GROUPED_RAM_DOWNLOAD;
        return MTD_FAIL;
    }

    /* Puts PHY in download mode and then processes the header file. */
    MTD_ATTEMPT(mtdMdioRamDownload(devPtr,appData,appSize,port,errCode));
    
    return MTD_OK;  /* Ram download is done*/    
}


MTD_STATUS mtdParallelUpdateRamImage
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
    MTD_U16 i;

    *errCode = 0;

    if(numPorts > MTD_MAX_PORTS_TO_DOWNLOAD)
    {
        *errCode = MTD_NUM_PORTS_TOO_LARGE;
        return MTD_FAIL;
    }

    for(i = 0; i < numPorts; i++) /* Initialize all port status to no error */   
    {
        erroredPorts[i] = 0;
    }

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_U16 firstPort;
        MTD_BOOL groupFound = MTD_FALSE;
        MTD_BOOL pairFound = MTD_FALSE;
        MTD_BOOL visitedPorts[MTD_MAX_PORTS_TO_DOWNLOAD];

        if (numPorts % 2)
        {
            MTD_DBG_ERROR("X35X0/E2540 device must download firmware on paired ports(P0/P1 or P2/P3, etc.). numPorts must be groups of 2!\n");
            *errCode = MTD_GROUPED_PORTS_NOT_FOUND;
            return MTD_FAIL;        
        }

        for (i = 0; i < MTD_MAX_PORTS_TO_DOWNLOAD; i++)
        {
            visitedPorts[i] = MTD_FALSE;
        }

        for (i = 0; i < numPorts; i++)
        {
            visitedPorts[ports[i]] = MTD_TRUE;
        }

        for (firstPort = 0; firstPort < MTD_MAX_PORTS_TO_DOWNLOAD; firstPort+=2)
        {
            groupFound = (visitedPorts[firstPort] || visitedPorts[firstPort+1]) ? MTD_TRUE : MTD_FALSE;

            if (groupFound == MTD_TRUE)
            {
                pairFound = (visitedPorts[firstPort] && visitedPorts[firstPort+1]) ? MTD_TRUE : MTD_FALSE;
                if (pairFound == MTD_FALSE)
                {
                    MTD_DBG_ERROR("X3540/E2540 device must download firmware on paired ports(P0/P1 or P2/P3). Expected port group is not found in the ports[]\n");
                    *errCode = MTD_GROUPED_PORTS_NOT_FOUND;
                    return MTD_FAIL;
                }
            }
        }
    }

    /* Puts phy in download mode and handles processing the header file
    then starts the app code running, puts PHY in parallel write mode as needed, or when
    reading an MDIO register, removes parallel write mode as needed */  
    MTD_ATTEMPT(mtdParallelMdioRamDownload(devPtr,appData,appSize, ports, \
                                       numPorts,erroredPorts,errCode));    
    
    for(i = 0; i < numPorts; i++)
    {
        if(erroredPorts[i] != 0)
        {
            *errCode = MTD_PAR_DOWNLOAD_FAILED;
            return MTD_FAIL;  /* Parallel Ram download is done, at least one port has error*/
        }
    }

    return MTD_OK;  /* Parallel Ram download is done, successful on all ports*/    
}

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
)
{
    MTD_U16 i;
    MTD_BOOL appStarted;
     *errCode = 0;

    if(numPorts > MTD_MAX_PORTS_TO_DOWNLOAD)
    {
        *errCode = MTD_NUM_PORTS_TOO_LARGE;
        return MTD_FAIL;
    }
    for(i = 0; i < numPorts; i++) /* initialize to no errors on all ports */
    {
        erroredPorts[i] = 0;
    }



    /******************************************************************************* 
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/
    if (appSize > mtdGetDevMaxImageSize(devPtr) || slaveSize > MTD_MAX_SLAVE_SIZE)
    {
        /* App size cannot be larger than the device memory size. Code download cannot proceed */
        MTD_DBG_ERROR("Image is larger than max flash image or slave code is too large!\n");
        *errCode = MTD_IMAGE_TOO_LARGE_TO_DOWNLOAD; 
        return MTD_FAIL;
    }

    /*******************************************************************************
             Download slave code to phy's RAM and start it
    *******************************************************************************/
    /* Puts phy in download mode, processes the header file
      then starts the slave code running. If returns MTD_OK, there may be some ports
      that failed to download correctly, they will be stored in erroredPorts and should be skipped. 
      Major error will return MTD_FAIL and errCode will be non-zero and will not move forward. */
    MTD_ATTEMPT(mtdParallelMdioRamDownload(devPtr,slaveData,slaveSize, ports, \
                                       numPorts, erroredPorts, errCode));   
        
    /* Parallel download bit should be in the OFF position here (anywhere a "read" takes place) */
    /* Reads must be done in series and results collected */
    /* make sure the slave app code started */
    for(i = 0; i < numPorts; i++)
    {
        if(erroredPorts[i] == 0)
        {
            TRY_IO(mtdDidPhyAppCodeStart(devPtr,ports[i],&appStarted));
            
            if (appStarted == MTD_FALSE)
            {
                MTD_DBG_ERROR("Slave code did not start on MDIO port %d.\n",(int)ports[i]);
                MTD_DBG_ERROR("Port will be skipped.\n");
                erroredPorts[i] = MTD_SLAVE_CODE_DID_NOT_START;            
            }
            else
            {
#if DB_TEST_FLASHCODE
                MTD_DBG_INFO("Slave code started on MDIO port %d.\n", ports[i]);
#endif
            }
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Slave code wasn't downloaded to MDIO port %d\n", ports[i]);
#endif
        }        
    }

    /*******************************************************************************
       Write the image to flash with slave's help which is running now. Skip
       any ports that have had an error. Try the ports that haven't had an 
       error.
    *******************************************************************************/

    MTD_ATTEMPT(mtdParallelMdioFlashDownload(devPtr, appData, appSize, ports, numPorts, erroredPorts, errCode));    

    /*******************************************************************************
       Let slave verify image
    *******************************************************************************/

    /* Using slave code to verify image.
       This commands slave to read in entire flash image and calculate checksum and make sure
       checksum matches the checksum in the header. A failure means flash was corrupted.
      
       Another method would be to reset the phy (with SPI_CONFIG = 0) and see that the new code
       starts successfully, since a bad checksum will result in the code not being started */
#if DB_TEST_FLASHCODE
    MTD_DBG_INFO("Flash programming complete. Verifying image via slave.\n");
#endif

    TRY_IO(mtdTryParallelSlaveCommand(devPtr, VERIFY_FLASH, ports, numPorts, erroredPorts, MTD_FLASH_ERASE_WRITE));
   
    for(i = 0; i < numPorts; i++)
    {
        if(erroredPorts[i] != 0)
        {
            *errCode = MTD_PAR_DOWNLOAD_FAILED;            
            return MTD_FAIL;  /* at least one port has error*/
        }           
    }

    return MTD_OK;
}

MTD_STATUS mtdGetMinMaxFlashLoadTime
(
   IN MTD_DEV_PTR devPtr,
   IN MTD_U8 appData[],
   IN MTD_U32 appSize,
   OUT MTD_U16 minMs,
   OUT MTD_U16 maxMs
)
{
    const MTD_U16 deviceBase = MTD_DEVID_GETBASE(devPtr->deviceId);
    const float flashSpeeds[4] = {1220700.0, 2441400.0, 4882812.0, 9765625.0}; /* Flash selection interface speeds in Mhz */    
    MTD_U16 flashSpeedSelection;
    float timeInSecs, timeInMs;
    MEM_SIZE_BYTES maxImageSize;

    minMs = maxMs = 0;

    if (appSize < MTD_HEADER_SIZE)
    {
        return MTD_FAIL;    
    }   

    flashSpeedSelection = ((appData[11] & 0x18)>> 4); 

    maxImageSize = mtdGetDevMaxImageSize(devPtr);

    if (!maxImageSize)
    {
        return MTD_FAIL;
    }

    switch (deviceBase)
    {
        case MTD_X32X0_BASE: /* Only has 2 speeds, 4.8 and 9.6 Mhz */
            flashSpeedSelection += 2;
            if (flashSpeedSelection > 3)
            {
                return MTD_FAIL;
            }
            MPD_FALLTHROUGH
        case MTD_X33X0_BASE: /* Has all 4 speeds available */           
        case MTD_E21X0_BASE:            
        case MTD_X35X0_BASE:
            timeInSecs = 1.0f/flashSpeeds[flashSpeedSelection] * 8.0f * ((float)maxImageSize);
            timeInMs = timeInSecs* 1000.0f;
            minMs = (MTD_U16)timeInMs;
            maxMs = 3 * minMs;
            break;
            
        default:
            return MTD_FAIL;
            break;
    }

    return MTD_OK;
}


MTD_STATUS mtdGetTunitFwVersionFromImage
(
   IN MTD_U8 appData[],
   IN MTD_U32 appSize,
   OUT MTD_U8 *major, 
   OUT MTD_U8 *minor, 
   OUT MTD_U8 *inc, 
   OUT MTD_U8 *test
)
{
    MTD_FILE_HEADER_TYPE headersArry[MTD_MAX_X35X0_HEADERS];
    MTD_U16 sectionIndex;
    MTD_BOOL done;
    MTD_U32 imageIndex, tUnitExecutableOffset;

    /* Assumes there are three formats for the HDR file or IHDR file:
       1) Older PHYs before X3540 had a single HDR section or if 2
          HDR sections, the second was the ICMD section which was not
          indicated as data-only in the header. There were no
          images with more than 2 sections for those PHYs.
       2) X3540 and later data-only sections were always indicated
          regardless how many sections. The last image in the HDR/IHDR
          file was always the T unit firmware. It might be the serdes
          loader if it was serdes-only code, or T unit if T-unit-only
          code, or if combined, it's the last executable section. IHDR
          sections were placed before the executables. 

        Algorithm for finding the version:
        If one section, return A.B.C.D from 0x100-0x103 in the section.
        If two sections, depends on if DATA/EXECUTABLE will return
            A.B.C.D at 0x100-0x103 in second section because this
            is an X3540 or later image
            if EXECUTABLE/EXECUTABLE this is assumed to be an IHDR
            image from pre-X3540 days and will return 0x100-0x103
            in first section, since there are no X3540 images
            that have this format.
        If more than two sections, will return 0x100-0x103 from 
            the last executable section. */

    if (appSize < (MTD_IMAGE_VERSION_OFFSET+MTD_HEADER_SIZE+8))/* 4 bytes for version and something for image, but should be much bigger 
                                                                  at least avoids accessing past end of image passed in */
    {
        return MTD_FAIL; /* appSize for sure has to be bigger than this, and non-zero */    
    }

    mtdMemSet((MTD_PVOID)headersArry, 0, sizeof(headersArry)); /* Zero out headers */

    /* Go through the image and collect pointers to all of the headers, counting the number of
       sections as we go. */
    done = MTD_FALSE;
    imageIndex = 0;
    sectionIndex = 0;
    do
    {
        if (mtdGetHeader(&headersArry[sectionIndex], &appData[imageIndex]) == MTD_FALSE)
        {
            MTD_DBG_ERROR("mtdGetTunitFwVersionFromImage: image corrupted, bad header\n") ;
            return MTD_FAIL;
        }

        if (headersArry[sectionIndex].nextHeaderOffset != 0 &&
            headersArry[sectionIndex].nextHeaderOffset != 0xFFFFFFFF &&
            sectionIndex < MTD_MAX_X35X0_HEADERS)
        {
            /* there's another section coming */
            imageIndex = headersArry[sectionIndex].nextHeaderOffset; /* this way sections don't have to be in order in the image */
            sectionIndex++;
        }
        else
        {
            /* processed all of the sections */
            done = MTD_TRUE;
        }

        if (!done && (imageIndex >= appSize || sectionIndex >= MTD_MAX_X35X0_HEADERS))
        {
            /* there's a problem with this image, attempting to go past end of image */
            MTD_DBG_ERROR("mtdGetTunitFwVersionFromImage: image format error, tried to go past end of image\n") ;
            return MTD_FAIL;
        }
    }while (!done);
    
    /* have collected all of the headers, sectionIndex has the index to the last section */
    if (sectionIndex == 0)
    {
        /* single section, it's either an older image before X3540 with one section, or
           it's an X3540 T-unit-only code, version is at 0x100 past header */
        tUnitExecutableOffset = 0;
    }
    else if (sectionIndex == 1)
    {
    
        /* there are 2 sections, could be two cases here:
           if 2 executable sections, then it's a pre-X3540 IHDR image, the version is in the first section (there
           are no X3540 images with 2 sections only with both sections executable)
           if 1 data and 1 executable, then it's a X3540 image or later, the version is in the second section,
           for X3540 images the last section is always the executable */
        if (headersArry[0].data_only)
        {
            /* this is an X3540 or later image probably an IHDR file with T unit only */
            tUnitExecutableOffset = headersArry[0].nextHeaderOffset;             
        }
        else
        {
            /* this must be an IHDR file for pre-X3540 image */
            tUnitExecutableOffset = 0;
        }
    }
    else
    {
        /* there are more than 2 sections, it's X3540 image or later, so the version of the last executable
           section will be returned */
           tUnitExecutableOffset = headersArry[sectionIndex-1].nextHeaderOffset; 
    }

    *major = appData[tUnitExecutableOffset + MTD_IMAGE_VERSION_OFFSET + MTD_HEADER_SIZE];
    *minor = appData[tUnitExecutableOffset + MTD_IMAGE_VERSION_OFFSET + MTD_HEADER_SIZE + 1];
    *inc = appData[tUnitExecutableOffset + MTD_IMAGE_VERSION_OFFSET + MTD_HEADER_SIZE + 2];
    *test = appData[tUnitExecutableOffset + MTD_IMAGE_VERSION_OFFSET + MTD_HEADER_SIZE + 3];    

    return MTD_OK;
}

