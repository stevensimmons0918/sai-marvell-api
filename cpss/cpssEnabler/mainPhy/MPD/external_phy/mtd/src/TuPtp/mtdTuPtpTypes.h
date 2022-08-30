/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains Precision Time Protocol(PTP) definitions and data 
structures for the 88X35X0 Ethernet PHYs.
********************************************************************/

#ifndef MTD_TU_PTP_TYPES_H
#define MTD_TU_PTP_TYPES_H

#if MTD_TU_PTP

#ifdef __cplusplus
extern "C" {
#endif

/* T-unit PTP Device Number Definitions */
#define MTD_TU_PTP_DEV   3

/* PTP Control/Status/Clock Registers Definitions */
#define MTD_TU_PTP_CLK_REG      0xD800
#define MTD_TU_PTP_BLK_RESET    0xD801
#define MTD_TU_PTP_CNTL_REG0    0xD822
#define MTD_TU_PTP_CNTL_REG1    0xD823
#define MTD_TU_PTP_STATUS_REG0  0xD824
#define MTD_TU_PTP_STATUS_REG1  0xD825
#define MTD_TU_PTP_MAC_0        0xD826
#define MTD_TU_PTP_MAC_1        0xD827
#define MTD_TU_PTP_MAC_2        0xD828

/* PTP Port Register Definitions */
#define MTD_TU_PTP_PORT_REG_0    0xD900
#define MTD_TU_PTP_PORT_REG_1    0xD901
#define MTD_TU_PTP_PORT_REG_2    0xD902
#define MTD_TU_PTP_PORT_REG_3    0xD903
#define MTD_TU_PTP_PORT_REG_8    0xD908
#define MTD_TU_PTP_PORT_IG_MEAN_DELAY  0xD91C
#define MTD_TU_PTP_PORT_IG_DELAY_ASYM  0xD91D
#define MTD_TU_PTP_PORT_EG_DELAY_ASYM  0xD91E

/* PTP Global Register Definitions */
#define MTD_TU_PTP_GLOBAL_ETHTYPE    0xD960
#define MTD_TU_PTP_GLOBAL_MSG_TYPE   0xD961
#define MTD_TU_PTP_GLOBAL_TS_ARR     0xD962
#define MTD_TU_PTP_GLOBAL_CONFIG_PTP 0xD967
#define MTD_TU_PTP_GLOBAL_INTERRUPT  0xD968
#define MTD_TU_PTP_GLOBAL_DRIFT_COMP 0xD96D
#define MTD_TU_PTP_GLOBAL_TOD_LOADL  0xD970
#define MTD_TU_PTP_GLOBAL_TOD_LOADH  0xD971
#define MTD_TU_PTP_GLOBAL_TOD_CNTL   0xD972
#define MTD_TU_PTP_GLOBAL_TOD_REG_13 0xD973
#define MTD_TU_PTP_GLOBAL_PULSE_CNTL 0xD97E

/* PTP TAI Global Register Definitions */
#define MTD_TU_PTP_TAI_GLOBAL_REG            0xD940
#define MTD_TU_PTP_TAI_GLOBAL_TS_CLK_PERIOD  0xD941
#define MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN       0xD942
#define MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_COMP  0xD944
#define MTD_TU_PTP_TAI_GLOBAL_TRIG_CLK_PULSE 0xD945
#define MTD_TU_PTP_TAI_GLOBAL_EVT            0xD949
#define MTD_TU_PTP_TAI_GLOBAL_TIME           0xD94E
#define MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN_TIME  0xD950

/*TOD Drift Compensation feature index */
#define MTD_TU_PTP_TRIG_CLK_CNTL      0x0 /* TOD Trig and Clock Control */
#define MTD_TU_PTP_TAI_MIN_DRIFT_TH   0x1 /* Minimum Drift Threshold */
#define MTD_TU_PTP_TAI_MAX_DRIFT_TH1  0x2 /* Maximum Drift Threshold 1 */
#define MTD_TU_PTP_TAI_MAX_DRIFT_TH2  0x3 /* Maximum Drift Threshold 2 */
#define MTD_TU_PTP_TAI_INTERRUPT      0x4 /* TAI Interrupt Register */
#define MTD_TU_PTP_TAI_INTERRUPT_MASK 0x5 /* TAI Interrupt Mask Register */

/*
* Typedef: enum MTD_TU_PTP_SPEC
*
* Description: Defines the PTP (Precise Time Protocol) SPEC type
*
* Fields:
*      MTD_PTP_IEEE_1588       - IEEE 1588
*      MTD_PTP_IEEE_802_1AS    - IEEE 802.1as
*/
typedef enum _MTD_TU_PTP_SPEC
{
    MTD_TU_PTP_IEEE_1588 = 0x0,
    MTD_TU_PTP_IEEE_802_1AS = 0x1
} MTD_TU_PTP_SPEC;

/*
*  typedef: struct MTD_TU_PTP_PORT_CONFIG
*
*  Description: PTP configuration parameters for each port
*
*  Fields:
*      disablePTP   - enable/disable PTP port; 0=enable; 1=disable 
*      transSpec    - This is to differentiate between various timing protocols.
*      disTSpec     - Disable Transport specific check
*      etJump         - offset to Ether type start address in bytes
*      ipJump         - offset to IP header start address counting from Ether type offset
*      ptpArrIntEn    - PTP port arrival interrupt enable
*      ptpDepIntEn    - PTP port departure interrupt enable
*      disTSOverwrite - disable time stamp counter overwriting until the corresponding
*                       timer valid bit is cleared.
*      filterAct      - Filter 802.1 Act from LEDs
*      hwAccPtp       - Hardware Accelerated PTP
*      kpFramwSa      - KS = Keep Frame source addr
*      ExthwAcc       - External Hardware Acceleration enable
*      arrTSMode      - Arrival Time Stamp mode
*      arrLedCtrl     - LED control for packets entering the device.
*      depLedCtrl     - LED control for packets departing the device.
*/
typedef struct _MTD_TU_PTP_PORT_CONFIG
{
    MTD_BOOL disablePTP;
    MTD_TU_PTP_SPEC transSpec;    
    MTD_BOOL disTSpec;     
    MTD_U16  etJump;       
    MTD_U16  ipJump;       
    MTD_BOOL ptpArrIntEn;  
    MTD_BOOL ptpDepIntEn;
    MTD_BOOL disTSOverwrite; 
    MTD_BOOL filterAct;      
    MTD_BOOL hwAccPtp;       
    MTD_BOOL keepFmSA;      
    MTD_BOOL ExthwAcc;       
    MTD_U16  arrTSMode;      
    MTD_U8   arrLedCtrl;     
    MTD_U8   depLedCtrl;     
}MTD_TU_PTP_PORT_CONFIG;

/*
* Typedef: enum MTD_TU_PTP_TIME
*
* Description: Defines the PTP Time to be read
*
* Fields:
*      PTP_ARR0_TIME            - PTP Arrival Time 0
*      PTP_ARR1_TIME            - PTP Arrival Time 1
*      PTP_DEP_TIME            - PTP Departure Time
*/
typedef enum _MTD_TU_PTP_TIME
{
    MTD_TU_PTP_ARR0_TIME = 0x0,
    MTD_TU_PTP_ARR1_TIME = 0x1,
    MTD_TU_PTP_DEP_TIME = 0x2
} MTD_TU_PTP_TIME;

/*
* Typedef: enum MTD_TU_PTP_INT_STATUS
*
* Description: Defines the PTP Port interrupt status for time stamp
*
* Fields:
*      PTP_INT_NORMAL        - No error condition occurred
*      PTP_INT_OVERWRITE     - PTP logic has to process more than one PTP frame
*                                  that needs time stamping before the current read.
*                                Only the latest one is saved.
*      PTP_INT_DROP          - PTP logic has to process more than one PTP frame
*                                  that needs time stamping before the current read.
*                                Only the oldest one is saved.
*
*/
typedef enum
{
    MTD_TU_PTP_INT_NORMAL = 0x0,
    MTD_TU_PTP_INT_OVERWRITE = 0x1,
    MTD_TU_PTP_INT_DROP = 0x2
} MTD_TU_PTP_INT_STATUS;

/*
*  typedef: struct MTD_TU_PTP_TS_STATUS
*
*  Description: PTP port status of time stamp
*
*  Fields:
*      isValid        - time stamp is valid
*      status        - time stamp error status
*      timeStamped    - time stamp value of a PTP frame that needs to be time stamped
*      ptpSeqId    - sequence ID of the frame whose time stamp information has been captured
*/
typedef struct _MTD_TU_PTP_TS_STATUS
{
    MTD_BOOL isValid;
    MTD_U32 timeStamped;
    MTD_U16 ptpSeqId;
    MTD_TU_PTP_INT_STATUS status;
} MTD_TU_PTP_TS_STATUS;

/*
*  typedef: struct MTD_TU_PTP_GLOBAL_CONFIG
*
*  Description: PTP global configuration parameters
*
*  Fields:
*      ptpEType  - PTP Ether Type
*      msgIdTSEn - Message IDs that needs time stamp
*      tsArrPtr  - Time stamp arrival time counter pointer (either Arr0Time or Arr1Time)
*/
typedef struct _MTD_TU_PTP_GLOBAL_CONFIG
{
    MTD_U16 ptpEType;
    MTD_U16 msgIdTSEn;
    MTD_U16 tsArrPtr;
} MTD_TU_PTP_GLOBAL_CONFIG;

/*
*  typedef: struct MTD_TU_PTP_INTERRUPT_STATUS
*
*  Description: PTP global interrupt status structure after Opus
*
*  Fields:
*     ptpTrigGenInt;                Trigger generate mode Interrupt
*     ptpEventInt;                  Event Capture Interrupt
*     ptpUpperPortInt;              Upper port Interrupt
*     ptpIntStLowerPortVect;        Precise Time Protocol Interrupt for Ports 0 - 10.
*     ptpIntStUpperPortVect;        Precise Time Protocol Interrupt for Ports 11 - 26.
*/
typedef struct _MTD_TU_PTP_INTERRUPT_STATUS
{
    MTD_U8  ptpTrigGenInt;
    MTD_U8  ptpEventInt;
    MTD_U8  ptpUpperPortInt;
    MTD_U16 ptpIntStLowerPortVect;
    MTD_U16 ptpIntStUpperPortVect;
} MTD_TU_PTP_INTERRUPT_STATUS;

/*
*  typedef: struct MTD_TU_PTP_TIME_ARRAY
*
*  Description: PTP Time Array consists of:
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
*  Fields:
*     todLoadPoint:        Time of Day Load Point
*     clkActive:           Clock Active bit, whether Time Array will start keeping time
*     domainNumber:        Domain Number, to be associated with the selected Time Array
*     todNanoseconds:      Time Array Time of Day, Nano second 32 bits
*     todSecondsLow:       Time Array Time of Day, second portion 0-31 bits
*     todSecondsHigh:      Time Array Time of Day, second portion 32-47 bits
*     Nanoseconds1722Low:  Time Array 1722 Time of Day in Nano seconds 0-31 bits
*     Nanoseconds1722High: Time Array 1722 Time of Day in Nano seconds 32-63 bits
*     todCompensation:     Time Array Time of Day Compensation 32 bits
*/
typedef struct _MTD_TU_PTP_TIME_ARRAY
{
    MTD_U32  todLoadPoint;
    MTD_BOOL clkActive;
    MTD_U8   domainNumber;
    MTD_U32  todNanoseconds;
    MTD_U32  todSecondsLow;
    MTD_U32  todSecondsHigh;
    MTD_U32  Nanoseconds1722Low;
    MTD_U32  Nanoseconds1722High;
    MTD_U32  todCompensation;
}MTD_TU_PTP_TIME_ARRAY;

/*
* Typedef: enum MTD_TU_PTP_TOD_STORE_OPERATION
*
* Description: Defines the PTP time array Store Operation type
*
* Fields:
*      PTP_TOD_STORE_COMP           - Store Comp register only to selected TimeArray
*      PTP_TOD_STORE_ALL            - Store All Registers to selected TimeArray
*/
typedef enum _MTD_TU_PTP_TOD_STORE_OPERATION
{
    MTD_TU_PTP_TOD_STORE_COMP = 0x2,
    MTD_TU_PTP_TOD_STORE_ALL = 0x3,
    MTD_TU_PTP_TOD_CAPTURE_TA = 0x4
} MTD_TU_PTP_TOD_STORE_OPERATION;

/*
*  typedef: struct MTD_TU_PTP_PULSE_STRUCT
*
*  Description: PTP 1pps Pulse parameters
*
*  Fields:
*     ptpPulseWidth; Pulse Width for the 1 Pulse Per Second on the Second signal
*     ptp1ppsWidthRange; Pulse Width Range for the 1 Pulse Per Second on the Second signal
*     ptp1ppsPhase; Phase of the 1 Pulse Per Second on the Second signal
*     ptp1ppsSelect; Select for the 1 Pulse Per Second on the Second signal.
*/
typedef struct _MTD_TU_PTP_PULSE_STRUCT
{
    MTD_U8 ptpPulseWidth;
    MTD_U8 ptpAltTCAMTimeSel;
    MTD_U8 ptp1ppsWidthRange;
    MTD_U8 ptpTCAMTimeSel;
    MTD_U8 ptp1ppsPhase;
    MTD_U8 ptp1ppsSelect;
} MTD_TU_PTP_PULSE_STRUCT;

/* 
   typedef: struct MTD_TU_PTP_TAI_CONFIG
 
    PTP Time Application Interface(TAI) Configuration 
    PTP Event Request Capture, PTP Trigger Generate, PTP Global time 
    increment/decrement, multi-ptp device time sync function etc.
*/
typedef enum _MTD_TU_PTP_TAI_CONFIG
{
    MTD_TU_PTP_TAI_TRIG_GENREQ     = 0, /* Trigger Generation Request */
    MTD_TU_PTP_TAI_TRIG_MODE       = 1, /* Trigger Mode */
    MTD_TU_PTP_TAI_MULT_SYNC       = 2, /* Multi PTP device Sync Mode */
    MTD_TU_PTP_TAI_BLOCK_UPDATE    = 3, /* Block Update */
    MTD_TU_PTP_TAI_TRIG_LOCK       = 7, /* Trigger Lock Enable */
    MTD_TU_PTP_TAI_EVENT_CAP_INTR  = 8, /* Event Capture Interrupt Enable */
    MTD_TU_PTP_TAI_TRIG_GEN_INTR   = 9, /* Trigger Generator Interrupt */
    MTD_TU_PTP_TAI_TIME_INCDEC_EN  = 11,/* Time Increment Decrement Enable */ 
    MTD_TU_PTP_TAI_EVENT_CNT_START = 14,/* Event Counter Start */ 
    MTD_TU_PTP_TAI_EVENT_CAP_OW    = 15 /* Event Capture Overwrite */
} MTD_TU_PTP_TAI_CONFIG;



#ifdef __cplusplus
}
#endif

#endif /* MTD_TU_PTP */

#endif /* MTD_TU_PTP_TYPES_H */
