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
* mtdTuPTPConfigPTPBlock
*******************************************************************************/
MTD_STATUS mtdTuPTPConfigPTPBlock
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enablePTPBlock,
    IN MTD_BOOL doSwReset
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    if (enablePTPBlock == MTD_ENABLE)
    {
        /* disable PTP bypass */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 0, 1, 0));
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 1, 0xC04A, 0, 1, 0x1));
    }
    else
    {
        /* bypass PTP */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 0, 1, 1));
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 1, 0xC04A, 0, 1, 0x0));
    }

    if (doSwReset == MTD_TRUE)
    {
        MTD_ATTEMPT(mtdSoftwareReset(devPtr, port, 1000));
        MTD_ATTEMPT(mtdTuPTPSoftReset(devPtr, port));
    }

    /* Always enable One-step input control to PTP block when the PTP block is enabled */
    /* User should only use PTP internal register(set by mtdTuPTPSetOneStep) to control One-step/Two-step */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 12, 1,
                                    ((enablePTPBlock) ? 1 : 0)));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSoftReset
*******************************************************************************/
MTD_STATUS mtdTuPTPSoftReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 1, 1, 0x1));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 1, 1, 0x0));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetBypass
*******************************************************************************/
MTD_STATUS mtdTuPTPSetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpBypass /* 0: no PTP bypass; 1:enable PTP bypass */
)
{

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    /* Set to make XGMII/GMII path bypass PTP at ptp_wrap block */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 0, 1, 
                                    ((ptpBypass) ? 1 : 0)));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetBypass
*******************************************************************************/
MTD_STATUS mtdTuPTPGetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpBypass
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_CNTL_REG1, 0, 1, &data));

    *ptpBypass = data;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetEtherType
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEtherType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etherType
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_ETHTYPE, etherType));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetEtherType
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEtherType 
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *etherType
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_ETHTYPE, etherType));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetMsgTSEnable - Message Type Time Stamp Enable
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMsgTSEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 msgTSEnable
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_MSG_TYPE, msgTSEnable));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetMsgTSEnable - Message Type Time Stamp Enable
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMsgTSEnable 
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *msgTSEnable
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_MSG_TYPE, msgTSEnable));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPSetAltScheme - Set Alternate Scheme
*******************************************************************************/
MTD_STATUS mtdTuPTPSetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 altScheme 
)
{
    MTD_U16 data, writeData;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    writeData = (data & 0xEF) | ((altScheme & 0x1) << 4);

    MTD_ATTEMPT(mtdTuPTPGlobalConfWrite(devPtr, port, featureIndex, writeData));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPGetAltScheme - Get Alternate Scheme
*******************************************************************************/
MTD_STATUS mtdTuPTPGetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *altScheme 
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 4, 1, altScheme));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPSetGrandMaster - Set device as Grand Master or not
*******************************************************************************/
MTD_STATUS mtdTuPTPSetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 grandMaster 
)
{
    MTD_U16 data, writeData;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    writeData = (data & 0xF7) | ((grandMaster & 0x1) << 3);

    MTD_ATTEMPT(mtdTuPTPGlobalConfWrite(devPtr, port, featureIndex, writeData));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPGetGrandMaster - Query if the device is acting as Grand Master
*******************************************************************************/
MTD_STATUS mtdTuPTPGetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *grandMaster 
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 3, 1, grandMaster));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetPTPMode - Set the PTP Mode on the given port
 *******************************************************************************/
MTD_STATUS mtdTuPTPSetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpMode 
)
{
    MTD_U16 data, writeData;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    writeData = (data & 0xFC) | (ptpMode & 0x3);

    MTD_ATTEMPT(mtdTuPTPGlobalConfWrite(devPtr, port, featureIndex, writeData));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPGetPTPMode - Get the PTP Mode on the given port
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpMode
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    *ptpMode = data & 0x3;

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPSetOneStep - Set to use One Step or Two Step frame formats
*******************************************************************************/
MTD_STATUS mtdTuPTPSetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 oneStep 
)
{
    MTD_U16 data, writeData;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    writeData = (data & 0xFB) | ((oneStep & 0x1) << 2);

    MTD_ATTEMPT(mtdTuPTPGlobalConfWrite(devPtr, port, featureIndex, writeData));

    return MTD_OK;
}


/*******************************************************************************
 mtdTuPTPGetOneStep - query if device is using One Step or Two Step frame formats
*******************************************************************************/
MTD_STATUS mtdTuPTPGetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *oneStep 
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 2, 1, oneStep));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgTransSpec - PTP Transport Specific value
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgTransSpec
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 transSpec 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0, 12, 4, 
                                    transSpec));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgDisableTSpecCheck - Disable Transport Specific Check.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgDisableTSpecCheck
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disableTSpecCheck 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0, 11, 1, 
                                    disableTSpecCheck));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgDisTimeStmpCntOverwrite - Disable Time Stamp Counter Overwriting
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgDisTimeStmpCntOverwrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disTimeStmpCntOverwrite 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0, 1, 1, 
                                    disTimeStmpCntOverwrite));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgIPJump - Internet Protocol Jump
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgIPJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ipJump 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_1, 8, 6, 
                                    ipJump));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgETJump - EtherType Jump points to the start of the frame's EtherType 
                         (assuming it is not 802.1Q tagged).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgETJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etJump 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_1, 0, 5, 
                                    etJump));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgArrTSMode - Arrival Time Stamp Mode
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgArrTSMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalTSMode 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 8, 8, 
                                    arrivalTSMode));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetCfgHWAccel - Port PTP Hardware Acceleration enable
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgHWAccel
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 hwAccel 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 6, 1, 
                                    hwAccel));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetKeepSourceAddr - Keep Frame's Source Address
*******************************************************************************/
MTD_STATUS mtdTuPTPSetKeepSourceAddr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 keepSA 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 5, 1, 
                                    keepSA));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetExtHwAccEnable - External Hardware Acceleration enable
*******************************************************************************/
MTD_STATUS mtdTuPTPSetExtHwAccEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 extHwAccEnable 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 2, 1, 
                                    extHwAccEnable));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetDepartureIntEn - Precise Time Protocol Port Departure Interrupt enable
*******************************************************************************/
MTD_STATUS mtdTuPTPSetDepartureIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 departureIntEn 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 1, 1, 
                                    departureIntEn));

    return MTD_OK;
}

/*******************************************************************************
  mtdTuPTPSetArrivalIntEn - Precise Time Protocol Port Arrival Interrupt enable
*******************************************************************************/
MTD_STATUS mtdTuPTPSetArrivalIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalIntEn 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, 0, 1, 
                                    arrivalIntEn));

    return MTD_OK;
}


/*******************************************************************************
* mtdTuPTPGetPortConfig
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPortConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PORT_CONFIG *ptpData
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0, &data));
    ptpData->transSpec = data >> 12;
    ptpData->disTSpec = ((data >> 11) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->disTSOverwrite = ((data >> 1) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->disablePTP = (data & 0x1) ? MTD_FALSE : MTD_TRUE;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_1, &data));
    ptpData->ipJump = (data >> 8) & 0x3F;
    ptpData->etJump = data & 0x1F;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_2, &data));
    ptpData->ptpDepIntEn = ((data >> 1) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->ptpArrIntEn = (data & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->arrTSMode = (data & 0xFF00) >> 8;
    ptpData->filterAct = ((data >> 7) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->hwAccPtp = ((data >> 6) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->keepFmSA = ((data >> 5) & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpData->ExthwAcc = ((data >> 2) & 0x1) ? MTD_TRUE : MTD_FALSE;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_3, &data));
    ptpData->arrLedCtrl = (data & 0xFF00) >> 8;
    ptpData->depLedCtrl = data & 0xFF;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetTimeStamp  //Done
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStamp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_TU_PTP_TS_STATUS *ptpStatus
)
{
    MTD_U16 regOffset, value, valueHi, valueLo;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    switch (timeToRead)
    {
        case MTD_TU_PTP_ARR0_TIME:
            regOffset = 0x8;
            break;
        case MTD_TU_PTP_ARR1_TIME:
            regOffset = 0xC;
            break;
        case MTD_TU_PTP_DEP_TIME:
            regOffset = 0x10;
            break;
        default:
            MTD_DBG_ERROR(("mtdTuPTPGetTimeStamp: Invalid time to be read\n"));
            return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0+regOffset, &value));
    ptpStatus->isValid = (value & 0x1) ? MTD_TRUE : MTD_FALSE;
    ptpStatus->status = (MTD_TU_PTP_INT_STATUS)((value >> 1) & 0x3);

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0+regOffset+1, &valueLo));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0+regOffset+2, &valueHi));
    ptpStatus->timeStamped = (MTD_U32)((valueHi << 16) | valueLo);  /*TBD*/

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_REG_0+regOffset+3, &value));
    ptpStatus->ptpSeqId = value;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuGetPTPTSValidSt
*******************************************************************************/
MTD_STATUS mtdTuGetPTPTSValidSt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_BOOL *isValid
)
{
    MTD_U16 data;
    MTD_U16 baseReg;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    baseReg = MTD_TU_PTP_PORT_REG_8;

    switch (timeToRead)
    {
    case MTD_TU_PTP_ARR0_TIME:
        baseReg += 0;
        break;
    case MTD_TU_PTP_ARR1_TIME:
        baseReg += 4;
        break;
    case MTD_TU_PTP_DEP_TIME:
        baseReg += 8;
        break;
    default:
        MTD_DBG_ERROR(("Invalid time to be read\n"));
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, baseReg, &data));
    *isValid = (data & 0x1) ? MTD_TRUE : MTD_FALSE;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPTimeStampReset
*******************************************************************************/
MTD_STATUS mtdTuPTPTimeStampReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToReset
)
{
    MTD_U16 index;
    MTD_U16 baseReg;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    baseReg = MTD_TU_PTP_PORT_REG_8;

    switch (timeToReset)
    {
    case MTD_TU_PTP_ARR0_TIME:
        baseReg += 0;
        break;
    case MTD_TU_PTP_ARR1_TIME:
        baseReg += 4;
        break;
    case MTD_TU_PTP_DEP_TIME:
        baseReg += 8;
        break;
    default:
        MTD_DBG_ERROR(("mtdTuPTPTimeStampReset: Invalid time to be read\n"));
        return MTD_FAIL;
    }

    for (index = 0; index < 4; index++)
    {
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, (baseReg + index), 0x0));
    }

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetTimeStampArrivalPtr
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTimeStampArrivalPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 tsArrPtr
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TS_ARR, tsArrPtr));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetTimeStampArrivalPtr
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStampArrivalPtr 
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsArrPtr
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TS_ARR, tsArrPtr));

    return MTD_OK;
}


/*******************************************************************************
* mtdTuPTPGetIntStatus //Done
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIntStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_INTERRUPT_STATUS *ptpIntSt
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_INTERRUPT, &data));
    ptpIntSt->ptpTrigGenInt = (data >> 15) & 1;
    ptpIntSt->ptpEventInt = (data >> 14) & 1;
    ptpIntSt->ptpUpperPortInt = (data >> 11) & 1;

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 0, 10, &ptpIntSt->ptpIntStLowerPortVect));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_INTERRUPT+1, 
                           &ptpIntSt->ptpIntStUpperPortVect));

    return MTD_FAIL;
}

/*******************************************************************************
* mtdTuPTPSetIgrMeanPathDelay
*******************************************************************************/
MTD_STATUS mtdTuPTPSetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 delay
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_IG_MEAN_DELAY, delay));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetIgrMeanPathDelay
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16    *delay
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_IG_MEAN_DELAY, delay));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetIgrPathDelayAsym
*******************************************************************************/
MTD_STATUS mtdTuPTPSetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_IG_DELAY_ASYM, asym));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetIgrPathDelayAsym
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16    *asym
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_IG_DELAY_ASYM, asym));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPEgrPathDelayAsymSet
*******************************************************************************/
MTD_STATUS mtdTuPTPEgrPathDelayAsymSet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_EG_DELAY_ASYM, asym));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPEgrPathDelayAsymGet
*******************************************************************************/
MTD_STATUS mtdTuPTPEgrPathDelayAsymGet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *asym
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_PORT_EG_DELAY_ASYM, asym));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetTODBusyBitStatus
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTODBusyBitStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *busyStatus
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_CNTL, &data));

    *busyStatus = data & 0x8000 ? MTD_TRUE : MTD_FALSE;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPTODStoreOperation
*******************************************************************************/
MTD_STATUS mtdTuPTPTODStoreOperation
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TOD_STORE_OPERATION storeOp,
    IN MTD_U8 timeArrayIndex,
    IN MTD_TU_PTP_TIME_ARRAY *timeArray
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    data = (MTD_U16)(timeArray->todLoadPoint & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_LOADL, data));
    data = (MTD_U16)((timeArray->todLoadPoint >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_LOADH, data));

    data = (MTD_U16)(timeArray->todNanoseconds & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13, data));
    data = (MTD_U16)((timeArray->todNanoseconds >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+1, data));

    data = (MTD_U16)(timeArray->todSecondsLow & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+2, data));
    data = (MTD_U16)((timeArray->todSecondsLow >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+3, data));
    data = (MTD_U16)(timeArray->todSecondsHigh & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+4, data));

    data = (MTD_U16)(timeArray->Nanoseconds1722Low & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+5, data));
    data = (MTD_U16)((timeArray->Nanoseconds1722Low >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+6, data));
    data = (MTD_U16)(timeArray->Nanoseconds1722High & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+7, data));
    data = (MTD_U16)((timeArray->Nanoseconds1722High >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+8, data));

    data = (MTD_U16)(timeArray->todCompensation & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+9, data));
    data = (MTD_U16)((timeArray->todCompensation >> 16) & 0xFFFF);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_REG_13+10, data));

    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_READPLUS_TOD)); /* Wait for ToDBusy cleared */

    data = 0;
    data = ((1 << 15) | (storeOp << 12) |
            ((timeArrayIndex & 3) << 9) | (((timeArray->clkActive) & 1) << 8) |
            (timeArray->domainNumber & 0xFF));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_CNTL, data));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPTODCaptureAll
*******************************************************************************/
MTD_STATUS mtdTuPTPTODCaptureAll
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 timeArrayIndex,
    OUT MTD_TU_PTP_TIME_ARRAY *timeArray
)
{
    MTD_U16 regOffset;
    MTD_U16 data;
    MTD_U64 data64Bits;
    MTD_U16 numOf16bitsWords = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    if (!(/*timeArrayIndex >= 0 &&*/ timeArrayIndex < 4))
    {
        MTD_DBG_ERROR(("mtdTuPTPTODCaptureAll: Time array index is over range.\n"));
        return MTD_FAIL;
    }

    /* Get the origin operation register value */
    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_READPLUS_TOD)); /* Wait for ToDBusy cleared */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_CNTL, &data));

    /* clkActive and domain number */
    timeArray->clkActive = ((data >> 8) & 1) == 1 ? MTD_TRUE : MTD_FALSE;
    timeArray->domainNumber = (MTD_U8)(data & 0xFF);

    data = data & 0x89FF;    /* clear ToDOp bits and timeArrayIndex bits */

    /* Set Capture Operation */
    /* ToDBusy bit15;  ToDOp bit[14:12] */
    data = ((1 << 15) | (MTD_TU_PTP_TOD_CAPTURE_TA << 12) | ((timeArrayIndex & 3) << 9)) | data;
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_TOD_CNTL, data));
    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(devPtr, port, MTD_TUPTP_READPLUS_TOD)); /* Wait for ToDBusy cleared */

    /* TOD Load Point */
    regOffset = 0x10;
    numOf16bitsWords = 2;
    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TOD, numOf16bitsWords, &data64Bits));
    timeArray->todLoadPoint = (MTD_U32)data64Bits;

    /* TOD Nano Seconds */
    regOffset = 0x13;
    numOf16bitsWords = 2;
    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TOD, numOf16bitsWords, &data64Bits));
    timeArray->todNanoseconds = (MTD_U32)data64Bits;

    /* TOD Seconds */
    regOffset = 0x15;
    numOf16bitsWords = 3;
    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TOD, numOf16bitsWords, &data64Bits));
    timeArray->todSecondsLow = (MTD_U32)data64Bits;
    timeArray->todSecondsHigh = (MTD_U32)(data64Bits >> 32) & 0xFFFF;   /* TOD seconds 32-47 bits */

    /* 1722 Nano Seconds */
    regOffset = 0x18;
    numOf16bitsWords = 4;
    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TOD, numOf16bitsWords, &data64Bits));
    timeArray->Nanoseconds1722Low = (MTD_U32)data64Bits;
    timeArray->Nanoseconds1722High = (MTD_U32)(data64Bits >> 32);

    /* 1722 Nano Seconds */
    regOffset = 0x1C;
    numOf16bitsWords = 2;
    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TOD, numOf16bitsWords, &data64Bits));
    timeArray->todCompensation = (MTD_U32)data64Bits;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPSetPulse
*******************************************************************************/
MTD_STATUS mtdTuPTPSetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_PULSE_STRUCT *pulsePara
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    data = ((((pulsePara->ptpPulseWidth) & 0xf) << 12) |
            (((pulsePara->ptpAltTCAMTimeSel) & 0x1) << 11) |
            (((pulsePara->ptp1ppsWidthRange) & 7) << 8) |
            (((pulsePara->ptpTCAMTimeSel) & 0xf) << 4) |
            (((pulsePara->ptp1ppsPhase) & 1) << 3) |
            (pulsePara->ptp1ppsSelect & 0x3));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_PULSE_CNTL, data));

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetPulse
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PULSE_STRUCT *pulsePara
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_GLOBAL_PULSE_CNTL, &data));

    pulsePara->ptpPulseWidth = (MTD_U8)((data >> 12) & 0xF);
    pulsePara->ptpAltTCAMTimeSel = (MTD_U8)((data >> 11) & 1);
    pulsePara->ptp1ppsWidthRange = (MTD_U8)((data >> 8) & 7);
    pulsePara->ptpTCAMTimeSel = (MTD_U8)((data >> 4) & 0xF);
    pulsePara->ptp1ppsPhase = (MTD_U8)((data >> 3) & 1);
    pulsePara->ptp1ppsSelect = (MTD_U8)(data & 0x3);

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPGetPTPGlobalTime  //Done
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPTPGlobalTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *ptpGlobalTime
)
{
    MTD_U16 regOffset = 0xE;
    MTD_U64 data64Bits;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TAI, 2, &data64Bits));
    *ptpGlobalTime = (MTD_U32)data64Bits;

    return MTD_OK;
}

/*******************************************************************************
* mtdTuPTPEventCaptureTime  
*******************************************************************************/
MTD_STATUS mtdTuPTPEventCaptureTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *evtCaptureTime
)
{
    MTD_U16 regOffset = 0xA;
    MTD_U64 data64Bits;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPReadPlus(devPtr, port, regOffset, MTD_TUPTP_READPLUS_TAI, 2, &data64Bits));
    *evtCaptureTime = (MTD_U32)data64Bits;


    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetEventCaptureCounter
 Read the Event Capture Counter.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureCounter
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 0, 8, eventCaptureCounter));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPClearEventCaptureCounter
 Clear Event Capture Counter.
*******************************************************************************/
MTD_STATUS mtdTuPTPClearEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 0, 8, 0));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetEventCaptureValid
 Get the Event Capture Valid bit
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureValid
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureValid
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 8, 1, eventCaptureValid));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetEventCaptureErr
 Get the Event Capture Error bit
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureErr
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 9, 1, eventCaptureErr));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPClearEventCaptureErr
 Clear the Event Capture Error bit
*******************************************************************************/
MTD_STATUS mtdTuPTPClearEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 9, 1, 0));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetEventCaptureTrigger
 Set the Capture Event Trigger
 0 = Capture PTP_EVREQ pin events
 1 = Capture PTP_TRIG internal events
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 eventCaptureTrigger 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 14, 1, eventCaptureTrigger));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetEventCaptureTrigger 
 Get the Capture Event Trigger
 0 = Capture PTP_EVREQ pin events
 1 = Capture PTP_TRIG internal events
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureTrigger 
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_EVT, 14, 1, eventCaptureTrigger));

    return MTD_OK;
}

/*******************************************************************************
 TOD Drift Compensation Commands 
*******************************************************************************/
/*******************************************************************************
 mtdTuPTPSetDriftComp
 Enable Drift Comp bit enables frequency drift compensation function for 
 Time of Day (TOD)
*******************************************************************************/
MTD_STATUS mtdTuPTPSetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableDriftComp /* MTD_ENABLE or MTD_DISABLE */
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, ((enableDriftComp) ? 1 : 0), 7, 1, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetDriftComp
 Get the Enable Drift Comp bit that enables frequency drift compensation function for 
 Time of Day (TOD)
*******************************************************************************/
MTD_STATUS mtdTuPTPGetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableDriftComp  /* MTD_ENABLE or MTD_DISABLE */
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 7, 1, enableDriftComp));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetTodClkPeriod
 TOD Clock Period bit determines the period of TOD alignment clock 
 
 Clock_Period 
 0x0 : 125 ms 
 0x1 : 250 ms 
 0x2 : 500 ms 
 0x3 : 1.0 sec
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 todClkPeriod 
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, todClkPeriod, 5, 2, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTodClkPeriod
 Gets the TOD Clock Period bit which determines the period of TOD alignment clock 
 
 Clock_Period 
 0x0 : 125 ms 
 0x1 : 250 ms 
 0x2 : 500 ms 
 0x3 : 1.0 sec
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *todClkPeriod
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 5, 2, todClkPeriod));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetEnableTodClk
 Enables TAI Master to generate a periodic clock
 
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodClk /* 1: enable; 0:disable */
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, ((enableTodClk) ? 1 : 0), 4, 1, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetEnableTodClk
 Gets the TAI Master state that enable or disable the generating of a periodic 
 clock

*******************************************************************************/
MTD_STATUS mtdTuPTPGetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodClk /* 1: enable; 0:disable */
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 4, 1, enableTodClk));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetSyncTod
 Enables TAI slave device to sync-up its TOD with TAI Master's TOD by 
 updating its TOD with PTP Global Time Array register values 

 It is recommended that the sync_tod bit is set and master and slave's 
 TOD's sync-up operation is performed at the very beginning of the 
 frequency drift compensation operation such that master and slave's 
 TOD's are aligned before the start of the frequency drift compensation.
 
*******************************************************************************/
MTD_STATUS mtdTuPTPSetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableSyncTod /* 1: enable; 0: disable */
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, ((enableSyncTod) ? 1 : 0), 2, 1, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetSyncTod
 Gets the TAI slave device to sync-up state

*******************************************************************************/
MTD_STATUS mtdTuPTPGetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableSyncTod /* 1: enable; 0:disable */
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 2, 1, enableSyncTod));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetTodTrigRequest
 TOD Trigger Request enables TAI Master to generate a single pulse 
 at "tod_trig_out" pin when master's TOD time matches that of PTP Global 
 Time Array register. The pulse width is controlled by the Pulse Width and 
 Pulse Width Range 

 This bit is self-cleared after the detection of a pulse at "tod_trig_out" pin.
 
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodTrigRequest /* 1: enable; 0: disable */
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, ((enableTodTrigRequest) ? 1 : 0), 1, 1, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTodTrigRequest
 Get the TOD Trigger Request state
 "TOD Trigger Request" bit:1 enables TAI Master to generate a single pulse 
 at "tod_trig_out" pin when master's TOD time matches that of PTP Global 
 Time Array register. The pulse width is controlled by the Pulse Width and 
 Pulse Width Range 

*******************************************************************************/
MTD_STATUS mtdTuPTPGetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodTrigRequest /* 1: enable; 0: disable */
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 1, 1, enableTodTrigRequest));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetMinDriftThreshold
 Minimum Drift Threshold defines the lower threshold. If frequency 
 drift measured between two consecutive alignment clock edges is equal 
 or below this threshold, no frequency drift compensation be performed.
 The unit for this register is in 1ns. 
 The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 minDriftThreshold 
)
{
    MTD_U16 data = 0;
    MTD_U16 featureIndex = 1;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, minDriftThreshold, 0, 4, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetMinDriftThreshold
 Get the Minimum Drift Threshold
 "Minimum Drift Threshold" defines the lower threshold. If frequency 
 drift measured between two consecutive alignment clock edges is equal 
 or below this threshold, no frequency drift compensation be performed.
 The unit for this register is in 1ns. 
 The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)

*******************************************************************************/
MTD_STATUS mtdTuPTPGetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *minDriftThreshold
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 0, 4, minDriftThreshold));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetMaxDriftThreshold
 Maximum Drift Threshold defines the upper threshold. If frequency drift 
 measured between two consecutive alignment clock edges exceeds this 
 threshold, no frequency drift compensation be performed.
 The unit for this register is in 1us. 
 The default is set to 0x64 (or 100us). 
 For instance, the largest possible setting of 0x3FF (=1,023us) is bigger 
 than 1000ppm difference between master and slave's crystals over 1 second 
 period.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maxDriftThreshold 
)
{
    MTD_U16 data, dataH, dataL;
    MTD_U16 featureIndex2, featureIndex3;
    
    featureIndex2 = 2;
    featureIndex3 = 3;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    dataL = maxDriftThreshold & 0xFF;
    dataH = (maxDriftThreshold >> 8) & 0x3;

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex2, &data));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, dataL, 0, 8, &data));
    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex2, data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex3, &data));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, dataH, 0, 2, &data));
    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, featureIndex3, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetMaxDriftThreshold
 Get the Maximum Drift Threshold

 Maximum Drift Threshold defines the upper threshold. If frequency drift 
 measured between two consecutive alignment clock edges exceeds this 
 threshold, no frequency drift compensation be performed.
 The unit for this register is in 1us. 
 The default is set to 0x64 (or 100us). 
 For instance, the largest possible setting of 0x3FF (=1,023us) is bigger 
 than 1000ppm difference between master and slave's crystals over 1 second 
 period.

*******************************************************************************/
MTD_STATUS mtdTuPTPGetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maxDriftThreshold
)
{
    MTD_U16 data, dataH = 0, dataL = 0;
    MTD_U16 featureIndex2, featureIndex3;
    
    featureIndex2 = 2;
    featureIndex3 = 3;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex2, &data));
    dataL = data & 0xFF;

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex3, &data)); /* was featureIndex2 */
    dataH = data & 0x3;

    *maxDriftThreshold = (dataH << 8) | dataL;

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetFreqDriftIntr
 Frequency Drift Interrupt is asserted when the frequency drift 
 exceeds maximum drift threshold.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetFreqDriftIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *freqDriftIntr
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 4;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 3, 1, freqDriftIntr));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetClkGenIntr
 Clock Generation Interrupt is asserted when rising edge of TOD 
 alignment clock is generated at the 'tod_clk_out' pin.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetClkGenIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *clkGenIntr
)
{
    MTD_U16 data;
    MTD_U16 featureIndex = 4;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, featureIndex, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 2, 1, clkGenIntr));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetMaskFreqDriftInt
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maskFreqDriftInt 
)
{
    MTD_U16 data = 0;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, MTD_TU_PTP_TAI_INTERRUPT_MASK, &data));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(data, maskFreqDriftInt, 0, 4, &data));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompWrite(devPtr, port, MTD_TU_PTP_TAI_INTERRUPT_MASK, data));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetMaskFreqDriftInt
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftInt
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, MTD_TU_PTP_TAI_INTERRUPT_MASK, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 0, 4, maskFreqDriftInt));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetFreqDriftIntTriggered
*******************************************************************************/
MTD_STATUS mtdTuPTPGetFreqDriftIntTriggered
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftIntTriggered
)
{
    MTD_U16 data;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdTuPTPGlobalConfCompRead(devPtr, port, MTD_TU_PTP_TAI_INTERRUPT, &data));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(data, 0, 4, maskFreqDriftIntTriggered));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetTAIGlobalCfg
 PTP Time Application Interface(TAI) Configuration 
 PTP Event Request Capture, PTP Trigger Generate, PTP Global time 
 increment/decrement, multi-ptp device time sync function etc.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    IN MTD_U16 taiCfgVal
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_REG, taiCfgOptionBit, 1, taiCfgVal));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTAIGlobalCfg
 PTP Time Application Interface(TAI) Configuration 
 PTP Event Request Capture, PTP Trigger Generate, PTP Global time 
 increment/decrement, multi-ptp device time sync function etc.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    OUT MTD_U16 *taiCfgVal
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_REG, taiCfgOptionBit, 1, taiCfgVal));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTimeStampClkPeriod
 Time Stamping Clock Period in pico seconds.
 This field specifies the clock period for the time stamping clock supplied 
 to the PTP hardware.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStampClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsClkPeriod
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, MTD_TU_PTP_TAI_GLOBAL_TS_CLK_PERIOD, tsClkPeriod));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetTrigGenAmt
 Set the Trigger Generation Amount bits in 32-bit register. 
 Bit 0-15  in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN
 Bit 16-31 in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN+1
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigGenAmt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenAmt
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN, (MTD_U16)trigGenAmt));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN+1, (MTD_U16)(trigGenAmt>>16)));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTrigGenAmt 
 Read back the Trigger Generation Amount bits in 32-bit register. 
 Bit 0-15  in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN
 Bit 16-31 in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN+1
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigGenAmt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 *trigGenAmt
)
{
    MTD_U16 tmpLow, tmpHigh;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN, &tmpLow));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN+1, &tmpHigh));

    *trigGenAmt = (((MTD_U32)tmpHigh) << 16) | tmpLow;

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetTrigCompensateDir
 Set the Trig Clock Compensation Direction.
 When the accumulated TrigClkComp amount (below) exceeds the value in TSClkPer 
 (TAI offset 0x01), one TSClkPer amount gets added to or subtracted from the next 
 PTP_TRIG clock output. This bit determines which as follows:
 0 = Add one TSClkPer to the next PTP_TRIG cycle
 1 = Subtract one TSClkPer from the next PTP_TRIG cycle
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateDir
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP, 15, 1, trigCompensateDir));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetTrigCompensateDir
 Read the Trig Clock Compensation Direction.
 When the accumulated TrigClkComp amount (below) exceeds the value in TSClkPer 
 (TAI offset 0x01), one TSClkPer amount gets added to or subtracted from the next 
 PTP_TRIG clock output. This bit determines which as follows:
 0 = Add one TSClkPer to the next PTP_TRIG cycle
 1 = Subtract one TSClkPer from the next PTP_TRIG cycle
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateDir
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP, 15, 1, trigCompensateDir));
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetTrigCompensate
 This value is in pico seconds as an unsigned 15-bit.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensate
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP, 0, 15, (trigCompensate & 0x7FFF)));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetTrigCompensate
 This value is in pico seconds as an unsigned 15-bit.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensate
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP, 0, 15, trigCompensate));
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetTrigCompensatePS
 This value is in Sub Pico seconds as an unsigned 8-bit number.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateSubPS
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP+1, 0, 8, 
                                   (trigCompensateSubPS & 0xFF)));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetTrigCompensatePS
 This value is in Sub Pico seconds as an unsigned 8-bit number.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateSubPS
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP+1, 0, 8, trigCompensateSubPS));
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetTrigPulseWidth
 Sets Pulse width 4-bit length for PTP_TRIG.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidth
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_PULSE, 12, 4, (trigPulseWidth & 0xF)));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetTrigPulseWidth
 Read back Pulse width 4-bit length for PTP_TRIG.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidth
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_PULSE, 12, 4, trigPulseWidth));
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPSetTrigPulseWidthRange
 Sets Pulse width 3-bit length for PTP_TRIG.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidthRange
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_PULSE, 8, 3, (trigPulseWidthRange & 0xF)));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdTuPTPGetTrigPulseWidthRange
 Read back Pulse width 3-bit length for PTP_TRIG.
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidthRange
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_TU_PTP_DEV, 
                                    MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_PULSE, 8, 3, trigPulseWidthRange));
    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPSetTrigGenTime
 Set the Trigger Generation Time in 32-bit register. 
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenTime
)
{
    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN_TIME, (MTD_U16)trigGenTime));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN_TIME+1, (MTD_U16)(trigGenTime>>16)));

    return MTD_OK;
}

/*******************************************************************************
 mtdTuPTPGetTrigGenTime
 Read back the Trigger Generation Time in 32-bit register. 
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 *trigGenTime
)
{
    MTD_U16 tmpLow, tmpHigh;

    MTD_ATTEMPT(mtdTuPTPIsDevValid(devPtr));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN_TIME, &tmpLow));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_TU_PTP_DEV, 
                                MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN_TIME+1, &tmpHigh));

    *trigGenTime = (((MTD_U32)tmpHigh) << 16) | tmpLow;

    return MTD_OK;
}

#endif /* MTD_TU_PTP */
