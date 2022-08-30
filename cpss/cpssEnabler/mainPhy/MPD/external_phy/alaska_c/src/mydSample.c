/*******************************************************************************
Copyright (C) 2014 - 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/***********************************************************************
This file contains sample functions code for calling the driver initialization
and loading firmware/ROM images to the Marvell X7120/X6181/X6141/X6142 PHY.

For Reference Only. 
***********************************************************************/

#include "mydApiTypes.h"
#include "mydApi.h"
#include "mydHwCntl.h"
#include "mydUtils.h"
#include "mydApiRegs.h"
#include "mydFwDownload.h"
#include "mydInitialization.h"
#include "mydHwSerdesCntl.h"
#include "mydDiagnostics.h"
#include "mydIntrIOConfig.h"

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

        if(pDev->hostContext)
        {
        	free(pDev->hostContext);
        }
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

    pDev->devEEPROM = MYD_TRUE; /* set to MYD_TRUE if EEPROM is available; default is MYD_FALSE */

    if (loadImage)
    {
        /* Not all devices require Z80 f/w */
        mydSampleLoadImageFile(MYD_FIRMWARE_TYPE, MYD_Z80_IMAGE, &z80Size, &pZ80Image);

        mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SBUS_IMAGE, &busMasterSize, &pBusMasterImage);
        mydSampleLoadImageFile(MYD_ROM_TYPE, MYD_SERDES_IMAGE, &serdesSize, &pSerdesImage);

        MYD_DBG_INFO("mydSampleInitDrv: z80Size:%u busMasterSize:%u serdesSize:%u\n", z80Size, busMasterSize, serdesSize);

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

   Sample includes the optional MYD_MODE_OPTION_PARAMS before calling mydSetModeSelection().
   Mode options include supporting long channel, short channel, temperature ramping, etc...

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

    /* optional MYD_MODE_ICAL_EFFORT_0 modeOption to speed up linkup time
       when MYD_MODE_FORCE_RECONFIG is set, it re-configures the lane(s) on a port to the new 
       mode */
    status = mydSetModeSelection(pDev, mdioPort, laneOffset, hostMode, lineMode, 
                                 (modeOption|MYD_MODE_FORCE_RECONFIG|MYD_MODE_ICAL_EFFORT_0),
                                 &result);

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
 MYD_STATUS mydSampleSetModeOption

   Sample code to show how to set the advance mode options for customized configuration.
   The advance mode setting options are configured before calling the 
   mydSetModeSelection(). These advance mode options allow customized initialization of
   various tunning options.

   Below are the mode options available:
   MYD_MODE_FORCE_RECONFIG    ** force override current operational mode;
                                 this option needs to be set when switching 
                                 between modes of different number of lane used.  
                                 e.g. changing from a 2-lane P100UP to a 1-lane 
                                 P50UP on the same port and laneOffset 
   
                              ** iCal Effort 0.5 default if iCal option bit 1:2 not set
   MYD_MODE_ICAL_EFFORT_0     ** shorten linkup time for some modes 
   MYD_MODE_ICAL_EFFORT_1     ** improves performance for some modes 
   MYD_MODE_ICAL_EFFORT_0_FV  ** iCal Effort 0 Fixed Vernier
   MYD_MODE_INIT_CTLE         ** Option to set the line/host CTLE HF/LF initial values 
                                 in mode selection.
                                 Refer to xxInitHF/xxInitHF in MYD_MODE_OPTION_STRUCT
                                 for ranges in these values 
   MYD_MODE_CTLE              ** Option to set the line/host CTLE HF/LF in
                                 mode selection.
                                 Refer to xxMaxHF/xxMinLF in MYD_MODE_OPTION_STRUCT
                                 for ranges in these values
   MYD_MODE_TEMPERATURE_RAMP  ** Option to set the Common mode adjustment for
                                 line and/or host sides  
   MYD_MODE_CHANNEL_LEN       ** Long or Short channel
                                 Refer to channel options in MYD_MODE_OPTION_STRUCT
                                 for option available to line/host
   
   Some of these options requires setting the fields in the MYD_MODE_OPTION_STRUCT.
   Refer to the MYD_MODE_OPTION_STRUCT for more information on the field description and
   the ranges within the fields.
*******************************************************************************/
MYD_STATUS mydSampleSetModeOption
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

    /* ** Sample only **: setting for Host MYD_P100QP and Line MYD_P100QP mode */
    hostMode = MYD_P100QP;
    lineMode = MYD_P100QP;

#if 0 /* optional sample setting long channel */
    /* MYD_MODE_CTLE */ 
    pDev->modeParams.hostMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.hostMinLF = MYD_MODE_OPTION_IGNORE;

    pDev->modeParams.lineMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMinHF = 0x5;
    pDev->modeParams.lineMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.lineMinLF = 0x2;

    /* MYD_MODE_TEMPERATURE_RAMP */
    pDev->modeParams.hostTemperatureRampVal = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineTemperatureRampVal = MYD_MODE_OPTION_DEFAULT;

    modeOption = MYD_MODE_CTLE | MYD_MODE_TEMPERATURE_RAMP | MYD_MODE_FORCE_RECONFIG;
#endif

#if 0/* optional sample setting short channel */
    /* MYD_MODE_CTLE */ 
    pDev->modeParams.hostMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.hostMinLF = MYD_MODE_OPTION_IGNORE;

    pDev->modeParams.lineMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.lineMinLF = MYD_MODE_OPTION_IGNORE;

    /* MYD_MODE_CHANNEL_LEN */
    pDev->modeParams.lineChannelLenOption = MYD_CHANNEL_SHORT;
    pDev->modeParams.lineChannelLenVal = MYD_MODE_OPTION_DEFAULT;

    modeOption = MYD_MODE_CTLE | MYD_MODE_CHANNEL_LEN | MYD_MODE_FORCE_RECONFIG;
#endif

   
#if 0  /* optional sample setting tuning the CTLE coefficients */
    /* MYD_MODE_CTLE */ 
    pDev->modeParams.hostMaxHF = 0x6;
    pDev->modeParams.hostMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.hostMinLF = MYD_MODE_OPTION_IGNORE;

    pDev->modeParams.lineMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMinHF = 0x3;
    pDev->modeParams.lineMaxLF = MYD_MODE_OPTION_DEFAULT;
    pDev->modeParams.lineMinLF = 0x2;

    /* MYD_MODE_TEMPERATURE_RAMP */
    pDev->modeParams.hostTemperatureRampVal = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineTemperatureRampVal = MYD_MODE_OPTION_DEFAULT;

    modeOption = MYD_MODE_CTLE | MYD_MODE_TEMPERATURE_RAMP | MYD_MODE_FORCE_RECONFIG;
#endif

#if 0 /* optional sample overwrite initial CTLE HF and LF */
    /* MYD_MODE_INIT_CTLE */ 
    pDev->modeParams.hostInitHF = 0;
    pDev->modeParams.hostInitLF = 4;
    pDev->modeParams.lineInitHF = 0;
    pDev->modeParams.lineInitLF = 4;

    /* To use pre-defined initial HF and LF  */
    /* pDev->modeParams.hostInitHF = MYD_MODE_OPTION_DEFAULT; */
    /* pDev->modeParams.hostInitLF = MYD_MODE_OPTION_DEFAULT; */
    /* pDev->modeParams.lineInitHF = MYD_MODE_OPTION_DEFAULT; */
    /* pDev->modeParams.lineInitLF = MYD_MODE_OPTION_DEFAULT; */

    modeOption = MYD_MODE_INIT_CTLE | MYD_MODE_FORCE_RECONFIG;
#endif

#if 1  /* optional sample setting tuning the CTLE coefficients - Short Channel P25CN */
    /* MYD_MODE_CTLE */ 
    pDev->modeParams.hostMaxHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMaxLF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostMinLF = MYD_MODE_OPTION_IGNORE;

    pDev->modeParams.lineMaxHF = 0x6;
    pDev->modeParams.lineMinHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMaxLF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineMinLF = MYD_MODE_OPTION_IGNORE;

        /* MYD_MODE_INIT_CTLE */ 
    pDev->modeParams.hostInitHF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.hostInitLF = MYD_MODE_OPTION_IGNORE;
    pDev->modeParams.lineInitHF = 0;
    pDev->modeParams.lineInitLF = 4;

    modeOption = MYD_MODE_CTLE | MYD_MODE_INIT_CTLE | MYD_MODE_FORCE_RECONFIG;
#endif
    status = mydSetModeSelection(pDev, mdioPort, laneOffset, hostMode, lineMode, modeOption, &result);

    if (status != MYD_OK)
    {
        MYD_DBG_ERROR("mydSampleSetModeOption: mydSetModeSelection failed with result:0x%x\n", result);
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
                MYD_DBG_INFO("mydSampleSetModeOption: link is up in %u seconds\n", i);
                /* refer to the MYD_DETAILED_STATUS_TYPE structure for detail link status */
                break;
            }
        }
        mydWait(pDev, 1000);
    }

    if ( i >= linkCheckDelay )
    {
        MYD_DBG_INFO("mydSampleSetModeOption: link is down\n");
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
            MYD_DBG_ERROR("Waited for %u milliseconds for PRBS lock\n", waitTimer*100);
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
    MYD_U16 mdioIndex;
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
    mdioIndex = MYD_GET_PORT_IDX(pDev,mdioPort);
    if (mdioIndex >= MYD_MAX_PORTS) {
    	return MYD_FAIL;
    }
    if (host_or_line == MYD_HOST_SIDE)
    {
        sAddr = MYD_MAPPED_SERDES_ID(mdioIndex, MYD_HOST_MODE_IDX, laneOffset);
    }
    else
    {
        sAddr = MYD_MAPPED_SERDES_ID(mdioIndex, MYD_LINE_MODE_IDX, laneOffset);
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
    eyeDataBuf = mydWrapperEyeAllocDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[mdioIndex]);

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

    if (eyeDataBuf == NULL) {
    	MYD_DBG_INFO("eyeDataBuf is NULL\n");
    	return MYD_FAIL;
    }

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
    mydWrapperEyeFreeDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[mdioIndex], eyeDataBuf);

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
#endif /* MYD_ENABLE_SERDES_API */

/******************************************************************************
MYD_STATUS mydSampleX6141GPIOPinConfig

    Sample code to set given Pin to GPIO output mode on X6141. Other devices are
    similar but have different Pins.
    This sample code set the given pin to MYD_PIN_MODE_GPIO, configure the GPIO
    direction and write GPIO data to the given Pin.

    Please refer to structures and definitions of mydSetPinMode(), mydSetGPIOPinDirection()
    and mydSetGPIOPinData() for details.

    Inputs:
        pDev - pointer to MYD_DEV initialized by mydInitDriver() call
        gpioPinId - The pin to configure
        gpioData - The data want to drive on the given Pin in the GPIO mode
*******************************************************************************/
MYD_STATUS mydSampleX6141GPIOPinConfig
(
    IN MYD_DEV_PTR pDev,
    IN MYD_PIN_ID gpioPinId,
    IN MYD_U16 gpioData
)
{
    /* Recommend only call mydSetPinMode() with same Pin once in the whole program */
    /* The Pin mode MUST match schematic. */
    ATTEMPT(mydSetPinMode(pDev, gpioPinId, MYD_PIN_MODE_GPIO));

    /* Set the given Pin to GPIO output mode */
    ATTEMPT(mydSetGPIOPinDirection(pDev, gpioPinId, MYD_GPIO_OUTPUT_MODE));

    ATTEMPT(mydSetGPIOPinData(pDev, gpioPinId, gpioData));

    return MYD_OK;
}

/******************************************************************************
MYD_STATUS mydSampleX7120LEDPinConfig

    Sample code to configure MYD_LED2, MYD_LED3 and MYD_LED4 on X7120.
    This sample code set the given Pin to LED mode, set the LED blink rate option
    and config LEDs as described in the inline comments.

    Please refer to structures and definitions of mydSetPinMode(), mydConfigLEDPin()
    for details and other devices' LED settings.

    Each LED only represents the activities on the corresponding port.
    X7120 Mapping:                          |   X6141 Mapping:
    ledId       PIN         Port/LEDTimer   |   ledId          PIN        Port/LEDTimer
    MYD_LED0    GPIO1             0         |   MYD_LED0       GPIO0           0
    MYD_LED1    GPIO2             1         |   MYD_LED1       GPIO1           0
    MYD_LED2    GPIO3             2         |   MYD_LED2       GPIO2           0
    MYD_LED3    GPIO4             3         |   MYD_LED3       GPIO3           0
    MYD_LED4    CLK_OUT_SE1       3         |   MYD_LED4       CLK_OUT_SE1     0
    MYD_LED5    CLK_OUT_SE2       3         |   MYD_LED5       CLK_OUT_SE2     0

    Inputs:
        pDev - pointer to MYD_DEV initialized by mydInitDriver() calls

*******************************************************************************/
MYD_STATUS mydSampleX7120LEDPinConfig
(
    IN MYD_DEV_PTR pDev
)
{
    MYD_LED_CTRL led2Ctrl, led3Ctrl, led4Ctrl, ledCtrlUpdate;
    MYD_LED_TIMER_CONFIG p2LedTimer, p3LedTimer, ledTimerUpdate;
    MYD_U32 modeOption;

    /* Recommend only call mydSetPinMode() with same Pin once in the whole program */
    /* The Pin mode MUST match schematic. */
    ATTEMPT(mydSetPinMode(pDev, MYD_PIN_GPIO3, MYD_PIN_MODE_LED));         /* X7120 MYD_LED2 */
    ATTEMPT(mydSetPinMode(pDev, MYD_PIN_GPIO4, MYD_PIN_MODE_LED));         /* X7120 MYD_LED3 */
    ATTEMPT(mydSetPinMode(pDev, MYD_PIN_CLK_OUT_SE1, MYD_PIN_MODE_LED));   /* X7120 MYD_LED4 */

    /* Port2 (MYD_LED2) timer config */
    p2LedTimer.blinkRate1 = 3; /* 322 ms */
    p2LedTimer.blinkRate2 = 0; /* 40 ms */
    p2LedTimer.pulseStretchDuration = 0; /* No pulse stretching */

    /* Port3 (MYD_LED3 and MYD_LED4) timer config */
    p3LedTimer.blinkRate1 = 4; /* 644 ms */
    p3LedTimer.blinkRate2 = 3; /* 322 ms */
    p3LedTimer.pulseStretchDuration = 0; /* No pulse stretching */

    led2Ctrl.laneSelect = 0;
    led2Ctrl.interfaceSelect = MYD_HOST_SIDE;
    led2Ctrl.blinkActivity = MYD_LED_ACT_LANE_LINK_UP; /* when Link UP, MYD_LED2 blinks */
    led2Ctrl.solidActivity = MYD_LED_ACT_SOLID_OFF; /* ignored, blink activity has higher priority */
    led2Ctrl.polarity = MYD_LED_ACTIVE_HIGH; /* LED On - drive Pin high, LED Off - drive Pin low */
    led2Ctrl.mixRateLevel = 0; /* 0% mix rate */
    led2Ctrl.blinkRateSelect = 1; /* Select Blink Rate 1 on the Port2, 322ms */

    led3Ctrl.laneSelect = 0;
    led3Ctrl.interfaceSelect = MYD_HOST_SIDE;
    led3Ctrl.blinkActivity = MYD_LED_ACT_LANE_LINK_UP; /* when Link UP, MYD_LED3 blinks */
    led3Ctrl.solidActivity = MYD_LED_ACT_SOLID_OFF; /* ignored, blink activity has higher priority */
    led3Ctrl.polarity = MYD_LED_ACTIVE_HIGH; /* LED On - drive Pin high, LED Off - drive Pin low */
    led3Ctrl.mixRateLevel = 0; /* 0% mix rate */
    /* Blink Rate Option shared with MYD_LED4, select Blink Rate 1 on the Port3, 644 ms */
    led3Ctrl.blinkRateSelect = 1;

    led4Ctrl.laneSelect = 0;
    led4Ctrl.interfaceSelect = MYD_HOST_SIDE;
    led4Ctrl.blinkActivity = MYD_LED_ACT_LANE_LINK_UP; /* when Link UP, MYD_LED4 blinks */
    led4Ctrl.solidActivity = MYD_LED_ACT_SOLID_OFF; /* ignored, blink activity has higher priority */
    led4Ctrl.polarity = MYD_LED_ACTIVE_HIGH; /* LED On - drive Pin high, LED Off - drive Pin low */
    led4Ctrl.mixRateLevel = 0; /* 0% mix rate */
    /* Blink Rate Option shared with MYD_LED3 select Blink Rate 2 on the Port3, 322 ms */
    led4Ctrl.blinkRateSelect = 2;

    ATTEMPT(mydConfigLEDPin(pDev, MYD_LED2, led2Ctrl, p2LedTimer));
    ATTEMPT(mydConfigLEDPin(pDev, MYD_LED3, led3Ctrl, p3LedTimer));
    ATTEMPT(mydConfigLEDPin(pDev, MYD_LED4, led4Ctrl, p3LedTimer));

    /* Set mode and poll for link UP */
    modeOption = 0;
    ATTEMPT(mydSampleSetPCSMode(pDev, 2, 0, modeOption, MYD_P100UP, MYD_P100UP));
    ATTEMPT(mydSampleSetPCSMode(pDev, 3, 0, modeOption, MYD_P100UP, MYD_P100UP));

    /* All three LEDs start blinking at set rate */
    MYD_DBG_INFO("mydSampleX7120LEDPinConfig: MYD_LED2, 3 and 4 should blink at set rate \n");
    ATTEMPT(mydWait(pDev, 5000));

    ledTimerUpdate.blinkRate1 = 0; /* 40 ms */
    ledTimerUpdate.blinkRate2 = 5; /* 1.3 s */
    ledTimerUpdate.pulseStretchDuration = MYD_LED_CONFIG_UNCHANGED; /* keep unchanged */

    ledCtrlUpdate.laneSelect = MYD_LED_CONFIG_UNCHANGED; /* keep unchanged */
    ledCtrlUpdate.interfaceSelect = MYD_LED_CONFIG_UNCHANGED; /* keep unchanged */
    ledCtrlUpdate.blinkActivity = MYD_LED_ACT_LANE_TX; /* after link UP, the LED blinks when transmit packet */
    ledCtrlUpdate.solidActivity = MYD_LED_ACT_LANE_LINK_UP; /* when link UP, the LED solid on */
    ledCtrlUpdate.polarity = MYD_LED_CONFIG_UNCHANGED; /* keep unchanged */
    ledCtrlUpdate.mixRateLevel = MYD_LED_CONFIG_UNCHANGED; /* keep unchanged */
    ledCtrlUpdate.blinkRateSelect = 2; /* switch to Blink Rate 2, it will be updated to 1.3 s */

    /* Update MYD_LED3 configurations */
    ATTEMPT(mydConfigLEDPin(pDev, MYD_LED3, ledCtrlUpdate, ledTimerUpdate));

    /* Now the MYD_LED3 should be solid ON */
    MYD_DBG_INFO("mydSampleX7120LEDPinConfig: MYD_LED3 should be solid ON\n");
    ATTEMPT(mydWait(pDev, 5000));

    /* Enable Packet Generator */
    {
        MYD_U16 led3Port = 3;
        MYD_U16 led3Lane = 0;
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
        ATTEMPT(mydConfigurePktGeneratorChecker(pDev, led3Port, MYD_HOST_SIDE, led3Lane, readToClear,
                            dontuseSFDinChecker, pktPatternControl, generateCRCoff, initialPayload,
                            frameLengthControl, numPktsToSend, randomIPG, ipgDuration));

        /* reset the packet counter to 0 */
        ATTEMPT(mydPktGeneratorCounterReset(pDev, led3Port, MYD_HOST_SIDE, led3Lane));

        /* give it a little time 500ms to reset the counter */
        ATTEMPT(mydWait(pDev, 500));

        /* counter should be zero after reset */
        ATTEMPT(mydPktGeneratorGetCounter(pDev, led3Port, MYD_HOST_SIDE, led3Lane,
                                          MYD_PKT_GET_RX, &rxPktCount, &rxByteCount));

        MYD_DBG_INFO("mydSampleX7120LEDPinConfig: counters should be zero - rxPktCount:%u rxByteCount:%u\n", 
                      rxPktCount, rxByteCount);

        /* this call enable and starting the packet generator and checker */
        ATTEMPT(mydEnablePktGeneratorChecker(pDev, led3Port, MYD_HOST_SIDE, led3Lane, 
                                             enableGenerator, enableChecker));

        /* let the packet generator run for 5s, MYD_LED3 should start blinking */
        MYD_DBG_INFO("mydSampleX7120LEDPinConfig: MYD_LED3 should start blinking\n");
        ATTEMPT(mydWait(pDev, 5000));

        /* Disabled packet gen to stop the traffic */
        ATTEMPT(mydEnablePktGeneratorChecker(pDev, led3Port, MYD_HOST_SIDE, led3Lane, 
                                             MYD_FALSE, MYD_FALSE));
    }

    return MYD_OK;
}

/******************************************************************************
 MYD_STATUS mydSampleX7120RClkPinsConfig

    Sample code to set Pin CLK_OUT_SE2 to RCLK mode on X7120. Other devices are
    similar but have different number of Ports and Pins, which need user to pass in
    different size of structure. This sample code set RCLK source for Pin CLK_OUT_SE1
    to Port3, Pin CLK_OUT_SE2 to Port2 and set both of them to Host Side, Lane 0.

    Please refer to structures and definitions of mydSetPinMode(), mydConfigRClkMux()
    and mydConfigRClkPin() for details.
    * All mentioned "PortN(Port number)" are relative port number (port index).

    Inputs:
        pDev - pointer to MYD_DEV initialized by mydInitDriver() call
*******************************************************************************/
MYD_STATUS mydSampleX7120RClkPinsConfig
(
    IN MYD_DEV_PTR pDev
)
{
    MYD_RCLK_MUX rClkMux;
    MYD_U16 interfaceSelect;
    MYD_U16 portIndex, interfaceIndex;
    MYD_U16 se1_portSelect, se1_mdioPortIndex, se1_muxOptionSelect;
    MYD_U16 se2_portSelect, se2_mdioPortIndex, se2_muxOptionSelect;

    /* Initialized the structure to be ALL 0 */
    for (portIndex = 0; portIndex < pDev->portCount; portIndex++)
    {
        for (interfaceIndex = 0; interfaceIndex < MYD_NUM_INTERFACE; interfaceIndex++)
        {
            rClkMux.x7120.mux1[portIndex][interfaceIndex].srcClockSelect = 0;
            rClkMux.x7120.mux1[portIndex][interfaceIndex].laneSelect = 0;
            rClkMux.x7120.mux1[portIndex][interfaceIndex].divideRatio = 0;
            rClkMux.x7120.mux1[portIndex][interfaceIndex].dividerConfig = 0;

            rClkMux.x7120.mux2[portIndex][interfaceIndex].srcClockSelect = 0;
            rClkMux.x7120.mux2[portIndex][interfaceIndex].laneSelect = 0;
            rClkMux.x7120.mux2[portIndex][interfaceIndex].divideRatio = 0;
            rClkMux.x7120.mux2[portIndex][interfaceIndex].dividerConfig = 0;
        }
    }

    /* Noticed that MYD_PIN_CLK_OUT_SE1 can only output Port2 and Port3's Recovered clock */
    se1_portSelect = 3;
    se1_mdioPortIndex = MYD_GET_PORT_IDX(pDev, se1_portSelect);
    se1_muxOptionSelect = 1;

    se2_portSelect = 2;
    se2_mdioPortIndex = MYD_GET_PORT_IDX(pDev, se2_portSelect);
    se2_muxOptionSelect = 2;

    /* Recommend only call mydSetPinMode() with same Pin once in the whole program */
    /* The Pin mode MUST match schematic. */
    ATTEMPT(mydSetPinMode(pDev, MYD_PIN_CLK_OUT_SE1, MYD_PIN_MODE_RCLK));
    ATTEMPT(mydSetPinMode(pDev, MYD_PIN_CLK_OUT_SE2, MYD_PIN_MODE_RCLK));

    /* Set Port3 Mux 1 in the array */
    rClkMux.x7120.mux1[se1_mdioPortIndex][MYD_HOST_MODE_IDX].srcClockSelect = 0; /* 10T/20T/40T */
    rClkMux.x7120.mux1[se1_mdioPortIndex][MYD_HOST_MODE_IDX].laneSelect = 0;
    rClkMux.x7120.mux1[se1_mdioPortIndex][MYD_HOST_MODE_IDX].divideRatio = 0x08;
    rClkMux.x7120.mux1[se1_mdioPortIndex][MYD_HOST_MODE_IDX].dividerConfig = 0x2; /* Enable the divider */

    /* Set Port2 Mux 2 in the array */
    rClkMux.x7120.mux2[se2_mdioPortIndex][MYD_HOST_MODE_IDX].srcClockSelect = 0; /* 10T/20T/40T */
    rClkMux.x7120.mux2[se2_mdioPortIndex][MYD_HOST_MODE_IDX].laneSelect = 0;
    rClkMux.x7120.mux2[se2_mdioPortIndex][MYD_HOST_MODE_IDX].divideRatio = 0x08;
    rClkMux.x7120.mux2[se2_mdioPortIndex][MYD_HOST_MODE_IDX].dividerConfig = 0x2; /* Enable the divider */

    /* In this sample, we only configure the Host side of Port2 MUX 2 and Port3 MUX 1 */
    /* MUX configuration of all other Port, interface will be configured to 0 as provided in the array */
    ATTEMPT(mydConfigRClkMux(pDev, &rClkMux));

    /* Select Port2, Host Side, Mux 1 */
    interfaceSelect = MYD_HOST_SIDE;
    ATTEMPT(mydConfigRClkPin(pDev, MYD_PIN_CLK_OUT_SE1, se1_portSelect, interfaceSelect, se1_muxOptionSelect));
    ATTEMPT(mydConfigRClkPin(pDev, MYD_PIN_CLK_OUT_SE2, se2_portSelect, interfaceSelect, se2_muxOptionSelect));

    return MYD_OK;
}


/******************************************************************************
 MYD_STATUS mydSampleNIMB_ANegLaneSwap_iCal

   Sample code to show how to call the mydNIMBxxx APIs to swap the AutoNeg lane 
   with auto iCal calibration. 

   1. This code shows how to setup a port with host as MYD_R200U and line as MYD_R200Q 
   repeater mode.
   2. Call mydExtendDeviceAutoNegTimer to extend the AN timer for repeater w/ AN Lane Swap.
   3. Call mydNIMBSetANegLaneSwap to setup the AN Lane Swap
   4. Call mydSetModeSelection to setup a port with host as MYD_R200U and line as MYD_R200Q 
   repeater mode.
   5. Call mydNIMBSetEyeCoeff to customize the eye coefficient
   6. Call mydNIMBSetOp to start the iCal polling and auto calibration

   This feature is only support in limited number of modes and requires 
   the supported Z80 firmware loaded into the RAM.

*******************************************************************************/
MYD_STATUS mydSampleNIMB_ANegLaneSwap_iCal
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
    MYD_U16 result, opStatus;
    MYD_BOOL perLaneStatus;
    MYD_U16 linkCheckDelay;
    MYD_REPEATER_LINK_STATUS statusDetailRepeater;

    /* ** Sample only **: setting for Host MYD_R200U and Line MYD_R200Q repeater mode */
    hostMode = MYD_R200U;
    lineMode = MYD_R200Q;

    modeOption = MYD_MODE_ICAL_EFFORT_0;
    laneOffset = 0; /* use lane 0 for MYD_R200 R4 mode */

    /* mydNIMBSetOp must be stopped when calling API that issues interrupt calls to the SerDes 
       and SBus master. */
    ATTEMPT(mydNIMBGetOpStatus(pDev, mdioPort, laneOffset, &opStatus));
    if (opStatus) /* Stop the iCal polling and auto calibration if started */
    {
        ATTEMPT(mydNIMBSetOp(pDev, mdioPort, laneOffset, 0));
        mydWait(pDev, 500);
    }

    /* optional operations to invert the polarity on all lanes if needed */
    /* ATTEMPT(mydSetTxPolarity(pDev, mdioPort, MYD_HOST_SIDE, laneOffset0/1/2/3, 1, 0)); */
    /* ATTEMPT(mydSetRxPolarity(pDev, mdioPort, MYD_HOST_SIDE, laneOffset0/1/2/3, 1, 0)); */
    /* ATTEMPT(mydSetTxPolarity(pDev, mdioPort, MYD_LINE_SIDE, laneOffset0/1/2/3, 1, 0)); */
    /* ATTEMPT(mydSetRxPolarity(pDev, mdioPort, MYD_LINE_SIDE, laneOffset0/1/2/3, 1, 0)); */

    /* Refers to mydNIMBSetANegLaneSwap instructions before setting this value, 
       only b[11:0] should be used in register 3|4.0xF08F
       0x0AA8 is for 200GR4 repeater mode, AN on lane3(instead of lane0)
       Do not call this API if AN lane swapped is not needed on the Repeater AN modes. */
    ATTEMPT(mydNIMBSetANegLaneSwap(pDev, mdioPort, MYD_LINE_SIDE, 0x0AA8));

    /* optional MYD_MODE_ICAL_EFFORT_0 setting for faster linkup. 
       Refers to the Mode Option List for options available for more selection */
    /* setting the Host to MYD_R200U and Line to MYD_R200Q repeater mode */
    status = mydSetModeSelection(pDev, mdioPort, laneOffset, hostMode, lineMode, 
                                 modeOption, &result);

    if (status != MYD_OK)
    {
        MYD_DBG_ERROR("mydSampleNIMB_ANegLaneSwap_iCal: mydSetModeSelection failed with result:0x%x\n", result);
        return status;
    }

    /* Refers to mydNIMBSetEyeCoeff for more description on the eye coefficient.
       Set the eye range to trigger an iCal calibration; MYD_NIMB_EYE_DELTA_DEFAULT will use the suggested values */
    /* ATTEMPT(mydNIMBSetEyeCoeff(pDev, mdioPort, laneOffset, 0, 0, 0, MYD_NIMB_EYE_DELTA_DEFAULT)); */

    /* Customized eye coefficient matrix for all channels. Below values are customized eye coefficient for 
       unique setup and will not work on all setup. Use MYD_NIMB_EYE_DELTA_DEFAULT otherwise. */
    ATTEMPT(mydNIMBSetEyeCoeff(pDev, mdioPort, laneOffset, 0xA, 0x10, 0x10, MYD_NIMB_EYE_DELTA_UPDATE));

    /* mydNIMBSetOp should be called as the last step of repeater w/ and w/o AN Lane Swap initialization */
    /* start the iCal polling and auto calibration */
    ATTEMPT(mydNIMBSetOp(pDev, mdioPort, laneOffset, 1));

    /* Below polls the link status. Refer to MYD_REPEATER_LINK_STATUS structure for link status details */
    mydMemSet(&statusDetailRepeater, 0, sizeof(MYD_REPEATER_LINK_STATUS));

    linkCheckDelay = 30;
    perLaneStatus = MYD_FALSE;

    for (i=0; i<linkCheckDelay; i++)
    {
        status = mydCheckRepeaterLinkStatus(pDev, mdioPort, laneOffset, perLaneStatus, 
                                            &currentStatus, &statusDetailRepeater);
        if (status == MYD_OK)
        {
            if (currentStatus == MYD_LINK_UP)
            {
                MYD_DBG_INFO("mydSampleNIMB_ANegLaneSwap_iCal: link is up in %u seconds\n", i);
                /* refer to the MYD_REPEATER_LINK_STATUS structure for detail link status */
                break;
            }
        }
        mydWait(pDev, 1000);
    }

    if ( i >= linkCheckDelay )
    {
        MYD_DBG_INFO("mydSampleNIMB_ANegLaneSwap_iCal: link is down\n");
    }

    return status;
}
