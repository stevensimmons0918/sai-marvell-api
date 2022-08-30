/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDragonite.c
*
* @brief Dragonite appDemo code.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#if defined(_linux) && !defined(ASIC_SIMULATION)


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define APP_DEMO_DRAGONITE_FW_FILE_SIZE     65536
#define APP_DEMO_DRAGONITE_FW_FILE_PATH     "/usr/bin/dragonite.bin"

static GT_U8 dragoniteDevNum = 0;

typedef struct POE_CUSTOMER_INFO_DATA_STC
{
  GT_U32 version_number; /* struct version */
  GT_U32 struct_size; /* struct sizeof */
  GT_U8  *sign_date; /* date of generate the version  */
  GT_U8  *sign_time; /* time of generate the version  */
  GT_U8  *chr_array_ptr; /* TBD string */
  GT_U16 *software_pn; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U8  *software_build; /* this value is the same as in the 15 BYTE communication protocol doc */
  GT_U8  *product_number; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U16 *kernel_version; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U8  *salad_param_code; /* this value is the same as in the 15 BYTE communication protocol doc  */

}POE_CUSTOMER_INFO_DATA_STC;


GT_STATUS appDemoDragoniteMessageCheckSumCalc
(
    IN  GT_U8   *bufferPtr,
    IN  GT_U32  msgLen,
    OUT GT_U16  *checkSumPtr
)
{
    GT_U16  checkSum = 0;

    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);
    CPSS_NULL_PTR_CHECK_MAC(checkSumPtr);

    while (msgLen > 0)
    {
        msgLen--;
        checkSum += bufferPtr[msgLen];
    }

    *checkSumPtr = checkSum;

    return GT_OK;
}

static GT_STATUS prvDrvCpssWriteMasked(
    IN  void *drvData,
    IN  GT_U32 addrSpace,
    IN  GT_U32 regAddr,
    IN  GT_U32 data,
    IN  GT_U32 mask)
{
    GT_U8 dev = (GT_U8)((GT_UINTPTR)drvData);
    GT_STATUS rc = GT_FAIL;
    if (addrSpace == 0)
        return cpssDrvPpHwRegBitMaskWrite(dev, 0, regAddr, mask, data);
    if (mask != 0xffffffff)
    {
        GT_U32 oldVal;
        if (addrSpace == 1)
            rc = cpssDrvPpHwInternalPciRegRead(dev, 0, regAddr, &oldVal);
        if (addrSpace == 2)
            rc = cpssDrvHwPpResetAndInitControllerReadReg(dev, regAddr, &oldVal);
        if (rc != GT_OK)
            return rc;
        data &= mask;
        data |= oldVal & (~mask);
    }
    if (addrSpace == 1)
        return cpssDrvPpHwInternalPciRegWrite(dev, 0, regAddr, data);
    if (addrSpace == 2)
        return cpssDrvHwPpResetAndInitControllerWriteReg(dev, regAddr, data);
    return GT_NOT_SUPPORTED;
}

/**
* @internal appDemoDragoniteDevSet function
* @endinternal
*
* @brief   Select device with dragonite unit
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - device not exists/not initialized
*/
GT_STATUS appDemoDragoniteDevSet(
    IN  GT_U8   devNum
)
{
    if (cpssDrvHwPpHwInfoStcPtrGet(devNum, 0) == NULL)
    {
        cpssOsPrintf("device %d not exists/not initialized\n", devNum);
        return GT_BAD_PARAM;
    }
    dragoniteDevNum = devNum;
    return GT_OK;
}

/**
* @internal appDemoDragoniteSoftwareDownload function
* @endinternal
*
* @brief   Example to show Dragonite FW download.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteSoftwareDownload()
{

    GT_STATUS rc = GT_OK;
    char *buf = NULL;
    GT_U32 nr;
    GT_BOOL msg;
    GT_32 rfd = 0, i;
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    CPSS_HW_INFO_STC    *hwInfoPtr;

    hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dragoniteDevNum, 0);
    if (hwInfoPtr == NULL)
    {
        cpssOsPrintf("device %d is not initialized or doesn't exists\n", dragoniteDevNum);
        cpssOsPrintf("use appDemoDragoniteDevSet(devId) to select device\n");
        return GT_FAIL;
    }
    /* Initialize Dragonite driver */
    rc = extDrvDragoniteDriverInit(
            hwInfoPtr, (void*)((GT_UINTPTR)dragoniteDevNum),
            (DRAGONITE_HW_WRITE_MASKED_FUNC)prvDrvCpssWriteMasked);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* config and enable dragonite sub-system */
    rc = extDrvDragoniteDevInit();
    if(rc != GT_OK)
    {
        return rc;
    }

    buf = cpssOsMalloc(APP_DEMO_DRAGONITE_FW_FILE_SIZE);
    if (buf == NULL)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: malloc failed\r\n");
        return GT_OUT_OF_CPU_MEM;
    }

    /* read FW file */
    rfd = open(APP_DEMO_DRAGONITE_FW_FILE_PATH, O_RDONLY);
    if (rfd <= 0)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: open file failed\r\n");
        rc = GT_BAD_STATE;
        goto fail_label;
    }

    nr = read(rfd, buf, APP_DEMO_DRAGONITE_FW_FILE_SIZE);
    if (nr != APP_DEMO_DRAGONITE_FW_FILE_SIZE)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: read file failed\r\n");
        rc = GT_BAD_STATE;
        goto fail_label;
    }

    rc = extDrvDragoniteFwDownload(buf, nr);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: download FW failed\r\n");
        goto fail_label;
    }

   /* Init DTCM with 0xFF */
    for (i = 0; i < 0xB00; i++)
    {
        rc = extDrvDragoniteRegisterWrite(HOST_OUTPUT_TO_POE_REG_ADDR + i, 0xFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("appDemoDragoniteSoftwareDownload: write register failed, addr=0x%x\r\n", (HOST_OUTPUT_TO_POE_REG_ADDR + i));
            goto fail_label;
        }
    }

    /* Init Host protocol version to '1' in order to 'speak' with Dragonite with the same 'language' */
    rc = extDrvDragoniteRegisterWrite(HOST_PROT_VER_REG_ADDR, 0x1);
    if(rc != GT_OK)
    {
        goto fail_label;
    }

    msg = GT_TRUE;
    rc = extDrvDragoniteUnresetSet(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: Dragonite UnReset failed\r\n");
        goto fail_label;
    }

    cpssOsTimerWkAfter(500);

    /* Get system status message from POE that is ready after unreset */
    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: MSG receive was failed\r\n");
        goto fail_label;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n\r\n\r\n", msg_reply[8]);



/* fallthrough in case ok */
fail_label:
    if(buf != NULL)
    {
        cpssOsFree(buf);
    }

    close(rfd);

    return rc;
}


/**
* @internal appDemoDragoniteProtocolVersionShow function
* @endinternal
*
* @brief   Print version info from FW binary image.
*
* @param[in] buf                      - (pointer to) FW buffer
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteProtocolVersionShow
(
    IN char *buf
)
{
    POE_CUSTOMER_INFO_DATA_STC *customerInfoPtr;
    GT_U32 verInfoOffset;

    CPSS_NULL_PTR_CHECK_MAC(buf);

    verInfoOffset = *(GT_U32*)(buf + APP_DEMO_DRAGONITE_FW_FILE_SIZE - 8);
    customerInfoPtr = (POE_CUSTOMER_INFO_DATA_STC*)(buf + verInfoOffset);

    osPrintf("buf 0x%x, offset: 0x%x, ptr: 0x%x\n", buf, verInfoOffset, customerInfoPtr);

    osPrintf("sw build: %d\n", *(buf + (GT_UINTPTR)customerInfoPtr->software_build) );
    osPrintf("sw pn: %d\n", *(GT_U16*)(buf + (GT_UINTPTR)customerInfoPtr->software_pn) );
    osPrintf("date: %.15s\n", buf + (GT_UINTPTR)customerInfoPtr->sign_date);

    return GT_OK;
}



/**
* @internal appDemoDragoniteMcuVersionRead function
* @endinternal
*
* @brief   Read MCU Protocol Version register.
*
* @param[out] versionPtr               - (pointer to) version value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteMcuVersionRead
(
    IN GT_U32 *versionPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(versionPtr);

    return extDrvDragoniteRegisterRead(PROT_VER_REG_ADDR ,versionPtr);
}

/**
* @internal appDemoDragoniteMcuResetCauseUpdate function
* @endinternal
*
* @brief   Inform the POE software about the reason that the host decided to
*         reset the POE MCU
* @param[in] cause                    -  reset
*                                      0xFF - power reset
*                                      0x01 - POE Not responding
*                                      0x02 - Application request
*                                      0x03 - Communication errors
*                                      0x04 - Host Application Reboot
*                                      0x05 - Undefined cause
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoDragoniteMcuResetCauseUpdate
(
    GT_U32 cause
)
{
    GT_STATUS  rc;
    GT_U32     regVal;
    GT_U16     counter;
    GT_U32     data;

    rc = extDrvDragoniteRegisterRead(HOST_RST_CAUSE_REG_ADDR, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    counter = (regVal & 0xFF);

    if (counter == MEM_INIT_VAL)
    {
        counter = 1;
    }
    else
    {
        counter++;
    }
    data = cause<<24 | counter;

    rc = extDrvDragoniteRegisterWrite(HOST_RST_CAUSE_REG_ADDR, data);
    return rc;
}

/**
* @internal appDemoDragoniteReadRegister function
* @endinternal
*
* @brief   Print value of Dragonite register
*
* @param[in] addr                     - address to read.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteReadRegister
(
    GT_U32 addr
)
{
    GT_STATUS rc;
    GT_U32 regValue;

    rc = extDrvDragoniteRegisterRead(addr, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\r\n\r\n extDrvDragoniteRegisterRead: FAILURE: rc = %d\r\n", rc);
        return rc;
    }
    cpssOsPrintf("\r\n\r\nextDrvDragoniteRegisterRead: value = 0x%x\r\n", regValue);

    return GT_OK;
}


/**
* @internal appDemoDragoniteWriteRegister function
* @endinternal
*
* @brief   Write value to Dragonite register.
*
* @param[in] addr                     - address to write.
* @param[in] regValue                 - value to write.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteWriteRegister
(
    GT_U32 addr,
    GT_U32 regValue
)
{
    GT_STATUS rc;

    rc = extDrvDragoniteRegisterWrite(addr, regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\r\n\r\n appDemoDragoniteReadRegister: FAILURE: rc = %d\r\n", rc);
        return rc;
    }

    return GT_OK;
}


/**
* @internal appDemoDragoniteTestExample1 function
* @endinternal
*
* @brief   POE 15B protocol command example
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteTestExample1()
{
    GT_STATUS rc;
    GT_U8 msg[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U16  checkSum = 0;

    /*
        Host Request:
            Get System Status
            DATA[0] = 0x2 - Request
            DATA[2] = 0x7 - Global
            DATA[3] = 0x3d - SystemStatus

        Controller Response:
            DATA[0] = 0x3 - Telemetry
            DATA[2] = CPU status 1
            DATA[3] = CPU status 2
            DATA[4] = Factory Default
            DATA[5] = GIE
            DATA[6] = Private Label
            DATA[7] = User byte
            DATA[8] = Device Fail
            DATA[9] = Temperature Disconnect
            DATA[10] = Temperature Alarm
            DATA[11] = Interrupt register
            DATA[12] = Interrupt register

        CPU status-1: Bit0 = ‘1’ indicates PoE controller error. Bit1 = ‘1’ indicates that firmware download is required.
        CPU status-2: Bit0 = ‘1’ indicates PoE controller memory error. Bit1 = ‘1’ indicates error (if there are less than eight
        PoE devices in the system, then Bit1= ‘1’).
        Factory default: Bit0 = ‘1’ indicates that factory default parameters are currently set.
        GIE (General Internal Error): When different from 0x00,it  indicates a general internal error.
        Private Label: Saved in the RAM. Equals 0x00 after reset. Refer to "Set System Status" command.
        User Byte - Saved in memory. Equals 0xFF, once set to factory default.
        Device Fail:  (1) Bits 0 to 7 indicate a failed PoE device(s).
            ’1’ = Fail or missing PoE Device, ‘0’ = PoE Device is OK.
        Temperature disconnect:  (1)  Bits 0 to 7 indicate over - heated PoE device(s).
        This over-heating causes disconnection of all ports.
            '1' = This PoE device caused disconnection due to high temperature, '0' = Temperature is OK.
        Temperature alarm:  (1)  Bits 0 to 7 indicate over heated PoE device(s).
        if temperature exceeds a pre-defined user defined limit, (Set PoE Device Params command),
        then the appropriate bit changes to ‘1’.
            '1' = High temperature, '0' = Temperature is OK.
        Interrupt register latches a transition when an event occurs. The transition might be one or more of several port
        status changes, PoE device status event/s or system event/s, depending on event definition.
        (1)  Bit 0 – PoE Device-‘0’, Bit 1 – PoE Device-‘1’, Bit 2 – PoE Device-2, Bit 7 – PoE Device-7.
        Device number is set according to its connectivity to the controller.

    */
    msg[0] = 0x2;
    msg[2] = 0x7;
    msg[3] = 0x3d;
    /* Fill byte[14] and byte[15] in 15B protocol with checksum */
    appDemoDragoniteMessageCheckSumCalc(msg, 13, &checkSum);
    msg[13] = U32_GET_FIELD_MAC(checkSum, 8, 8);
    msg[14] = U32_GET_FIELD_MAC(checkSum, 0, 8);

    rc = extDrvDragoniteMsgWrite(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
        return rc;
    }

    cpssOsTimerWkAfter(100);

    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
        return rc;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n", msg_reply[8]);
    cpssOsPrintf("REC_DATA[9] = 0x%x\r\n", msg_reply[9]);
    cpssOsPrintf("REC_DATA[10] = 0x%x\r\n", msg_reply[10]);
    cpssOsPrintf("REC_DATA[11] = 0x%x\r\n", msg_reply[11]);
    cpssOsPrintf("\r\n\r\n");

    return GT_OK;
}


/**
* @internal appDemoDragoniteTestExample2 function
* @endinternal
*
* @brief   POE 15B protocol command example
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteTestExample2()
{
    GT_STATUS rc;
    GT_U8 msg[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U16  checkSum = 0;

    /*
        Host Request:
            Get System Status
            DATA[0] = 0x2 - Request
            DATA[2] = 0x7 - Global
            DATA[3] = 0xb - Supply
            DATA[4] = 0x17 - Main

        Controller Response:
            DATA[0] = 0x3 - Telemetry
            DATA[2] = Power Consumption
            DATA[3] = Power Consumption
            DATA[4] = Max ShutDown Voltage
            DATA[5] = Max ShutDown Voltage
            DATA[6] = Min ShutDown Voltage
            DATA[7] = Min ShutDown Voltage
            DATA[8] = N
            DATA[9] = Power Bank
            DATA[10] = Power Limit
            DATA[11] = Power Limit
            DATA[12] = N

            Telemetry for main power supply parameters (within the working power budget):
            Power Consumption: Actual momentary total power consumption (units are in watts).
            Max Shutdown Voltage: Maximum voltage level set; above this level, PoE ports shutdown. Units are in decivolts.
            Min Shutdown Voltage: Minimum voltage level set; below this level, PoE ports shutdown. Units are given in decivolts.
            Power Bank: The current active Power Bank.
            Power Limit: If power consumption exceeds this level, lowest priority ports will be disconnected. To set the desired value, refer to
            (units are in watts).

    */
    msg[0] = 0x2;
    msg[2] = 0x7;
    msg[3] = 0xb;
    msg[4] = 0x17;
    /* Fill byte[14] and byte[15] in 15B protocol with checksum */
    appDemoDragoniteMessageCheckSumCalc(msg, 13, &checkSum);
    msg[13] = U32_GET_FIELD_MAC(checkSum, 8, 8);
    msg[14] = U32_GET_FIELD_MAC(checkSum, 0, 8);

    rc = extDrvDragoniteMsgWrite(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
        return rc;
    }

    cpssOsTimerWkAfter(100);

    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
        return rc;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n", msg_reply[8]);
    cpssOsPrintf("REC_DATA[9] = 0x%x\r\n", msg_reply[9]);
    cpssOsPrintf("REC_DATA[10] = 0x%x\r\n", msg_reply[10]);
    cpssOsPrintf("REC_DATA[11] = 0x%x\r\n", msg_reply[11]);
    cpssOsPrintf("\r\n\r\n");

    return GT_OK;
}


#endif



