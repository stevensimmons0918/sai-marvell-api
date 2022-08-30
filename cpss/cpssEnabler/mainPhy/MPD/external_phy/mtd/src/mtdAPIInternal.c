/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and data that are strictly internal
for the proper functioning of the API on the Marvell 88X32X0, 88X33X0, 
88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#include <mtdApiRegs.h>
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdDiagnostics.h>
#include <mtdAPIInternal.h>
#include <mtdFwDownload.h>
#include <mtdUtils.h>
#include <mtdCunit.h>
#include <mpdTypes.h>


#if DB_TEST_ENVIRONMENT
#include "time.h"
#include <Windows.h>
#include <WinError.h>
#include "tstSMIInterface.h" 
#endif

#if DB_TEST_FLASHCODE
void amuse_user(void);
MTD_STATIC void amuse_user_ram(MTD_U32 buffCount);
#endif

MTD_STATIC void mtdGetU32fromBuf(MTD_U32 *num, MTD_U8 *buf);
MTD_STATIC void mtdGetU16fromBuf(MTD_U16 *num, MTD_U8 *buf);

MTD_STATIC MTD_STATUS mtdPutMultipleDownloadMode(MTD_DEV_PTR devPtr, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 erroredPorts[], MTD_U16 *errCode);
MTD_STATIC MTD_STATUS mtdStartMultipleAppCode(MTD_DEV_PTR devPtr, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 portsSkip, MTD_BOOL waitDone, MTD_U16 sectionCount, MTD_U16 erroredPorts[], MTD_U16 *errCode);

/* Internal PHY Registers for downloading to RAM */
#define MTD_LOW_WORD_REG     3,0xD0F0 /* register to set the low part of the address */
#define MTD_HI__WORD_REG     3,0xD0F1 /* register to set the hi part of the address */
#define MTD_RAM_DATA_REG     3,0xD0F2 /* register to write or read to/from ram */
#define MTD_RAM_CHECKSUM_REG 3,0xD0F3 /* register to read the checksum from */



MTD_STATUS mtdRamDownloadCalcChecksums(MTD_DEV_PTR devPtr, MTD_U16 port,
                                       MTD_U8 data[], MTD_U32 size, /* size is in bytes, but must be even */
                                       MTD_U32 ramAddress, /* place in RAM to download to */
                                       MTD_U16 *ramChecksumByByte, /* RAM interface or HDR file checksum (inverted) */
                                       MTD_U16 *wordChecksum, /* used by slave bin file protocol for flash download */
                                       MTD_U16 *errCode);


/* This handles downloading an image pointed to by data which is size bytes long
   to the phy's flash interfacing with the slave code as a helper program.
   Size must be an even number (the flash can only be written to in words).*/
MTD_STATUS mtdMdioFlashDownload(MTD_DEV_PTR devPtr,MTD_U16 port,
                                MTD_U8 data[],MTD_U32 size, MTD_U16 *errCode)
{
    MTD_U16 buf_checksum, reported_checksum, words_written, slaveReply, tmpLow, tmpHigh;
    MTD_U32 maxBuffSize = 0, numTransfers, lastTransferSize, transferIndex, slaveRamBuffAddress;
    MTD_U16 tempMaxBuffSize = 0, checksumByByte;
    MTD_U32 byteIndex;
#if DB_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif

    *errCode = 0;

    if (size%2)
    {
        /* it's an error, size must be an even number of bytes */
        *errCode = MTD_SIZE_NOT_EVEN;
        return MTD_FAIL;
    }

    /* first erase the flash*/
#if DB_TEST_FLASHCODE
    MTD_DBG_INFO("Slave will now erase/verify flash. This may take up to %d seconds depending on flash type.\n", (MTD_FLASH_ERASE_WRITE/1000));
#endif

#if DB_TEST_FLASHCODE
    time(&start_time);
#endif

    if (mtdTrySlaveCommand(devPtr, port, ERASE_FLASH_VERIFY_PROGRAM_AREA, &slaveReply, MTD_FLASH_ERASE_WRITE) == MTD_FAIL)
    {
        *errCode = MTD_SLAVE_CMD_TIMEOUT;
        return MTD_FAIL;
    }

    if (slaveReply == MTD_SLAVE_ERR_CMD)
    {
        *errCode = MTD_ERR_ERASING_FLASH;
        return MTD_FAIL;        
    }
    else
    {
        if (slaveReply == MTD_SLAVE_OK)
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Flash program areas have been erased.\n");
#endif
        }
        else
        {
            /* unexpected value read back from download code*/
            MTD_DBG_ERROR("Unexpected response from slave was 0x%04X. Exiting...\n",(int)slaveReply);
            *errCode = MTD_ERR_VALUE_READ_BACK;
            return MTD_FAIL;                       
        }
    }

    /* read in the max buffer size from the slave*/
    /* this is the maximum size that can be written at any 1 time*/
    TRY_IO(mtdHwXmdioRead(devPtr,port,MAX_BUFF_SIZE_OUT_REG,&tempMaxBuffSize));
    if (tempMaxBuffSize == 0)
    {
        *errCode = MTD_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL; /* To catch divide by 0, something wrong on slave side */
        return MTD_FAIL;
    }
    
    maxBuffSize = tempMaxBuffSize;
    maxBuffSize *= 2; /* now size is in bytes*/
    numTransfers = size/maxBuffSize;
    lastTransferSize = size%maxBuffSize;

    /* handle all the full transfers */
    byteIndex=0;
    for(transferIndex=0; transferIndex < numTransfers; transferIndex++)
    {        
        /* Tell the slave we are going to start filling data, and get location to write data to */
        if (mtdTrySlaveCommand(devPtr, port, FILL_BUFFER, &slaveReply, MTD_FLASH_FAST_RESPONSE) == MTD_FAIL)
        {
            *errCode = MTD_SLAVE_CMD_TIMEOUT;
            return MTD_FAIL;
        }

        if (slaveReply != MTD_SLAVE_OK)
        {
            MTD_DBG_ERROR("Expected %d from slave got %d. Exiting...\n", MTD_SLAVE_OK, (int)slaveReply);
            *errCode = MTD_ERR_START_WRITE_DATA;
            return MTD_FAIL;            
        }

        /* Read where to write data to in the SLAVE's memory */
        TRY_IO(mtdHwXmdioRead(devPtr,port,LOW_ADDRESS_REG,&tmpLow));
        TRY_IO(mtdHwXmdioRead(devPtr,port,HIGH_ADDRESS_REG,&tmpHigh));
        slaveRamBuffAddress = (((MTD_U32)tmpHigh)<<16) | tmpLow;

        /* Set the flash start address*/
        TRY_IO(mtdHwXmdioWrite(devPtr,port,LOW_ADDRESS_REG, (MTD_U16)byteIndex));
        TRY_IO(mtdHwXmdioWrite(devPtr,port,HIGH_ADDRESS_REG, (MTD_U16)(byteIndex>>16)));
        
        /* Set the size of the buffer we're going to send*/
        TRY_IO(mtdHwXmdioWrite(devPtr,port,ACTUAL_BUFF_SIZE_IN_REG, (MTD_U16)(maxBuffSize/2)));

        /* Write a buffer of data to the slave RAM */
        MTD_ATTEMPT(mtdRamDownloadCalcChecksums(devPtr, port, &data[byteIndex], maxBuffSize, slaveRamBuffAddress, &checksumByByte, &buf_checksum, errCode));
        byteIndex += maxBuffSize;

#if DB_TEST_FLASHCODE            
        MTD_DBG_INFO("."); /* Amuse the user*/
#endif

        /* One full RAM buffer inside DSP is ready to write to flash now*/
        /* Tell the slave to write it*/
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\nTelling slave to save %d bytes of buffer %d. This can take up to %d seconds, depends on flash type and size of buffer.\n",\
            (int)maxBuffSize, (int)transferIndex, (MTD_FLASH_ERASE_WRITE/1000));
#endif

        if (mtdTrySlaveCommand(devPtr, port, WRITE_VERIFY_BUFFER, &slaveReply, MTD_FLASH_ERASE_WRITE) == MTD_FAIL)
        {
            *errCode = MTD_SLAVE_CMD_TIMEOUT;
            return MTD_FAIL;
        }

        if (slaveReply == MTD_SLAVE_ERR_CMD)
        {
            MTD_DBG_ERROR("\nSome kind of error occurred on Slave. Exiting\n");
            *errCode = MTD_ERR_ON_SLAVE;
            return MTD_FAIL;
        }
        else
        {
            if (slaveReply != MTD_SLAVE_OK)
            {                
                MTD_DBG_ERROR("Expected 0x%04X from slave got 0x%04X. Exiting...\n", (int)MTD_SLAVE_OK, (int)slaveReply);
                *errCode = MTD_ERR_ON_SLAVE;
                return MTD_FAIL;
            }
        }

        /* readback checksum of what was stored in flash */
        TRY_IO(mtdHwXmdioRead(devPtr,port,CHECKSUM_REG,&reported_checksum)); 
        if (reported_checksum != buf_checksum)
        {
            MTD_DBG_ERROR("Expected 0x%04X checksum but got 0x%04X. Exiting...\n",\
                (int)buf_checksum, (int)reported_checksum);
            *errCode = MTD_ERR_CHECKSUM;
            return MTD_FAIL;                    
        }  

        TRY_IO(mtdHwXmdioRead(devPtr,port,WORDS_WRITTEN_REG,&words_written));
        if (words_written != (maxBuffSize/2))
        {
            MTD_DBG_ERROR("\nSlave didn't write enough words to flash. Expecting %d words got %d words. Exiting\n",(int)(maxBuffSize/2),(int)words_written);
            *errCode = MTD_ERR_SLAVE_WRITE_FULL;
            return MTD_FAIL;            
        }
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\n");
#endif
    }

    /* now handle last transfer */
    if (lastTransferSize)
    {
        /* Tell the slave we are going to start filling data, and get location to write data to */
        if (mtdTrySlaveCommand(devPtr, port, FILL_BUFFER, &slaveReply, MTD_FLASH_FAST_RESPONSE) == MTD_FAIL)
        {
            *errCode = MTD_SLAVE_CMD_TIMEOUT;
            return MTD_FAIL;
        }

        if (slaveReply != MTD_SLAVE_OK)
        {
            MTD_DBG_ERROR("Expected %d from slave got %d. Exiting...\n", MTD_SLAVE_OK, (int)slaveReply);
            *errCode = MTD_ERR_START_WRITE_DATA;
            return MTD_FAIL;            
        }

        /* Read where to write data to in the SLAVE's memory */
        TRY_IO(mtdHwXmdioRead(devPtr,port,LOW_ADDRESS_REG,&tmpLow));
        TRY_IO(mtdHwXmdioRead(devPtr,port,HIGH_ADDRESS_REG,&tmpHigh));
        slaveRamBuffAddress = (((MTD_U32)tmpHigh)<<16) | tmpLow;

        /* Set the flash start address*/
        TRY_IO(mtdHwXmdioWrite(devPtr,port,LOW_ADDRESS_REG, (MTD_U16)byteIndex));
        TRY_IO(mtdHwXmdioWrite(devPtr,port,HIGH_ADDRESS_REG, (MTD_U16)(byteIndex>>16)));
        
        /* Set the size of the buffer we're going to send*/
        TRY_IO(mtdHwXmdioWrite(devPtr,port,ACTUAL_BUFF_SIZE_IN_REG, (MTD_U16)(lastTransferSize/2)));

        /* Write a buffer of data to the slave RAM */
        MTD_ATTEMPT(mtdRamDownloadCalcChecksums(devPtr, port, &data[byteIndex], lastTransferSize, slaveRamBuffAddress, &checksumByByte, &buf_checksum, errCode));

#if DB_TEST_FLASHCODE            
        MTD_DBG_INFO("."); /* Amuse the user*/
#endif

        /* One full RAM buffer inside DSP is ready to write to flash now*/
        /* Tell the slave to write it*/
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\nTelling slave to save %d bytes of last buffer. This can take up to %d seconds, depends on flash type and size of buffer.\n",\
            (int)lastTransferSize, (MTD_FLASH_ERASE_WRITE/1000));
#endif

        if (mtdTrySlaveCommand(devPtr, port, WRITE_VERIFY_BUFFER, &slaveReply, MTD_FLASH_ERASE_WRITE) == MTD_FAIL)
        {
            *errCode = MTD_SLAVE_CMD_TIMEOUT;
            return MTD_FAIL;
        }

        if (slaveReply == MTD_SLAVE_ERR_CMD)
        {
            MTD_DBG_ERROR("\nSome kind of error occurred on Slave. Exiting\n");
            *errCode = MTD_ERR_LAST_TRANSFER;
            return MTD_FAIL;
        }
        else
        {
            if (slaveReply != MTD_SLAVE_OK)
            {                
                MTD_DBG_ERROR("Expected 0x%04X from slave got 0x%04X. Exiting...\n", (int)MTD_SLAVE_OK, (int)slaveReply);
                *errCode = MTD_ERR_ON_SLAVE;
                return MTD_FAIL;
            }
        }

        /* readback checksum of what was stored in flash */
        TRY_IO(mtdHwXmdioRead(devPtr,port,CHECKSUM_REG,&reported_checksum)); 
        if (reported_checksum != buf_checksum)
        {
            MTD_DBG_ERROR("Expected 0x%04X checksum but got 0x%04X. Exiting...\n",\
                (int)buf_checksum, (int)reported_checksum);
            *errCode = MTD_ERR_CHECKSUM;
            return MTD_FAIL;                    
        }  

        TRY_IO(mtdHwXmdioRead(devPtr,port,WORDS_WRITTEN_REG,&words_written));
        if (words_written != (lastTransferSize/2))
        {
            MTD_DBG_ERROR("\nSlave didn't write enough words to flash. Expecting %d words got %d words. Exiting\n",(int)(lastTransferSize/2),(int)words_written);
            *errCode = MTD_ERR_SLAVE_WRITE_FULL;
            return MTD_FAIL;            
        }

#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\n");
#endif
    }

#if DB_TEST_FLASHCODE
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    MTD_DBG_INFO("\nTime = %d seconds\n",(int)download_time);
#endif

    return MTD_OK;
}

/* This function does a download to RAM. It returns the expected byte checksum and expected word checksum 
   It verifies the size sent matches the size received by looking at the final address. 
   It always clears the checksum register before and after, and verifies the checksum matches.
   NOTE: The ram checksum register and boot code sums byte=by-byte in a 16-bit accumulator as words are written.
         The slave bin calculates a checksum by summing 16-bit words. 
         Both ignore any overflow. */
MTD_STATUS mtdRamDownloadCalcChecksums(MTD_DEV_PTR devPtr, MTD_U16 port,
                                       MTD_U8 data[], MTD_U32 size, /* size is in bytes, but must be even */
                                       MTD_U32 ramAddress, /* place in RAM to download to */
                                       MTD_U16 *ramChecksumByByte, /* RAM interface or HDR file checksum (inverted) */
                                       MTD_U16 *wordChecksum, /* used by slave bin file protocol for flash download */
                                       MTD_U16 *errCode)
{
    MTD_U32 buffCount;
    MTD_U8  lowByte, highByte;
    MTD_U16 byteChecksum, byteChecksumRead;
    
    /* this read clears the RAM checksum register */
    TRY_IO(mtdHwXmdioRead(devPtr,port, MTD_RAM_CHECKSUM_REG, &byteChecksum));        

    /* Set starting address in RAM */
    TRY_IO(mtdHwXmdioWrite(devPtr,port,MTD_LOW_WORD_REG,(MTD_U16)ramAddress));
    TRY_IO(mtdHwXmdioWrite(devPtr,port,MTD_HI__WORD_REG,(MTD_U16)(ramAddress>>16)));

    /* Copy the code to the phy's internal RAM, calculating checksum as we go */
    buffCount=0;
    byteChecksum = 0; 
    *wordChecksum = 0;
    while(buffCount < size)
    {
        lowByte = data[buffCount++];
        highByte = data[buffCount++];
        
        byteChecksum += lowByte; /* HDR file and RAM interface calc checksum this way*/
        byteChecksum += highByte;

        *wordChecksum += ((((MTD_U16)highByte)<<8)|lowByte); /* slave bin file calculates checksum this way */

#if MTD_USE_MDIO_BLOCK_WRITE == 0
        TRY_IO(mtdHwXmdioWrite(devPtr,port,MTD_RAM_DATA_REG,(((MTD_U16)highByte)<<8)|lowByte));        
#endif

#if DB_TEST_FLASHCODE
        /* Let the user know something's going on... */
        amuse_user_ram(buffCount);
#endif
    }    

#if MTD_USE_MDIO_BLOCK_WRITE == 1
    /* do a block write instead of writing one word at-a-time */
    if (BlockMdioWriteDataToRam(devPtr, (unsigned long)port, (unsigned char *)data, (unsigned int)size) < S_OK)
    {
        MTD_DBG_ERROR("BlockMdioWriteDataToRam failed!\n");
        *errCode = MTD_IO_ERROR;
        return MTD_FAIL;
    }
#endif

    *ramChecksumByByte = byteChecksum;

    /* Read hardware checksum register and see if it matches the locally computed checksum */
    /* this will indicate all the data got there ok, i.e. none dropped or corrupted between here */
    /* and RAM receive register */
    TRY_IO(mtdHwXmdioRead(devPtr,port,MTD_RAM_CHECKSUM_REG,&byteChecksumRead));        
    
    if(byteChecksum != byteChecksumRead)
    {
        MTD_DBG_ERROR( "Error downloading code. Expected RAM HW checksum to be 0x%04X but it was 0x%04X", (int)byteChecksum, (int)byteChecksumRead);
        *errCode = MTD_RAM_HW_CHECKSUM_ERR;
        return MTD_FAIL;
    }    

    return MTD_OK;
}
   
  
/* This function downloads code to RAM in the processor then acts on the header based
   on options in the header. It validates the header then processes it based on the
   options specified there. Image passed in contains one or more header sections */
MTD_STATUS mtdMdioRamDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],
                              MTD_U32 size, MTD_U16 port, 
                              MTD_U16 *errCode) 
{
    MTD_U16 word_checksum, byte_checksum, sectionCount = 0, maxSections, basePort, relativePort;
    MTD_BOOL downloadMode;
    MTD_U32 imageIndex = 0;
    MTD_BOOL appStarted;    
    MTD_FILE_HEADER_TYPE fHeader;
#if DB_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif

    *errCode = 0;
    mtdMemSet(&fHeader, 0, sizeof(MTD_FILE_HEADER_TYPE));
    if (MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        maxSections = MTD_MAX_X35X0_HEADERS;
    }
    else
    {
        maxSections = MTD_MAX_HEADERS;
    }

    if (size%2)
    {
        /* it's an error, size must be an even number of bytes */
        *errCode = MTD_SIZE_NOT_EVEN;
        return MTD_FAIL;
    }

    /* Put PHY in download mode and reset PHY */
    MTD_ATTEMPT(mtdPutPhyInMdioDownloadMode(devPtr,port)); /* will leave errCode as 0 if fails */
    
    /* Allow reset to complete */
    MTD_ATTEMPT(mtdWait(devPtr,50)); 
    
    /* Make sure we can access the PHY
       and it's in the correct mode (waiting for download) */
    TRY_IO(mtdIsPhyInMdioDownloadMode(devPtr,port,&downloadMode));
        
    if (downloadMode == MTD_FALSE)
    {
        MTD_DBG_ERROR("PHY is not in waiting on download mode.\n");
        MTD_DBG_ERROR("Download failed\n");
        *errCode = MTD_PHY_NOT_IN_DOWNLOAD_MODE;
        return MTD_FAIL;
    }
    else
    {
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("Downloading code to PHY RAM, please wait...\n");
#endif
    }

#if DB_TEST_FLASHCODE
    time(&start_time);
#endif

    do
    {
        if (imageIndex > size)
        {
            MTD_DBG_ERROR("Image size and image data do not match. Went past end of image.\n");
            /* went past end of buffer, host passed in wrong size */
            *errCode = MTD_ERR_PROCESSING_IMAGE_FILE;
            return MTD_FAIL;                    
        }   
        
        /* Now process one or more headers */
        if (mtdGetHeader(&fHeader, &data[imageIndex]) == MTD_FALSE)
        {
            MTD_DBG_ERROR("Header corrupted\n");
            *errCode = MTD_BAD_HEADER;
            return MTD_FAIL;
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Processing section %d at offset %d\n",(int)sectionCount,(int)imageIndex);
#endif            
        }
        imageIndex += MTD_HEADER_SIZE;

        basePort = ((port/(devPtr->numPortsPerDevice))*(devPtr->numPortsPerDevice));
        relativePort = port - basePort;
        if (!(fHeader.port_skip & (1<<relativePort)))
        {
            MTD_ATTEMPT(mtdRamDownloadCalcChecksums(devPtr, port, &data[imageIndex], fHeader.dataLength, fHeader.dataDestination, &byte_checksum, &word_checksum, errCode));

            byte_checksum = (~byte_checksum);
            if (byte_checksum != fHeader.secChecksum)
            {
                MTD_DBG_ERROR( "Error downloading code. Expected RAM HDR checksum to be 0x%04X but it was 0x%04X on section %d", (int)fHeader.secChecksum, (int)(byte_checksum), (int)sectionCount);
                *errCode = MTD_HDR_RAM_CHECKSUM_BAD;
                return MTD_FAIL;        
            }

            /* if this is an executable section, execute the code */
            if (!fHeader.data_only)
            {
                /* Now start code which was downloaded */
                TRY_IO(mtdHwSetPhyRegField(devPtr,port,MTD_BOOT_STATUS_REG,MTD_CODE_DOWNLOADED_POS,1,1));
                
                MTD_ATTEMPT(mtdWait(devPtr,200)); /* Give app code time to start */

                /* make sure the app code started */
                TRY_IO(mtdDidPhyAppCodeStart(devPtr,port,&appStarted))
                
                if (appStarted == MTD_FALSE)
                {
                    MTD_DBG_ERROR("App code did not start.\n");
                    MTD_DBG_ERROR("App download failed.\n");
                    *errCode = MTD_APP_CODE_DID_NOT_START;
                    return MTD_FAIL;
                }
                else
                {
#if DB_TEST_FLASHCODE
                    MTD_DBG_INFO("App code started section %d on port %d.\n", (int)sectionCount, (int)port);
#endif
                }                

                /* If more data to download is coming, need to wait for this one to be done. */
                if ((fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF) && ((sectionCount+1) < maxSections))
                {
                    /* There's another section to process coming, wait until the executable is done */
                    /* This is assuming the executable sets mode/state in 1.C001 to 0xB and 6 respectively when it's done. */
                    /* This code assumes the first executable is the serdes loader which sets this status when done. */
                    if (mtdHwWaitForRegFieldValue(devPtr, port, MTD_TUNIT_XG_EXT_STATUS, 0, 12, MTD_SERDES_LOADER_DONE, MTD_EXECUTABLE_TO) == MTD_FAIL)
                    {
                        *errCode = MTD_EXECECUTABLE_TIMED_OUT;
                        return MTD_FAIL;
                    }

#if DB_TEST_FLASHCODE
                    MTD_DBG_INFO("App code finished on section %d on port %d.\n", (int)sectionCount, (int)port);
#endif

                    /* reset T unit processor and put in download mode to clear memory to prepare for next sections */
                    MTD_ATTEMPT(mtdPutPhyInMdioDownloadMode(devPtr,port)); /* will leave errCode as 0 if fails */
                    
                    /* Allow reset to complete */
                    MTD_ATTEMPT(mtdWait(devPtr,50)); 
                    
                    /* Make sure we can access the PHY
                       and it's in the correct mode (waiting for download) for next section */
                    TRY_IO(mtdIsPhyInMdioDownloadMode(devPtr,port,&downloadMode));
    
                    if (downloadMode == MTD_FALSE)
                    {
                        MTD_DBG_ERROR("PHY is not in waiting on download mode for section %d.\n", (int)(sectionCount+1));
                        MTD_DBG_ERROR("Download failed\n");
                        *errCode = MTD_PHY_NOT_IN_DOWNLOAD_MODE;
                        return MTD_FAIL;
                    }
                    else
                    {
#if DB_TEST_FLASHCODE
                        MTD_DBG_INFO("Downloading code to PHY RAM processing section %d, please wait...\n", (int)(sectionCount+1));
#endif
                    }                                    
                }

            }
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Section %d skipped for port %d\n", (int)sectionCount, (int)port);
#endif
        }

        /* Go to the next section if more are coming */
        if (fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF)
        {
            sectionCount++;
            imageIndex = fHeader.nextHeaderOffset; /* sections don't need to be sequential using this approach */ 
                                                   /* could += fHeader.dataLength if they are sequential and want to force */
                                                   /* them to be */
        }
    }while((fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF) && (sectionCount < maxSections));

    if ((sectionCount >= maxSections) && (fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF))
    {
        /* there's something wrong with the file, maximum section number reached but more sections are coming */
        MTD_DBG_ERROR("Max sections reached but more data is coming. Error in input data.\n");
        *errCode = MTD_TOO_MANY_SECTIONS;
        return MTD_FAIL;
    }

#if DB_TEST_FLASHCODE
    /* Calculate download time and print out results */
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    MTD_DBG_INFO("\nDownload of code to RAM complete. Time = %d seconds\n",(int)download_time);
#endif

    return MTD_OK;
}


/********************************************************************/

/**************** Parallel Download *********************/

/********************************************************************/

/* parallel version of mtdRamDownloadCalcChecksums() */
MTD_STATUS mtdParRamDownloadCalcChecksums(MTD_DEV_PTR devPtr, MTD_U16 ports[],
                                          MTD_U16 numPorts, MTD_U16 portsSkip, 
                                          MTD_U16 sectionCount, /* for RAM download this is the HDR section, for flash download this is the buffer number */
                                          MTD_U16 erroredPorts[],
                                          MTD_U8 data[], MTD_U32 size, /* size is in bytes, but must be even */
                                          MTD_U32 ramAddress, /* place in RAM to download to */
                                          MTD_U16 *ramChecksumByByte, /* RAM interface or HDR file checksum (inverted) */
                                          MTD_U16 *wordChecksum, /* used by slave bin file */
                                          MTD_U16 *errCode)
{
    MTD_U32 buffCount;
    MTD_U8  lowByte, highByte;
    MTD_U16 byteChecksum, byteChecksumRead, tmp;
    MTD_U16 i;
    MTD_U16 localPorts[MTD_MAX_PORTS_TO_DOWNLOAD],  /* list of MDIO port numbers to download to (not skipped) */
            portsIndexes[MTD_MAX_PORTS_TO_DOWNLOAD], /* for each port number in localPorts[] contains original location in ports[] */
            localNumPorts; /* number of ports stored in the first two arrays above */
    
    MTD_U16 basePort, relativePort;
    MPD_UNUSED_PARAM(sectionCount);
    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }
    /* Skip any ports in the portsSkip bitmask */
    localPorts[0] = 0;
    for (i=0, localNumPorts=0; i < numPorts; i++)
    {
        basePort = ((ports[i]/(devPtr->numPortsPerDevice))*(devPtr->numPortsPerDevice));
        relativePort = ports[i] - basePort;
        if (!(portsSkip & (1<<relativePort)))
        {
            localPorts[localNumPorts] = ports[i]; /* this port should be processed (not skipped) */
            portsIndexes[localNumPorts] = i; /* store original location to avoid searching later to be used to index erroredPorts[] */
            localNumPorts++;
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Writing section/buffer %d to port %d in parallel mode.\n", (int)sectionCount, (int)ports[i]);
#endif
            
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Section %d skipped for port %d for downloading to ram\n", (int)sectionCount, (int)ports[i]);
#endif
            
        }
    }
    /* localPorts[] is a list of ports to be taken action on, all of the portsSkip have been removed */
    
    /* this read clears the RAM checksum register */
    for(i = 0; i < localNumPorts; i++)
    {
        TRY_IO(mtdHwXmdioRead(devPtr,localPorts[i], MTD_RAM_CHECKSUM_REG,&tmp)); /* clear the register by reading */
    }    

    /*Write to all the localPorts at the same time using the PHY over write pin (PHYPRT_OW) = 1.
      Writing it to one port will ignore the port and write to all the localPorts connected on that pin.
      When reading set the pin to 0*/
    for(i = 0; i < localNumPorts; i++)
    {
        TRY_IO(mtdPutPhyInParallelWriteMode(devPtr,localPorts[i]));
    }

    /* VERY IMPORTANT: From here until it's removed can have no MDIO reads on this entire MDIO bus! Otherwise bogus data will be returned since */
    /* multiple devices will respond on the read request. */

    /* Set starting address in RAM */
    TRY_IO(mtdHwXmdioWrite(devPtr,localPorts[0],MTD_LOW_WORD_REG,(MTD_U16)ramAddress));
    TRY_IO(mtdHwXmdioWrite(devPtr,localPorts[0],MTD_HI__WORD_REG,(MTD_U16)(ramAddress>>16)));

    /* Copy the code to the phy's internal RAM, calculating checksum as we go */
    buffCount=0;
    byteChecksum = 0; 
    *wordChecksum = 0;
    while(buffCount < size)
    {
        lowByte = data[buffCount++];
        highByte = data[buffCount++];
        
        byteChecksum += lowByte; /* HDR file and RAM interface calc checksum this way*/
        byteChecksum += highByte;

        *wordChecksum += ((((MTD_U16)highByte)<<8)|lowByte); /* slave bin file calculates checksum this way */

#if MTD_USE_MDIO_BLOCK_WRITE == 0
        TRY_IO(mtdHwXmdioWrite(devPtr,localPorts[0],MTD_RAM_DATA_REG,(((MTD_U16)highByte)<<8)|lowByte));  
#endif
        
#if DB_TEST_FLASHCODE
        /* Let the user know something's going on... */
        amuse_user_ram(buffCount);
#endif
    }    

#if MTD_USE_MDIO_BLOCK_WRITE == 1
    /* do a block write instead of writing one word at-a-time */
    if (BlockMdioWriteDataToRam(devPtr, (unsigned long)localPorts[0], (unsigned char *)data, (unsigned int)size) < S_OK)
    {
        MTD_DBG_ERROR("BlockMdioWriteDataToRam failed!\n");
        *errCode = MTD_IO_ERROR;
        return MTD_FAIL;
    }
#endif

    *ramChecksumByByte = byteChecksum;

    /* writing to 1 clears all */
    TRY_IO(mtdRemovePhyFromParallelWriteMode(devPtr,localPorts[0])); 

    for(i = 0; i < localNumPorts; i++)
    {
        /* read the hardware register checksum and see if it matches with the locally computed checksum */
        TRY_IO(mtdHwXmdioRead(devPtr,localPorts[i],MTD_RAM_CHECKSUM_REG,&byteChecksumRead)); 
        
        if(byteChecksum != byteChecksumRead)   
        {
            MTD_DBG_ERROR( "Error downloading code on MDIO port %d. Expected RAM HW checksum to be 0x%04X but it was 0x%04X",(int)localPorts[i],\
                          (int)byteChecksum, (int)byteChecksumRead);
            if (erroredPorts[portsIndexes[i]] == 0)
            {
                /* only overwrite error if there was no earlier error, we're tracking the first error that happened not the last */
                erroredPorts[portsIndexes[i]] = MTD_RAM_HW_CHECKSUM_ERR;
            }
        }
    }

    return MTD_OK;
}


/* This function downloads code to several ports' RAM in the DSP "in parallel" meaning
   multiple ports are downloaded at the same time and then starts the application
   which was downloaded.*/
/* Make sure "size" is an even number(memory can only be written word-wise) */
/* Images passed in are in header format with one or more sections */
MTD_STATUS mtdParallelMdioRamDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],MTD_U32 size,
                                      MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 erroredPorts[],
                                      MTD_U16 *errCode) 
{
    MTD_U16 word_checksum, byte_checksum, sectionCount = 0, maxSections;
    MTD_U32 imageIndex = 0;    
    MTD_FILE_HEADER_TYPE fHeader;
    
#if DB_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif

    *errCode = 0;
    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }
    mtdMemSet(&fHeader, 0, sizeof(MTD_FILE_HEADER_TYPE));
    if (MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        maxSections = MTD_MAX_X35X0_HEADERS;
    }
    else
    {
        maxSections = MTD_MAX_HEADERS;
    }

    if (size%2)
    {
        /* it's an error, size must be an even number of bytes */
        *errCode = MTD_SIZE_NOT_EVEN;
        return MTD_FAIL;
    }

#if DB_TEST_FLASHCODE
    time(&start_time);
#endif    

    /* Put all the ports in the download mode and make sure they are in the download mode. */
    MTD_ATTEMPT(mtdPutMultipleDownloadMode(devPtr, ports, numPorts, erroredPorts, errCode));

    do
    {
        if (imageIndex > size)
        {
            MTD_DBG_ERROR("Image size and image data do not match. Went past end of image.\n");
            /* went past end of buffer, host passed in wrong size */
            *errCode = MTD_ERR_PROCESSING_IMAGE_FILE;
            return MTD_FAIL;                    
        }        

        /* Now process one or more headers */
        if (mtdGetHeader(&fHeader, &data[imageIndex]) == MTD_FALSE)
        {
            MTD_DBG_ERROR("Header corrupted\n");
            *errCode = MTD_BAD_HEADER;
            return MTD_FAIL;
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Processing section %d at offset %d\n",(int)sectionCount,(int)imageIndex);
#endif            
        }

        if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
            /* Download header*/
            MTD_U32 addressOffset = 0x28000000 + MTD_HEADER_SIZE*sectionCount;
            
            MTD_ATTEMPT(mtdParRamDownloadCalcChecksums(devPtr, ports, numPorts, fHeader.port_skip, sectionCount, erroredPorts, &data[imageIndex], MTD_HEADER_SIZE, addressOffset, &byte_checksum, &word_checksum, errCode));
            /* Ignoring byte_checksum for now. Could check it. */
        }
        imageIndex += MTD_HEADER_SIZE;

        MTD_ATTEMPT(mtdParRamDownloadCalcChecksums(devPtr, ports, numPorts, fHeader.port_skip, sectionCount, erroredPorts, &data[imageIndex], fHeader.dataLength, fHeader.dataDestination, &byte_checksum, &word_checksum, errCode));

        byte_checksum = (~byte_checksum);
        if (byte_checksum != fHeader.secChecksum)
        {
            MTD_DBG_ERROR( "Error downloading code. Expected RAM HDR checksum to be 0x%04X but it was 0x%04X on section %d", (int)fHeader.secChecksum, (int)(byte_checksum), (int)sectionCount);
            *errCode = MTD_HDR_RAM_CHECKSUM_BAD;
            return MTD_FAIL;        
        }

        /* if this is an executable section, execute the code */
        if (!fHeader.data_only)
        {
            MTD_BOOL moreSectionsComing = (fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF) && ((sectionCount+1) < maxSections);

#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Executing code in section %d on some ports\n", (int)(sectionCount));
#endif

            /* Starts the code on those not skipped. If more sections are coming will wait until they are all done then reset all of them (including those skipped)*/
            MTD_ATTEMPT(mtdStartMultipleAppCode(devPtr, ports, numPorts, fHeader.port_skip, moreSectionsComing, sectionCount, erroredPorts, errCode));
        }
        
        /* Go to the next section if more are coming */
        if (fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF)
        {
            sectionCount++;
            imageIndex = fHeader.nextHeaderOffset; /* sections don't need to be sequential using this approach */ 
                                                   /* could += fHeader.dataLength if they are sequential and want to force */
                                                   /* them to be */
        }        
    }while((fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF) && (sectionCount < maxSections));
    

    if ((sectionCount >= maxSections) && (fHeader.nextHeaderOffset != 0 && fHeader.nextHeaderOffset != 0xFFFFFFFF))
    {
        /* there's something wrong with the file, maximum section number reached but more sections are coming */
        MTD_DBG_ERROR("Max sections reached but more data is coming. Error in input data.\n");
        *errCode = MTD_TOO_MANY_SECTIONS;
        return MTD_FAIL;
    }

#if DB_TEST_FLASHCODE
    /* Calculate download time and print out results */
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    MTD_DBG_INFO("\nDownload of code to RAM complete. Time = %d seconds\n",(int)download_time);
#endif

    return MTD_OK;
}


MTD_STATUS mtdParallelMdioFlashDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],
                                 MTD_U32 size, 
                                 MTD_U16 ports[], MTD_U16 numPorts,
                                 MTD_U16 erroredPorts[],
                                 MTD_U16 *errCode)
{

    MTD_U16 buf_checksum, words_written, tmpLow, tmpHigh, checksumByByte;
    MTD_U32 maxBuffSize = 0, numTransfers, lastTransferSize, transferIndex;
    MTD_U16 tempMaxBuffSize = 0;
    MTD_U32 byteIndex, slaveRamBuffAddress;
#if DB_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif
    MTD_U16 i;

    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }
    if (size%2)
    {
        /* it's an error, size must be an even number of bytes */
        *errCode = MTD_SIZE_NOT_EVEN;
        return MTD_FAIL;
    }

    /* first erase the flash*/
#if DB_TEST_FLASHCODE
    MTD_DBG_INFO("Slave will now erase/verify flash. This may take up to %d seconds.\n", (MTD_FLASH_ERASE_WRITE/1000));
#endif

#if DB_TEST_FLASHCODE
    time(&start_time);
#endif

    TRY_IO(mtdTryParallelSlaveCommand(devPtr, ERASE_FLASH_VERIFY_PROGRAM_AREA, ports, numPorts, erroredPorts, MTD_FLASH_ERASE_WRITE)); /* some flashes can take a really long time to erase */

    for(i = 0; i < numPorts; i++)
    {
        if(erroredPorts[i] == 0)
        {
            MTD_DBG_INFO("Flash program areas have been erased for port %d.\n",ports[i]);
        }
    }

    /* read in the max buffer size from the slave*/
    /* this is the maximum size that can be written at any 1 time, it will be the same on all slaves */
    TRY_IO(mtdHwXmdioRead(devPtr,ports[0],MAX_BUFF_SIZE_OUT_REG,&tempMaxBuffSize));
    if (tempMaxBuffSize == 0)
    {
        *errCode = MTD_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL; /* To catch divide by 0, something wrong on slave side */
        return MTD_FAIL;
    }
    
    maxBuffSize = tempMaxBuffSize;
    maxBuffSize *= 2; /* now it's in bytes*/

    numTransfers = size/maxBuffSize;
    lastTransferSize = size%maxBuffSize;

    /* handle all the full transfers */
    
    byteIndex=0;
    for(transferIndex=0; transferIndex < numTransfers; transferIndex++)
    {

        /* sends the FILL_BUFFER command to the slaves, will set an error message and print an error on errored ports */
        /* will also print an informational message on success for each port */
        TRY_IO(mtdTryParallelSlaveCommand(devPtr, FILL_BUFFER, ports, numPorts, erroredPorts, MTD_FLASH_FAST_RESPONSE));

        /* Read where to write data to in the SLAVE's memory */
        TRY_IO(mtdHwXmdioRead(devPtr,ports[0],LOW_ADDRESS_REG,&tmpLow));   /* all ports will be the same with the same slave code */
        TRY_IO(mtdHwXmdioRead(devPtr,ports[0],HIGH_ADDRESS_REG,&tmpHigh)); /* this assume first port is good/was downloaded ok and no error */
        slaveRamBuffAddress = (((MTD_U32)tmpHigh)<<16) | tmpLow;

        for(i = 0; i < numPorts; i++)
        {
            /* put all ports in parallel write mode */
            TRY_IO(mtdPutPhyInParallelWriteMode(devPtr,ports[i]));  
        }

        /* Set the flash start address in all the ports */
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],LOW_ADDRESS_REG, (MTD_U16)byteIndex));
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],HIGH_ADDRESS_REG, (MTD_U16)(byteIndex>>16)));

        /* Set the size of the buffer we're going to send*/
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],ACTUAL_BUFF_SIZE_IN_REG, (MTD_U16)(maxBuffSize/2)));

        /* writing to 1 clears all */
        TRY_IO(mtdRemovePhyFromParallelWriteMode(devPtr,ports[0]));

        /* Write one block of data to all the slaves RAMs, will also verify the checksum that it go there ok and flag errored ports */
        MTD_ATTEMPT(mtdParRamDownloadCalcChecksums(devPtr, ports, numPorts, 0, (MTD_U16)transferIndex, erroredPorts, &data[byteIndex], maxBuffSize, slaveRamBuffAddress, &checksumByByte, &buf_checksum, errCode));
        byteIndex += maxBuffSize; /* increment to next block of data to send */


        /* One full RAM buffer inside DSP is ready to write to flash now*/
        /* Tell the slave to write it*/
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\nTelling slaves to save and verify %d bytes of buffer %d\n",\
            (int)maxBuffSize, (int)transferIndex); 
#endif

        /* Tell the slave to write the block to the flash and verify it got written correctly */
        /* Will also verify command processed correctly and flag errored ports */        
        TRY_IO(mtdTryParallelSlaveCommand(devPtr, WRITE_VERIFY_BUFFER, ports, numPorts, erroredPorts, MTD_FLASH_ERASE_WRITE));

#if DB_TEST_FLASHCODE            
        MTD_DBG_INFO("."); /* Amuse the user */
#endif

        for(i = 0; i < numPorts; i++)
        {
            if(erroredPorts[i] == 0)
            {
                /* see if the words written are the correct number */
                TRY_IO(mtdHwXmdioRead(devPtr,ports[i],WORDS_WRITTEN_REG,&words_written));
                
                if (words_written != maxBuffSize/2)
                {
                    /* Host might want to issue a retry here instead failing*/
                    /* Note that the flash start address must be reset before resending the buffer*/
                    MTD_DBG_ERROR( "Slave failed to write correct number of data on MDIO port %d \n",ports[i]);
                    erroredPorts[i] = MTD_WRITE_VERIFY_DATA;            
                }
            }
        }        
    }

#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("\nNow handling last block to transfer.\n");
#endif

    /* now handle last transfer */
    
    if (lastTransferSize)
    {
        /* sends the FILL_BUFFER command to the slaves, will set an error message and print an error on errored ports */
        /* will also print an informational message on success for each port */
        TRY_IO(mtdTryParallelSlaveCommand(devPtr, FILL_BUFFER, ports, numPorts, erroredPorts, MTD_FLASH_FAST_RESPONSE));

        /* Read where to write data to in the SLAVE's memory */
        TRY_IO(mtdHwXmdioRead(devPtr,ports[0],LOW_ADDRESS_REG,&tmpLow));   /* all ports will be the same with the same slave code */
        TRY_IO(mtdHwXmdioRead(devPtr,ports[0],HIGH_ADDRESS_REG,&tmpHigh)); /* this assume first port is good/was downloaded ok and no error */
        slaveRamBuffAddress = (((MTD_U32)tmpHigh)<<16) | tmpLow;

        for(i = 0; i < numPorts; i++)
        {
            /* put all ports in parallel write mode */
            TRY_IO(mtdPutPhyInParallelWriteMode(devPtr,ports[i]));  
        }

        /* Set the flash start address in all the ports */
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],LOW_ADDRESS_REG, (MTD_U16)byteIndex));
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],HIGH_ADDRESS_REG, (MTD_U16)(byteIndex>>16)));

        /* Set the size of the buffer we're going to send*/
        TRY_IO(mtdHwXmdioWrite(devPtr,ports[0],ACTUAL_BUFF_SIZE_IN_REG, (MTD_U16)(lastTransferSize/2)));

        /* writing to 1 clears all */
        TRY_IO(mtdRemovePhyFromParallelWriteMode(devPtr,ports[0]));

        /* Write one block of data to all the slaves RAMs, will also verify the checksum that it go there ok and flag errored ports */
        MTD_ATTEMPT(mtdParRamDownloadCalcChecksums(devPtr, ports, numPorts, 0, (MTD_U16)transferIndex, erroredPorts, &data[byteIndex], lastTransferSize, slaveRamBuffAddress, &checksumByByte, &buf_checksum, errCode));
        byteIndex += lastTransferSize; /* there actually is no more data, this moves to end of buffer though */

        /* One full RAM buffer inside DSP is ready to write to flash now*/
        /* Tell the slave to write it*/
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\nTelling slaves to save and verify %d bytes of last buffer\n",\
            (int)lastTransferSize); 
#endif

        /* Tell the slave to write the block to the flash and verify it got written correctly */
        /* Will also verify command processed correctly and flag errored ports */        
        TRY_IO(mtdTryParallelSlaveCommand(devPtr, WRITE_VERIFY_BUFFER, ports, numPorts, erroredPorts, MTD_FLASH_ERASE_WRITE));

#if DB_TEST_FLASHCODE            
        MTD_DBG_INFO("."); /* Amuse the user */
#endif

        for(i = 0; i < numPorts; i++)
        {
            if(erroredPorts[i] == 0)
            {
                /* see if the words written are the correct number */
                TRY_IO(mtdHwXmdioRead(devPtr,ports[i],WORDS_WRITTEN_REG,&words_written));
                
                if (words_written != lastTransferSize/2)
                {
                    /* Host might want to issue a retry here instead failing*/
                    /* Note that the flash start address must be reset before resending the buffer*/
                    MTD_DBG_ERROR( "Slave failed to write correct number of data on MDIO port %d \n",ports[i]);
                    erroredPorts[i] = MTD_WRITE_VERIFY_DATA;            
                }
            }
        }        
    }


#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("\n");
#endif

#if DB_TEST_FLASHCODE
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    MTD_DBG_INFO("\nTime = %d seconds\n",(int)download_time);
#endif

                   /* Done */
    return MTD_OK; /* caller should go through and if any erroredPorts[] are not 0, determine there were errors */
}


/* This sends a flash command to the slave code and waits for a response until "timeoutMs" polling in 10 ms increments */
/* returns MTD_FAIL if there is a timeout or any IO error */
MTD_STATUS mtdTrySlaveCommand(MTD_DEV_PTR devPtr, MTD_U16 port, MTD_U16 command, MTD_U16 *response, MTD_U16 timeoutMs)
{
    MTD_U16 timeoutCounter=0;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,COMMAND_REG,command));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,COMMAND_REG, response));    
    while((*response == command || *response == MTD_SLAVE_FLASH_BUSY) && (timeoutCounter++ < timeoutMs))
    {
        MTD_ATTEMPT(mtdWait(devPtr,1)); 
                                                
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,COMMAND_REG,response));
    }

    if (timeoutCounter >= timeoutMs)
    {
        MTD_DBG_ERROR("mtdTrySlaveCommand: slave timeout on port %d on command 0x%04X\n",(int)port,(int)command);
        return MTD_FAIL;
    }

    return MTD_OK;
}

/* This is a parallel version of the mtdTrySlaveCommand(). It puts all those PHYS in parallel write mode, */
/* sends the command to the slave in parallel, takes it out of parallel write mode, then polls all until they've responded for */
/* for "timeoutMs" milliseconds looking for some response. If no response, those slaves that did not responsd will */
/* be flagged in the erroredPorts list so they are skipped for the rest of the attempts to download */
/* */
/* timing is going to be very inaccurate based on number of ports being polled and MDIO data rate, which will add delay */
MTD_STATUS mtdTryParallelSlaveCommand(MTD_DEV_PTR devPtr, MTD_U16 command, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 erroredPorts[], MTD_U16 timeoutMs)
{
    MTD_U16  portCtr, doneCtr, timeoutCounter, response; 
    MTD_BOOL portDone[MTD_MAX_PORTS_TO_DOWNLOAD];

    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }

    for(portCtr = 0; portCtr < numPorts; portCtr++)
    {
        MTD_ATTEMPT(mtdPutPhyInParallelWriteMode(devPtr,ports[portCtr]));  /* parallel write */
        portDone[portCtr] = MTD_FALSE;
    }

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,ports[0],COMMAND_REG, command)); /* write comand in parallel */

    /* writing to 1 clears all */
    MTD_ATTEMPT(mtdRemovePhyFromParallelWriteMode(devPtr,ports[0]));   /*remove parallel write*/

    doneCtr = 0;
    timeoutCounter = 0;
    while (doneCtr < numPorts && timeoutCounter <= timeoutMs) /* do this until all are done or there is a timeout */
    {
        MTD_ATTEMPT(mtdWait(devPtr,2));
        timeoutCounter += 2;

        /* go through all ports and see which are done, skip those that have errors already */
        for(portCtr = 0; portCtr < numPorts; portCtr++)
        {
            if (erroredPorts[portCtr] != 0)
            {
                portDone[portCtr] = MTD_TRUE; /* mark this port as done */
            }
            else if (portDone[portCtr] == MTD_FALSE) /* only poll the ones we are waiting on a result from */
            {
                /* check and see if the result is ready */                
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,ports[portCtr],COMMAND_REG, &response)); 
                if (response != command && response != MTD_SLAVE_FLASH_BUSY)
                {
                    /* it's either done or got an error, either way mark it as done */
                    portDone[portCtr] = MTD_TRUE;
                    
                    if (response != MTD_SLAVE_OK)
                    {
                        if (response == MTD_SLAVE_ERR_CMD)
                        {
                            switch(command)
                            {
                                case ERASE_FLASH_VERIFY_PROGRAM_AREA:
                                    erroredPorts[portCtr] = MTD_ERR_ERASING_FLASH;
                                    break;
                                    
                                case FILL_BUFFER:
                                    erroredPorts[portCtr] = MTD_ERR_START_WRITE_DATA;
                                    break;
                                    
                                case WRITE_VERIFY_BUFFER:
                                    erroredPorts[portCtr] = MTD_WRITE_VERIFY_DATA;
                                    break;
                                    
                                case VERIFY_FLASH:
                                    erroredPorts[portCtr] = MTD_VERIFY_ERR;
                                    break;

                                default:
                                    /* command not recognized */
                                    erroredPorts[portCtr] = MTD_ERR_ON_SLAVE;
                                    break;
                            }
                            MTD_DBG_ERROR("mtdTryParallelSlaveCommand: Slave reported error 0x%04X on MDIO port %d. Command = 0x%04X. Port flagged with = 0x%04X\n",(int)response,(int)ports[portCtr],(int)command,(int)erroredPorts[portCtr]);
                        }
                        else
                        {
                            MTD_DBG_ERROR("mtdTryParallelSlaveCommand: Unexpected slave response on MDIO port %d. Command = 0x%04X Response = 0x%04X\n", (int)ports[portCtr], (int)command, (int)response);
                            erroredPorts[portCtr] = MTD_ERR_VALUE_READ_BACK;
                        }

                    }
                    else
                    {
                        MTD_DBG_INFO("mtdTryParallelSlaveCommand: slave command 0x%04X success on MDIO port %d\n",(int)command,(int)ports[portCtr]);
                    }
                }
            }
        }

        /* count number are done */
        for(portCtr = 0, doneCtr = 0; portCtr < numPorts; portCtr++)
        {
            if (portDone[portCtr] == MTD_TRUE)
            {
                doneCtr++;
            }
        }
    }

    /* if timeout was reached, go through and mark those ports as done and mark them as errored */
    if (timeoutCounter >= timeoutMs)
    {
        for(portCtr = 0; portCtr < numPorts; portCtr++)
        {
            if (portDone[portCtr] == MTD_FALSE)
            {
                MTD_DBG_ERROR("mtdTryParallelSlaveCommand: slave timeout on port %d on command 0x%04X\n",(int)ports[portCtr],(int)command);               
                portDone[portCtr] = MTD_TRUE;
                erroredPorts[portCtr] = MTD_SLAVE_CMD_TIMEOUT;
            }
        }
    }

    return MTD_OK;   
}


/* Returns the memory size available in the connected device for application code */
/* must have previously called mtdLoadDriver() to initialize devPtr */
MEM_SIZE_BYTES mtdGetDevMemorySize(MTD_DEV_PTR devPtr)
{
    MEM_SIZE_BYTES ramSize=0;

    /* make sure mtdLoadDriver() has been called so deviceId is valid */
    if (devPtr->devEnabled)
    {
        if (MTD_IS_X32X0_BASE(devPtr->deviceId) || MTD_IS_X33X0_BASE(devPtr->deviceId) || MTD_IS_E21X0_BASE(devPtr->deviceId))
        {
            ramSize = MTD_MAX_APP_SIZE;
        }  
        if (MTD_IS_X35X0_BASE(devPtr->deviceId))
        {
            ramSize = MTD_MAX_X35X0_APP_SIZE;
        }
    }

    return(ramSize);
}

/* Returns the memory size available in the connected device for application code */
/* must have previously called mtdLoadDriver() to initialize devPtr */
MEM_SIZE_BYTES mtdGetDevMaxImageSize(MTD_DEV_PTR devPtr)
{
    MEM_SIZE_BYTES maxImageSize=0;

    /* make sure mtdLoadDriver() has been called so deviceId is valid */
    if (devPtr->devEnabled)
    {
        if (MTD_IS_X32X0_BASE(devPtr->deviceId) || MTD_IS_X33X0_BASE(devPtr->deviceId) || MTD_IS_E21X0_BASE(devPtr->deviceId))
        {
            maxImageSize = MTD_MAX_FLASH_IMAGE_SIZE;
        }  
        if (MTD_IS_X35X0_BASE(devPtr->deviceId))
        {
            maxImageSize = MTD_MAX_X35X0_FLASH_IMAGE_SIZE;
        }
    }

    return(maxImageSize);
}

#if MTD_ORIGSERDES

MTD_STATUS mtdDoMeasHalfEye
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 curSelLane,
    IN MTD_U16 reg_8xBE,
    OUT MTD_U32 data[][MTD_EYEDIAGRAM_NCOLS]
)
{
    MTD_U16 tmp, tmpAddr;
    MTD_U16 amp = (MTD_EYEDIAGRAM_NROWS - 1)/ 2 + 2;        /*27*/
    MTD_U16 reg11 = 0, reg14 = 0;
    MTD_U16 lsb, msb;
    MTD_U16 i,j, cnt;
    MTD_U16 onlyOnePhase = 0; /* 0: run all 64 phases, 1: run with center phase only*/

    tmpAddr = 0x8000 + curSelLane*MTD_XFI_LANEOFFSET;

    for (i = 0; i < (MTD_EYEDIAGRAM_NROWS + 1) / 2; i++) /* through all amplitudes*/
    {
        /*i) write amp*/
        if (i < (MTD_EYEDIAGRAM_NROWS + 1) / 4) /*13*/
            amp = amp - 2;
        else if (i == (MTD_EYEDIAGRAM_NROWS + 1) / 4) /*13*/
            amp = 0;
        else if (i > (MTD_EYEDIAGRAM_NROWS + 1) / 4) /*13*/
            amp = amp + 2;
        /*write eye amplitude $V.$PL11.10:6*/
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,&reg11));
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,((reg11 & 0xF83F) | (amp << 6))));

        /*ii) read phase value*/
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,&reg11));
        for (j = 0; j < 64; j++)  /* through 64 phases (64 columns)*/
        {
            if (onlyOnePhase && (j != (int)reg_8xBE))
            {
            }
            else
            {
                /*write phase*/
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,((reg11 & 0xFFC0) | j)));
                /*begin measure*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,&tmp));
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,(tmp | 0x8000)));
                /*wait for 1s for measurement to be done*/
                for (cnt = 0; cnt < 1000; cnt++)
                {
                    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x11,&tmp));
                    if ((tmp & 0x8000) == 0)
                        break;
                    MTD_ATTEMPT(mtdWait(devPtr,1)); /* wait for 1ms*/
                    if (cnt == 999)
                        return MTD_FAIL;
                }
                /*$V.$PL14 -- Read reg14*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x14,&reg14));
                /*write $V.$PL14.11:9 = 010; Read P even eye error counter*/
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_XFI_DSP,tmpAddr+0x14,((reg14 & 0xF00F) | 0x200)));
                /*read measure result*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x12,&tmp));
                lsb = (tmp & 0xFFFF);
                /*sw1.WriteLine(reg24 & 0xffff);*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x13,&tmp));
                msb = (tmp & 0xFFFF);
                /*sw2.WriteLine(reg25 & 0xffff);*/
                data[amp][j] = (msb << 16) + lsb;

                /*write $V.$PL14.11:9 = 011; Read P odd eye error counter*/
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_XFI_DSP,tmpAddr+0x14,((reg14 & 0xF00F) | 0x300)));
                /*read measure result*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x12,&tmp));
                lsb = (tmp & 0xFFFF);
                /*sw1.WriteLine(reg24 & 0xffff);*/
                MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_XFI_DSP,tmpAddr+0x13,&tmp));
                msb = (tmp & 0xFFFF);
                /*sw2.WriteLine(reg25 & 0xffff);*/
                data[amp][j+64] = (msb << 16) + lsb;
            }
        }
    }

    return MTD_OK;
}

#endif /* MTD_ORIGSERDES */

/* internal device registers */
#define MTD_REG_ER     3,0xD810
#define MTD_REG_EC0    3,0xD811
#define MTD_REG_EC1    3,0xD812
#define MTD_REG_ERDL   3,0xD813
#define MTD_REG_ERDH   3,0xD814
#define MTD_REG_CCCR9 31,0xF05E
#define MTD_REG_SCR   31,0xF0F0
#define MTD_REG_ECSR  31,0xF0F5
#define MTD_REG_LW     3,0xD80F
#define MTD_REG_HW     3,0xD810

MTD_STATIC MTD_U32 mtdGetFeatureList(MTD_DEV_PTR devPtr, MTD_U16 port, MTD_U8 loc)
{
    const MTD_U16 offS = 0x10+loc;
    MTD_U16 dataH, dataL;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC0,0x00A0));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC1,offS));  
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC0,0x00E0));  
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC0,0x00A0));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC1,0x0400|(offS)));  
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC1,offS));  
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC0,0x0000));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_EC1,0x0000));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_ERDH,&dataH));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_ERDL,&dataL));


    return ((dataH<<16)|dataL);
}

MTD_STATIC MTD_U16 mtdCombine(MTD_U16 dataH, MTD_U16 dataL)
{
    MTD_U32 tempU32, temp2U32;
    MTD_U16 temp3U16;

    tempU32 = dataL;
    tempU32 |= (((MTD_U32)(dataH))<<16);
    
    temp2U32 = (tempU32 & 0x55555555) | ((tempU32 & 0xAAAAAAAA)>>1);
    temp2U32 = (temp2U32 | (temp2U32>>1)) & 0x33333333;
    temp2U32 = (temp2U32 | (temp2U32>>2)) & 0x0f0f0f0f;
    temp2U32 = (temp2U32 | (temp2U32>>4)) & 0x00ff00ff;
    temp2U32 = (temp2U32 | (temp2U32>>8)) & 0x0000ffff;
    temp3U16 = (MTD_U16)temp2U32;

    return temp3U16;
}

MTD_STATUS mtdCheckDeviceCapabilities
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 baseType,
    IN MTD_BOOL ignoreFwReportedVals,
    OUT MTD_BOOL *phyHasMacsec,
    OUT MTD_BOOL *phyHasCopperInterface,
    OUT MTD_BOOL *is5GDevice,
    OUT MTD_BOOL *is2P5GDevice
)
{
    MTD_U8 major,minor,inc,test;
    MTD_U16 abilities;

    *phyHasMacsec = MTD_TRUE;
    *phyHasCopperInterface = MTD_TRUE;
    *is5GDevice = MTD_FALSE; 
    *is2P5GDevice = MTD_FALSE;
    
    if (mtdGetFirmwareVersion(devPtr,port,&major,&minor,&inc,&test) == MTD_FAIL)
    {
        major = minor = inc = test = 0; /* firmware not running will produce this case */
    }

    if ((major == 0 && minor == 0 && inc == 0 && test == 0) || ignoreFwReportedVals)
    {
        if (MTD_IS_E21X0_BASE(baseType))
        {

            MTD_U16 tempL, tempH, tempCombined; 

            *phyHasMacsec = MTD_FALSE;
            
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_LW,&tempL));
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_HW,&tempH));

            tempCombined = mtdCombine(tempH, tempL);
            
            if((tempCombined & (1<<15)) && (tempCombined & (1<<9)) && (tempCombined & (1<<11)))
            {
                *is2P5GDevice = MTD_TRUE;
            }
        }
        else if (MTD_IS_X35X0_BASE(baseType))
        {
            MTD_U16 bitVal;
            MTD_U16 counter = 0; 

            *phyHasMacsec = MTD_FALSE;

            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ER,0xFFFF));

            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_REG_ER,1,1,&bitVal));   

            while (bitVal == 0 && counter < 100)
            {
                MTD_ATTEMPT(mtdWait(devPtr,1));
                MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_REG_ER,1,1,&bitVal));   
                counter++;
            }

            if (counter < 100)
            {
                MTD_U8 idx; 
                MTD_U16 values16[4];
                MTD_U32 values32[4];

                for (idx=0; idx < 4; idx++)
                {
                    values32[idx] = mtdGetFeatureList(devPtr, port, idx);
                    values16[idx] = mtdCombine((MTD_U16)(values32[idx]>>16), (MTD_U16)values32[idx]);
                }

                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ER,0x0));

                MTD_ATTEMPT(mtdHwGetRegFieldFromWord(values16[0],0,1,&bitVal));

                if (bitVal == 0x0)
                {
                    *is5GDevice = MTD_FALSE;
                }
                else
                {
                    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(values16[0],5,1,&bitVal));
                    if(bitVal)
                    {
                        *is5GDevice = MTD_TRUE;
                    }
                    else
                    {
                        *is5GDevice = MTD_FALSE;
                    }
                }

                /* Might want to check X3580 values16[1] bits 0 or 1 to cross-check id */
                /* Ports 0-3 will set bit 0 and ports 4-7 will set bit 1 */
            }
            else
            {
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ER,0x0));
                MTD_DBG_ERROR("mtdCheckDeviceCapabilities: MTD_REG_ER register cannot be read correctly\n");
                return MTD_FAIL;
            }
        }
        else /* PHYs before E21X0 and X35X0 */
        {
            /* no code loaded into internal processor */
            /* have to read it from the device itself the hard way */
            MTD_U16 reg2,reg3; /* To save previous values for restore */
            MTD_U16 index,index2;
            MTD_U16 temp;
            MTD_U16 bit16thru23[8];

            /* save these registers */
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_SCR,&reg2));
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_ECSR,&reg3));

            /* clear these bit indications */
            for (index=0;index<8;index++)
            {
                bit16thru23[index]=0;
            }
            
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_CCCR9,0x0300)); 
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_SCR,0x0102)); 
            mtdWait(devPtr,1);
            
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x06D3)); 
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0593)); 
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0513)); 
            mtdWait(devPtr,1);
                    
            index = 0;
            index2 = 0;    
            while(index < 24)
            {

                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0413)); 
                mtdWait(devPtr,1);
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0513)); 
                mtdWait(devPtr,1);

                if (index >= 16)
                {
                	/* KW */
                	if (index2 >= 8) {
                        MTD_DBG_ERROR("mtdCombine: index2 out of bounds %d.\n", index2);
                        return MTD_FAIL;

                	}
                	/* End KW */
                    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_ECSR,&bit16thru23[index2++]));
                }
                else
                {
                    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_REG_ECSR,&temp));
                }
                mtdWait(devPtr,1);
                index++;
            }
            
            if (((bit16thru23[0]>>11)&1) | ((bit16thru23[1]>>11)&1))
            {
                *phyHasMacsec = MTD_FALSE;
            }
            if (((bit16thru23[4]>>11)&1) | ((bit16thru23[5]>>11)&1))
            {
                *phyHasCopperInterface = MTD_FALSE;
            }

            if (((bit16thru23[6]>>11)&1) | ((bit16thru23[7]>>11)&1))
            {
                *is5GDevice = MTD_TRUE;
            }

            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0413));
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0493)); 
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0413)); 
            mtdWait(devPtr,1);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,0x0513)); 
            mtdWait(devPtr,1);

            /* restore the registers */
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_CCCR9,0x5440));         
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_SCR,reg2));
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_REG_ECSR,reg3));
        }
    }
    else
    {
        /* should just read it from the firmware status register */
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_TUNIT_XG_EXT_STATUS,&abilities));

        if (abilities & (1<<12))
        {
            *phyHasMacsec = MTD_FALSE;
        }

        if (abilities & (1<<13))
        {
            *phyHasCopperInterface = MTD_FALSE;
        }       

        if (MTD_IS_E21X0_BASE(baseType))
        {
            /* check device capabilities, see if 5G and below device or 2.5G and below device  */
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_TUNIT_IEEE_PMA_SPEED_CAP,&abilities));

            if (abilities & (1<<14))
            {
                *is5GDevice = MTD_TRUE;
            }
            else 
            {
                if (abilities & (1<<13))
                {
                    *is2P5GDevice = MTD_TRUE;
                }
            }
        }
        else 
        {
            if (abilities & (1<<14))
            {
                *is5GDevice = MTD_TRUE;
            }
        }

    }

    return MTD_OK;
}


MTD_STATUS mtdIsPhyRevisionValid
(
    IN MTD_DEVICE_ID phyRev
)
{
    switch(phyRev)
    {
        /* list must match MTD_DEVICE_ID */
        case MTD_REV_3240P_Z2:
        case MTD_REV_3240P_A0:
        case MTD_REV_3240P_A1:
        case MTD_REV_3220P_Z2:
        case MTD_REV_3220P_A0:

        case MTD_REV_3240_Z2:
        case MTD_REV_3240_A0:
        case MTD_REV_3240_A1:
        case MTD_REV_3220_Z2:
        case MTD_REV_3220_A0:

        case MTD_REV_3310P_A0:
        case MTD_REV_3320P_A0:
        case MTD_REV_3340P_A0:
        case MTD_REV_3310_A0:
        case MTD_REV_3320_A0:
        case MTD_REV_3340_A0:

        case MTD_REV_E2010P_A0:
        case MTD_REV_E2020P_A0:
        case MTD_REV_E2040P_A0:
        case MTD_REV_E2010_A0:
        case MTD_REV_E2020_A0:
        case MTD_REV_E2040_A0:

        case MTD_REV_2340P_A0:
        case MTD_REV_2340_A0:

        case MTD_REV_3310P_A1:
        case MTD_REV_3320P_A1:
        case MTD_REV_3340P_A1:
        case MTD_REV_3310_A1:
        case MTD_REV_3320_A1:
        case MTD_REV_3340_A1:

        case MTD_REV_E2010P_A1:
        case MTD_REV_E2020P_A1:
        case MTD_REV_E2040P_A1:
        case MTD_REV_E2010_A1:
        case MTD_REV_E2020_A1:
        case MTD_REV_E2040_A1:            

        case MTD_REV_2340P_A1:
        case MTD_REV_2340_A1:
            
        case MTD_REV_2320P_A0:
        case MTD_REV_2320_A0:

        case MTD_REV_3310P_B0:
        case MTD_REV_3320P_B0:
        case MTD_REV_3340P_B0:
        case MTD_REV_3310_B0:
        case MTD_REV_3320_B0:
        case MTD_REV_3340_B0:

        case MTD_REV_E2010P_B0:
        case MTD_REV_E2020P_B0:
        case MTD_REV_E2040P_B0:
        case MTD_REV_E2010_B0:
        case MTD_REV_E2020_B0:
        case MTD_REV_E2040_B0:

        case MTD_REV_2340P_B0:
        case MTD_REV_2340_B0:

        case MTD_REV_3310P_B1:
        case MTD_REV_3320P_B1:
        case MTD_REV_3340P_B1:
        case MTD_REV_3310_B1:
        case MTD_REV_3320_B1:
        case MTD_REV_3340_B1:

        case MTD_REV_E2010P_B1:
        case MTD_REV_E2020P_B1:
        case MTD_REV_E2040P_B1:
        case MTD_REV_E2010_B1:
        case MTD_REV_E2020_B1:
        case MTD_REV_E2040_B1:

        case MTD_REV_2340P_B1:
        case MTD_REV_2340_B1:

        case MTD_REV_E2110_Z1:
        case MTD_REV_E2110_A0:
        case MTD_REV_E2111_A0:            
        case MTD_REV_E2140_A0:
        case MTD_REV_E2180_A0:
        case MTD_REV_E2180_A1:
        case MTD_REV_E2181_A1:            

        case MTD_REV_X3540_A0:
        case MTD_REV_X3540_B0:
        case MTD_REV_E2540_A0:
        case MTD_REV_E2540_B0: 
        case MTD_REV_X3580_A0:
        case MTD_REV_E2580_A0:
            return MTD_OK;
            break;

        /* unsupported PHYs */    
        case MTD_REV_2340P_A1_NA:
        case MTD_REV_2340_A1_NA:
        default:
            return MTD_FAIL; /* is either MTD_REV_UNKNOWN or not in the above list */
            break;
    }
        
    return MTD_FAIL; /* shouldn't come here*/
}


MTD_STATUS deviceFamilyValid
(
    IN MTD_DEVICE_ID phyRev
)
{
    if (MTD_IS_X32X0_BASE(phyRev) ||  MTD_IS_X33X0_BASE(phyRev) || MTD_IS_E21X0_BASE(phyRev) || MTD_IS_X35X0_BASE(phyRev))
    {
        return MTD_OK;
    }
    else
    {
        return MTD_FAIL;
    }
}

/* Decodes the header and return MTD_TRUE if the checksum is good. 
   Returns MTD_FALSE if the checksum is bad. */
MTD_BOOL mtdGetHeader(MTD_FILE_HEADER_TYPE *hdr, MTD_U8 *buf)
{
    MTD_U16 calcdChecksum = 0, hdrChecksum;
    MTD_U16 hdrSizeInBytes = MTD_HEADER_SIZE - MTD_CKSUM_OFFSET; /* excludes header checksum */
    MTD_U8 *bufPtr = buf;

    /* First make sure it's a good header */
    while(hdrSizeInBytes--)
    {
        calcdChecksum += *bufPtr++;
    }
    calcdChecksum = (~calcdChecksum);
    mtdGetU16fromBuf(&hdrChecksum, &buf[MTD_CKSUM_OFFSET]);

    if (hdrChecksum != calcdChecksum)
    {
        return MTD_FALSE;
    }

    /* Now fill in the data */
    mtdGetU32fromBuf(&(hdr->dataLength), buf);      buf += 4;
    mtdGetU32fromBuf(&(hdr->dataDestination), buf); buf += 4;
    mtdGetU16fromBuf(&(hdr->secChecksum), buf);     buf += 2;
    hdr->data_only = ((*buf++) & 0x40)>>6;
    hdr->port_skip = *buf++;
    mtdGetU32fromBuf(&(hdr->nextHeaderOffset), buf);

    return MTD_TRUE;
    
}


/********************************************************************/
/* Local functions */
/********************************************************************/

/* Gets little endian unsigned long */
MTD_STATIC void mtdGetU32fromBuf(MTD_U32 *num, MTD_U8 *buf)
{
    unsigned counter;

    for(*num = 0, counter = 0; counter < 4; counter++)
    {
        *num += (((MTD_U32)(buf[counter]))<<(counter*8));
    }
}

/* Gets little endian unsigned short */
MTD_STATIC void mtdGetU16fromBuf(MTD_U16 *num, MTD_U8 *buf)
{
    unsigned counter;
    
    for(*num = 0, counter = 0; counter < 2; counter++)
    {
        *num += (((MTD_U16)(buf[counter]))<<(counter*8));
    }   
}



/* Starts the code downloaded skipping any relative ports indicated in the portsSkip bitmask. */
/* Checks if code started, if any port didn't start erroredPorts contain MTD_APP_CODE_DID_NOT_START */
/* If waitDone is MTD_TRUE will wait for the code to be done and reset the T unit processor to clear the memory */
/* if there's a timeout on the code to be done, it will set erroredPorts to MTD_EXECECUTABLE_TIMED_OUT */
/* If waitDone is MTD_FALSE doesn't wait, just returns. */
/* Set portsSkip = 0 and waitDone = MTD_FALSE to start all the ports in ports[] and just return. */
MTD_STATIC MTD_STATUS mtdStartMultipleAppCode(MTD_DEV_PTR devPtr, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 portsSkip, MTD_BOOL waitDone, MTD_U16 sectionCount, MTD_U16 erroredPorts[], MTD_U16 *errCode)
{
    MTD_U16 localPorts[MTD_MAX_PORTS_TO_DOWNLOAD],  /* list of MDIO port numbers to download to (not skipped) */
            portsIndexes[MTD_MAX_PORTS_TO_DOWNLOAD], /* for each port number in localPorts[] contains original location in ports[] */
            localNumPorts; /* number of ports stored in the first two arrays above */
    
    MTD_U16 i, basePort, relativePort;
    MPD_UNUSED_PARAM(sectionCount);
    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }
    /* Skip any ports in the portsSkip bitmask */
    for (i=0, localNumPorts=0; i < numPorts; i++)
    {
        basePort = ((ports[i]/(devPtr->numPortsPerDevice))*(devPtr->numPortsPerDevice));
        relativePort = ports[i] - basePort;
        if (!(portsSkip & (1<<relativePort)))
        {
            localPorts[localNumPorts] = ports[i]; /* this port should be processed (not skipped) */
            portsIndexes[localNumPorts] = i; /* store original location to avoid searching later to be used to index erroredPorts[] */
            localNumPorts++;
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("Section %d skipped for port %d for starting app code\n", (int)sectionCount, (int)ports[i]);
#endif
            
        }
    }
    /* localPorts[] is a list of ports to be taken action on, all of the portsSkip have been removed */

    for (i=0; i < localNumPorts; i++)
    {
        /* Now start code which was downloaded */
        TRY_IO(mtdHwSetPhyRegField(devPtr,localPorts[i],MTD_BOOT_STATUS_REG,MTD_CODE_DOWNLOADED_POS,1,1));        
    }
    
    MTD_ATTEMPT(mtdWait(devPtr,200)); /* Give app code time to start */

    for (i=0; i < localNumPorts; i++)
    {
        MTD_BOOL appStarted;
        
        /* make sure the app code started */
        TRY_IO(mtdDidPhyAppCodeStart(devPtr,localPorts[i],&appStarted));


        if (appStarted == MTD_FALSE)
        {

            MTD_DBG_ERROR("App code did not start on port %d.\n", (int)localPorts[i]);
            MTD_DBG_ERROR("App download failed.\n");
            
            if (erroredPorts[portsIndexes[i]] == 0)
            {
                erroredPorts[portsIndexes[i]] = MTD_APP_CODE_DID_NOT_START;
            }
            *errCode = MTD_APP_CODE_DID_NOT_START;
            return MTD_FAIL;
        }
        else
        {
#if DB_TEST_FLASHCODE
            MTD_DBG_INFO("App code started section %d on port %d.\n", (int)sectionCount, (int)localPorts[i]);
#endif
        }                
    }

    if (waitDone)
    {
        MTD_BOOL timedOut=MTD_FALSE;
        /* wait for all the ports to indicate they are done */

        i=0;
        while(!timedOut && i < localNumPorts)
        {
            /* There's another section to process coming, wait until the executable is done */
            /* This is assuming the executable sets mode/state in 1.C001 to 0xB and 6 respectively when it's done. */
            /* This code assumes the first executable is the serdes loader which sets this status when done. */
            if (erroredPorts[portsIndexes[i]] == 0)
            {
                if (mtdHwWaitForRegFieldValue(devPtr, localPorts[i], MTD_TUNIT_XG_EXT_STATUS, 0, 12, MTD_SERDES_LOADER_DONE, MTD_EXECUTABLE_TO) == MTD_FAIL)
                {
                    timedOut = MTD_TRUE;
                    erroredPorts[portsIndexes[i]] = MTD_EXECECUTABLE_TIMED_OUT;
                    *errCode = MTD_EXECECUTABLE_TIMED_OUT;
                    return MTD_FAIL;
                }
            }
            
            i++;
        }
        
#if DB_TEST_FLASHCODE
        MTD_DBG_INFO("App code finished on section %d on all ports not skipped.\n", (int)sectionCount);
#endif

        /* all are done executing, now reset and put in download mode again to clear memory for another download */
        /* reset all ports including the ones that were skipped - if you don't want this behavior this gets more complicated */
        /* this assumes you don't want to download some data to some ports, then execute on some and leave the data in the others */
        /* which is our usage case for X3540 - it doesn't hurt to clear all the ports since the ones being skipped are completely skipped */
        MTD_ATTEMPT(mtdPutMultipleDownloadMode(devPtr, ports, numPorts, erroredPorts, errCode));
    }

    return MTD_OK;
}

/* Puts multiple ports in MDIO download mode. Returns MTD_FAIL if any port fails. */
/* Checks if all ports are in MDIO download mode. Returns MTD_FAIL if any port failed and erroredPorts[] for that */
MTD_STATIC MTD_STATUS mtdPutMultipleDownloadMode(MTD_DEV_PTR devPtr, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 erroredPorts[], MTD_U16 *errCode)
{
    MTD_U32 i;
    MTD_BOOL downloadMode;
    MTD_STATUS status = MTD_OK;;

    if (numPorts > MTD_MAX_PORTS_TO_DOWNLOAD) {
    	return MTD_FAIL;
    }

    for (i = 0; i < numPorts; i++)
    {
        TRY_IO(mtdPutPhyInMdioDownloadMode(devPtr,ports[i])); 
    }

    /* Allow reset to complete */
    MTD_ATTEMPT(mtdWait(devPtr, 50)); /* if fails, returns errCode of 0 */   

    /* Make sure we can access the device
      And it's in the correct mode (waiting for download) */
    for(i = 0; i < numPorts; i++)
    {
        TRY_IO(mtdIsPhyInMdioDownloadMode(devPtr,ports[i],&downloadMode));
        
        if (downloadMode == MTD_FALSE)
        {
            MTD_DBG_ERROR( "T unit in port %d is not in waiting on download mode.\n",(int)ports[i]);
            MTD_DBG_ERROR( "Download failed on MDIO port %d\n",(int)ports[i]);
            erroredPorts[i] = MTD_PHY_NOT_IN_DOWNLOAD_MODE;
            status = MTD_FAIL;
        }
    } 

    if (status == MTD_FAIL)
    {
        *errCode = MTD_PHY_NOT_IN_DOWNLOAD_MODE;
    }

    return status;
}


#if DB_TEST_FLASHCODE
/******************************************
 Program host most likely does not need
******************************************/
void amuse_user(void)
{
    static unsigned dot_counter;

    dot_counter++;
    if (dot_counter%2)
    {
        MTD_DBG_INFO(".");
    }
}

/***********************************************************************************
   Function most likely not needed by host (from the ram download)
***********************************************************************************/
MTD_STATIC void amuse_user_ram(MTD_U32 buffCount)
{
    static unsigned colCount;
    if ((buffCount%512)==0)
    {
        MTD_DBG_INFO(".");
        colCount++;
        if ((colCount%10)== 0)
        {
            MTD_DBG_INFO(" %d bytes\n", buffCount);
        }
    }
}
#endif


