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
* cm3BootChannel.c
*
* DESCRIPTION:
*       This file implements boot channel comunication example
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <asicSimulation/SKernel/smain/smain.h>
#include <cm3FileOps.h>
#include <cm3ConfigFileParser.h>
#include <stdio.h>      /* printf, NULL */
#include <string.h>
#include <cm3NetPort.h>
#include <asicSimulation/SCIB/scib.h>
#include <cm3BootChannel.h>
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/wmApi.h>

#define IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN     252 /* Same as MI */


#define HOST_OWNERSHIP_BIT 1
#define HEADER_SIZE_IN_BYTES 4

#define HEADER_SIZE_IN_WORDS (HEADER_SIZE_IN_BYTES/4)
#define OPCODE_SIZE_IN_BYTES 2
#define PARAM_SIZE_IN_BYTES  4


#define SUPPORTED_PROFILE_NUM 9

static GT_BOOL usePex = GT_TRUE;

#define UBOOT_CM3_PEX_START_ADRESS 0xf6400000

#define HARRIER_UBOOT_BOOT_CHANNEL_OFFSET cm3SimBootChannelAdressGet(UBOOT_CM3_PEX_START_ADRESS)
#define HARRIER_DIRECT_BOOT_CHANNEL_OFFSET cm3SimBootChannelAdressGet(MG0_CM3_BASE)
#define TRAFFIC_CONF_FILE_NAME  "cont_traf.txt"


/*First two bytes is for opcode.Each parameter is 4 bytes*/
#define BC_PARAM(_msg,_i)     _msg[2+_i*4]  /* First index 0 */

typedef struct{

    GT_U32         word1;

    GT_U32         word2;

    GT_U32         buffPointer;

    GT_U32         nextDescPointer;

} CM3_SIM_TX_DESC_STC;


GT_CHAR_PTR opcodeToString[]=
{   "N/A",
    "N/A",
    "MICRO_INIT_BOOT_CH_OPCODE_EPROM_LIKE_E",/* = 2,*/
    "MICRO_INIT_BOOT_CH_OPCODE_CLI_LIKE_E",
    "MICRO_INIT_BOOT_CH_OPCODE_RERUN_FILES_E",
    "MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION",
    "MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS",
    "MICRO_INIT_BOOT_CH_OPCODE_GET_ALL_FILES"
};


GT_CHAR_PTR commands[] =
    {
        "interface ethernet 0/16 speed 25000 mode 25GBase-KR",
        "config list"
    };


extern GT_STATUS snetFalconPortStatisticFramesTransmittedCounterGet
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_U32                           macPort,
    IN GT_BOOL                          rx,
    OUT  GT_U32                         *valuePtr
);

extern SKERNEL_DEVICE_OBJECT* smemTestDeviceIdToDevPtrConvert
(
    IN  GT_U32                      deviceId
);


/**
* @internal ASIC_SIMULATION_ReadMemory function
* @endinternal
*
* @brief   Read memory from ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
static void example_ASIC_SIMULATION_ReadMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
)
{
    if(sinit_global_usePexLogic == GT_FALSE)
    {
        GT_U32 deviceId = scibGetDeviceId(drvBaseAddr);
        scibReadMemory(deviceId,memAddr,length,dataPtr);
        return;
    }

#if __WORDSIZE == 64
    (void)wmMemPciRead(pexFinalAddr>>32,(GT_U32)pexFinalAddr,length,dataPtr);
#else
    (void)wmMemPciRead(0/*addr high*/,pexFinalAddr,length,dataPtr);
#endif

    return;
}

/**
* @internal ASIC_SIMULATION_WriteMemory function
* @endinternal
*
* @brief   Write to memory of a ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
* @param[in] dataPtr                  - pointer to copy read data.
*
*
*/
static void example_ASIC_SIMULATION_WriteMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
)
{
    if(sinit_global_usePexLogic == GT_FALSE)
    {
        GT_U32 deviceId = scibGetDeviceId(drvBaseAddr);
        scibWriteMemory(deviceId,memAddr,length,dataPtr);
        return;
    }
#if __WORDSIZE == 64
    (void)wmMemPciWrite(pexFinalAddr>>32,(GT_U32)pexFinalAddr,length,dataPtr);
#else
    (void)wmMemPciWrite(0/*addr high*/,pexFinalAddr,length,dataPtr);
#endif

    return;
}


GT_STATUS cm3BootChannelSendGetAllFilesCommand
(
    GT_U32 bitOffset
);

GT_U32 cm3BootChannelOffsetGet
(
    GT_VOID
)
{
    GT_U32 initOffset;

    if(GT_TRUE==usePex)
    {
        initOffset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    }
    else
    {
        initOffset = HARRIER_DIRECT_BOOT_CHANNEL_OFFSET;
    }

    return initOffset;

}

GT_STATUS cm3BootChannelWriteWord
(
   GT_U32 offset,
   GT_U32 value
)
{

    SIM_CM3_LOG("Write [P %d adress 0x%08x data 0x%08x]\n",usePex,offset,value);

    if(GT_TRUE==usePex)
    {
      example_ASIC_SIMULATION_WriteMemory(0, offset, 1, &value , offset);
    }
    else /*direct mode*/
    {
      cm3GenSwitchRegisterSet(offset,value,0);
    }

    return GT_OK;

}

GT_STATUS cm3BootChannelReadWord
(
   GT_U32 offset,
   GT_U32 *valuePtr
)
{

    if(GT_TRUE==usePex)
    {
      example_ASIC_SIMULATION_ReadMemory(0, offset, 1, valuePtr , offset);
    }
    else /*direct mode*/
    {
      cm3GenSwitchRegisterGet(offset,valuePtr,0);
    }

    SIM_CM3_LOG("Read [P %d adress 0x%x data 0x%x]\n",usePex,offset,*valuePtr);

    return GT_OK;

}


GT_STATUS cm3BootChannelAnswerGet
(
    GT_VOID
)
{
    GT_32  maxIter= 10;
    GT_U32 value,result;
    GT_U32 offset = cm3BootChannelOffsetGet();

    do
    {
        SIM_OS_MAC(simOsSleep)(100);
        cm3BootChannelReadWord(offset,&value);

        maxIter--;
        if((value >>31)^HOST_OWNERSHIP_BIT)
        {
            /*check result*/
            result = value & 0xff;
            SIM_CM3_LOG("Receive result code %d\n",result);
            return  result==0?GT_OK:GT_FAIL;
        }


    }
    while(maxIter);

    return GT_TIMEOUT;
}


/**************************************************

R= ready bit

                ========================================
Word 0   |R                                       HEADER           SIZE                          |
Word 1   |                     OPCODE              |                                                 |
.
.
.==============================================
*/
static GT_STATUS cm3BootChannelSendCliCommand
(
    GT_CHAR_PTR command,
    GT_BOOL     log
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 offset,value;
    GT_U8 * tmp;


    GT_U8 commandSize,sizeWithOpcode;
    GT_U32 msg[64];
    commandSize =strlen(command);
    sizeWithOpcode=commandSize+OPCODE_SIZE_IN_BYTES;/*additional 2 bytes for opcode*/

    /*start with second line since "ready" bit is in the first word*/
    offset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    tmp =(GT_U8 *) &msg[1];
    *tmp= MICRO_INIT_BOOT_CH_OPCODE_EPROM_LIKE_E;
    wordsToWrite = HEADER_SIZE_IN_WORDS + (sizeWithOpcode+4)/4 ;/*clip to words*/
    if(GT_TRUE==log)
    {
        SIM_CM3_LOG("Send message %d [%s] %d lines.\n", msg[1],opcodeToString[msg[1]],wordsToWrite);
    }

    tmp+=OPCODE_SIZE_IN_BYTES;

    memcpy(tmp,command,commandSize);

    for(i=1;i<wordsToWrite;i++)
    {
         value = msg[i];

         cm3BootChannelWriteWord(offset,value);

         offset+=4;

    }

    offset =HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    value = msg[0];

    cm3BootChannelWriteWord(offset,value);


    /*get the answer*/
    return cm3BootChannelAnswerGet();

}


GT_VOID cm3BootChannelErrorCodeGet
(
    GT_VOID
)
{
    GT_U32 offset[2],value;

    offset[0] = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET+cm3SimIpcRegionOffsetGet(CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E);
    offset[1] =offset[0];
    offset[0]+= (CM3_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E*4);
    offset[1]+= (CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E*4);

    example_ASIC_SIMULATION_ReadMemory(0, offset[0], 1, &value , offset[0]);

    SIM_CM3_LOG("CM3_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E 0x%08x\n",value);

    example_ASIC_SIMULATION_ReadMemory(0, offset[1], 1, &value , offset[1]);

    SIM_CM3_LOG("CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E 0x%08x\n",value);
}


GT_STATUS cm3BootChannelExecuteCliCommands
(
    GT_VOID
)
{
    GT_U32 size,i;
    GT_STATUS rc;

    size = sizeof(commands)/sizeof(commands[0]);
    for(i=0;i<size;i++)
    {
        SIM_CM3_LOG("Send command %s via boot channel\n",commands[i]);

        rc =cm3BootChannelSendCliCommand(commands[i],GT_TRUE);

        /*only if command reception was ACKed then check the result*/
        if(GT_OK!=rc)
        {
            if(GT_TIMEOUT!=rc)
            {
                SIM_CM3_LOG("Command reception is confirmed.But error code returned.\n");
                cm3BootChannelErrorCodeGet();
            }
            else
            {
                SIM_CM3_LOG("Command reception is NOT confirmed\n");
            }
       }

    }

    return GT_OK;
}


/**
* @internal cm3BootChannelTriggerTraffic function
* @endinternal
*
* @brief   Trigger traffic from CPU
*
* @param[in] macPort                  - MAC port number
* @param[in] rx              -  if equal GT_TRUE then RX_aFramesReceivedOK is printed ,else TX_aFramesTransmittedOK.
*
*
*/
GT_STATUS cm3BootChannelTriggerTraffic
(
    GT_VOID
)
{
    GT_U32 size,i;
    GT_STATUS rc;
    GT_BOOL logRedirect = simCm3LogRedirectGet();

    GT_CHAR_PTR triggerTraffic[] =
    {   "wrd 0 0 0x0d000968 0x00000000 0xffffffff",/*SMEM_SIP6_RXDMA_CHANNEL_PCH_CONFIG_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber)*/
        "wrd 0 0 0x0d000a68 0x0000003f 0xffffffff",/*SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber)*/
        "wrd 0 0 0x0d000868 0x00000001 0xffffffff",/*SMEM_SIP6_RXDMA_CHANNEL_CASCADE_PORT_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber)*/
        "wrd 0 0 0x000f8268 0x05c40010 0xffffffff",/*SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl6*/
        "wrd 0 0 0x3C20020C 0x3C200003 0xffffffff",
        "wrd 0 0 0x3c2026dc 0x3C200000 0xffffffff",
        "wrd 0 0 0x3c202868 0x00000080 0xffffffff"/*trigger*/
    };

    GT_CHAR  buf[60];

    GT_U32  data[20]= {0x22110100,0x02003323,0x44343322,0xfe1f8250,0x004008a8,0x00c007a0,
        0x00080000};

    CM3_SIM_TX_DESC_STC desc;
    GT_U32   descAddrLow = ((uintptr_t)&desc)&0xFFFFFFFF;
    FILE * fp = NULL;

#if __WORDSIZE == 64
     GT_U32 limit;
     GT_U32   descAddrHi = 0;
     descAddrHi = (((uintptr_t)&desc))>>32;
     /*32 high oATU*/
#endif

    fp = fopen(TRAFFIC_CONF_FILE_NAME,"rb");
    if(fp==NULL)
    {
       SIM_CM3_LOG("Error opening %s file\n",TRAFFIC_CONF_FILE_NAME);
       return GT_FAIL;
    }

    i=0;
    /*disable log*/
    simCm3LogRedirectSet(GT_FALSE);

    /*Open configuration file and send configurations throgh boot channel*/
    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        if(i&&i%20==0)
        {
            SIM_CM3_LOG("%d lines sent\n",i);
        }
        rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);

        /*only if command reception was ACKed then check the result*/
        if(GT_OK!=rc)
        {
           return GT_FAIL;
        }
        i++;
    }

    simCm3LogRedirectSet(logRedirect);

    desc.word1 = 0x80300000;
    desc.word2 = 0x00500000;
    desc.buffPointer = ((uintptr_t)data)&0xFFFFFFFF;
    desc.nextDescPointer = 0x00000000;
#if __WORDSIZE == 64
    /*configure ATU window to access DDR that hold the paket*/

    SIM_CM3_LOG("descAddrHi 0x%x\n",descAddrHi);
    example_ASIC_SIMULATION_WriteMemory(0, 0xf6001218, 1, &descAddrHi , 0xf6001218);/*oATU_UPPER_TARGET_ADDRESS_REG*/

    SIM_CM3_LOG("descAddrLow 0x%x\n",descAddrLow);
    example_ASIC_SIMULATION_WriteMemory(0, 0xf6001208, 1, &descAddrLow , 0xf6001208);/*oATU_LOWER_BASE_ADDRESS_REG*/

    example_ASIC_SIMULATION_WriteMemory(0, 0xf6001214, 1, &descAddrLow , 0xf6001214);/*oATU_LOWER_TARGET_ADDRESS_REG*/

    limit = descAddrLow+0xFFFF;
    example_ASIC_SIMULATION_WriteMemory(0, 0xf6001210, 1, &limit , 0xf6001210);/*oATU_LIMIT_ADDRESS_REG*/
#endif

    size = sizeof(triggerTraffic)/sizeof(triggerTraffic[0]);
    /*configure remaining configurations(pointer to packet*/
    for(i=0;i<size;i++)
    {
        strcpy(buf,triggerTraffic[i]);

        if(strstr(buf,"0x3C20020C"))
        {
            sprintf(buf,"wrd 0 0 0x3C20020C 0x%x 0xffffffff",(descAddrLow&0xFFFFFFF0)|0x3);
        }
        else if (strstr(buf,"0x3c2026dc"))
        {
            sprintf(buf,"wrd 0 0 0x3c2026dc 0x%x 0xffffffff",descAddrLow);
        }


        SIM_CM3_LOG("Send command %s via boot channel\n",buf);

        rc =cm3BootChannelSendCliCommand(buf,GT_FALSE);

        /*only if command reception was ACKed then check the result*/
        if(GT_OK!=rc)
        {
            if(GT_TIMEOUT!=rc)
            {
                SIM_CM3_LOG("Command reception is confirmed.But error code returned.\n");
                cm3BootChannelErrorCodeGet();
            }
            else
            {
                SIM_CM3_LOG("Command reception is NOT confirmed\n");
            }
       }

    }

    return GT_OK;
}





GT_STATUS cm3BootChannelVersionGet
(
    GT_VOID
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 initOffset,offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];


    sizeWithOpcode= OPCODE_SIZE_IN_BYTES;/*additional 2 bytes for opcode*/

    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    initOffset = cm3BootChannelOffsetGet();

    /*start with second line since "ready" bit is in the first word*/
    offset=initOffset+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION;

    wordsToWrite = HEADER_SIZE_IN_WORDS + (sizeWithOpcode)/4;

    SIM_CM3_LOG("Send message %d [%s] %d lines.\n", msg[1],opcodeToString[msg[1]],wordsToWrite);
    for(i=1;i<wordsToWrite;i++)
    {
        value = msg[i];
        cm3BootChannelWriteWord(offset,value);
        offset+=4;
    }

    offset =initOffset;
    value = msg[0];

    cm3BootChannelWriteWord(offset,value);
    /*get the answer*/
    return cm3BootChannelAnswerGet();
}


GT_VOID cm3BootChannelReadData
(
    GT_U32  * bufferPtr,
    GT_U32    sizeInWords,
    GT_U32    firstWord
)
{
    GT_U32 offset,i;
    GT_U32 initOffset = cm3BootChannelOffsetGet();

    offset = initOffset+cm3SimIpcRegionOffsetGet(CM3_SIM_IPC_SHM_DATA_REGION_BC_DATA_E);
    offset +=(firstWord*4);


    for(i=0;i<sizeInWords;i++,bufferPtr++,offset+=4)
    {
      cm3BootChannelReadWord(offset,bufferPtr);
    }

}



GT_STATUS cm3BootChannelExecuteVersionGet
(
    GT_VOID
)
{
    /*
            Additional 1 is for  '\0',just to be sure.
         */
    GT_CHAR buffer[BOOT_CHANEL_VERSION_STRING_LEN+1];
    GT_U32 firstWord =0,i;
    GT_CHAR_PTR names [] = {"SuperImage","MI code","Booton"};
    if(GT_OK==cm3BootChannelVersionGet())
    {
        SIM_CM3_LOG("Command reception is confirmed.Check the version.\n");
        for(i=0;i<3;i++)
        {
            cm3BootChannelReadData((GT_U32 *)buffer,BOOT_CHANEL_VERSION_STRING_LEN/4,firstWord);
            buffer[BOOT_CHANEL_VERSION_STRING_LEN]='\0';
            SIM_CM3_LOG("%s version %d [%s]\n",names[i],i,buffer);
            firstWord +=(BOOT_CHANEL_VERSION_STRING_LEN/4);
       }
    }
    else
    {
        SIM_CM3_LOG("Command reception is NOT confirmed\n");
    }

    return GT_OK;
}

GT_STATUS cm3BootChannelLuaWrapExecuteVersionGet
(
    GT_U32 * superImagePtr,
    GT_U32 * fwPtr,
    GT_U32 * bootonPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    digits[3]={0,0,0};
    /*
            Additional 1 is for  '\0',just to be sure.
         */
    GT_CHAR buffer[BOOT_CHANEL_VERSION_STRING_LEN+1];
    GT_U32 firstWord =0,i;
    GT_CHAR tmp;

    /*GT_CHAR_PTR names [] = {"SuperImage","MI code","Booton"};*/
    if(GT_OK==cm3BootChannelVersionGet())
    {
        for(i=0;i<3;i++)
        {
            cm3BootChannelReadData((GT_U32 *)buffer,BOOT_CHANEL_VERSION_STRING_LEN/4,firstWord);
            buffer[BOOT_CHANEL_VERSION_STRING_LEN]='\0';

            firstWord +=(BOOT_CHANEL_VERSION_STRING_LEN/4);

            switch(i)
            {
                case 0:
                    sscanf(buffer,"%d%c%d",&digits[0],&tmp,&digits[1],&tmp,&digits[2]);
                    *superImagePtr = digits[1]<<8|digits[0];

                 break;
                case 1:
                     sscanf(buffer,"%d%c%d%c%d",&digits[0],&tmp,&digits[1],&tmp,&digits[2]);
                    *fwPtr = digits[2]<<16|digits[1]<<8|digits[0];
                 break;
                case 2:
                    sscanf(buffer,"%d%c%d%c%d",&digits[0],&tmp,&digits[1],&tmp,&digits[2]);
                    *bootonPtr = digits[2]<<16|digits[1]<<8|digits[0];
                 break;
            }
        }
    }
    else
    {
        rc = GT_TIMEOUT;
    }

    return rc;
}





GT_STATUS cm3BootChannelExecuteGetAllFiles
(
    GT_U32 bitOffset
)
{
    /*
       Additional 1 is for  '\0',just to be sure.
     */
    GT_CHAR buffer[IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN+1];

    if(GT_OK==cm3BootChannelSendGetAllFilesCommand(bitOffset))
    {
        SIM_CM3_LOG("Command reception is confirmed.Check the version.\n");
        cm3BootChannelReadData((GT_U32 *)buffer,IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN/4,0);
        buffer[IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN]='\0';
        SIM_CM3_LOG("%s \n",buffer);
    }
    else
    {
        SIM_CM3_LOG("Command reception is NOT confirmed\n");
    }

    return GT_OK;
}


GT_STATUS cm3BootChannelSendPortStatusGetCommand
(
    GT_U8 portNum
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];
    GT_U8 * tmp;

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES+2*PARAM_SIZE_IN_BYTES;/*2  params + 2 bytes for opcode*/


    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    /*start with second line since "ready" bit is in the first word*/
    offset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS;
    wordsToWrite = HEADER_SIZE_IN_WORDS + sizeWithOpcode/4;

    SIM_CM3_LOG("Send message %d [%s] %d lines.\n", msg[1],opcodeToString[msg[1]],wordsToWrite);

    tmp =(GT_U8 *) &msg[1];

    /*data*/
    BC_PARAM(tmp,1)=portNum;



    for(i=1;i<wordsToWrite;i++)
    {
         value = msg[i];

         cm3BootChannelWriteWord(offset,value );

         offset+=4;

    }

    offset =HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    value = msg[0];

    cm3BootChannelWriteWord(offset,value );


    /*get the answer*/

    return cm3BootChannelAnswerGet();
}

GT_STATUS cm3BootChannelSendGetAllFilesCommand
(
    GT_U32 bitOffset
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];
    GT_U8 * tmp;

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES+2*PARAM_SIZE_IN_BYTES;/*2  params + 2 bytes for opcode*/


    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    /*start with second line since "ready" bit is in the first word*/
    offset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = MICRO_INIT_BOOT_CH_OPCODE_GET_ALL_FILES;
    wordsToWrite = HEADER_SIZE_IN_WORDS + sizeWithOpcode/4;

    SIM_CM3_LOG("Send message %d [%s] %d lines.\n", msg[1],opcodeToString[msg[1]],wordsToWrite);

    tmp =(GT_U8 *) &msg[1];

    /*data*/
    BC_PARAM(tmp,0)= bitOffset;
    BC_PARAM(tmp,1)= 0;/*group*/

    for(i=1;i<wordsToWrite;i++)
    {
         value = msg[i];

         cm3BootChannelWriteWord(offset,value );

         offset+=4;
    }

    offset =HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    value = msg[0];

    cm3BootChannelWriteWord(offset,value );


    /*get the answer*/

    return cm3BootChannelAnswerGet();
}


GT_STATUS cm3BootChannelLuaWrapExecutePortStatusGetCommand
(
    GT_U8 portNum,
    GT_U32 *bmpPtr
)
{
    GT_U32 rc = GT_OK;
    /*only if command reception was ACKed then check the result*/
    if(GT_OK==cm3BootChannelSendPortStatusGetCommand(portNum))
    {
        cm3BootChannelReadData(bmpPtr,1,0);
    }
    else
    {
        rc = GT_TIMEOUT;
    }

    return rc;
}

GT_STATUS cm3BootChannelExecutePortStatusGetCommand
(
    GT_U8 portNum
)
{
    GT_U32 buffer;
    /*only if command reception was ACKed then check the result*/
    if(GT_OK==cm3BootChannelSendPortStatusGetCommand(portNum))
    {
        SIM_CM3_LOG("Command reception is confirmed.Check status.\n");
        cm3BootChannelReadData(&buffer,1,0);
        SIM_CM3_LOG("BMP 0x%x\n",buffer&0xFF);
        SIM_CM3_LOG("Mode %d\n",buffer>>8);
    }
    else
    {
        SIM_CM3_LOG("Command reception is NOT confirmed\n");
    }

    return GT_OK;
}



GT_STATUS cm3BootChannelSendRerunFilesCommand
(
    GT_U32 shift
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];
    GT_U8 * tmp;
    GT_U32 initOffset = cm3BootChannelOffsetGet();

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES+PARAM_SIZE_IN_BYTES;

    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    /*start with second line since "ready" bit is in the first word*/
    offset = initOffset+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = MICRO_INIT_BOOT_CH_OPCODE_RERUN_FILES_E;
    wordsToWrite = HEADER_SIZE_IN_WORDS + sizeWithOpcode/4;

    SIM_CM3_LOG("\nSend message %d [%s] %d lines.\n", msg[1],opcodeToString[msg[1]],wordsToWrite);

    tmp =(GT_U8 *) &msg[1];

    /*data*/
    BC_PARAM(tmp,0)= shift;/*rerun default files 1<<0)*/

    for(i=1;i<wordsToWrite;i++)
    {
         value = msg[i];

         cm3BootChannelWriteWord(offset,value);

         offset+=4;

    }

    offset =initOffset;
    value = msg[0];
    cm3BootChannelWriteWord(offset,value);

    /*get the answer*/

    return cm3BootChannelAnswerGet();
}


GT_STATUS cm3BootChannelChangeProfile
(
    GT_U32 profileNum
)
{

    if(profileNum>SUPPORTED_PROFILE_NUM)
    {
        return GT_BAD_PARAM;
    }

    return cm3BootChannelSendRerunFilesCommand(profileNum);

}

/**
* @internal cm3BootChannelUsePexSet function
* @endinternal
*
* @brief  Configure comunication type with CM3.Whether direct or using PEX adresses.
*
* @param[in] val      if equal GT_TRUE then CM3 is adressed throgh PEX,otherwise direct.
*
*
*/
GT_STATUS cm3BootChannelUsePexSet
(
    GT_BOOL val
)
{
    usePex =val;
    return GT_OK;
}


/**
* @internal cm3PortStatisticCounterInternalGet function
* @endinternal
*
* @brief   Gets port frames counter.
*
* @param[in] macPort                  - MAC port number
* @param[in] rx              -  if equal GT_TRUE then RX_aFramesReceivedOK is returned ,else TX_aFramesTransmittedOK.
* @param[out] valuePtr                  - (pointer to) counter value
*
*
*/
GT_STATUS cm3PortStatisticCounterInternalGet
(
    IN GT_U32                           macPort,
    IN GT_BOOL                          rx,
    OUT  GT_U32                         *valuePtr
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(SIM_CM3_DEVICE_ID);

    return snetFalconPortStatisticFramesTransmittedCounterGet(devObjPtr,macPort,rx,valuePtr);
}

/**
* @internal cm3PortStatisticCounterDump function
* @endinternal
*
* @brief   Dump port frames counter.Used for debug.
*
* @param[in] macPort                  - MAC port number
* @param[in] rx              -  if equal GT_TRUE then RX_aFramesReceivedOK is printed ,else TX_aFramesTransmittedOK.
*
*
*/
GT_STATUS cm3PortStatisticCounterDump
(
    IN GT_U32                           macPort,
    IN GT_BOOL                          rx
)
{
    GT_U32 val;
    cm3PortStatisticCounterInternalGet(macPort,rx,&val);
    SIM_CM3_LOG("\nMac %d %s frames count %d\n", macPort,rx?"RX":"TX",val);
    return GT_OK;
}

/**
* @internal cm3BootChannelProcessorInitializedGet function
* @endinternal
*
* @brief   Validate magic is initialized.
*
*
*/

GT_STATUS cm3BootChannelProcessorInitializedGet
(
    GT_U32   *statePtr
)
{
    GT_U32 offset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    GT_U32 data;
    GT_STATUS rc = GT_OK;

    offset += (BOOT_CHANNEL_SIZE-4);

    cm3BootChannelReadWord(offset,&data);

    SIM_CM3_LOG("Magic [end] 0x%08x at adress 0x%08x\n", data,offset);

    if(data!=IPC_RESVD_MAGIC)
    {
        SIM_CM3_LOG("Bad magic.\n", offset,data);
        return GT_NOT_INITIALIZED;
    }

     offset -=4;

     /*read pointer*/

     cm3BootChannelReadWord(offset,&data);

     SIM_CM3_LOG("Pointer 0x%08x at adress 0x%08x\n",data, offset);

     offset = data+UBOOT_CM3_PEX_START_ADRESS;

     cm3BootChannelReadWord(offset,&data);

     SIM_CM3_LOG("Magic [start] 0x%08x at adress 0x%08x\n", data,offset);

     if(data!=IPC_RESVD_MAGIC)
     {
         SIM_CM3_LOG("Bad magic.\n", offset,data);
         return GT_NOT_INITIALIZED;
     }

     cm3BootChannelReadWord(offset+4,&data);

     switch(data&3)
     {
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E:
            SIM_CM3_LOG("State MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E\n");
            break;
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E:
            SIM_CM3_LOG("State MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E\n");
            break;
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E:
            SIM_CM3_LOG("State MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E\n");
            break;
        default:
            SIM_CM3_LOG("Bad state %d.\n", (data&3));
            rc = GT_BAD_STATE;
            break;
     }

     if(statePtr)
     {
        *statePtr = data&3;
     }
    return rc;
}



GT_STATUS cm3BootChannelExecuteCliCommandsLb
(
    GT_U32 port,
    GT_BOOL enable
)
{
    GT_U32 size,i;
    GT_STATUS rc;


    GT_CHAR lbCommand[60];

    size = 1;

    if(enable)
    {
        sprintf(lbCommand,"interface ethernet 0/%d loopback mode mac_tx2rx",port);
    }
    else
    {
        sprintf(lbCommand,"interface ethernet 0/%d loopback mode no_loopback",port);
    }

    for(i=0;i<size;i++)
    {

        SIM_CM3_LOG("Send command %s via boot channel\n",lbCommand);

        rc =cm3BootChannelSendCliCommand(lbCommand,GT_TRUE);

        /*only if command reception was ACKed then check the result*/
        if(GT_OK!=rc)
        {
            if(GT_TIMEOUT!=rc)
            {
                SIM_CM3_LOG("Command reception is confirmed.But error code returned.\n");
                cm3BootChannelErrorCodeGet();
            }
            else
            {
                SIM_CM3_LOG("Command reception is NOT confirmed\n");
            }
       }

    }

    return GT_OK;
}


GT_STATUS cm3BootChannelExecuteCliCommandsPve
(
    GT_U32 port,
    GT_BOOL enable
)
{
    GT_U32 size,i;
    GT_STATUS rc;
    GT_U32    data =0;
    GT_U32    mask = 1<<14;
    GT_U32    addr;


    GT_CHAR brgPrvEdgeVlanEnableCommand[60];
    GT_CHAR brgPrvEdgeVlanPortEnableCommand[60];
    GT_CHAR_PTR command = brgPrvEdgeVlanEnableCommand;

    size = 4;

    if(enable)
    {
        data=1<<14;

    }

    sprintf(brgPrvEdgeVlanEnableCommand,"wrd 0 0 0x03800000 0x%08x  0x%08x",data,mask);
    data =0;
    mask=0;

    addr = 0x3d00000 + 16*port;
    if(enable)
    {
        data=1<<11;
        data |= port<<13;

    }

    mask = 0x7FFF800;/*bits 11-26*/

    sprintf(brgPrvEdgeVlanPortEnableCommand,"wrd 0 0 0x%08x 0x%08x  0x%08x",addr,data,mask);

    for(i=0;i<size;i++)
    {

        if(i==1)
        {
            command = brgPrvEdgeVlanPortEnableCommand;
        }
        else if(i==2)
        {
           mask=0;
           data =0x02000100;
           addr+=4;

           sprintf(brgPrvEdgeVlanPortEnableCommand,"wrd 0 0 0x%08x 0x%08x  0x%08x",addr,data,mask);
        }
        else if(i>2)
        {
            data =0x0;
            addr+=4;
            sprintf(brgPrvEdgeVlanPortEnableCommand,"wrd 0 0 0x%08x 0x%08x  0x%08x",addr,data,mask);
        }

        SIM_CM3_LOG("Send command %s via boot channel\n",command);

        rc =cm3BootChannelSendCliCommand(command,GT_TRUE);

        /*only if command reception was ACKed then check the result*/
        if(GT_OK!=rc)
        {
            if(GT_TIMEOUT!=rc)
            {
                SIM_CM3_LOG("Command reception is confirmed.But error code returned.\n");
                cm3BootChannelErrorCodeGet();
            }
            else
            {
                SIM_CM3_LOG("Command reception is NOT confirmed\n");
            }
       }

    }

    return GT_OK;
}


GT_STATUS cm3BootChannelSendMiStatusGetCommand
(
)
{
    GT_U32 wordsToWrite,i;
    GT_U32 offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES+2*PARAM_SIZE_IN_BYTES;/*2  params + 2 bytes for opcode*/


    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    /*start with second line since "ready" bit is in the first word*/
    offset = HARRIER_UBOOT_BOOT_CHANNEL_OFFSET+4;

    memset(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = MICRO_INIT_BOOT_CH_OPCODE_GET_MI_STATE;
    wordsToWrite = HEADER_SIZE_IN_WORDS + sizeWithOpcode/4;

    SIM_CM3_LOG("Send message %d with %d lines.\n", msg[1],msg[1],wordsToWrite);

    for(i=1;i<wordsToWrite;i++)
    {
         value = msg[i];

         cm3BootChannelWriteWord(offset,value );

         offset+=4;

    }

    offset =HARRIER_UBOOT_BOOT_CHANNEL_OFFSET;
    value = msg[0];

    cm3BootChannelWriteWord(offset,value );


    /*get the answer*/

    return cm3BootChannelAnswerGet();
}


GT_STATUS cm3BootChannelExecuteMiStatusGetCommand
(
    GT_U32 *miStatusPtr
)
{
    GT_U32 buffer;
    /*only if command reception was ACKed then check the result*/
    if(GT_OK==cm3BootChannelSendMiStatusGetCommand())
    {
        SIM_CM3_LOG("Command reception is confirmed.Check status.\n");
        cm3BootChannelReadData(&buffer,1,0);
        SIM_CM3_LOG("Mi status 0x%x\n",buffer);
        if(miStatusPtr)
        {
            *miStatusPtr = buffer;
        }
    }
    else
    {
        SIM_CM3_LOG("Command reception is NOT confirmed\n");
    }

    return GT_OK;
}

GT_STATUS cm3BootChannelVlanAction
(
    GT_U32 port,
    GT_U32 vlan,
    GT_BOOL add,
    GT_BOOL tag
)
{
    GT_U32 adress;
    GT_U32 offset,word,data = 0,line;
    GT_CHAR  buf[60];
    GT_STATUS rc;
    GT_U32 i;

    if(GT_TRUE == add)
    {
        /*CP/<L2I> L2I/Units/L2I_IP_Units/Bridge Tables/Ingress Bridge Vlan Table*/
        adress = 0x03C40000 + 0x10*vlan;


        for(i=0;i<4;i++)
        {
            sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+i*4,(i==0)?1:0,(i==0)?1:0);
            SIM_CM3_LOG("Send command %s via boot channel\n",buf);

            rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
            if(rc!=GT_OK)
            {
                return rc;
            }
        }
    }

     /* EGF sht Common/EGF eVLAN Members Table/eVLAN Members Table Entry Structure*/

    adress = 0x7a00000 + 0x10*vlan;
    word = port>>5;
    offset = port&0x1F;

    if(GT_TRUE==add)
    {
        data=1;
    }

    for(i=0;i<4;i++)
    {
        sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+i*4,(i==word)?(data<<offset):0,(i==word)?(1<<offset):0);
        SIM_CM3_LOG("Send command %s via boot channel\n",buf);

        rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

     /* <L2I> L2I/Units/L2I_IP_Units/Bridge Tables/Ingress Bridge Port Membership Table*/
     adress = 0x03B40000 +128*vlan;

    for(i=0;i< 32;i++)
    {
       sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+i*4,(i==word)?(data<<offset):0,(i==word)?(1<<offset):0);
       SIM_CM3_LOG("Send command %s via boot channel\n",buf);

       rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
       if(rc!=GT_OK)
       {
          return rc;
       }
    }

   if(add&&tag)
   {
       /*<EGF>EGF_IP/<EGF> EGF_IP_qag/Units/EGF_IP_qag/Distributor/eVLAN Tag Command Table*/
       data = tag?1:0;

       adress = 0x07900000 +0x40*vlan;
       line = port>>4;
       offset = (port&0xF)*3;

       for(i=0;i<2;i++)
       {
           if(offset<=27)
           {
               if(offset<=27)
               {
                 sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+line*8+i*4,i==0? (data<<offset):0,i==0? (7<<offset):0);
               }

               SIM_CM3_LOG("Send command %s via boot channel\n",buf);

               rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
               if(rc!=GT_OK)
               {
                  return rc;
               }
           }
           else if(offset>=33)
           {
               if(i==1)
               {
                offset-=32;
               }
               sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+line*8+i*4,i==1? (data<<offset):0,i==1? (7<<offset):0);

               SIM_CM3_LOG("Send command %s via boot channel\n",buf);

               rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
               if(rc!=GT_OK)
               {
                  return rc;
               }
           }
           else /*offset is 30*/
           {
                sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+line*8,0<<offset,1<<offset);
                SIM_CM3_LOG("Send command %s via boot channel\n",buf);
                rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
                if(rc!=GT_OK)
                {
                   return rc;
                }
                sprintf(buf,"wrd 0 0 0x%x 0x%x 0x%x",adress+line*8+4,data,3);
                SIM_CM3_LOG("Send command %s via boot channel\n",buf);
                rc = cm3BootChannelSendCliCommand(buf,GT_FALSE);
                if(rc!=GT_OK)
                {
                   return rc;
                }

                break;
           }
       }

   }

   return GT_OK;
}



