/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains API functions and global data for higher-level 
Precision Time Protocol(PTP) functions for the 88X35X0 Ethernet PHYs.
********************************************************************/
#include <mtdFeatures.h>

#if MTD_TU_PTP

#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <TuPtp/mtdTuPtpTypes.h>
#include <TuPtp/mtdTuPtp.h>
#include <TuPtp/mtdTuPtpInternal.h>


/*******************************************************************************
 Pass-in device struct sanity check for T-unit PTP 
*******************************************************************************/
MTD_STATUS mtdTuPTPIsDevValid
(
    IN MTD_DEV_PTR devPtr
)
{
    if (devPtr == NULL)
    {
        MTD_DBG_ERROR(("mtdTuPTPIsDevValid: devPtr is NULL.\n"));
        return MTD_FAIL;
    }

    if (!(MTD_IS_X35X0_BASE(devPtr->deviceId)))
    {
        MTD_DBG_ERROR(("mtdTuPTPIsDevValid: Unsupported PTP device type\n"));
        return MTD_FAIL;
    }

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPReadPlus
 Internally supporting register call to read 16 bits time register consecutively,
     up to 64 bits.
 For TAI reading TrigGenAmt(0x02-03), EventCapTime(0x0A-0B), PTPGlobTime(0x0E-0F),
                 TrigGenTime(0x10-11), etc.
 For TOD reading ToD Nano(0x13-14), Tod Sec(0x15-17), 1722 Nano(0x18-1B),
                 ToD Comp(0x1C-1D), etc.
*******************************************************************************/
MTD_STATUS mtdTuPTPReadPlus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 regOffset,
    IN MTD_U16 readPlusType,
    IN MTD_U16 numOf16bitsWords,
    OUT MTD_U64 *data64Bits
)
{
    MTD_U16 regVal, icr;

    if (numOf16bitsWords < 1 || numOf16bitsWords > 4)
    {
        MTD_DBG_ERROR("mtdTuPTPReadPlus: Only supports to read back registers value from 16 bits up to 64 bits\n");
        return MTD_FAIL;
    }

    switch (readPlusType)
    {
        case MTD_TUPTP_READPLUS_TAI:    /* based on MTD_TU_PTP_TAI_GLOBAL_REG */
            /* regOffset expected from 0x0 to 0x10 */
            regVal = (regOffset & 0x1F) | 0x8E00;
            break;

        case MTD_TUPTP_READPLUS_TOD:    /* based on MTD_TU_PTP_GLOBAL_ETHTYPE */
            /* regOffset expected from 0x10 to 0x1F */
            regVal = (regOffset & 0x1F) | 0x8F10;
            break;

        default:
            MTD_DBG_ERROR("mtdTuPTPReadPlus: Invalid read plus type\n");
            return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, 0xD96E, regVal));

    *data64Bits = 0;
    for (icr = 0; icr < numOf16bitsWords; icr++)
    {
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, 0xD96F, &regVal));
        *data64Bits |= (MTD_U64)regVal << (16*icr);
    }

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPWaitGlobalBitSC 
 Internal PTP supporting register write call for wait ready
*******************************************************************************/
MTD_STATUS mtdTuPTPWaitGlobalBitSC
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 waitType
)
{
    MTD_U16 regAddr;
    MTD_U16 readPlusType;
    MTD_U16 waitBit, waitCnt=0;
    MTD_U64 data64Bits;

    switch (waitType)
    {
        case MTD_TUPTP_WAITTYPE_PTPGLOBAL:
            regAddr = 0x7;
            readPlusType = MTD_TUPTP_READPLUS_TAI;
            break;

        case MTD_TUPTP_WAITTYPE_PTPGLOBALCOMP:
            regAddr = 0xD;
            readPlusType = MTD_TUPTP_READPLUS_TAI;
            break;

        case MTD_TUPTP_WAITTYPE_TOD:
            regAddr = 0x12;
            readPlusType = MTD_TUPTP_READPLUS_TOD;
            break;

        default:
            MTD_DBG_ERROR("mtdTuPTPWaitGlobalBitSC: Invalid wait type\n");
            return MTD_FAIL;
            break;
    }

    while (1)
    {
        MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regAddr, readPlusType, 1, &data64Bits));
        waitBit = ((MTD_U16)data64Bits >> 15) & 0x1;

        if (waitBit == 1)
        {
            MTD_ATTEMPT(mtdWait(devPtr, 10));
            waitCnt++;
        }
        else
        {
            break;
        }

        if (waitCnt >= 100)
        {
            MTD_DBG_INFO("mtdTuPTPWaitGlobalBitSC: exceeds 1 second\n");
            break;
        }
    }

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGlobalConfCompRead 
 Internal supporting register read call for TOD Drift Compensation Command
*******************************************************************************/
MTD_STATUS mtdTuPTPGlobalConfCompRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 featureIndex,
    OUT MTD_U16 *data
)
{
    MTD_U16 regVal = 0;

    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_WAITTYPE_PTPGLOBALCOMP));

    regVal = featureIndex << 8;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_DRIFT_COMP, regVal));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_DRIFT_COMP, &regVal));

    *data = regVal & 0xFF;

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGlobalConfCompWrite 
 Internal supporting register write call for TOD Drift Compensation Command
*******************************************************************************/
MTD_STATUS mtdTuPTPGlobalConfCompWrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 featureIndex,
    IN MTD_U16 regVal
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_WAITTYPE_PTPGLOBALCOMP));

    data = (0x8000 | (featureIndex << 8)) | regVal;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_DRIFT_COMP, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGlobalConfCompRead 
 Internal supporting register read call for Global Configuration Command
*******************************************************************************/
MTD_STATUS mtdTuPTPGlobalConfRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 featureIndex,
    OUT MTD_U16 *data
)
{
    MTD_U16 regVal = 0;

    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_WAITTYPE_PTPGLOBAL));

    regVal = featureIndex << 8;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_CONFIG_PTP, regVal));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_CONFIG_PTP, &regVal));

    *data = regVal & 0xFF;

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGlobalConfCompWrite 
 Internal supporting register write call for Global Configuration Command
*******************************************************************************/
MTD_STATUS mtdTuPTPGlobalConfWrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 featureIndex,
    IN MTD_U16 regVal
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_WAITTYPE_PTPGLOBAL));

    data = (0x8000 | (featureIndex << 8)) | regVal;

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_CONFIG_PTP, data));

    return MTD_OK;
}

#endif /* MTD_TU_PTP */
