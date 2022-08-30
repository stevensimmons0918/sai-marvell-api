/*******************************************************************************
Copyright (C) 2014 - 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/***********************************************************************
This file contains sample functions code for calling the driver initialization
and loading firmware/ROM images to the Marvell X7120/X6181/X6141 PHY.

For Reference Only.
***********************************************************************/

#include "mydApiTypes.h"
#include "mydAPI.h"
#include "mydHwCntl.h"
#include "mydUtils.h"
#include "mydApiRegs.h"
#include "mydFwDownload.h"
#include "mydInitialization.h"
#include "mydHwSerdesCntl.h"
#include "mydDiagnostics.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MYD_ROM_TYPE            1  /* Serdes/SBus ROM file type */
#define MYD_FIRMWARE_TYPE       2  /* Z80 firmware file type */
#define MYD_Z80_EEPROM_FW_TYPE  3  /* EEPROM Z80 firmware file type */

/* rename these files and locations to match the image filename */
#define MYD_Z80_IMAGE  "..//Scripts//z80_firmware.txt"
#define MYD_Z80_EEPROM_IMAGE  "..//Scripts//z80_EEPROM_firmware.txt"
#define MYD_SBUS_IMAGE "..//Scripts//sbus_master.swap.rom"
#define MYD_SERDES_IMAGE "..//Scripts//serdes.rom"

#if 0
/******************************************************************************
 MYD_STATUS mydSampleLoadImageFile
*******************************************************************************/
MYD_STATUS mydSampleLoadImageFile
(
    IN MYD_U32 fileType,
    IN const MYD_U8 *filename,   /* Full path to a valid firmware image */
    OUT MYD_U16 *rom_size,       /* Address to receive length of image */
    OUT MYD_U16 **rom_ptr        /* Address to receive image */
)
{
    MYD_U16 *rom, addr = 0;
    MYD_U8 mem_buffer[6];
    MYD_U32 filesize;

    FILE *file;
    if (!filename)
        return MYD_FAIL;

    file = fopen(filename, "r");

    *rom_ptr = 0;
    if( !file )
    {
        *rom_size = 0;
        return MYD_FAIL;
    }

    fseek(file, 0, SEEK_END);

    if (fileType == MYD_ROM_TYPE || fileType == MYD_FIRMWARE_TYPE)
    {
        filesize = (MYD_U32)ftell(file);
        *rom_size = (MYD_U16)(filesize / 4);
    }
    else if (fileType == MYD_Z80_EEPROM_FW_TYPE)
    {
        filesize = (MYD_U32)ftell(file);
        *rom_size = (MYD_U16)(filesize / 3);
    }
    else
    {
        *rom_size = 0;
        fclose(file);
        return MYD_FAIL;
    }

    rewind(file);
    if( *rom_size <= 0 )
    {
        *rom_size = 0;
        fclose(file);
        return MYD_FAIL;
    }

    rom = (MYD_U16*)malloc(sizeof(MYD_U16) * (*rom_size + 2));
    if( !rom )
    {
        *rom_size = 0;
        fclose(file);
        return MYD_FAIL;
    }

    while( fgets(mem_buffer, 6, file) )
    {
        MYD_U8 *ptr;
        rom[addr] = (MYD_U16)strtol(mem_buffer, &ptr, 16);

        if (fileType == MYD_ROM_TYPE)
        {
            if( ptr != mem_buffer+3 && (ptr != mem_buffer+4 || mem_buffer[3] != '\r') )
            {
                fclose(file);
                free(rom);
                MYD_DBG_ERROR("mydSampleLoadImageFile: Incorrect file format for Serdes/SBus image \n");
                return MYD_FAIL;
            }
        }

        if (fileType == MYD_FIRMWARE_TYPE)
        {
            if( ptr != mem_buffer+4 && (ptr != mem_buffer+5 || mem_buffer[4] != '\n') )
            {
                fclose(file);
                free(rom);
                MYD_DBG_ERROR("mydSampleLoadImageFile: Incorrect file format for Z80 MDC/MDIO image \n");
                return MYD_FAIL;
            }
        }

        if (fileType == MYD_Z80_EEPROM_FW_TYPE)
        {
            if( ptr != mem_buffer+2 && (ptr != mem_buffer+3 || mem_buffer[2] != '\n') )
            {
                fclose(file);
                free(rom);
                MYD_DBG_ERROR("mydSampleLoadImageFile: Incorrect file format for Z80 EEPROM image \n");
                return MYD_FAIL;
            }
        }

        addr ++;
    }
    *rom_size = addr;

    rom[*rom_size+0] = 0;
    rom[*rom_size+1] = 0;

    fclose(file);
    *rom_ptr = rom;
    return MYD_OK;
}


/******************************************************************************
 MYD_STATUS mydSampleUnloadDrv
*******************************************************************************/
MYD_STATUS mydSampleUnloadDrv
(
    IN MYD_DEV_PTR pDev
)
{
    MYD_STATUS status = MYD_FAIL;

    if (pDev && (pDev->devEnabled == MYD_TRUE))
    {
        status = mydUnloadDriver(pDev);
    }

    if(pDev->hostContext)
    {
        free(pDev->hostContext);
    }

    if (status == MYD_OK)
    {
        MYD_DBG_INFO("mydSampleUnloadDrv: Free Dev structure\n");
    }
    else
    {
        MYD_DBG_ERROR("mydSampleUnloadDrv: mydUnloadDriver failed\n");
    }

    return MYD_OK;
}

/******************************************************************************
 MYD_STATUS mydSampleInitDrv

 MDIO readMdio and writeMdio functions provided in below format required to be
 passed into MYD driver initialization.

 typedef MYD_STATUS (*FMYD_READ_MDIO)(MYD_DEV_PTR pDev, MYD_U16 mdioPort,
                                      MYD_U16 mmd, MYD_U16 reg, MYD_U16* value);

 typedef MYD_STATUS (*FMYD_WRITE_MDIO)(MYD_DEV_PTR pDev, MYD_U16 mdioPort,
                                       MYD_U16 mmd, MYD_U16 reg, MYD_U16 value);
*******************************************************************************/
MYD_STATUS mydSampleInitDrv
(
    IN MYD_PVOID pHostContext,  /* optional host context */
    IN MYD_U16 mdioFirstPort,   /* 1st MDIO number on the device  */
    IN MYD_BOOL loadImage,      /* 1:load image; 0:do not load */
    IN FMYD_READ_MDIO  readMdio,
    IN FMYD_WRITE_MDIO writeMdio,
    OUT MYD_DEV_PTR pDev        /* MYD device to be used in all API calls */
)
{
    MYD_U16 *pZ80Image;
    MYD_U16 z80Size;
    MYD_U16 *pBusMasterImage;
    MYD_U16 busMasterSize;
    MYD_U16 *pSerdesImage;
    MYD_U16 serdesSize;
    MYD_STATUS retVal = MYD_OK;

    mydMemSet(pDev, 0, (sizeof(MYD_DEV)));

    pDev->devEEPROM = MYD_TRUE;

    if (loadImage)
    {
        /* Not all devices require Z80 f/w */
        mydSampleLoadImageFile(MYD_FIRMWARE_TYPE, MYD_Z80_IMAGE, &z80Size, &pZ80Image);

        mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SBUS_IMAGE, &busMasterSize, &pBusMasterImage);
        mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SERDES_IMAGE, &serdesSize, &pSerdesImage);

        MYD_DBG_INFO("mydSampleUnloadDrv: z80Size:%u busMasterSize:%u serdesSize:%u\n", z80Size, busMasterSize, serdesSize);

        /* Not all devices require Z80 f/w; Set either z80Size=0 or pZ80Image=NULL to skip Z80 download  */
        retVal = mydInitDriver(readMdio, writeMdio, mdioFirstPort, pZ80Image, z80Size, pBusMasterImage,
                               busMasterSize, pSerdesImage, serdesSize, pHostContext, pDev);

        if (pZ80Image)
        {
            free(pZ80Image);
        }
        if (pBusMasterImage)
        {
            free(pBusMasterImage);
        }
        if (pSerdesImage)
        {
            free(pSerdesImage);
        }
    }
    else
    {
        /* initialized the dev structure without loading the ROMs and firmware */
        retVal = mydInitDriver(readMdio, writeMdio, mdioFirstPort, NULL, 0, NULL, 0, NULL, 0,
                               pHostContext, pDev);
    }

    return retVal;
}

/******************************************************************************
 MYD_STATUS mydSampleReloadDriver

 This sample function shows how to reload the driver with the option to read the
 mode register of all lanes and refill the config info. Refer to the MYD_RELOAD_xxx
 definition for more options.

 This reload driver API can be used when the driver was previous unloaded.  The
 reload driver will preserve the exiting configured modes and continue with the
 existing operations without disruption to the configuration.

 Note: mydReloadDriver do not perform resets or firmware download to the device.
*******************************************************************************/
MYD_STATUS mydSampleReloadDriver
(
    IN MYD_U16 mdioPort, /* 1st MDIO port address on this device */
    IN MYD_PVOID pHostContext,
    OUT MYD_DEV_PTR pDev
)
{
    MYD_STATUS retVal = MYD_OK;
    MYD_U16 reloadOption = 0;

    if (!pDev)
    {
        MYD_DBG_ERROR("mydSampleReloadDriver: Error - Null pDev pointer\n");
        return MYD_FAIL;
    }

    mydMemSet(pDev, 0, (sizeof(MYD_DEV)));

    retVal = mydReloadDriver((FMYD_READ_MDIO)NULL, (FMYD_WRITE_MDIO)NULL,
                             mdioPort, pHostContext, (reloadOption|MYD_RELOAD_CONFIG), pDev);

    if (retVal == MYD_OK)
    {
        MYD_U16 portIndex, laneOffset;

        MYD_DBG_INFO("pDev->deviceId:%u \n", pDev->deviceId);
        MYD_DBG_INFO("pDev->chipRevision:%u \n", pDev->chipRevision);
        MYD_DBG_INFO("pDev->mdioPort:%u \n", pDev->mdioPort);
        MYD_DBG_INFO("pDev->portCount:%u \n", pDev->portCount);
        MYD_DBG_INFO("pDev->devEnabled:%u \n", pDev->devEnabled);
        MYD_DBG_INFO("pDev->fmydReadMdio:0x%X\n", pDev->fmydReadMdio);
        MYD_DBG_INFO("pDev->fmydWriteMdio:0x%X \n", pDev->fmydWriteMdio);

        for (portIndex=0; portIndex<pDev->portCount; portIndex++)
        {
            for (laneOffset=0; laneOffset<MYD_NUM_LANES; laneOffset++)
            {
                MYD_DBG_INFO("Host Port:%u Lane:%u Mode:%u\n", portIndex, laneOffset,
                                pDev->hostConfig[portIndex][laneOffset].opMode );

                MYD_DBG_INFO("Line Port:%u Lane:%u Mode:%u\n", portIndex, laneOffset,
                                pDev->lineConfig[portIndex][laneOffset].opMode );
            }

            MYD_DBG_INFO("pDev->devInfo[%u]:0x%X \n", portIndex, pDev->devInfo[portIndex]);
#if MYD_ENABLE_SERDES_API
            MYD_DBG_INFO("pDev->serdesDev[%u]:0x%X \n", portIndex, pDev->serdesDev[portIndex]);
#endif
        }
    }
    else
    {
        MYD_DBG_ERROR("mydSampleReloadDriver: mydReloadDriver failed\n");
        return MYD_FAIL;
    }

    return retVal;
}

/******************************************************************************
 MYD_STATUS mydSampleUpdateLoadEEPROM

 Sample function to update the EEPROM with the provided images. Both
 Serdes and SBus images must be provided or all 3 images including the Z80.
 If Z80 image is not provided, set the z80Size size to 0

*******************************************************************************/

MYD_STATUS mydSampleUpdateLoadEEPROM
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16 mdioPort
)
{
    MYD_STATUS retVal = MYD_OK;

    MYD_U16   port = 0;
    MYD_U16   *pZ80Image;
    MYD_U16   z80Size = 0;
    MYD_U16   *pBusMasterImage;
    MYD_U16   busMasterSize = 0;
    MYD_U16   *pSerdesImage;
    MYD_U16   serdesSize = 0;
    MYD_U16   errCode;

    if (!pDev)
    {
        MYD_DBG_ERROR("mydSampleUpdateLoadEEPROM: Error - Null pDev pointer\n");
        return MYD_FAIL;
    }

    /* optional to load Z80; comment out if Z80 is not provided */
    mydSampleLoadImageFile(MYD_Z80_EEPROM_FW_TYPE, MYD_Z80_EEPROM_IMAGE, &z80Size, &pZ80Image);

    mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SBUS_IMAGE, &busMasterSize, &pBusMasterImage);
    mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SERDES_IMAGE, &serdesSize, &pSerdesImage);

    retVal = mydUpdateEEPROMImage(pDev, mdioPort, pBusMasterImage, busMasterSize,
                                  pSerdesImage, serdesSize, pZ80Image, z80Size, &errCode);

    if (pZ80Image)
    {
        free(pZ80Image);
    }
    if (pBusMasterImage)
    {
        free(pBusMasterImage);
    }
    if (pSerdesImage)
    {
        free(pSerdesImage);
    }

    return retVal;
}

/******************************************************************************
 MYD_STATUS mydSampleSetPCSMode

   Sample code to configure the speed and mode on a MDIO port using the
   mydSetModeSelection(). This function provides an option to individually
   select the operational mode for both host and line interfaces.
   If the configuration is successful, the mydCheckPCSLinkStatus() is polled to
   check for the link status.

   To set the host and line interfaces mode, provide the operational modes from the
   MYD_OP_MODE selection to both the hostMode and the lineMode inputs.

   Note: For the Repeater mode, similar calls will be made with the inputs Repeater
   modes on both Host and Line sides. Call the mydCheckRepeaterLinkStatus() for the
   Repeater mode link status.
*******************************************************************************/
MYD_STATUS mydSampleSetPCSMode
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16     mdioPort,
    IN MYD_U16     laneOffset,
    IN MYD_U32     modeOption,
    IN MYD_OP_MODE hostMode,
    IN MYD_OP_MODE lineMode
)
{
    MYD_STATUS status;
    MYD_U16 latchedStatus;
    MYD_U16 i;
    MYD_U16 currentStatus;
    MYD_U16 result;
    MYD_U16 linkCheckDelay;
    MYD_PCS_LINK_STATUS statusDetail;

    /* ** Sample only **: setting for Host MYD_P100UP and Line MYD_P100UP mode */
    hostMode = MYD_P100UP;
    lineMode = MYD_P100UP;

    /* optional MYD_MODE_ICAL_EFFORT_0 setting to speed up linkup time
       when MYD_MODE_FORCE_RECONFIG is set, it re-configures the lane(s) on a port to the new
       mode */
    status = mydSetModeSelection(pDev, mdioPort, laneOffset, hostMode, lineMode,
                                 (modeOption|MYD_MODE_ICAL_EFFORT_0), &result);

    if (status != MYD_OK)
    {
        MYD_DBG_ERROR("mydSampleSetPCSMode: mydSetModeSelection failed with result:0x%x\n", result);
        return MYD_FAIL;
    }

    /* Below polls the link status. Refer to MYD_PCS_LINK_STATUS structure for link status details */
    /* If Repeater mode is configured, call mydCheckRepeaterLinkStatus() to check link status */

    mydMemSet(&statusDetail, 0, sizeof(MYD_PCS_LINK_STATUS));

    linkCheckDelay = 10;

    for (i=0; i<linkCheckDelay; i++)
    {
        status = mydCheckPCSLinkStatus(pDev, mdioPort, laneOffset, &currentStatus, &latchedStatus, &statusDetail);
        if (status == MYD_OK)
        {
            if (currentStatus == MYD_LINK_UP)
            {
                MYD_DBG_INFO("mydSampleSetPCSMode: link is up in %u seconds\n", i);
                /* refer to the MYD_DETAILED_STATUS_TYPE structure for detail link status */
                break;
            }
        }
        mydWait(pDev, 1000);
    }

    if ( i >= linkCheckDelay )
    {
        MYD_DBG_INFO("mydSampleSetPCSMode: link is down\n");
    }

    return status;
}

/******************************************************************************
 MYD_STATUS mydSamplePRBSTest
 Sample code for running the PRBS pattern test.  This test uses 2 devices where
 the pDev0 performs the transmits and pDev1 receives the PRBS pattern signal.
 This sample should be a reference with the steps needed to run the PRBS test.
*******************************************************************************/
MYD_STATUS mydSamplePRBSTest
(
    IN MYD_DEV_PTR pDev0,
    IN MYD_DEV_PTR pDev1,
    IN MYD_U16 mdioPort,
    IN MYD_U16 laneOffset
)
{
    MYD_STATUS status;
    MYD_U16 latchedStatus;
    MYD_U16 i;
    MYD_U16 currentStatus, disableWaitforLock;
    MYD_BOOL prbsLocked;
    MYD_U16 result;
    MYD_OP_MODE hostMode;
    MYD_OP_MODE lineMode;
    MYD_U16 waitTimer;
    MYD_PCS_LINK_STATUS statusDetail;
    MYD_PRBS_SELECTOR_TYPE typePRBS = MYD_PRBS31;

    /* ** Sample only **: setting for Host MYD_P100UP and Line MYD_P100UP mode */
    hostMode = MYD_P100LN;
    lineMode = MYD_P100LN;

    /* When MYD_MODE_FORCE_RECONFIG is set, it re-configures the lane(s) on a port to the new
       mode */
    ATTEMPT(mydSetModeSelection(pDev0, mdioPort, laneOffset, hostMode, lineMode,
                                MYD_MODE_FORCE_RECONFIG, &result));

    ATTEMPT(mydSetModeSelection(pDev1, mdioPort, laneOffset, hostMode, lineMode,
                                MYD_MODE_FORCE_RECONFIG, &result));

    /* Below polls the link status. Refer to MYD_PCS_LINK_STATUS structure for link status details */
    /* If Repeater mode is configured, call mydCheckRepeaterLinkStatus() to check link status */
    mydMemSet(&statusDetail, 0, sizeof(MYD_PCS_LINK_STATUS));

    waitTimer = 10;

    for (i=0; i<waitTimer; i++)
    {
        status = mydCheckPCSLinkStatus(pDev0, mdioPort, laneOffset, &currentStatus, &latchedStatus, &statusDetail);
        if (status == MYD_OK)
        {
            if (currentStatus == MYD_LINK_UP)
            {
                MYD_DBG_INFO("mydSamplePRBSTest: link is up in %u seconds\n", i);
                /* refer to the MYD_DETAILED_STATUS_TYPE structure for detail link status */
                break;
            }
        }
        mydWait(pDev0, 1000);
    }

    if ( i >= waitTimer )
    {
        MYD_DBG_ERROR("mydSamplePRBSTest: link is down\n");
        return MYD_FAIL;
    }

    /* set the Tx and Rx to use the matching PRBS pattern. */
    ATTEMPT(mydSetPRBSPattern(pDev0, mdioPort, MYD_LINE_SIDE, laneOffset, typePRBS, MYD_PRBS_NONE));
    ATTEMPT(mydSetPRBSPattern(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, typePRBS, MYD_PRBS_NONE));

    /* wait for locking before counting */
    disableWaitforLock = 0;
    ATTEMPT(mydSetPRBSWaitForLock(pDev0, mdioPort, MYD_LINE_SIDE, laneOffset, disableWaitforLock));
    ATTEMPT(mydSetPRBSWaitForLock(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, disableWaitforLock));

    /* Device0 sends PRBS pattern; Device1 receives on line side */
    ATTEMPT(mydSetPRBSEnableTxRx(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset,
                                 0, /* Tx Disable */
                                 1, /* Rx Enable  */
                                 typePRBS));
    ATTEMPT(mydSetPRBSEnableTxRx(pDev0, mdioPort, MYD_LINE_SIDE, laneOffset,
                                 1, /* Tx Enable */
                                 0, /* Rx Disable */
                                 typePRBS));

    /* checks for PRBS lock before getting the bit count */
    for (i=0; i<waitTimer; i++)
    {
        ATTEMPT(mydGetPRBSLocked(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, &prbsLocked));
        if (prbsLocked == MYD_TRUE)
        {
            MYD_DBG_ERROR("Waited for %u millseconds for PRBS lock\n", waitTimer*100);
            break;
        }
        mydWait(pDev1, 100);
    }

    if ( i >= waitTimer )
    {
        MYD_DBG_ERROR("mydSamplePRBSTest: PRBS locking fails\n");
        return MYD_FAIL;
    }

    /* reset the PRBS counter */
    ATTEMPT(mydPRBSCounterReset(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset));

    MYD_DBG_INFO("Running PRBS pattern test. Wait for 10 secs... \n");

    /* run the PRBS test for 10 secs; sample only. The actual PRBS test should be much longer to
       gather more accurate data */
    ATTEMPT(mydWait(pDev0, 10000));

    {
        MYD_U64 txBitCount;
        MYD_U64 rxBitCount;
        MYD_U64 rxBitErrorCount;
        double bitErrorRate = 0;

        ATTEMPT(mydGetPRBSCounts(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, typePRBS,
                                 &txBitCount, &rxBitCount, &rxBitErrorCount));

        bitErrorRate = (double)rxBitErrorCount/(double)rxBitCount;
        MYD_DBG_INFO("mydSamplePRBSTest: txBitCount:%llu rxBitCount:%llu rxBitErrorCount:%llu BER:%1.12f\n",
                      txBitCount, rxBitCount, rxBitErrorCount, bitErrorRate);
    }

    /* Disabled the PRBS and do a lane soft reset after completing the PRBS test */
    ATTEMPT(mydSetPRBSEnableTxRx(pDev0, mdioPort, MYD_LINE_SIDE, laneOffset, 0, 0, typePRBS));
    ATTEMPT(mydSetPRBSEnableTxRx(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, 0, 0, typePRBS));

    ATTEMPT(mydLaneSoftReset(pDev0, mdioPort, MYD_LINE_SIDE, laneOffset, waitTimer));
    ATTEMPT(mydLaneSoftReset(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, waitTimer));

    return MYD_OK;
}

/******************************************************************************
 MYD_STATUS mydSampleLoopbackPacketGen
 Sample code for running the PRBS pattern test.  This test uses 2 devices where
 the pDev0 performs the packet generator tx and Rx and the pDev1 link partner
 loopback the packets.  This sample should be a reference with the steps needed
 to run the packet generator and loopback features.
 *******************************************************************************/
MYD_STATUS mydSampleLoopbackPacketGen
(
    IN MYD_DEV_PTR pDev0,
    IN MYD_DEV_PTR pDev1,
    IN MYD_U16 mdioPort,
    IN MYD_U16 laneOffset
)
{
    MYD_STATUS status;
    MYD_U16 latchedStatus;
    MYD_U16 i;
    MYD_U16 currentStatus;
    MYD_U16 result;
    MYD_OP_MODE hostMode;
    MYD_OP_MODE lineMode;
    MYD_U16 waitTimer;
    MYD_PCS_LINK_STATUS statusDetail;

    /* ** Sample only **: setting for Host MYD_P100UP and Line MYD_P100UP mode */
    hostMode = MYD_P100LN;
    lineMode = MYD_P100LN;

    /* optional MYD_MODE_ICAL_EFFORT_0 setting to speed up linkup time
       when MYD_MODE_FORCE_RECONFIG is set, it re-configures the lane(s) on a port to the new
       mode */
    ATTEMPT(mydSetModeSelection(pDev0, mdioPort, laneOffset, hostMode, lineMode,
                                (MYD_MODE_FORCE_RECONFIG|MYD_MODE_ICAL_EFFORT_0), &result));

    ATTEMPT(mydSetModeSelection(pDev1, mdioPort, laneOffset, hostMode, lineMode,
                                (MYD_MODE_FORCE_RECONFIG|MYD_MODE_ICAL_EFFORT_0), &result));

    /* Below polls the link status. Refer to MYD_PCS_LINK_STATUS structure for link status details */
    /* If Repeater mode is configured, call mydCheckRepeaterLinkStatus() to check link status */
    mydMemSet(&statusDetail, 0, sizeof(MYD_PCS_LINK_STATUS));

    waitTimer = 10;

    for (i=0; i<waitTimer; i++)
    {
        status = mydCheckPCSLinkStatus(pDev0, mdioPort, laneOffset, &currentStatus, &latchedStatus, &statusDetail);
        if (status == MYD_OK)
        {
            if (currentStatus == MYD_LINK_UP)
            {
                MYD_DBG_INFO("mydSampleLoopbackPacketGen: link is up in %u seconds\n", i);
                /* refer to the MYD_DETAILED_STATUS_TYPE structure for detail link status */
                break;
            }
        }
        mydWait(pDev0, 1000);
    }

    if ( i >= waitTimer )
    {
        MYD_DBG_ERROR("mydSampleLoopbackPacketGen: link is down\n");
        return MYD_FAIL;
    }

    {
        MYD_U64 rxPktCount, rxByteCount;

        MYD_BOOL readToClear = MYD_FALSE;
        MYD_BOOL dontuseSFDinChecker = MYD_TRUE;
        MYD_U16 pktPatternControl = 0;
        MYD_BOOL generateCRCoff = MYD_FALSE;
        MYD_U16 initialPayload = 0;
        MYD_U16 frameLengthControl = 64;
        MYD_U16 numPktsToSend = 0xFFFF;
        MYD_BOOL randomIPG = MYD_FALSE;
        MYD_U16 ipgDuration = 2;
        MYD_BOOL enableGenerator = MYD_TRUE;
        MYD_BOOL enableChecker = MYD_TRUE;

        /* using above pre-defined parameters, this call will configure the packet generator */
        ATTEMPT(mydConfigurePktGeneratorChecker(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset, readToClear,
                            dontuseSFDinChecker, pktPatternControl, generateCRCoff, initialPayload,
                            frameLengthControl, numPktsToSend, randomIPG, ipgDuration));

        /* reset the packet counter to 0 */
        ATTEMPT(mydPktGeneratorCounterReset(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset));

        /* give it a little time 500ms to reset the counter */
        ATTEMPT(mydWait(pDev1, 500));

        /* counter should be zero after reset */
        ATTEMPT(mydPktGeneratorGetCounter(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset,
                                          MYD_PKT_GET_RX, &rxPktCount, &rxByteCount));

        MYD_DBG_INFO("mydSampleLoopbackPacketGen: counters should be zero - rxPktCount:%u rxByteCount:%u\n",
                      rxPktCount, rxByteCount);

        /* this call enable and starting the packet generator and checker */
        ATTEMPT(mydEnablePktGeneratorChecker(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset,
                                             enableGenerator, enableChecker));

        /* set the remote link partner to loopback the generated packets */
        ATTEMPT(mydSetPCSLineLoopback(pDev0, mdioPort, laneOffset, MYD_SHALLOW_PCS_LB, MYD_ENABLE));

        /* let the packet generator run for 10s with remote link partner loopback */
        ATTEMPT(mydWait(pDev0, 10000));

        /* after 10s runtime, read the packet counter */
        ATTEMPT(mydPktGeneratorGetCounter(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset,
                                          MYD_PKT_GET_RX, &rxPktCount, &rxByteCount));

        MYD_DBG_INFO("mydSampleLoopbackPacketGen: counters shouldn't be zero - rxPktCount:%u rxByteCount:%u\n",
                      rxPktCount, rxByteCount);

        /* Disabled packet gen to stop the traffic */
        ATTEMPT(mydEnablePktGeneratorChecker(pDev1, mdioPort, MYD_LINE_SIDE, laneOffset,
                                             MYD_FALSE, MYD_FALSE));

        /* Disabled loopback when test completed */
        ATTEMPT(mydSetPCSLineLoopback(pDev0, mdioPort, laneOffset, MYD_SHALLOW_PCS_LB, MYD_DISABLE));
    }

    return MYD_OK;
}

/******************************************************************************
 MYD_STATUS mydSampleNIMBSetICal

   Sample code to show how to call the mydNIMBxxx APIs to poll and auto calibrate
   the iCal. This feature is only support in limited number of modes and requires
   the supported Z80 firmware.

*******************************************************************************/
MYD_STATUS mydSampleNIMBSetICal
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16     mdioPort,
    IN MYD_U16     laneOffset,
    IN MYD_U32     modeOption,
    IN MYD_OP_MODE hostMode,
    IN MYD_OP_MODE lineMode
)
{
    MYD_STATUS status;
    MYD_U16 i;
    MYD_U16 currentStatus;
    MYD_U16 result, iCalNIMBStatus;
    MYD_BOOL perLaneStatus;
    MYD_U16 linkCheckDelay;
    MYD_REPEATER_LINK_STATUS statusDetailRepeater;

    /* ** Sample only **: setting for Host MYD_R50U and Line MYD_R50U repeater mode */
    hostMode = MYD_R50U;
    lineMode = MYD_R50U;

    /* optional operations to invert the polarity if needed */
    /* ATTEMPT(mydSetTxPolarity(pDev, mdioPort, MYD_HOST_SIDE, laneOffset, 1, 0)); */
    /* ATTEMPT(mydSetRxPolarity(pDev, mdioPort, MYD_HOST_SIDE, laneOffset, 1, 0)); */
    /* ATTEMPT(mydSetTxPolarity(pDev, mdioPort, MYD_LINE_SIDE, laneOffset, 1, 0)); */
    /* ATTEMPT(mydSetRxPolarity(pDev, mdioPort, MYD_LINE_SIDE, laneOffset, 1, 0)); */

    /* mydNIMBSetICal must be stopped when calling API that issues interrupt calls to the SerDes
       and SBus master. */
    ATTEMPT(mydNIMBGetICalStatus(pDev, mdioPort, laneOffset, &iCalNIMBStatus));
    if (iCalNIMBStatus) /* Stop the iCal polling and auto calibration is started */
    {
        ATTEMPT(mydNIMBSetICal(pDev, mdioPort, laneOffset, 0));
        mydWait(pDev, 500);
    }

    /* optional MYD_MODE_SHORT_CHANNEL setting for short channel configuration.
       Refers to the Mode Option List for options available for more selection */
    status = mydSetModeSelection(pDev, mdioPort, laneOffset, hostMode, lineMode,
                                 MYD_MODE_SHORT_CHANNEL, &result);

    if (status != MYD_OK)
    {
        MYD_DBG_ERROR("mydSampleNIMBSetICal: mydSetModeSelection failed with result:0x%x\n", result);
        return status;
    }

    /* Set the eye range to trigger an iCal calibration; MYD_NIMB_EYE_DEFAULT will use the suggested values */
    ATTEMPT(mydNIMBSetEyeCoeff(pDev, mdioPort, laneOffset, 0, 0, 0, MYD_NIMB_EYE_DEFAULT));

    /* mydNIMBSetICal must be stopped when calling API that issues interrupt calls to the SerDes
       and SBus master. All APIs that required MYD_ENABLE_SERDES_API definition issued interrupt. */
    /* start the iCal polling and auto calibration */
    ATTEMPT(mydNIMBSetICal(pDev, mdioPort, laneOffset, 1));

    /* Below polls the link status. Refer to MYD_REPEATER_LINK_STATUS structure for link status details */
    mydMemSet(&statusDetailRepeater, 0, sizeof(MYD_REPEATER_LINK_STATUS));

    linkCheckDelay = 10;
    perLaneStatus = MYD_FALSE;

    for (i=0; i<linkCheckDelay; i++)
    {
        status = mydCheckRepeaterLinkStatus(pDev, mdioPort, laneOffset, perLaneStatus,
                                            &currentStatus, &statusDetailRepeater);
        if (status == MYD_OK)
        {
            if (currentStatus == MYD_LINK_UP)
            {
                MYD_DBG_INFO("mydSampleNIMBSetICal: link is up in %u seconds\n", i);
                /* refer to the MYD_REPEATER_LINK_STATUS structure for detail link status */
                break;
            }
        }
        mydWait(pDev, 1000);
    }

    if ( i >= linkCheckDelay )
    {
        MYD_DBG_INFO("mydSampleNIMBSetICal: link is down\n");
    }

    return status;
}


#if MYD_ENABLE_SERDES_API
/******************************************************************************
MYD_STATUS mydSampleGetEye
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16 mdioPort,
    IN MYD_U16 host_or_line,
    IN MYD_U16 laneOffset,
    IN MYD_U16 sampleSize,
    IN MYD_U16 highDefinition,
    IN MYD_U16 serdesLogLevel
)

 Inputs:
    pDev - pointer to MYD_DEV initialized by mydInitDriver() call
    host_or_line - which interface is being read:
                   MYD_HOST_SIDE
                   MYD_LINE_SIDE
    laneOffset - 0..3 for lanes 0-3.
    sampleSize - The eye sample size based on the max dwell bit.
                       Range [4 to 8] where x is the exponential
                       e.g. 4 = 10,000; 8 = 100,000,000
    highDefinition - MYD_DISABLE: disable high resolution;
                     MYD_ENABLE: enable high definition
                     The high definition option increases the y resolution to 1 step
    serdesLogLevel - The level of logging in the serdes APIs
                     Range [0 - 15]. Set to 0 when not debugging.

 Outputs:
    None

 Returns:
    Returns MYD_OK if execution is successful, MYD_FAIL otherwise

 Description:
    Sample code to extract the Eye diagram by calling the mydSerdesGetEye API.
    Refer to the mydSerdesGetEye function prototype for details on the parameters.

 Side effects:
    None

 Notes/Warnings:
    The caller to mydSerdesGetEye() must call mydWrapperEyeAllocDataSt() to allocate
    the pEyeData buffer memory before passing in the pEyeData pointer. The caller
    must free the pEyeData by calling mydWrapperEyeFreeDataSt() after processing
    the eye data.

    If pCal is running before plotting the eye, the pCal must be disabled before
    plotting the eye.  After completion, the pCal DFE tuning needs to be re-enabled
    after eye measurement.

*******************************************************************************/
MYD_STATUS mydSampleGetEye
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16 mdioPort,
    IN MYD_U16 host_or_line,
    IN MYD_U16 laneOffset,
    IN MYD_U16 sampleSize,
    IN MYD_U16 highDefinition,
    IN MYD_U16 serdesLogLevel
)
{
    MYD_U16 sAddr;   /* Refer to mydMappedSerdesIDs for more info */
    MYD_SER_EYE_DATA_TYPE *eyeDataBuf = (MYD_SER_EYE_DATA_TYPE*)NULL;
    MYD_STATUS status;
    MYD_BOOL dfeStatusPCalInProgress, dfeStatusRunPCal;
    MYD_BOOL tempVal;
    MYD_U16 i, eyeIndex;
    MYD_U16 waitTimeOut = 10;
    double db;
    double td;
    const double q06 = 4.74;
    const double q10 = 6.36;
    const double q12 = 7.03;
    const double q15 = 7.94;
    const double q17 = 8.49;
    MYD_BOOL pCalConfig = MYD_FALSE; /* Disabled pCal only if pCal is running.
                                        The pCal must be re-enabled after eye plotting */

    serdesLogLevel = 2; /* This log level will print the eye diagram and data */

    if (host_or_line == MYD_HOST_SIDE)
    {
        sAddr = MYD_MAPPED_SERDES_ID(MYD_GET_PORT_IDX(pDev,mdioPort), MYD_HOST_MODE_IDX, laneOffset);
    }
    else
    {
        sAddr = MYD_MAPPED_SERDES_ID(MYD_GET_PORT_IDX(pDev,mdioPort), MYD_LINE_MODE_IDX, laneOffset);
    }

    /* run a status check on the pCal if it's running */
    mydSerdesGetRxDFEStatus(pDev, sAddr, &tempVal, &dfeStatusPCalInProgress,
                        &tempVal, &tempVal, &dfeStatusRunPCal, &tempVal);

    /* Disabled pCal only if pCal is running. The pCal will be re-enabled after eye plotting */
    if (dfeStatusPCalInProgress && dfeStatusRunPCal)
    {
        /* Disabled pCal */
        mydSerdesTune_pCal(pDev, sAddr, MYD_DFE_STOP_ADAPTIVE);
        pCalConfig = MYD_TRUE;

        for (i=0; i<waitTimeOut; i++)
        {
            mydSerdesGetRxDFEStatus(pDev, sAddr, &tempVal, &dfeStatusPCalInProgress,
                                    &tempVal, &tempVal, &dfeStatusRunPCal, &tempVal);

            if ((dfeStatusPCalInProgress == MYD_FALSE) && (dfeStatusRunPCal == MYD_FALSE))
            {
                MYD_DBG_INFO(" ****** pCal disabled in %u00 msec\n", i);
                break;
            }
            mydWait(pDev, 100);
        }

        if (i >= waitTimeOut)
        {
            MYD_DBG_INFO(" ****** pCal FAILED to disabled in %u00 msec\n", i);
        }
    }

    /* call mydWrapperEyeAllocDataSt to malloc the memory for the eye data structure */
    eyeDataBuf = mydWrapperEyeAllocDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[MYD_GET_PORT_IDX(pDev,mdioPort)]);

    status = mydSerdesGetEye(pDev, sAddr, sampleSize, highDefinition, serdesLogLevel, eyeDataBuf);

    /* Re-enabled pCal only if pCal was earlier disabled */
    if (pCalConfig)
    {
        /* run a status check on the pCal before re-enabling it */
        mydSerdesGetRxDFEStatus(pDev, sAddr, &tempVal, &dfeStatusPCalInProgress,
                                &tempVal, &tempVal, &dfeStatusRunPCal, &tempVal);

        mydSerdesTune_pCal(pDev, sAddr, MYD_DFE_START_ADAPTIVE);

        for (i=0; i<waitTimeOut; i++)
        {
            mydSerdesGetRxDFEStatus(pDev, sAddr, &tempVal, &dfeStatusPCalInProgress,
                                    &tempVal, &tempVal, &dfeStatusRunPCal, &tempVal);

            if ((dfeStatusPCalInProgress == MYD_TRUE) && (dfeStatusRunPCal == MYD_TRUE))
            {
                MYD_DBG_INFO(" ****** pCal enabled in %u00 msec\n", i);
                break;
            }
            mydWait(pDev, 100);
        }

        if (i >= waitTimeOut)
        {
            MYD_DBG_INFO(" ****** pCal FAILED to enabled in %u00 msec\n", i);
        }
    }

    MYD_DBG_INFO("x.UI:         %3d\n",eyeDataBuf->ed_x_UI);
    MYD_DBG_INFO("x.resolution: %3d\n",eyeDataBuf->ed_x_resolution);
    MYD_DBG_INFO("x.points:     %3d\n",eyeDataBuf->ed_x_points);
    MYD_DBG_INFO("x.min:        %3d\n",eyeDataBuf->ed_x_min);
    MYD_DBG_INFO("x.max:        %3d\n",eyeDataBuf->ed_x_max);
    MYD_DBG_INFO("x.step:       %3d\n",eyeDataBuf->ed_x_step);
    MYD_DBG_INFO("x.center:     %3d\n",(eyeDataBuf->ed_x_min + eyeDataBuf->ed_x_max) / 2);
    MYD_DBG_INFO("x.width:      %3d\n",eyeDataBuf->ed_width);
    MYD_DBG_INFO("x.width_mUI:  %3d\n",eyeDataBuf->ed_width_mUI);
    MYD_DBG_INFO("\n");

    MYD_DBG_INFO("y.resolution: %3d (DAC points)\n",eyeDataBuf->ed_y_resolution);
    MYD_DBG_INFO("y.points:     %3d\n",eyeDataBuf->ed_y_points);
    MYD_DBG_INFO("y.step:       %3d\n",eyeDataBuf->ed_y_step);
    MYD_DBG_INFO("y.min:        %3d (DAC for y=0)\n",eyeDataBuf->ed_y_min);
    MYD_DBG_INFO("y.mission_points: %3d\n",eyeDataBuf->ed_y_mission_points);
    MYD_DBG_INFO("y.mission_step:   %3d\n",eyeDataBuf->ed_y_mission_step);
    MYD_DBG_INFO("y.mission_min:    %3d (DAC for y=0)\n",eyeDataBuf->ed_y_mission_min);

    MYD_DBG_INFO("y.center:     %3d (DAC middle)\n",eyeDataBuf->ed_y_min + eyeDataBuf->ed_y_points / 2 * eyeDataBuf->ed_y_step);
    MYD_DBG_INFO("y.height:     %3d\n",eyeDataBuf->ed_height);
    MYD_DBG_INFO("y.height_mV:  %3d\n",eyeDataBuf->ed_height_mV);
    MYD_DBG_INFO("\n");

    MYD_DBG_INFO("dc_balance:    %6.2g\n", eyeDataBuf->ed_dc_balance);
    MYD_DBG_INFO("trans_density: %6.2g\n", eyeDataBuf->ed_trans_density);
    MYD_DBG_INFO("error_threshold: %d\n",  eyeDataBuf->ed_error_threshold);
    MYD_DBG_INFO("min_dwell_bits:  %g\n",  eyeDataBuf->ed_min_dwell_bits*1.0);
    MYD_DBG_INFO("max_dwell_bits:  %g\n",  eyeDataBuf->ed_max_dwell_bits*1.0);
    MYD_DBG_INFO("fast_dynamic:    %d\n",  eyeDataBuf->ed_fast_dynamic);
    MYD_DBG_INFO("\n");

    for (eyeIndex=0; eyeIndex<AAPL_MAX_EYE_COUNT; eyeIndex++)
    {
        db = eyeDataBuf->ed_vbtc[eyeIndex].dc_balance;

        if (eyeDataBuf->ed_vbtc[eyeIndex].data_column < 0)
            MYD_DBG_INFO("# Vertical Bathtub Curve (mission data channel %d..%d eye):\n", -1-eyeDataBuf->ed_vbtc[eyeIndex].data_column, 0-eyeDataBuf->ed_vbtc[eyeIndex].data_column);
        else if (eyeDataBuf->ed_vbtc[eyeIndex].data_column == eyeDataBuf->ed_vbtc[eyeIndex].total_columns / 2)
            MYD_DBG_INFO("# Vertical Bathtub Curve (calculated phase center):\n");
        else
            MYD_DBG_INFO("# Vertical Bathtub Curve (calculated phase center %+d):\n", eyeDataBuf->ed_vbtc[eyeIndex].data_column - eyeDataBuf->ed_vbtc[eyeIndex].total_columns / 2);

        if (eyeDataBuf->ed_vbtc[eyeIndex].points)
        {

            if (eyeDataBuf->ed_vbtc[eyeIndex].bottom_R_squared < 0.95 || eyeDataBuf->ed_vbtc[eyeIndex].top_R_squared < 0.95 ||
                eyeDataBuf->ed_vbtc[eyeIndex].bottom_slope <= 0.0 || eyeDataBuf->ed_vbtc[eyeIndex].top_slope >= 0.0)
                MYD_DBG_INFO("#\n# WARNING: This projection is based on measurements with poor correlation.\n#\n");

            MYD_DBG_INFO("BER/%g = Q at eye height of  0 mV: %4.2e, %5.2f\n", db, eyeDataBuf->ed_vbtc[eyeIndex].vert_ber_0mV, eyeDataBuf->ed_vbtc[eyeIndex].height_0mV);
            MYD_DBG_INFO("BER/%g = Q at eye height of 25 mV: %4.2e, %5.2f\n", db, eyeDataBuf->ed_vbtc[eyeIndex].vert_ber_25mV, eyeDataBuf->ed_vbtc[eyeIndex].height_25mV);

            if (eyeDataBuf->ed_vbtc[eyeIndex].Vmean > 0)
                MYD_DBG_INFO("Eye height (Vmean) at %g BER=Q(%4.2f): %3d mV\n",db,0.0,eyeDataBuf->ed_vbtc[eyeIndex].Vmean);

            MYD_DBG_INFO("Eye height at 1e-06 BER/%g = Q(%4.2f): %3d mV\n",db,q06,eyeDataBuf->ed_vbtc[eyeIndex].vert_eye_1e06);
            MYD_DBG_INFO("Eye height at 1e-10 BER/%g = Q(%4.2f): %3d mV\n",db,q10,eyeDataBuf->ed_vbtc[eyeIndex].vert_eye_1e10);
            MYD_DBG_INFO("Eye height at 1e-12 BER/%g = Q(%4.2f): %3d mV\n",db,q12,eyeDataBuf->ed_vbtc[eyeIndex].vert_eye_1e12);
            MYD_DBG_INFO("Eye height at 1e-15 BER/%g = Q(%4.2f): %3d mV\n",db,q15,eyeDataBuf->ed_vbtc[eyeIndex].vert_eye_1e15);
            MYD_DBG_INFO("Eye height at 1e-17 BER/%g = Q(%4.2f): %3d mV\n",db,q17,eyeDataBuf->ed_vbtc[eyeIndex].vert_eye_1e17);
            MYD_DBG_INFO("Slope bottom/top:       %6.2f, %6.2f mV/Q\n", eyeDataBuf->ed_vbtc[eyeIndex].bottom_slope, eyeDataBuf->ed_vbtc[eyeIndex].top_slope);
            MYD_DBG_INFO("X-intercept bottom/top: %6.2f, %6.2f    Q\n", eyeDataBuf->ed_vbtc[eyeIndex].bottom_intercept, eyeDataBuf->ed_vbtc[eyeIndex].top_intercept);
            MYD_DBG_INFO("Indexes bottom/top:     %6u, %6u\n",eyeDataBuf->ed_vbtc[eyeIndex].bottom_index,eyeDataBuf->ed_vbtc[eyeIndex].top_index);
            MYD_DBG_INFO("Est signal noise ratio: %6.2f\n", eyeDataBuf->ed_vbtc[eyeIndex].snr);
            MYD_DBG_INFO("R-squared fit bottom/top:%5.2f, %6.2f\n", eyeDataBuf->ed_vbtc[eyeIndex].bottom_R_squared, eyeDataBuf->ed_vbtc[eyeIndex].top_R_squared);
            MYD_DBG_INFO("No. points bottom/top:      %4u, %7u\n",eyeDataBuf->ed_vbtc[eyeIndex].bottom_points, eyeDataBuf->ed_vbtc[eyeIndex].top_points);
            MYD_DBG_INFO("Indexes bottom/top:         %4u, %7u\n",eyeDataBuf->ed_vbtc[eyeIndex].bottom_index, eyeDataBuf->ed_vbtc[eyeIndex].top_index);
        }

        td = eyeDataBuf->ed_hbtc[eyeIndex].trans_density;

        MYD_DBG_INFO("\n");
        if( eyeDataBuf->ed_hbtc[eyeIndex].data_row == eyeDataBuf->ed_hbtc[eyeIndex].total_rows/2 )
            MYD_DBG_INFO("# Horizontal Bathtub Curve for center eye:\n");
        else
            MYD_DBG_INFO("# Horizontal Bathtub Curve for eye at %+d mV (%+d rows) from center:\n", eyeDataBuf->ed_hbtc[eyeIndex].data_millivolts, eyeDataBuf->ed_hbtc[eyeIndex].data_row-eyeDataBuf->ed_hbtc[eyeIndex].total_rows/2);
        if( eyeDataBuf->ed_hbtc[eyeIndex].points)
        {

            if( eyeDataBuf->ed_hbtc[eyeIndex].left_R_squared < 0.95 || eyeDataBuf->ed_hbtc[eyeIndex].right_R_squared < 0.95 ||
                eyeDataBuf->ed_hbtc[eyeIndex].left_slope <= 0.0 || eyeDataBuf->ed_hbtc[eyeIndex].right_slope >= 0.0 )
                MYD_DBG_INFO("#\n# WARNING: This projection is based on measurements with poor correlation.\n#\n");

            MYD_DBG_INFO("BER/%g = Q at eye width of   0 mUI: %4.2e, %5.2f\n", td, eyeDataBuf->ed_hbtc[eyeIndex].horz_ber_0mUI, eyeDataBuf->ed_hbtc[eyeIndex].width_0mUI);
            MYD_DBG_INFO("BER/%g = Q at eye width of 100 mUI: %4.2e, %5.2f\n", td, eyeDataBuf->ed_hbtc[eyeIndex].horz_ber_100mUI, eyeDataBuf->ed_hbtc[eyeIndex].width_100mUI);
            MYD_DBG_INFO("Eye width at 1e-06 BER/%g = Q(%4.2f): %3d mUI\n", td, q06, eyeDataBuf->ed_hbtc[eyeIndex].horz_eye_1e06);
            MYD_DBG_INFO("Eye width at 1e-10 BER/%g = Q(%4.2f): %3d mUI\n", td, q10, eyeDataBuf->ed_hbtc[eyeIndex].horz_eye_1e10);
            MYD_DBG_INFO("Eye width at 1e-12 BER/%g = Q(%4.2f): %3d mUI\n", td, q12, eyeDataBuf->ed_hbtc[eyeIndex].horz_eye_1e12);
            MYD_DBG_INFO("Eye width at 1e-15 BER/%g = Q(%4.2f): %3d mUI\n", td, q15, eyeDataBuf->ed_hbtc[eyeIndex].horz_eye_1e15);
            MYD_DBG_INFO("Eye width at 1e-17 BER/%g = Q(%4.2f): %3d mUI\n", td, q17, eyeDataBuf->ed_hbtc[eyeIndex].horz_eye_1e17);
            MYD_DBG_INFO("Slope left/right:       %6.2f, %6.2f Q/UI\n", eyeDataBuf->ed_hbtc[eyeIndex].left_slope, eyeDataBuf->ed_hbtc[eyeIndex].right_slope);
            MYD_DBG_INFO("Y-intercept left/right: %6.2f, %6.2f    Q\n", eyeDataBuf->ed_hbtc[eyeIndex].left_intercept, eyeDataBuf->ed_hbtc[eyeIndex].right_intercept);
            MYD_DBG_INFO("Indexes left/right:     %6u, %6u\n",eyeDataBuf->ed_hbtc[eyeIndex].left_index,eyeDataBuf->ed_hbtc[eyeIndex].right_index);
            MYD_DBG_INFO("R-squared fit left/right:%5.2f, %6.2f\n", eyeDataBuf->ed_hbtc[eyeIndex].left_R_squared, eyeDataBuf->ed_hbtc[eyeIndex].right_R_squared);
            MYD_DBG_INFO("Est RJrms left/right:   %7.3f, %6.3f mUI\n",  eyeDataBuf->ed_hbtc[eyeIndex].left_rj, eyeDataBuf->ed_hbtc[eyeIndex].right_rj);
            MYD_DBG_INFO("Est DJpp:               %7.3f  mUI\n", eyeDataBuf->ed_hbtc[eyeIndex].dj);
            MYD_DBG_INFO("Est signal noise ratio: %6.2f\n", eyeDataBuf->ed_hbtc[eyeIndex].snr);
        }

        MYD_DBG_INFO("\n\n");
    }

    /* eyeDataBuf must be freed */
    mydWrapperEyeFreeDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[MYD_GET_PORT_IDX(pDev,mdioPort)], eyeDataBuf);

    return status;
}

/******************************************************************************
 static void mydTapReq
    Function used in mydSerdesPMDTrainingLog
*******************************************************************************/

static void mydTapReq(uint request, uint tap, char *str)
{
    if (request & 0x2000)
        strcpy(str,(tap > 0) ? " " : "PRESET");
    else if (request & 0x1000)
        strcpy(str,(tap > 0) ? " " : "INITIALIZE");
    else
        switch( (request >> (tap*2)) & 0x3 )
        {
        case 0  : {strcpy(str,"HOLD"); break;}
        case 1  : {strcpy(str,"INC");  break;}
        case 2  : {strcpy(str,"DEC");  break;}
        default : {strcpy(str,"RSVD"); break;}
        }
}

/******************************************************************************
 MYD_STATUS mydSamplePMDTrainingLog

   Sample code to extract the Training Log by calling the mydSerdesPMDTrainingLog
   API. Refer to the mydSerdesPMDTrainingLog function prototype for details on
   the parameters.

    Inputs:
    pDev - pointer to MYD_DEV initialized by mydInitDriver() call
    sAddr - SerDes lane addr. Range[1-(8*portCount)];
            Refer to SerdesID and mydMappedSerdesIDs for more info on the
            mapping of SerdesID and Host/Line lane offset
*******************************************************************************/
MYD_STATUS mydSamplePMDTrainingLog
(
    IN MYD_DEV_PTR pDev,
    IN MYD_U16 sAddr
)
{
    MYD_TRAINING_INFO trainInfo;
    int i,j;
    char rem[3][12];
    char loc[3][12];

    mydSerdesPMDTrainingLog(pDev, sAddr, &trainInfo);

    MYD_DBG_INFO("Rx Eq Metric: %d (0x%04x)\n",trainInfo.rx_metric,trainInfo.rx_metric);
    MYD_DBG_INFO("+------+----------------------+----------------------+\n");
    MYD_DBG_INFO("| %4s | %20s | %20s |\n"," ","Remote Tx Requests","Local Tx Requests");
    MYD_DBG_INFO("| %4s | %6s %6s %6s | %6s %6s %6s |\n","REQ","PRE","CURSOR","POST","PRE","CURSOR","POST");
    MYD_DBG_INFO("+------+----------------------+----------------------+\n");
    MYD_DBG_INFO("| %4s | %6d %6s %6s | %6d %6s %6s |\n",
        "PRE",
        trainInfo.remote.preset, " ", " ",
        trainInfo.local.preset, " ", " ");
    MYD_DBG_INFO("| %4s | %6d %6s %6s | %6d %6s %6s |\n",
        "INIT",
        trainInfo.remote.initialize, " ", " ",
        trainInfo.local.initialize, " ", " ");
    MYD_DBG_INFO("| %4s | %6d %6d %6d | %6d %6d %6d |\n",
        "INC",
        trainInfo.remote.tap[0].inc,
        trainInfo.remote.tap[1].inc,
        trainInfo.remote.tap[2].inc,
        trainInfo.local.tap[0].inc,
        trainInfo.local.tap[1].inc,
        trainInfo.local.tap[2].inc);
    MYD_DBG_INFO("| %4s | %6d %6d %6d | %6d %6d %6d |\n",
        "DEC",
        trainInfo.remote.tap[0].dec,
        trainInfo.remote.tap[1].dec,
        trainInfo.remote.tap[2].dec,
        trainInfo.local.tap[0].dec,
        trainInfo.local.tap[1].dec,
        trainInfo.local.tap[2].dec);
    MYD_DBG_INFO("| %4s | %6d %6d %6d | %6d %6d %6d |\n",
        "MAX",
        trainInfo.remote.tap[0].max,
        trainInfo.remote.tap[1].max,
        trainInfo.remote.tap[2].max,
        trainInfo.local.tap[0].max,
        trainInfo.local.tap[1].max,
        trainInfo.local.tap[2].max);
    MYD_DBG_INFO("| %4s | %6d %6d %6d | %6d %6d %6d |\n",
        "MIN",
        trainInfo.remote.tap[0].min,
        trainInfo.remote.tap[1].min,
        trainInfo.remote.tap[2].min,
        trainInfo.local.tap[0].min,
        trainInfo.local.tap[1].min,
        trainInfo.local.tap[2].min);
    MYD_DBG_INFO("+------+----------------------+----------------------+\n");
    for (i=0; i<8; i++)
    {
        for (j=0; j<3; j++)
        {
            mydTapReq(trainInfo.last_remote_request[i],j,rem[j]);
            if (i > 0)
                strcpy(loc[j]," ");
            else
                mydTapReq(trainInfo.last_local_request,j,loc[j]);
        }
        MYD_DBG_INFO("| %4s | %6s %6s %6s | %6s %6s %6s |\n",
            "CMD",
            rem[0],
            rem[1],
            rem[2],
            loc[0],
            loc[1],
            loc[2]);
    }
    MYD_DBG_INFO("+------+----------------------+----------------------+\n");

    return MYD_OK;
}


#endif

#endif
