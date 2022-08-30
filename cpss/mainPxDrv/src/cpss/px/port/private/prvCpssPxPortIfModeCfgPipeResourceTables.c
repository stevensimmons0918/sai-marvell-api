/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssPxPortIfModeCfgPipeResourceTables.c
*
* @brief CPSS resource tables for Pipe
*
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>


#define ANY_CC (GT_U32)(~0)

/* after copying table calculated by MatLab       */
/* program:                                       */
/* added support of these speeds                  */
/* values interpolated from other speeds          */
/* CPSS_PORT_SPEED_11800_E                        */
/* CPSS_PORT_SPEED_107G_E                         */
/* speeds below not supported and                 */
/* 30000, 60000 and 110000 bypassed in speed list */
/* values in tables for new speeds:               */
/* for 11800 as for 12500                         */
/* for 107000 as for 110000                       */

/*------------------------------------------------------------*
 * This file was auto-generated at 15/8/2016 13:8:26 *
 *------------------------------------------------------------*/
const PRV_CPSS_PX_SPEED_PLACE_STC prv_PIPE_speedPlaceArr[] =
{
     {         CPSS_PORT_SPEED_1000_E,    1000,    1 }
    ,{         CPSS_PORT_SPEED_2500_E,    2500,    2 }
    ,{         CPSS_PORT_SPEED_5000_E,    5000,    3 }
    ,{        CPSS_PORT_SPEED_10000_E,   10000,    4 }
    ,{        CPSS_PORT_SPEED_11800_E,   11800,    5 }
    ,{        CPSS_PORT_SPEED_12500_E,   12500,    6 }
    ,{        CPSS_PORT_SPEED_20000_E,   20000,    7 }
    ,{        CPSS_PORT_SPEED_23600_E,   23600,    8 }
    ,{        CPSS_PORT_SPEED_25000_E,   25000,    9 }
    ,{        CPSS_PORT_SPEED_26700_E,   26700,   10 }
    ,{        CPSS_PORT_SPEED_40000_E,   40000,   11 }
    ,{        CPSS_PORT_SPEED_47200_E,   48000,   12 }
    ,{        CPSS_PORT_SPEED_50000_E,   50000,   13 }
    ,{        CPSS_PORT_SPEED_52500_E,   52500,   14 }
    ,{         CPSS_PORT_SPEED_100G_E,  100000,   16 } /* bypassed 60000 */
    ,{         CPSS_PORT_SPEED_102G_E,  102000,   17 }
    ,{         CPSS_PORT_SPEED_107G_E,  107000,   18 }
    ,{           CPSS_PORT_SPEED_NA_E,       0,    0 } /* bypassed 110000*/
};


const PRV_CPSS_PX_CORECLOCK_PLACE_STC prv_PIPE_coreClockPlaceArr[] =
{    /* real cc, computation, idx */
      {     288,     288,   1  }
     ,{     350,     350,   2 }
     ,{     450,     450,   3 }
     ,{     500,     500,   4 }
     ,{       0,       0,   0 }
};


/*-----------------------------------------------------------------------------------*
 * PIPE TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
/*ERRATUM PRV_CPSS_PX_SLOW_PORT_TXQ_CRDITS_WA_E,  credits for ports less that 10 giga should the same as 10 giga port credits */
const GT_U32 PIPE_TXDMA_SCDMA_TxQDescriptorCredit_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500, 20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,  60000, 100000, 102000,  107000,  110000 }
    ,{    288,      9,      9,      9,      9,      8,      8,    15,     14,     14,     14,     26,     30,     31,     31,     30,     59,     51,      51,      51 }
    ,{    350,      9,      9,      9,      9,      8,      8,    14,     13,     13,     13,     23,     27,     28,     28,     26,     50,     45,      45,      45 }
    ,{    450,      8,      8,      8,      8,      8,      8,    12,     11,     11,     11,     19,     22,     23,     23,     21,     41,     36,      36,      36 }
    ,{    500,      8,      8,      8,      8,      7,      7,    12,     11,     11,     11,     18,     20,     21,     21,     20,     37,     33,      33,      33 }
};
/*-----------------------------------------------------------------------------------*
 * PIPE TXDMA_SCDMA_burstAlmostFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 PIPE_TXDMA_SCDMA_burstAlmostFullThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500,  20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,  60000, 100000, 102000, 107000, 110000 }
    ,{    288,      2,      3,      3,      5,      6,      6,      8,      9,      9,      9,     14,     16,     17,     17,     20,     32,     34,     34,     34 }
    ,{    350,      2,      3,      4,      5,      6,      6,      8,      9,      9,      9,     12,     14,     15,     15,     17,     27,     30,     30,     30 }
    ,{    450,      2,      3,      4,      5,      5,      5,      7,      8,      8,      8,     11,     12,     12,     12,     14,     22,     24,     24,     24 }
    ,{    500,      2,      3,      4,      5,      5,      5,      6,      7,      7,      7,     10,     11,     12,     12,     13,     20,     22,     22,     22 }
};
/*-----------------------------------------------------------------------------------*
 * PIPE TXDMA_SCDMA_burstFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 PIPE_TXDMA_SCDMA_burstFullThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500,  20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,  60000, 100000, 102000, 107000, 110000 }
    ,{    288,      3,      4,      5,      9,      8,      8,     15,     14,     14,     14,     26,     30,     31,     31,     30,     59,     51,     51,     51 }
    ,{    350,      3,      5,      6,      9,      8,      8,     14,     13,     13,     13,     23,     27,     28,     28,     26,     50,     45,     45,     45 }
    ,{    450,      3,      4,      5,      8,      8,      8,     12,     11,     11,     11,     19,     22,     23,     23,     21,     41,     36,     36,     36 }
    ,{    500,      3,      4,      5,      8,      7,      7,     12,     11,     11,     11,     18,     20,     21,     21,     20,     37,     33,     33,     33 }
};
/*-----------------------------------------------------------------------------------*
 * PIPE TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 PIPE_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500,  20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,   60000, 100000, 102000, 107000, 110000 }
    ,{    288,      3,      4,      6,     12,     11,     11,     20,     20,     20,     20,     37,     44,     46,     46,      44,     89,     78,     78,     78 }
    ,{    350,      3,      5,      6,     11,     11,     11,     18,     18,     18,     18,     31,     38,     40,     40,      38,     75,     66,     66,     66 }
    ,{    450,      3,      5,      6,      9,      9,      9,     15,     14,     14,     14,     27,     30,     32,     32,      30,     59,     53,     53,     53 }
    ,{    500,      3,      4,      5,     10,      9,      9,     15,     13,     13,     13,     25,     27,     29,     29,      27,     53,     48,     48,     48 }
};
/*-----------------------------------------------------------------------------------*
 * PIPE TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 PIPE_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500,  20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,  60000, 100000, 102000, 107000, 110000 }
    ,{    288,      5,      6,      8,     14,     13,     13,     23,     23,     23,     23,     41,     48,     51,     51,     49,     97,     87,     87,     87 }
    ,{    350,      5,      7,      8,     13,     13,     13,     21,     21,     21,     21,     35,     42,     44,     44,     43,     82,     74,     74,     74 }
    ,{    450,      5,      7,      8,     11,     11,     11,     17,     17,     17,     17,     30,     34,     36,     36,     34,     65,     59,     59,     59 }
    ,{    500,      5,      6,      7,     12,     11,     11,     17,     16,     16,     16,     28,     30,     33,     33,     31,     58,     54,     54,     54 }
};
/*-----------------------------------------------------------------------------------*
 * PIPE TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 PIPE_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  11800,  12500,  20000,  23600,  25000,  26700,  40000,  48000,  50000,  52500,  60000, 100000, 102000, 107000, 110000 }
    ,{    288,      2,      2,      3,      3,      3,      3,      4,      4,      4,      4,      5,      5,      6,      6,      6,      9,     10,     10,     10 }
    ,{    350,      3,      2,      3,      3,      3,      3,      4,      4,      4,      4,      5,      5,      5,      5,      6,      8,      9,      9,      9 }
    ,{    450,      3,      2,      3,      3,      3,      3,      3,      4,      4,      4,      4,      5,      5,      5,      5,      7,      7,      7,      7 }
    ,{    500,      3,      2,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      5,      5,      5,      6,      7,      7,      7 }
};



