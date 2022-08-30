/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChBootChannelHandler.c
*
* @brief boot channel APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelHandler.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define HOST_OWNERSHIP_BIT 1
#define HEADER_SIZE_IN_BYTES 4
#define HEADER_SIZE_IN_WORDS (HEADER_SIZE_IN_BYTES/4)
#define OPCODE_SIZE_IN_BYTES 2
#define PRV_BOOT_CHANEL_VERSION_STRING_LEN (32)
#define PARAM_SIZE_IN_BYTES  4
#define SLEEP_TIME (100)

#define PRV_CONF_PROCESSOR_MEMORY_OFFSET 0x80000
#define PRV_DATA_REGION_REGISTERS_OFFSET 256 /*offset is in bytes*/
#define PRV_DATA_REGION_BC_DATA_OFFSET 4     /*offset is in bytes*/

/*First two bytes is for opcode.Each parameter is 4 bytes*/
#define BC_PARAM(_msg,_i)     _msg[2+_i*4]  /* First index 0 */

typedef struct
{
    MV_HWS_PORT_STANDARD                portMode;
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode;
    CPSS_PORT_SPEED_ENT                 speed;
}PRV_CPSS_BOOT_CH_PORT_MODE_TO_SPEED_AND_MODE_IF_STC;

extern GT_U32 prvCpssDrPpConInitMg0UnitBaseAddressGet
(
     IN GT_U8      devNum
);

static const PRV_CPSS_BOOT_CH_PORT_MODE_TO_SPEED_AND_MODE_IF_STC portModeToSpeedAndModeIf[] =
{
    {SGMII,                 CPSS_PORT_INTERFACE_MODE_SGMII_E,           CPSS_PORT_SPEED_1000_E},
    {_1000Base_X,           CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,      CPSS_PORT_SPEED_1000_E},
    {SGMII2_5,              CPSS_PORT_INTERFACE_MODE_SGMII_E,           CPSS_PORT_SPEED_2500_E},
    {QSGMII,                CPSS_PORT_INTERFACE_MODE_QSGMII_E,          CPSS_PORT_SPEED_2500_E},
    {_5GBaseR,              CPSS_PORT_INTERFACE_MODE_KR_E,              CPSS_PORT_SPEED_5000_E},
    {_10GBase_KR,           CPSS_PORT_INTERFACE_MODE_KR_E,              CPSS_PORT_SPEED_10000_E},
    {_10GBase_SR_LR,        CPSS_PORT_INTERFACE_MODE_SR_LR_E,           CPSS_PORT_SPEED_10000_E},
    {RXAUI,                 CPSS_PORT_INTERFACE_MODE_RXAUI_E,           CPSS_PORT_SPEED_10000_E},
    {_20GBase_KR,           CPSS_PORT_INTERFACE_MODE_KR_E,              CPSS_PORT_SPEED_20000_E},
    {_20GBase_KR2,          CPSS_PORT_INTERFACE_MODE_KR2_E,             CPSS_PORT_SPEED_20000_E},
    {_25GBase_KR,           CPSS_PORT_INTERFACE_MODE_KR_E,              CPSS_PORT_SPEED_25000_E},
    {_25GBase_CR,           CPSS_PORT_INTERFACE_MODE_CR_E,              CPSS_PORT_SPEED_25000_E},
    {_25GBase_CR_C,         CPSS_PORT_INTERFACE_MODE_CR_C_E,            CPSS_PORT_SPEED_25000_E},
    {_25GBase_CR_S,         CPSS_PORT_INTERFACE_MODE_CR_S_E,            CPSS_PORT_SPEED_25000_E},
    {_25GBase_KR_C,         CPSS_PORT_INTERFACE_MODE_KR_C_E,            CPSS_PORT_SPEED_25000_E},
    {_25GBase_KR_S,         CPSS_PORT_INTERFACE_MODE_KR_S_E,            CPSS_PORT_SPEED_25000_E},
    {_40GBase_KR4,          CPSS_PORT_INTERFACE_MODE_KR4_E,             CPSS_PORT_SPEED_40000_E},
    {_40GBase_SR_LR4,       CPSS_PORT_INTERFACE_MODE_SR_LR4_E,          CPSS_PORT_SPEED_40000_E},
    {_40GBase_CR4,          CPSS_PORT_INTERFACE_MODE_CR4_E,             CPSS_PORT_SPEED_40000_E},
    {_50GBase_KR2_C,        CPSS_PORT_INTERFACE_MODE_KR4_E,             CPSS_PORT_SPEED_50000_E},
    {_50GBase_CR2_C,        CPSS_PORT_INTERFACE_MODE_CR2_E,             CPSS_PORT_SPEED_50000_E},
    {_50GBase_SR_LR,        CPSS_PORT_INTERFACE_MODE_SR_LR_E,           CPSS_PORT_SPEED_50000_E},
    {_50GBase_KR,           CPSS_PORT_INTERFACE_MODE_KR_E,              CPSS_PORT_SPEED_50000_E},
    {_50GBase_KR2,          CPSS_PORT_INTERFACE_MODE_KR2_E,             CPSS_PORT_SPEED_50000_E},
    {_100GBase_CR4,         CPSS_PORT_INTERFACE_MODE_CR4_E,             CPSS_PORT_SPEED_100G_E},
    {_100GBase_KR2,         CPSS_PORT_INTERFACE_MODE_KR2_E,             CPSS_PORT_SPEED_100G_E},
    {_100GBase_KR4,         CPSS_PORT_INTERFACE_MODE_KR4_E,             CPSS_PORT_SPEED_100G_E},
    {_100GBase_SR4,         CPSS_PORT_INTERFACE_MODE_SR_LR4_E,          CPSS_PORT_SPEED_100G_E},
    {_200GBase_KR4,         CPSS_PORT_INTERFACE_MODE_KR4_E,             CPSS_PORT_SPEED_200G_E},

    /*must be last*/
    {GT_NA,                 GT_NA,                                      GT_NA}
};

static GT_U32 confProcesBootChannelStartAddrGet
(
    GT_U8     devNum
)
{
    GT_U32 confProcessorMemorySize = 0 ,bootChannelSize = 0;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        confProcessorMemorySize = _256K ;
        bootChannelSize = _2K ;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return (confProcessorMemorySize - bootChannelSize);
}

static GT_U32 getDirectBootChannelOffset
(
    GT_U8     devNum
)
{
    return  prvCpssDrPpConInitMg0UnitBaseAddressGet(devNum) +
            PRV_CONF_PROCESSOR_MEMORY_OFFSET +
            confProcesBootChannelStartAddrGet(devNum);
}

/*this function check error code from MI boot channel status register and reset it
  note : only STATUS_REG_OPCODE_ERR_CODE_E gets clear on read */
static GT_STATUS prvBootChannelHandlerErrorCodeCheck
(
    GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32 offset[2], value;

    offset[0] = getDirectBootChannelOffset(devNum) + PRV_DATA_REGION_REGISTERS_OFFSET +(PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E * 4);
    offset[1] = getDirectBootChannelOffset(devNum) + PRV_DATA_REGION_REGISTERS_OFFSET +(PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E * 4);

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, offset[0], &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (value != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E 0x%08x\n", value);
    }

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, offset[1], &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*reset value */
    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, offset[1], 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (value != 0)
    {
        if (value == 1 ) /* return value in case of port not init by MI  */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E 0x%08x\n", value);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*this function return massage return from MI*/
static GT_STATUS prvBootChannelHandlerReadData
(
    GT_U8     devNum,
    GT_U32   *bufferPtr,
    GT_U32    sizeInWords,
    GT_U32    firstWord
)
{
    GT_STATUS rc ;
    GT_U32 offset,ii;
    GT_U32 initOffset = getDirectBootChannelOffset(devNum);

    offset = initOffset+PRV_DATA_REGION_BC_DATA_OFFSET ; /*data region start after 4 bytes */
    offset +=(firstWord*4);

    rc = prvBootChannelHandlerErrorCodeCheck(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(ii=0;ii<sizeInWords;ii++,bufferPtr++,offset+=4)
    {
      rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,offset,bufferPtr);
      if (rc != GT_OK)
      {
          return rc ;
      }
    }
    return GT_OK;
}

/*this function used to check if answer returned from MI*/
static GT_STATUS prvBootChannelHandlerAnswerGet
(
    GT_U8 devNum
)
{
    GT_STATUS rc ;
    GT_32  maxIter= 10;
    GT_U32 value,result;
    GT_U32 offset = getDirectBootChannelOffset(devNum);

    do
    {
        cpssOsTimerWkAfter(SLEEP_TIME);
        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,offset,&value);
        if (rc != GT_OK)
        {
            return rc ;
        }

        maxIter--;
        if((value >>31)^HOST_OWNERSHIP_BIT)
        {
            /*check result*/
            result = value & 0xff;
            return  result==0?GT_OK:GT_FAIL;
        }
    }
    while(maxIter);

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, "time out during prvBootChannelHandlerAnswerGet ");
}




/*this function sends version request */
static GT_STATUS prvBootChannelHandlerVersionGet
(
    GT_U8 devNum
)
{
    GT_STATUS rc ;
    GT_U32 wordsToWrite,ii;
    GT_U32 initOffset,offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];

    /* fix jira: IPBUSW-13148 : ASIM-CN10KAS: Unable to set profile 4 and 6 in HS mode in the kernel */
    rc = prvBootChannelHandlerErrorCodeCheck(devNum);
    if (rc != GT_OK)
    {
        /* we ignore any 'last' previous error that may come from the uboot time */
    }

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES;/*additional 2 bytes for opcode*/

    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;
    initOffset = getDirectBootChannelOffset(devNum);

    /*start with second line since "ready" bit is in the first word*/
    offset=initOffset+4;

    cpssOsMemSet(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION;

    wordsToWrite = HEADER_SIZE_IN_WORDS + (sizeWithOpcode)/4;

    CPSS_LOG_INFORMATION_MAC("Send message [PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION] %d lines.\n",wordsToWrite);

    for(ii=1;ii<wordsToWrite;ii++)
    {
        value = msg[ii];
        CPSS_LOG_INFORMATION_MAC("Write adress 0x%08x data 0x%08x]\n",offset,value);
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,offset,value);
        if (rc != GT_OK)
        {
            return rc ;
        }
        offset+=4;
    }

    /*send execution bit */
    offset =initOffset;
    value = msg[0];
    CPSS_LOG_INFORMATION_MAC("Write adress 0x%08x data 0x%08x]\n",offset,value);
    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,offset,value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    /*get the answer*/
    return prvBootChannelHandlerAnswerGet(devNum);
}

/*this function sends port status request */
static GT_STATUS prvBootChannelHandlerPortStatusGet
(
    GT_U8                                  devNum,
    GT_U32                                 portMacNum
)
{
    GT_STATUS rc ;
    GT_U32 wordsToWrite,ii;
    GT_U32 offset,value;
    GT_U8 sizeWithOpcode;
    GT_U32 msg[64];
    GT_U8 * tmp;

    sizeWithOpcode= OPCODE_SIZE_IN_BYTES+2*PARAM_SIZE_IN_BYTES;/*2  params + 2 bytes for opcode*/

    /*clip to words*/
    sizeWithOpcode = ((sizeWithOpcode+3)/4)*4;

    /*start with second line since "ready" bit is in the first word*/
    offset = getDirectBootChannelOffset(devNum) + 4;

    cpssOsMemSet(msg,0,sizeof(msg));

    msg[0] = (HOST_OWNERSHIP_BIT<<31)|((sizeWithOpcode)<<18);/*header*/
    msg[1] = PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS;
    wordsToWrite = HEADER_SIZE_IN_WORDS + sizeWithOpcode/4;

    tmp =(GT_U8 *) &msg[1];

    /*data*/
    BC_PARAM(tmp,1)=portMacNum;
    CPSS_LOG_INFORMATION_MAC("Send message [PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS] for MAC [%d] %d lines.\n" ,portMacNum,wordsToWrite);

    for(ii=1;ii<wordsToWrite;ii++)
    {
        value = msg[ii];
        CPSS_LOG_INFORMATION_MAC("Write adress 0x%08x data 0x%08x]\n",offset,value);
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,offset,value);
        if (rc != GT_OK)
        {
            return rc ;
        }
        offset+=4;
    }

    /*send execution bit */
    offset = getDirectBootChannelOffset(devNum);
    value = msg[0];
    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,offset,value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    /*get the answer*/
    return prvBootChannelHandlerAnswerGet(devNum);
}

/*this function convert portMode into ifmode and speed*/
static GT_STATUS prvBootChannelHandlerPortModeToSpeedAndModeIf
(
    MV_HWS_PORT_STANDARD             portMode,
    CPSS_PORT_INTERFACE_MODE_ENT    *ifModePtr,
    CPSS_PORT_SPEED_ENT             *speedPtr
)
{
    GT_U32 ii;

    for (ii=0;portModeToSpeedAndModeIf[ii].portMode != GT_NA ; ii++)
    {
        if (portModeToSpeedAndModeIf[ii].portMode == portMode)
        {
            *ifModePtr = portModeToSpeedAndModeIf[ii].ifMode;
            *speedPtr  = portModeToSpeedAndModeIf[ii].speed;
            return GT_OK;
        }
    }

    /*portMode is not in supported list  */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChBootChannelHandlerVersionGet function
* @endinternal
*
* @brief   This function gets superImage, microInit and booton versions
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                     - device number
* @param[OUT] superImageVersionPtr      - (pointer to) super image version .
* @param[OUT] miVersionPtr              - (pointer to) MI version .
* @param[OUT] bootonVersionPtr          - (pointer to) booton version .
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE        - not applicable device
*
*/
GT_STATUS prvCpssDxChBootChannelHandlerVersionGet
(
    IN   GT_U8                      devNum,
    OUT  CPSS_VERSION_INFO_STC     *superImageVersionPtr,
    OUT  CPSS_VERSION_INFO_STC     *miVersionPtr,
    OUT  CPSS_VERSION_INFO_STC     *bootonVersionPtr
)
{
    GT_STATUS rc ;
    GT_CHAR buffer[PRV_BOOT_CHANEL_VERSION_STRING_LEN+1];
    GT_U32 firstWord =0,ii ;
    GT_CHAR_PTR names [] = {"SuperImage","MI ","Booton"};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(superImageVersionPtr);
    CPSS_NULL_PTR_CHECK_MAC(miVersionPtr);
    CPSS_NULL_PTR_CHECK_MAC(bootonVersionPtr);

    /*send version command and check answer*/
    rc = prvBootChannelHandlerVersionGet(devNum);
    if(GT_OK == rc )
    {
        CPSS_LOG_INFORMATION_MAC("Command reception is confirmed.Check the version.\n");
        for(ii=0;ii<3;ii++)
        {
            rc = prvBootChannelHandlerReadData(devNum,(GT_U32 *)buffer,PRV_BOOT_CHANEL_VERSION_STRING_LEN/4,firstWord);
            if (rc !=GT_OK)
            {
                return rc;
            }
            buffer[PRV_BOOT_CHANEL_VERSION_STRING_LEN] = '\0';
            CPSS_LOG_INFORMATION_MAC("%d %s version [%s]\n",ii,names[ii],buffer);
            firstWord +=(PRV_BOOT_CHANEL_VERSION_STRING_LEN/4);

            switch (ii)
            {
            case 0:
                cpssOsMemCpy(superImageVersionPtr->version, &buffer,sizeof(superImageVersionPtr->version));
                break;
            case 1:
                cpssOsMemCpy(miVersionPtr->version, &buffer,sizeof(superImageVersionPtr->version));
                break;
            case 2:
                cpssOsMemCpy(bootonVersionPtr->version,&buffer,sizeof(superImageVersionPtr->version));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        names[0] = names[0]; /* to avoid warnings */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "Command reception is NOT confirmed rc %d\n",rc);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBootChannelHandlerPortStatusGet function
* @endinternal
*
* @brief  Gets link Status the speed and mode of the port configured by MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] portInfoPtr             - (pointer to) port info stc
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_SUPPORTED         - on not supported port mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChBootChannelHandlerPortStatusGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT PRV_CPSS_BOOT_CH_PORT_STATUS_STC      *portInfoPtr
)
{
    GT_STATUS                       rc ;
    GT_U32                          buffer;
    MV_HWS_PORT_STANDARD            portMode;
    GT_U32                          portMacNum; /* MAC number */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_DXCH_PORT_FEC_MODE_ENT     fecMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /*only if command reception was ACKed then check the result*/
    rc = prvBootChannelHandlerPortStatusGet(devNum,portMacNum);
    if(GT_OK == rc)
    {
        CPSS_LOG_INFORMATION_MAC("Command reception is confirmed.Check status.\n");
        rc = prvBootChannelHandlerReadData(devNum,&buffer,1,0);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_LOG_INFORMATION_MAC("BMP 0x%x\n", buffer & 3);
        CPSS_LOG_INFORMATION_MAC("Mode %d\n",buffer>>8);
        portMode = buffer>>8;
        rc = prvBootChannelHandlerPortModeToSpeedAndModeIf(portMode, &ifMode, &speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portMode is not supported in HS  portMode %d  rc %d \n",portMode, rc);
        }

        /*get fec mode */
        rc = prvCpssDxChPortFecModeGet(devNum,portNum,portMode,&fecMode);
        if (rc != GT_OK)
        {
            return rc ;
        }
        portInfoPtr->ifMode = ifMode;
        portInfoPtr->speed = speed;
        portInfoPtr->fecMode = fecMode ;

        /*get Link status */
        portInfoPtr->isLinkUp = buffer & 0x1;

        /*
            Port Status Bitmap (32-bit):
            Bit 0 - PORT_LINK STATUS (1 - Link UP, 0- Link Down)
            Bit 1 - PORT_AP_RESOLUTION_DONE (1 - HCD Found, 0 - - Link Down)
            Bit 2 - PORT_AP_RESOLUTION_FAILED (1 - HCD Not found, 0 - Link UP)
        */
        if (portInfoPtr->isLinkUp == GT_TRUE)
        {   /*link UP check bit 1 for ap resolution done */
            portInfoPtr->apMode = (buffer & 0x2) ? GT_TRUE : GT_FALSE  ;
        }
        else
        {   /*link DOWN check bit 2 for ap resolution failed */
            portInfoPtr->apMode = (buffer & 0x4) ? GT_TRUE : GT_FALSE  ;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "Command reception is NOT confirmed rc %d\n",rc);
    }
    return GT_OK;
}
