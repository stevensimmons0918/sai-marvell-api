/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains internal support types and definitions for the 
Precision Time Protocol(PTP) for the 88X35X0 Ethernet PHYs.
********************************************************************/

#ifndef MTD_TU_PTP_INTERNAL_H
#define MTD_TU_PTP_INTERNAL_H

#if MTD_TU_PTP

#ifdef __cplusplus
extern "C" {
#endif

#define MTD_TUPTP_WAITTYPE_PTPGLOBAL     1
#define MTD_TUPTP_WAITTYPE_PTPGLOBALCOMP 2
#define MTD_TUPTP_WAITTYPE_TOD           3

#define MTD_TUPTP_READPLUS_TAI 1
#define MTD_TUPTP_READPLUS_TOD 2

/*******************************************************************************
 Sanity check on pass-in device struct for T-unit PTP 
*******************************************************************************/
MTD_STATUS mtdTuPTPIsDevValid
(
    IN MTD_DEV_PTR devPtr
);

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
);

/*******************************************************************************
 mtdTuPTPWaitGlobalBitSC 
 Internal PTP supporting register write call for wait ready
*******************************************************************************/
MTD_STATUS mtdTuPTPWaitGlobalBitSC
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 waitType
);

/*******************************************************************************
 mtdTuPTPGlobalConfCompRead 
 Internal supporting register write call for TOD Drift Compensation Command
*******************************************************************************/
MTD_STATUS mtdTuPTPGlobalConfCompRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 featureIndex,
    OUT MTD_U16 *data
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif /* MTD_TU_PTP */

#endif /* MTD_TU_PTP_INTERNAL_H */
