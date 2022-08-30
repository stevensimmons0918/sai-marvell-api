/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformPpPortMaps.c
*
* @brief This file contains the port map definitions and corresponding functions
*
* @version   1
********************************************************************************
*/
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPpUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*number of DQ ports in each of the 4 DQ units in TXQ */
#define ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS     25
/*macro to convert local port and data path index to TXQ port */
#define ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS)

#define ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort)               \
    (((globalMacPort) < 24) ? ((globalMacPort) / 12) : /*0/1*/           \
     ((globalMacPort) < 72) ? (2 + ((globalMacPort)-24)/24) : /*2/3*/  \
     /*globalMacPort == 72*/ 0)

#define ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort)          \
    (((globalMacPort) < 24) ? ((globalMacPort) % 12) : /*0..11*/         \
     ((globalMacPort) < 72) ? ((globalMacPort) % 24) : /*0..23*/         \
     /*globalMacPort == 72*/ 24)

/* build TXQ_port from global mac port */
#define ALDRIN2_TXQ_PORT(globalMacPort)                            \
    ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(                      \
        ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort),     \
        ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort))

#define ALDRIN2_MAC_AND_TXQ_PORT_MAC(macPort)  \
    (macPort), ALDRIN2_TXQ_PORT(macPort)

/* build TXQ_port for 'cpu port' */
#define ALDRIN2_TXQ_SDAM0_CPU_PORT    (25*3)+24/*99*/  /*(24 is for SDMA port in DQ[3]) */
#define ALDRIN2_TXQ_SDAM1_CPU_PORT    (25*1)+24/*49*/  /*(24 is for SDMA port in DQ[1])*/
#define ALDRIN2_TXQ_SDAM2_CPU_PORT    (25*2)+24/*74*/  /*(24 is for SDMA port in DQ[2])*/
#define ALDRIN2_TXQ_SDAM3_CPU_PORT    (25*0)+23/*23*/  /*(23 is for SDMA port in DQ[0])*/

#define ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM0_CPU_PORT
#define ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM1_CPU_PORT
#define ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM2_CPU_PORT
#define ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM3_CPU_PORT

static CPSS_DXCH_PORT_MAP_STC aldrinDefaultMap[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd,    reservePreemptiveChannel*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,      GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_24_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd,    reservePreemptiveChannel*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,      GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,      GT_FALSE}
};




static CPSS_DXCH_PORT_MAP_STC aldrinMap_30_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_22_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_16_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_18_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};


static CPSS_DXCH_PORT_MAP_STC aldrinMap_8_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_12_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

static  CPSS_DXCH_PORT_MAP_STC singleDp_aldrin2defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*0*/  {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA,  GT_FALSE}
/*1*/ ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA,  GT_FALSE}
/*2*/ ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA,  GT_FALSE}
/*3*/ ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA,  GT_FALSE}
/*4*/ ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA,  GT_FALSE}
/*5*/ ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA,  GT_FALSE}
/*6*/ ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA,  GT_FALSE}
/*7*/ ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA,  GT_FALSE}
/*8*/ ,{  79, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA,  GT_FALSE}
/*9*/ ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA,  GT_FALSE}
/************************  DMAs only *******************/
/*DP[0]*/
/*23*/,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,         0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}
};

/* use same ports as 'BC3/ARMSTRONG' (but must have different TXQ-ports) */
static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*12*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+12)/*36*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*13*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+13)/*37*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*14*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+14)/*38*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*15*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+15)/*39*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*16*/    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+16)/*40*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*17*/    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+17)/*41*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*18*/    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+18)/*42*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*19*/    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+19)/*43*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*20*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+20)/*44*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*21*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+21)/*45*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*22*/    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+22)/*46*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+23)/*47*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*12*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+12)/*60*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*13*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+13)/*61*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*14*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+14)/*62*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*15*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+15)/*63*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*16*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+16)/*64*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*17*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+17)/*65*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*18*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+18)/*66*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*19*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+19)/*67*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*20*/    ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+20)/*68*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*21*/    ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+21)/*69*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*22*/    ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+22)/*70*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*23*/    ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+23)/*71*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/********** special 1 last MAC port ***********************/
/*DP[0]*/
/*24*/    ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(72)   /*72*//*TXQ-24*/ ,  GT_FALSE,   GT_NA,  GT_FALSE}

/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA,  GT_FALSE}

};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_48_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/********** special 1 last MAC port ***********************/
/*DP[2]*/
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47)   /*47*//*TXQ-24*/ ,  GT_FALSE,   GT_NA,  GT_FALSE}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_24_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/********** special 1 last MAC port ***********************/
/*DP[2]*/
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47)   /*47*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_64_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 4*/     {   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[1]*/
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*12*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+12)/*36*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*13*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+13)/*37*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*14*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+14)/*38*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*15*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+15)/*39*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*16*/    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+16)/*40*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*17*/    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+17)/*41*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*18*/    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+18)/*42*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*19*/    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+19)/*43*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*20*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+20)/*44*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*21*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+21)/*45*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*22*/    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+22)/*46*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+23)/*47*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*12*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+12)/*60*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*13*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+13)/*61*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*14*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+14)/*62*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*15*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+15)/*63*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*16*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+16)/*64*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*17*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+17)/*65*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*18*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+18)/*66*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/********** special 1 last MAC port ***********************/
/*DP[0]*/
/*24*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(72)   /*72*//*TXQ-24*/ ,  GT_FALSE,   GT_NA,  GT_FALSE}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_48_special_map_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(13),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(14),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(15),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(16),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(17),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(18),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(19),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(20),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(21),  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(22),  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(23),  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/* 0*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(28),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(29),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(30),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(31),  GT_FALSE,   GT_NA,  GT_FALSE}

/* 4*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(36),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(37),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(38),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(39),  GT_FALSE,   GT_NA,  GT_FALSE}

/* 8*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(44),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(45),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 10*/   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(46),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 11*/   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47),  GT_FALSE,   GT_NA,  GT_FALSE}

/*DP[3]*/
/* 0*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(52),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 1*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(53),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 2*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(54),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 3*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(55),  GT_FALSE,   GT_NA,  GT_FALSE}

/* 4*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(60),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 5*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(61),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 6*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(62),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 7*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(63),  GT_FALSE,   GT_NA,  GT_FALSE}

/* 8*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(68),  GT_FALSE,   GT_NA,  GT_FALSE}
/* 9*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(69),  GT_FALSE,   GT_NA,  GT_FALSE}
/*10*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(70),  GT_FALSE,   GT_NA,  GT_FALSE}
/*11*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(71),  GT_FALSE,   GT_NA,  GT_FALSE}

/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA,  GT_FALSE}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA,  GT_FALSE}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA,  GT_FALSE}
};

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#if 0
/* Falcon */
typedef struct
{
    GT_PHYSICAL_PORT_NUM                startPhysicalPortNumber;
    GT_U32                              numOfPorts;
    GT_U32                              startGlobalDmaNumber;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              jumpDmaPorts;/* allow to skip DMA ports (from startGlobalDmaNumber).
                                                        in case that mappingType is CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E
                                                        this field Contains the cascade port number*/
}FALCON_PORT_MAP_STC;

static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_512_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 259 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  259/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 15 CPU ports , the existence of next ports : in DP[8..11].*/
    /* physical ports 64..260 , mapped to MAC 59..255 (in steps of 1) */
    ,{64              ,(255 - 59 + 1),  59  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 262 , mapped to MAC 256 */
    ,{261             , 1           ,  256    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 264..277 (no MAC/SERDES) DMA 260..273 (MG[2..15])*/
    ,{264              , 14          ,  260/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 280 (no MAC/SERDES) DMA 258 (MG[0])*/
    ,{280              , 1           ,  258/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
}

static GT_U32   actualNum_falcon_12_8_defaultMap_512_port_mode =
    sizeof(falcon_12_8_defaultMap_512_port_mode)
    /sizeof(falcon_12_8_defaultMap_512_port_mode[0]);
;
#endif

extern GT_STATUS cpssAppPlatformPpFalconPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
);

extern GT_STATUS cpssAppPlatformPpAc5xPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
);

/**
* @internal cpssAppPlatformPpPortMapGet function
* @endinternal
*
* @brief  Get port map details for a specific device.
*
* @param[in] devNum                - device number
* @param[out] mapArrPtr            - pointer to portMap pointer/Array
* @param[out] mapArrLen            - pointer to store portMap array length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on not filling the out params
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPpPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
)
{
    GT_STATUS                   rc = GT_FAIL;
    GT_U32                      coreClockDB;
    GT_U32                      coreClockHW;
    CPSS_PP_DEVICE_TYPE         devType;
    CPSS_DXCH_CFG_DEV_INFO_STC  devInfo;

    if (!mapArrPtr || !mapArrLen)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    rc = cpssDxChHwCoreClockGet(devNum, &coreClockDB, &coreClockHW);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwCoreClockGet);

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSingleDp = 0;
                #ifdef GM_USED
                    useSingleDp = 1;
                #endif/* GM_USED */
                if(useSingleDp)
                {
                    ARR_PTR_AND_SIZE_MAC(singleDp_aldrin2defaultMap,*mapArrPtr,*mapArrLen);
                    return GT_OK;
                }
                else
                {
                    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevInfoGet);

                    devType = devInfo.genDevInfo.devType;
                    if (devType == CPSS_98EX5524_CNS ||
                        devType == CPSS_98DX8524_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_48_ports, *mapArrPtr, *mapArrLen);
                    }
                    else
                    if (devType == CPSS_98DX8525_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_24_ports, *mapArrPtr, *mapArrLen);
                    }
                    else
                    if (devType == CPSS_98DX4310_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_64_ports, *mapArrPtr, *mapArrLen);
                    }
                    else
                    if (devType == CPSS_98DX8410_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_48_special_map_ports, *mapArrPtr, *mapArrLen);
                    }
                    else
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap, *mapArrPtr, *mapArrLen);
                        if (coreClockDB == 525)
                        {
                            /* Remove unused SDMA ports in order to meet BW limits */
                            *mapArrLen -= 3;
                        }
                    }
                }
            }
            break;

        /******************/
        /* ALDRIN devices */
        /******************/
        case CPSS_98DX8308_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_8_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DX8312_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_12_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DX8316_CNS:
        case CPSS_98DXH831_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_16_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DXH834_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_22_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DX3256_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_18_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DX3257_CNS:
        case CPSS_98DX8324_CNS:
        case CPSS_98DXH832_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_24_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DXH835_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_30_ports,*mapArrPtr,*mapArrLen);
            break;
        case CPSS_98DX3255_CNS:
        case CPSS_98DX3258_CNS:
        case CPSS_98DX3259_CNS:
        case CPSS_98DX3248_CNS:
        case CPSS_98DX3249_CNS:
        case CPSS_98DX8315_CNS:
        case CPSS_98DX8332_CNS:
        case CPSS_98DXZ832_CNS:
        case CPSS_98DXH833_CNS:
        case CPSS_98DX3265_CNS:
        case CPSS_98DX3268_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinDefaultMap,*mapArrPtr,*mapArrLen);
            break;

        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:

#ifndef GM_USED
            rc = cpssAppPlatformPpFalconPortMapGet(devNum, mapArrPtr, mapArrLen);
#endif
            break;

        case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:

#ifndef GM_USED
            rc = cpssAppPlatformPpAc5xPortMapGet(devNum, mapArrPtr, mapArrLen);
#endif

        default:
            break;
    }

    return rc;
}
