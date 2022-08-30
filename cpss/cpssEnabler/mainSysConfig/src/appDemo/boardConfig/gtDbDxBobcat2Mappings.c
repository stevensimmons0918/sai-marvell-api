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
* @file gtDbDxBobcat2Mappings.c
*
* @brief Initialization Mapping and Port for the Bobcat2 - SIP5 - board.
*
* @version   11
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*******************************************************************************
* configBoardAfterPhase1 : relevant data structures for port mapping
*
*       This function performs all needed configurations that should be done
*******************************************************************************/
/* #define __TEST_MAPPING_CNS */

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,      48,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,      49,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       50,      50,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       51,      51,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,      52,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,      53,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,      54,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,      55,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       60,      60,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       61,      61,           GT_FALSE,          GT_NA,GT_FALSE} /* virtual router port */
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       63,      62,           GT_FALSE,          GT_NA,GT_FALSE} /* don't use MAC 62 to avoid OOB port reconfiguration */
};

static CPSS_DXCH_PORT_MAP_STC bc2TmEnableMap[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,     GT_NA,         GT_TRUE,            0 ,GT_FALSE}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,     GT_NA,         GT_TRUE,            1 ,GT_FALSE}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,     GT_NA,         GT_TRUE,            2 ,GT_FALSE}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,     GT_NA,         GT_TRUE,            3 ,GT_FALSE}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,     GT_NA,         GT_TRUE,            4 ,GT_FALSE}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,     GT_NA,         GT_TRUE,            5 ,GT_FALSE}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,     GT_NA,         GT_TRUE,            6 ,GT_FALSE}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,     GT_NA,         GT_TRUE,            7 ,GT_FALSE}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,     GT_NA,         GT_TRUE,            8 ,GT_FALSE}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,     GT_NA,         GT_TRUE,            9 ,GT_FALSE}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,     GT_NA,         GT_TRUE,           10 ,GT_FALSE}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,     GT_NA,         GT_TRUE,           11 ,GT_FALSE}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,     GT_NA,         GT_TRUE,           12 ,GT_FALSE}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,     GT_NA,         GT_TRUE,           13 ,GT_FALSE}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,     GT_NA,         GT_TRUE,           14 ,GT_FALSE}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,     GT_NA,         GT_TRUE,           15 ,GT_FALSE}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,     GT_NA,         GT_TRUE,           16 ,GT_FALSE}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,     GT_NA,         GT_TRUE,           17 ,GT_FALSE}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,     GT_NA,         GT_TRUE,           18 ,GT_FALSE}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,     GT_NA,         GT_TRUE,           19 ,GT_FALSE}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,     GT_NA,         GT_TRUE,           20 ,GT_FALSE}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,     GT_NA,         GT_TRUE,           21 ,GT_FALSE}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,     GT_NA,         GT_TRUE,           22 ,GT_FALSE}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,     GT_NA,         GT_TRUE,           23 ,GT_FALSE}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,     GT_NA,         GT_TRUE,           24 ,GT_FALSE}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,     GT_NA,         GT_TRUE,           25 ,GT_FALSE}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,     GT_NA,         GT_TRUE,           26 ,GT_FALSE}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,     GT_NA,         GT_TRUE,           27 ,GT_FALSE}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,     GT_NA,         GT_TRUE,           28 ,GT_FALSE}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,     GT_NA,         GT_TRUE,           29 ,GT_FALSE}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,     GT_NA,         GT_TRUE,           30 ,GT_FALSE}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,     GT_NA,         GT_TRUE,           31 ,GT_FALSE}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,     GT_NA,         GT_TRUE,           32 ,GT_FALSE}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,     GT_NA,         GT_TRUE,           33 ,GT_FALSE}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,     GT_NA,         GT_TRUE,           34 ,GT_FALSE}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,     GT_NA,         GT_TRUE,           35 ,GT_FALSE}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,     GT_NA,         GT_TRUE,           36 ,GT_FALSE}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,     GT_NA,         GT_TRUE,           37 ,GT_FALSE}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,     GT_NA,         GT_TRUE,           38 ,GT_FALSE}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,     GT_NA,         GT_TRUE,           39 ,GT_FALSE}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,     GT_NA,         GT_TRUE,           40 ,GT_FALSE}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,     GT_NA,         GT_TRUE,           41 ,GT_FALSE}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,     GT_NA,         GT_TRUE,           42 ,GT_FALSE}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,     GT_NA,         GT_TRUE,           43 ,GT_FALSE}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,     GT_NA,         GT_TRUE,           44 ,GT_FALSE}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,     GT_NA,         GT_TRUE,           45 ,GT_FALSE}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,     GT_NA,         GT_TRUE,           46 ,GT_FALSE}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,     GT_NA,         GT_TRUE,           47 ,GT_FALSE}
    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,     GT_NA,         GT_TRUE,           48 ,GT_FALSE}
    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,     GT_NA,         GT_TRUE,           49 ,GT_FALSE}
    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       50,     GT_NA,         GT_TRUE,           50 ,GT_FALSE}
    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       51,     GT_NA,         GT_TRUE,           51 ,GT_FALSE}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,     GT_NA,         GT_TRUE,           56 ,GT_FALSE}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,     GT_NA,         GT_TRUE,           58 ,GT_FALSE}
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       60,        60,        GT_FALSE,        GT_NA ,GT_FALSE}
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       63,        62,        GT_FALSE,        GT_NA ,GT_FALSE} /* don't use MAC 62 to avoid OOB port reconfiguration */
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,        63,        GT_FALSE,        GT_NA ,GT_FALSE}
  /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd    */
  /* Port 128 + i  <---> channel i  0 <= i <= 63   tm  128+i                                                                     */
    ,{ 128, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        0,     GT_NA,         GT_TRUE,          128 ,GT_FALSE}
    ,{ 129, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        1,     GT_NA,         GT_TRUE,          129 ,GT_FALSE}
    ,{ 130, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        2,     GT_NA,         GT_TRUE,          130 ,GT_FALSE}
    ,{ 131, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        3,     GT_NA,         GT_TRUE,          131 ,GT_FALSE}
    ,{ 132, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        4,     GT_NA,         GT_TRUE,          132 ,GT_FALSE}
    ,{ 133, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        5,     GT_NA,         GT_TRUE,          133 ,GT_FALSE}
    ,{ 134, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        6,     GT_NA,         GT_TRUE,          134 ,GT_FALSE}
    ,{ 135, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        7,     GT_NA,         GT_TRUE,          135 ,GT_FALSE}
    ,{ 136, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        8,     GT_NA,         GT_TRUE,          136 ,GT_FALSE}
    ,{ 137, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        9,     GT_NA,         GT_TRUE,          137 ,GT_FALSE}
    ,{ 138, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       10,     GT_NA,         GT_TRUE,          138 ,GT_FALSE}
    ,{ 139, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       11,     GT_NA,         GT_TRUE,          139 ,GT_FALSE}
    ,{ 140, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       12,     GT_NA,         GT_TRUE,          140 ,GT_FALSE}
    ,{ 141, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       13,     GT_NA,         GT_TRUE,          141 ,GT_FALSE}
    ,{ 142, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       14,     GT_NA,         GT_TRUE,          142 ,GT_FALSE}
    ,{ 143, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       15,     GT_NA,         GT_TRUE,          143 ,GT_FALSE}
    ,{ 144, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       16,     GT_NA,         GT_TRUE,          144 ,GT_FALSE}
    ,{ 145, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       17,     GT_NA,         GT_TRUE,          145 ,GT_FALSE}
    ,{ 146, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       18,     GT_NA,         GT_TRUE,          146 ,GT_FALSE}
    ,{ 147, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       19,     GT_NA,         GT_TRUE,          147 ,GT_FALSE}
    ,{ 148, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       20,     GT_NA,         GT_TRUE,          148 ,GT_FALSE}
    ,{ 149, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       21,     GT_NA,         GT_TRUE,          149 ,GT_FALSE}
    ,{ 150, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       22,     GT_NA,         GT_TRUE,          150 ,GT_FALSE}
    ,{ 151, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       23,     GT_NA,         GT_TRUE,          151 ,GT_FALSE}
    ,{ 152, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       24,     GT_NA,         GT_TRUE,          152 ,GT_FALSE}
    ,{ 153, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       25,     GT_NA,         GT_TRUE,          153 ,GT_FALSE}
    ,{ 154, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       26,     GT_NA,         GT_TRUE,          154 ,GT_FALSE}
    ,{ 155, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       27,     GT_NA,         GT_TRUE,          155 ,GT_FALSE}
    ,{ 156, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       28,     GT_NA,         GT_TRUE,          156 ,GT_FALSE}
    ,{ 157, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       29,     GT_NA,         GT_TRUE,          157 ,GT_FALSE}
    ,{ 158, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       30,     GT_NA,         GT_TRUE,          158 ,GT_FALSE}
    ,{ 159, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       31,     GT_NA,         GT_TRUE,          159 ,GT_FALSE}
    ,{ 160, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       32,     GT_NA,         GT_TRUE,          160 ,GT_FALSE}
    ,{ 161, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       33,     GT_NA,         GT_TRUE,          161 ,GT_FALSE}
    ,{ 162, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       34,     GT_NA,         GT_TRUE,          162 ,GT_FALSE}
    ,{ 163, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       35,     GT_NA,         GT_TRUE,          163 ,GT_FALSE}
    ,{ 164, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       36,     GT_NA,         GT_TRUE,          164 ,GT_FALSE}
    ,{ 165, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       37,     GT_NA,         GT_TRUE,          165 ,GT_FALSE}
    ,{ 166, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       38,     GT_NA,         GT_TRUE,          166 ,GT_FALSE}
    ,{ 167, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       39,     GT_NA,         GT_TRUE,          167 ,GT_FALSE}
    ,{ 168, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       40,     GT_NA,         GT_TRUE,          168 ,GT_FALSE}
    ,{ 169, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       41,     GT_NA,         GT_TRUE,          169 ,GT_FALSE}
    ,{ 170, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       42,     GT_NA,         GT_TRUE,          170 ,GT_FALSE}
    ,{ 171, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       43,     GT_NA,         GT_TRUE,          171 ,GT_FALSE}
    ,{ 172, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       44,     GT_NA,         GT_TRUE,          172 ,GT_FALSE}
    ,{ 173, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       45,     GT_NA,         GT_TRUE,          173 ,GT_FALSE}
    ,{ 174, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       46,     GT_NA,         GT_TRUE,          174 ,GT_FALSE}
    ,{ 175, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       47,     GT_NA,         GT_TRUE,          175 ,GT_FALSE}
    ,{ 176, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       48,     GT_NA,         GT_TRUE,          176 ,GT_FALSE}
    ,{ 177, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       49,     GT_NA,         GT_TRUE,          177 ,GT_FALSE}
    ,{ 178, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       50,     GT_NA,         GT_TRUE,          178 ,GT_FALSE}
    ,{ 179, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       51,     GT_NA,         GT_TRUE,          179 ,GT_FALSE}
    ,{ 180, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       52,     GT_NA,         GT_TRUE,          180 ,GT_FALSE}
    ,{ 181, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       53,     GT_NA,         GT_TRUE,          181 ,GT_FALSE}
    ,{ 182, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       54,     GT_NA,         GT_TRUE,          182 ,GT_FALSE}
    ,{ 183, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       55,     GT_NA,         GT_TRUE,          183 ,GT_FALSE}
    ,{ 184, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       56,     GT_NA,         GT_TRUE,          184 ,GT_FALSE}
    ,{ 185, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       57,     GT_NA,         GT_TRUE,          185 ,GT_FALSE}
    ,{ 186, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       58,     GT_NA,         GT_TRUE,          186 ,GT_FALSE}
    ,{ 187, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       59,     GT_NA,         GT_TRUE,          187 ,GT_FALSE}
    ,{ 188, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       60,     GT_NA,         GT_TRUE,          188 ,GT_FALSE}
    ,{ 189, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       61,     GT_NA,         GT_TRUE,          189 ,GT_FALSE}
    ,{ 190, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       62,     GT_NA,         GT_TRUE,          190 ,GT_FALSE}
    ,{ 191, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       63,     GT_NA,         GT_TRUE,          191 ,GT_FALSE}
};



static CPSS_DXCH_PORT_MAP_STC bc2TmEnableMap_29_12_RD_board[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,     GT_NA,         GT_TRUE,            0 ,GT_FALSE}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,     GT_NA,         GT_TRUE,            1 ,GT_FALSE}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,     GT_NA,         GT_TRUE,            2 ,GT_FALSE}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,     GT_NA,         GT_TRUE,            3 ,GT_FALSE}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,     GT_NA,         GT_TRUE,            4 ,GT_FALSE}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,     GT_NA,         GT_TRUE,            5 ,GT_FALSE}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,     GT_NA,         GT_TRUE,            6 ,GT_FALSE}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,     GT_NA,         GT_TRUE,            7 ,GT_FALSE}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,     GT_NA,         GT_TRUE,            8 ,GT_FALSE}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,     GT_NA,         GT_TRUE,            9 ,GT_FALSE}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,     GT_NA,         GT_TRUE,           10 ,GT_FALSE}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,     GT_NA,         GT_TRUE,           11 ,GT_FALSE}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,     GT_NA,         GT_TRUE,           12 ,GT_FALSE}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,     GT_NA,         GT_TRUE,           13 ,GT_FALSE}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,     GT_NA,         GT_TRUE,           14 ,GT_FALSE}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,     GT_NA,         GT_TRUE,           15 ,GT_FALSE}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,     GT_NA,         GT_TRUE,           16 ,GT_FALSE}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,     GT_NA,         GT_TRUE,           17 ,GT_FALSE}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,     GT_NA,         GT_TRUE,           18 ,GT_FALSE}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,     GT_NA,         GT_TRUE,           19 ,GT_FALSE}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,     GT_NA,         GT_TRUE,           20 ,GT_FALSE}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,     GT_NA,         GT_TRUE,           21 ,GT_FALSE}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,     GT_NA,         GT_TRUE,           22 ,GT_FALSE}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,     GT_NA,         GT_TRUE,           23 ,GT_FALSE}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,     GT_NA,         GT_TRUE,           24 ,GT_FALSE}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,     GT_NA,         GT_TRUE,           25 ,GT_FALSE}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,     GT_NA,         GT_TRUE,           26 ,GT_FALSE}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,     GT_NA,         GT_TRUE,           27 ,GT_FALSE}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,     GT_NA,         GT_TRUE,           28 ,GT_FALSE}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,     GT_NA,         GT_TRUE,           29 ,GT_FALSE}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,     GT_NA,         GT_TRUE,           30 ,GT_FALSE}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,     GT_NA,         GT_TRUE,           31 ,GT_FALSE}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,     GT_NA,         GT_TRUE,           32 ,GT_FALSE}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,     GT_NA,         GT_TRUE,           33 ,GT_FALSE}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,     GT_NA,         GT_TRUE,           34 ,GT_FALSE}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,     GT_NA,         GT_TRUE,           35 ,GT_FALSE}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,     GT_NA,         GT_TRUE,           36 ,GT_FALSE}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,     GT_NA,         GT_TRUE,           37 ,GT_FALSE}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,     GT_NA,         GT_TRUE,           38 ,GT_FALSE}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,     GT_NA,         GT_TRUE,           39 ,GT_FALSE}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,     GT_NA,         GT_TRUE,           40 ,GT_FALSE}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,     GT_NA,         GT_TRUE,           41 ,GT_FALSE}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,     GT_NA,         GT_TRUE,           42 ,GT_FALSE}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,     GT_NA,         GT_TRUE,           43 ,GT_FALSE}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,     GT_NA,         GT_TRUE,           44 ,GT_FALSE}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,     GT_NA,         GT_TRUE,           45 ,GT_FALSE}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,     GT_NA,         GT_TRUE,           46 ,GT_FALSE}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,     GT_NA,         GT_TRUE,           47 ,GT_FALSE}
    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,     GT_NA,         GT_TRUE,           48 ,GT_FALSE}
    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,     GT_NA,         GT_TRUE,           49 ,GT_FALSE}
    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       50,     GT_NA,         GT_TRUE,           50 ,GT_FALSE}
    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       51,     GT_NA,         GT_TRUE,           51 ,GT_FALSE}
    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,     GT_NA,         GT_TRUE,           52 ,GT_FALSE}
    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,     GT_NA,         GT_TRUE,           53 ,GT_FALSE}
    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,     GT_NA,         GT_TRUE,           54 ,GT_FALSE}
    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,     GT_NA,         GT_TRUE,           55 ,GT_FALSE}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,     GT_NA,         GT_TRUE,           56 ,GT_FALSE}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,     GT_NA,         GT_TRUE,           57 ,GT_FALSE}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,     GT_NA,         GT_TRUE,           58 ,GT_FALSE}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,        59,         GT_FALSE,       GT_NA ,GT_FALSE}
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       60,        60,         GT_FALSE,       GT_NA ,GT_FALSE}
    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       61,        61,         GT_FALSE,       GT_NA ,GT_FALSE} /* virtual router port */
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,        62,         GT_FALSE,       GT_NA ,GT_FALSE}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,        63,         GT_FALSE,       GT_NA ,GT_FALSE}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,        68,         GT_FALSE,       GT_NA ,GT_FALSE}
  /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd    */
  /* Port 128 + i  <---> channel i  0 <= i <= 63   tm  128+i                                                                    */
    ,{ 128, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        0,      GT_NA,        GT_TRUE,          128 ,GT_FALSE}
    ,{ 129, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        1,      GT_NA,        GT_TRUE,          129 ,GT_FALSE}
    ,{ 130, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        2,      GT_NA,        GT_TRUE,          130 ,GT_FALSE}
    ,{ 131, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        3,      GT_NA,        GT_TRUE,          131 ,GT_FALSE}
    ,{ 132, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        4,      GT_NA,        GT_TRUE,          132 ,GT_FALSE}
    ,{ 133, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        5,      GT_NA,        GT_TRUE,          133 ,GT_FALSE}
    ,{ 134, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        6,      GT_NA,        GT_TRUE,          134 ,GT_FALSE}
    ,{ 135, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        7,      GT_NA,        GT_TRUE,          135 ,GT_FALSE}
    ,{ 136, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        8,      GT_NA,        GT_TRUE,          136 ,GT_FALSE}
    ,{ 137, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,        9,      GT_NA,        GT_TRUE,          137 ,GT_FALSE}
    ,{ 138, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       10,      GT_NA,        GT_TRUE,          138 ,GT_FALSE}
    ,{ 139, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       11,      GT_NA,        GT_TRUE,          139 ,GT_FALSE}
    ,{ 140, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       12,      GT_NA,        GT_TRUE,          140 ,GT_FALSE}
    ,{ 141, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       13,      GT_NA,        GT_TRUE,          141 ,GT_FALSE}
    ,{ 142, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       14,      GT_NA,        GT_TRUE,          142 ,GT_FALSE}
    ,{ 143, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       15,      GT_NA,        GT_TRUE,          143 ,GT_FALSE}
    ,{ 144, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       16,      GT_NA,        GT_TRUE,          144 ,GT_FALSE}
    ,{ 145, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       17,      GT_NA,        GT_TRUE,          145 ,GT_FALSE}
    ,{ 146, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       18,      GT_NA,        GT_TRUE,          146 ,GT_FALSE}
    ,{ 147, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       19,      GT_NA,        GT_TRUE,          147 ,GT_FALSE}
    ,{ 148, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       20,      GT_NA,        GT_TRUE,          148 ,GT_FALSE}
    ,{ 149, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       21,      GT_NA,        GT_TRUE,          149 ,GT_FALSE}
    ,{ 150, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       22,      GT_NA,        GT_TRUE,          150 ,GT_FALSE}
    ,{ 151, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       23,      GT_NA,        GT_TRUE,          151 ,GT_FALSE}
    ,{ 152, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       24,      GT_NA,        GT_TRUE,          152 ,GT_FALSE}
    ,{ 153, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       25,      GT_NA,        GT_TRUE,          153 ,GT_FALSE}
    ,{ 154, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       26,      GT_NA,        GT_TRUE,          154 ,GT_FALSE}
    ,{ 155, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       27,      GT_NA,        GT_TRUE,          155 ,GT_FALSE}
    ,{ 156, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       28,      GT_NA,        GT_TRUE,          156 ,GT_FALSE}
    ,{ 157, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       29,      GT_NA,        GT_TRUE,          157 ,GT_FALSE}
    ,{ 158, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       30,      GT_NA,        GT_TRUE,          158 ,GT_FALSE}
    ,{ 159, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       31,      GT_NA,        GT_TRUE,          159 ,GT_FALSE}
    ,{ 160, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       32,      GT_NA,        GT_TRUE,          160 ,GT_FALSE}
    ,{ 161, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       33,      GT_NA,        GT_TRUE,          161 ,GT_FALSE}
    ,{ 162, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       34,      GT_NA,        GT_TRUE,          162 ,GT_FALSE}
    ,{ 163, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       35,      GT_NA,        GT_TRUE,          163 ,GT_FALSE}
    ,{ 164, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       36,      GT_NA,        GT_TRUE,          164 ,GT_FALSE}
    ,{ 165, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       37,      GT_NA,        GT_TRUE,          165 ,GT_FALSE}
    ,{ 166, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       38,      GT_NA,        GT_TRUE,          166 ,GT_FALSE}
    ,{ 167, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       39,      GT_NA,        GT_TRUE,          167 ,GT_FALSE}
    ,{ 168, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       40,      GT_NA,        GT_TRUE,          168 ,GT_FALSE}
    ,{ 169, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       41,      GT_NA,        GT_TRUE,          169 ,GT_FALSE}
    ,{ 170, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       42,      GT_NA,        GT_TRUE,          170 ,GT_FALSE}
    ,{ 171, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       43,      GT_NA,        GT_TRUE,          171 ,GT_FALSE}
    ,{ 172, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       44,      GT_NA,        GT_TRUE,          172 ,GT_FALSE}
    ,{ 173, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       45,      GT_NA,        GT_TRUE,          173 ,GT_FALSE}
    ,{ 174, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       46,      GT_NA,        GT_TRUE,          174 ,GT_FALSE}
    ,{ 175, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       47,      GT_NA,        GT_TRUE,          175 ,GT_FALSE}
    ,{ 176, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       48,      GT_NA,        GT_TRUE,          176 ,GT_FALSE}
    ,{ 177, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       49,      GT_NA,        GT_TRUE,          177 ,GT_FALSE}
    ,{ 178, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       50,      GT_NA,        GT_TRUE,          178 ,GT_FALSE}
    ,{ 179, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       51,      GT_NA,        GT_TRUE,          179 ,GT_FALSE}
    ,{ 180, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       52,      GT_NA,        GT_TRUE,          180 ,GT_FALSE}
    ,{ 181, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       53,      GT_NA,        GT_TRUE,          181 ,GT_FALSE}
    ,{ 182, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       54,      GT_NA,        GT_TRUE,          182 ,GT_FALSE}
    ,{ 183, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       55,      GT_NA,        GT_TRUE,          183 ,GT_FALSE}
    ,{ 184, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       56,      GT_NA,        GT_TRUE,          184 ,GT_FALSE}
    ,{ 185, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       57,      GT_NA,        GT_TRUE,          185 ,GT_FALSE}
    ,{ 186, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       58,      GT_NA,        GT_TRUE,          186 ,GT_FALSE}
    ,{ 187, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       59,      GT_NA,        GT_TRUE,          187 ,GT_FALSE}
    ,{ 188, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       60,      GT_NA,        GT_TRUE,          188 ,GT_FALSE}
    ,{ 189, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       61,      GT_NA,        GT_TRUE,          189 ,GT_FALSE}
    ,{ 190, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       62,      GT_NA,        GT_TRUE,          190 ,GT_FALSE}
    ,{ 191, CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,               0,       63,      GT_NA,        GT_TRUE,          191 ,GT_FALSE}
};

static CPSS_DXCH_PORT_MAP_STC bc2TmEnableMap_29_4_RD_board[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,     GT_NA,         GT_TRUE,            64  ,GT_FALSE}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,     GT_NA,         GT_TRUE,            65  ,GT_FALSE}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,     GT_NA,         GT_TRUE,            66  ,GT_FALSE}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,     GT_NA,         GT_TRUE,            67  ,GT_FALSE}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA,GT_FALSE}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA,GT_FALSE}
};

static CPSS_DXCH_REVID_2_PORT_MAP_STC prv_revId2PortMapList[] =
{
    {   1, &bc2defaultMap[0],                 sizeof(bc2defaultMap)/sizeof(bc2defaultMap[0])   }
   ,{   2, &bc2TmEnableMap[0],                sizeof(bc2TmEnableMap)/sizeof(bc2TmEnableMap[0]) }
   ,{   3, &bc2defaultMap[0],                 sizeof(bc2defaultMap)/sizeof(bc2defaultMap[0])   }
   ,{   4, &bc2TmEnableMap_29_4_RD_board[0],  sizeof(bc2TmEnableMap_29_4_RD_board)/sizeof(bc2TmEnableMap_29_4_RD_board[0])   }
   ,{   5, &bc2defaultMap[0],                 sizeof(bc2defaultMap)/sizeof(bc2defaultMap[0])   }
   ,{   6, &bc2TmEnableMap[0],                sizeof(bc2TmEnableMap)/sizeof(bc2TmEnableMap[0]) }
   ,{  11, &bc2defaultMap[0],                 sizeof(bc2defaultMap)/sizeof(bc2defaultMap[0])   }
   ,{  12, &bc2TmEnableMap_29_12_RD_board[0], sizeof(bc2TmEnableMap_29_12_RD_board)/sizeof(bc2TmEnableMap_29_12_RD_board[0]) }
   ,{   0, NULL,                              0                                                } /* default map*/
};


/**
* @internal configBoardAfterPhase1MappingGet function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*/
GT_STATUS configBoardAfterPhase1MappingGet
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
)
{
    GT_U32  i;
    CPSS_DXCH_REVID_2_PORT_MAP_STC * mapPtr;
    CPSS_DXCH_REVID_2_PORT_MAP_STC   *revId2PortMapListPtr;
    GT_U32                            listSize;

    revId2PortMapListPtr = &prv_revId2PortMapList[0];
    listSize = sizeof(prv_revId2PortMapList)/sizeof(prv_revId2PortMapList[0]);

    if (revId2PortMapListPtr == 0 || mappingPtrPtr == NULL || mappingSizePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if (listSize == 0)
    {
        return GT_BAD_PARAM;
    }
    /* set last element to what we search : we find it always !!!*/
    revId2PortMapListPtr[listSize-1].boardRevId = boardRevId;
    mapPtr = &revId2PortMapListPtr[0];
    /* search */
    for (i = 0 ;  mapPtr->boardRevId != boardRevId ; i++, mapPtr++);
    *mappingPtrPtr  = mapPtr->mappingPtr;
    *mappingSizePtr = mapPtr->mappingSize;

    revId2PortMapListPtr[listSize-1].boardRevId = 0; /* retore any map sign*/
    return GT_OK;
}


GT_STATUS setDefaultMapping
(
    GT_U8   devNum,
    GT_U8   boardRevId
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_MAP_STC          *mappingPtr;
    GT_U32                           mappingSize;

    rc = configBoardAfterPhase1MappingGet(boardRevId,/*OUT*/&mappingPtr,&mappingSize);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChPortPhysicalPortMapSet(devNum, mappingSize, mappingPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
        if (GT_OK != rc)
        {
                return rc;
        }

        /*rc = appDemoDxChMaxMappedPortSet(devNum, mappingSize, mappingPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
        if (GT_OK != rc)
        {
                return rc;
        }*/

    return GT_OK;
}



/*******************************************************************************
* bcat2PortInterfaceInit
*
*******************************************************************************/
/*
   default
   0-47  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_QSGMII_E

   29,1,0
   ------
   0-47  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_QSGMII_E
  48-48  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_SGMII_E
  49-51  CPSS_PORT_SPEED_10000_E  CPSS_PORT_INTERFACE_MODE_KR_E,
  56,58,60,62  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_RXAUI_E,

   29,2,0
   ------

   29,100,0
   --------
   0-47                    CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_QSGMII_E
   49-52, 57, 58, 80, 82   CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E

*/
/*
static PortInitList_STC portInitlist_29_x_48x1G_Default[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,47,1, APP_INV_PORT_CNS}, CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_EMPTY,     {        APP_INV_PORT_CNS}, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E      }
};
*/




static PortInitList_STC portInitlist_360MHz_29_1_48x1G_QXGMII_3x10G_KR_4x10G_RXAUI[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_LIST,      {48,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E   }
    ,{ PORT_LIST_TYPE_LIST,      {49,50,51,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {56,58,80,82, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   }
    ,{ PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_360MHz_29_3_mtl_rd_48x1G_QXGMII_4x10G_LSSR[] =
{

     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  },
     { PORT_LIST_TYPE_LIST,      {64,65,66,67, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   },
     { PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS },  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_360MHz_29_4_mtl_rd_4x10G_2x40G[] =
{

     { PORT_LIST_TYPE_LIST,      {64,65,66, 67,  APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   },
     { PORT_LIST_TYPE_LIST,      {59,58,57,56,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   },
     { PORT_LIST_TYPE_LIST,      {71,70,69,68,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   },
    { PORT_LIST_TYPE_EMPTY,      {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};



static PortInitList_STC portInitlist_360MHz_29_2_47x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,47,1, APP_INV_PORT_CNS}, CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_EMPTY,     {        APP_INV_PORT_CNS}, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E      }
};


/*
static PortInitList_STC portInitlist_29_100_48x1G_QXGMII_7x10G_KR[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_LIST,      {49,50,51,    APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {57,58,80,82, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS },  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};
*/


static PortInitList_STC portInitlist_360MHz_29_11_12[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
    ,{ PORT_LIST_TYPE_LIST,      { 48,49,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_LIST,      { 50,51,52,53, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E }
    ,{ PORT_LIST_TYPE_LIST,      { 56,57,58,59, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E }
    ,{ PORT_LIST_TYPE_LIST,      { 80,81,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E }
    ,{ PORT_LIST_TYPE_LIST,      { 82,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E      }
    ,{ PORT_LIST_TYPE_LIST,      { 68,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E         }
    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};


static PortInitList_STC portInitlist_175MHz_29_1_7x1G_QXGMII_5x10G_KR[] =
{
     { PORT_LIST_TYPE_LIST,      { 0,1,2,3,4,5,6,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
    ,{ PORT_LIST_TYPE_LIST,      { 48,49,50,51,52, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};

static PortInitList_STC portInitlist_175MHz_29_2_8x1G_QXGMII_4x10G_KR[] =
{
     { PORT_LIST_TYPE_LIST,      { 0,1,2,3,4,36,40,44,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
    ,{ PORT_LIST_TYPE_LIST,      { 48,49,50,51, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};


static PortInitList_STC portInitlist_250MHz_29_1_48x1G_QXGMII_3x10G_KR[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
    ,{ PORT_LIST_TYPE_LIST,      { 48,49,50,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};

/*-----------------------------------------------------------------*
 *   Celum + Cetus
 *     1.        167Mhz
 *        a.    Caelum 56Gb BW:
 *            Ports 0-4: QSGMII with PHY 88E1548P
 *            Ports 56-59,64 10G KR
 *            Port 62External CPU
 *        b.    Cetus 56Gb BW :
 *            Ports 56-59,64 10G KR
 *            Ports 65-71 1000Base-x
 *            Port 62 External CPU
 *
 *     2.        200Mhz
 *        a.     Caelum 67Gb BW:
 *            Ports 0-45: QSGMII with PHY 88E1548P     46G
 *            Ports 56-57 10G KR                     + 20G
 *            Port 62 External CPU                   + 1G
 *                                                   = 67G
 *        b.    Cetus 67Gb BW :
 *            Ports 56-59,64-65 10G KR                 4+2 = 60G
 *            Ports 66-71 1000Base-x                 + 6G
 *            Port 62 External CPU                   + 1G
 *                                                   =  67G (OK)
 *
 *     3.        250Mhz
 *        a.        Caelum 84Gb BW:
 *            Ports 0-42: QSGMII with PHY 88E1548P  = 43G
 *            Ports 56-59 = 4                       = 40G
 *            Port 62External CPU                   = 1G
 *                                                Total = 84G
 *         b.   Cetus 84Gb BW :
 *            Ports 56-59,64-67 10G KR              = 4+4 = 80G
 *            Ports 68-70 1000Base-x                = 3G
 *            Port 62 External CPU                  = 1G
 *                                                Total = 84G
 *
 *---------------------------------------------------------*/

/*-----------------------------------------*
 * BobK Caelum                             *
 *-----------------------------------------*/
static PortInitList_STC portInitlist_BobK_Caelum_167MHz_29_1_BW_56G_ports_5x10G_5x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0, 4, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,58,59,64, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E        }
};

static PortInitList_STC portInitlist_BobK_Caelum_200MHz_29_1_BW_67G_ports_2x10G_46x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,45, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E        }
};

static PortInitList_STC portInitlist_BobK_Caelum_250MHz_29_1_BW_84G_ports_4x10G_43x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,42, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,58,59,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E        }
};

static PortInitList_STC portInitlist_BobK_Caelum_365MHz_29_1_BW_168G_ports_12x10G_48x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,58,59,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,71, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E        }
};

static PortInitList_STC portInitlist_BobK_Caelum_RD_Cygnus_365MHz_29_1_BW_168G_ports_4x10G_2x40G_48x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47, 1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,58,59,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E        }
    ,{ PORT_LIST_TYPE_LIST,      {64,68,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR4_E       }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E        }
};


/*-----------------------------------------*
 * BobK Cetus                              *
 *-----------------------------------------*/
static PortInitList_STC portInitlist_BobK_Cetus_167MHz_29_1_BW_56G_ports_5x10G_5x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {65,69,1,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E }
    ,{ PORT_LIST_TYPE_LIST,      {56,57,58,59,64, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E         }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};

static PortInitList_STC portInitlist_BobK_Cetus_200MHz_29_1_BW_67G_ports_6x10G_6x1G[]  =
{
     { PORT_LIST_TYPE_LIST,      {56,57,58,59,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E         }
    ,{ PORT_LIST_TYPE_LIST,      {64,65,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E         }
    ,{ PORT_LIST_TYPE_INTERVAL,  {66,71,1,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};


static PortInitList_STC portInitlist_BobK_Cetus_250MHz_29_1_BW_84G_ports_8x10G_3x1G[] =
{
     { PORT_LIST_TYPE_LIST,      {56,57,58,59,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E          }
    ,{ PORT_LIST_TYPE_LIST,      {64,65,66,67,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E          }
    ,{ PORT_LIST_TYPE_INTERVAL,  {68,70,1,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E  }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E          }
};


static PortInitList_STC portInitlist_BobK_Cetus_365MHz_29_1_BW_121G_ports_12x10G[] =
{
     { PORT_LIST_TYPE_LIST,      {56,57,58,59,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,71, 1,      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};


/*-----------------------------------------*
 * Aldrin Z0                                 *
 *-----------------------------------------*/
static PortInitList_STC portInitlist_Aldrin_Z0_29_1_365Mhz_BW_43_3x10G_8x1G_2x2_5G[] =
{
     { PORT_LIST_TYPE_LIST,      {0, 1, 11,        APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E         }
    ,{ PORT_LIST_TYPE_LIST,      {4,5,6,7,8,9,10,  APP_INV_PORT_CNS},  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E      }
    ,{ PORT_LIST_TYPE_LIST,      {2,3,             APP_INV_PORT_CNS},  CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};
static PortInitList_STC portInitlist_Aldrin_Z0_29_1_200Mhz_BW_67_6x10G_6x1G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,   5, 1,     APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {12, 17, 1,     APP_INV_PORT_CNS},  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E   }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};


/*-----------------------------------------*
 * Aldrin                                  *
 *-----------------------------------------*/
static PortInitList_STC portInitlist_Aldrin_200MHz_29_1_BW_121Gbps_12x10G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,   3, 1,       APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {12, 15, 1,       APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {24, 27, 1,       APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_Aldrin_250MHz_29_1_BW_161Gbps_16x10G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,   7, 1,       APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {12, 19, 1,       APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_Aldrin_365MHz_29_1_BW_321Gbps_32x10G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0, 31, 1,      APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,      CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_Aldrin_480MHz_29_1_BW_321Gbps_32x10G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0, 31, 1,      APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {32,            APP_INV_PORT_CNS},  CPSS_PORT_SPEED_1000_E,    CPSS_PORT_INTERFACE_MODE_SGMII_E   }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,      CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_AldrinRD_480MHz_29_3_BW_321Gbps_32x10G[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0, 31, 1,      APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {32,            APP_INV_PORT_CNS},  CPSS_PORT_SPEED_1000_E,    CPSS_PORT_INTERFACE_MODE_SGMII_E   }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,      CPSS_PORT_INTERFACE_MODE_NA_E      }
};


static boardRevId2PortsInitList_STC prv_boardRevId2PortsInitList[]=
{
/* BC2 */
    /*   175 MHz */
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  175,   1,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_175MHz_29_1_7x1G_QXGMII_5x10G_KR[0]              }
    ,{  175,   2,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_175MHz_29_2_8x1G_QXGMII_4x10G_KR[0]              }
    ,{  175,   3,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_175MHz_29_1_7x1G_QXGMII_5x10G_KR[0]              }
    ,{  175,  11,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_175MHz_29_1_7x1G_QXGMII_5x10G_KR[0]              }
    /* 250 MHz */
    ,{  250,   1,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_250MHz_29_1_48x1G_QXGMII_3x10G_KR[0]             }
    ,{  250,   3,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_250MHz_29_1_48x1G_QXGMII_3x10G_KR[0]             }
    ,{  250,  11,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_250MHz_29_1_48x1G_QXGMII_3x10G_KR[0]             }
    /*   360 MHz */
    /* clock, revid,  port list */
    ,{  362,   1,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_1_48x1G_QXGMII_3x10G_KR_4x10G_RXAUI[0] }
    ,{  362,   2,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_2_47x1G[0]                             }
    ,{  362,   3,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_3_mtl_rd_48x1G_QXGMII_4x10G_LSSR[0]    }
    ,{  362,   4,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_4_mtl_rd_4x10G_2x40G[0]         }
    ,{  362,  11,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_11_12[0]                               }
    ,{  362,  12,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_11_12[0]                               }
    /*   520 MHz */
    /* clock, revid,  port list */
    ,{  521,   1,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_1_48x1G_QXGMII_3x10G_KR_4x10G_RXAUI[0] }
    ,{  521,   2,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_2_47x1G[0]                             }
    ,{  521,   3,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_3_mtl_rd_48x1G_QXGMII_4x10G_LSSR[0]    }
    ,{  521,   4,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_4_mtl_rd_4x10G_2x40G[0]         }
    ,{  521,  11,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_11_12[0]                               }
    ,{  521,  12,  CPSS_PP_SUB_FAMILY_NONE_E,             0x1,  &portInitlist_360MHz_29_11_12[0]                               }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,             0x0,  NULL                                                           }   /* default port init */
};



/*------------------*
 * BC2 BOBK  Caelum *
 *------------------*/
static boardRevId2PortsInitList_STC prv_bobk_caelum_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  167,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_167MHz_29_1_BW_56G_ports_5x10G_5x1G[0]  }
    ,{  200,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_200MHz_29_1_BW_67G_ports_2x10G_46x1G[0] }
    ,{  250,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_250MHz_29_1_BW_84G_ports_4x10G_43x1G[0] }
    ,{  365,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_365MHz_29_1_BW_168G_ports_12x10G_48x1G[0]}
    ,{  167,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_167MHz_29_1_BW_56G_ports_5x10G_5x1G[0]  }
    ,{  200,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_200MHz_29_1_BW_67G_ports_2x10G_46x1G[0] }
    ,{  250,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_250MHz_29_1_BW_84G_ports_4x10G_43x1G[0] }
    ,{  365,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_365MHz_29_1_BW_168G_ports_12x10G_48x1G[0]}
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,             0x0,  NULL                                                              }   /* default port init */
};

/*-------------------------------------------*
 * BC2 BOBK Caelum-Cygnus-RD board port list *
 *-------------------------------------------*/
static boardRevId2PortsInitList_STC prv_bobk_caelum_rd_cygnus_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  365,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_BobK_Caelum_RD_Cygnus_365MHz_29_1_BW_168G_ports_4x10G_2x40G_48x1G[0]}
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,             0x0,  NULL                                                              }   /* default port init */
};


/*------------------*
 * BC2 BOBK  Cetus  *
 *------------------*/
static boardRevId2PortsInitList_STC prv_bobk_cetus_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  167,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_167MHz_29_1_BW_56G_ports_5x10G_5x1G[0]   }
    ,{  200,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_200MHz_29_1_BW_67G_ports_6x10G_6x1G[0]   }
    ,{  250,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_250MHz_29_1_BW_84G_ports_8x10G_3x1G[0]   }
    ,{  365,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_365MHz_29_1_BW_121G_ports_12x10G[0]      }
    ,{  167,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_167MHz_29_1_BW_56G_ports_5x10G_5x1G[0]   }
    ,{  200,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_200MHz_29_1_BW_67G_ports_6x10G_6x1G[0]   }
    ,{  250,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_250MHz_29_1_BW_84G_ports_8x10G_3x1G[0]   }
    ,{  365,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x2,  &portInitlist_BobK_Cetus_365MHz_29_1_BW_121G_ports_12x10G[0]      }
    ,{    0,   0,                CPSS_BAD_SUB_FAMILY,     0x0,  NULL                                                              }   /* default port init */
};

/*------------------*
 * Aldrin           *
 *------------------*/
static boardRevId2PortsInitList_STC prv_aldrin_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  200,       1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x7,  &portInitlist_Aldrin_200MHz_29_1_BW_121Gbps_12x10G[0] } /* 1G to CPU */
    ,{  250,       1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x7,  &portInitlist_Aldrin_250MHz_29_1_BW_161Gbps_16x10G[0] } /* 1G to CPU */
    ,{  365,       1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x7,  &portInitlist_Aldrin_365MHz_29_1_BW_321Gbps_32x10G[0] } /* 1G to CPU */
    ,{  480,       1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x7,  &portInitlist_Aldrin_480MHz_29_1_BW_321Gbps_32x10G[0] } /* 1G to CPU */
    ,{    0,       0,  CPSS_BAD_SUB_FAMILY,                    0x0,  NULL                                                              }   /* default port init */
};

static boardRevId2PortsInitList_STC prv_aldrinRD_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  480,       3,  CPSS_PP_SUB_FAMILY_NONE_E,              0x7,  &portInitlist_AldrinRD_480MHz_29_3_BW_321Gbps_32x10G[0]      } /* 1G to CPU */
    ,{    0,       0,  CPSS_BAD_SUB_FAMILY,                    0x0,  NULL                                                              }   /* default port init */
};

/*------------------*
 * Aldrin Z0        *
 *------------------*/
static boardRevId2PortsInitList_STC prv_aldrin_Z0_boardRevId2PortsInitList[]=
{
    {  365,   1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x3,  &portInitlist_Aldrin_Z0_29_1_365Mhz_BW_43_3x10G_8x1G_2x2_5G[0] } /* 1G to CPU */
   ,{  200,   1,  CPSS_PP_SUB_FAMILY_NONE_E,              0x3,  &portInitlist_Aldrin_Z0_29_1_200Mhz_BW_67_6x10G_6x1G[0] } /* 1G to CPU */
};


/*-----------------------------------*
 *  bobcat3 72 port XG               *
 *-----------------------------------*/
/* bobcat3 72 port Giga */
static PortInitList_STC portInitlist_bc3_72xGige[] =
{                                 /* allow physical port to be 0..99 */
     { PORT_LIST_TYPE_LIST,      {0,18,36,58,54, 1, 2, 3,64,65,80,59,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {APP_INV_PORT_CNS/*filled in runtime by 256+numbers above*/}, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }


    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

#ifndef GM_USED
static PortInitList_STC portInitlist_bc3_default[] =
{
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,78,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   /* next ports supporting the '512 ports mode' */
   ,{ PORT_LIST_TYPE_LIST,      {0 +256, 4+256,8+256,12+256,16+256,20+256,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24+256,58+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64+256,78+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};

static PortInitList_STC portInitlist_armstrong_rd[] =
{
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,76,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   /* next ports supporting the '512 ports mode' */
   ,{ PORT_LIST_TYPE_LIST,      {0 +256, 4+256,8+256,12+256,16+256,20+256,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24+256,58+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64+256,76+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};


static PortInitList_STC portInitlist_bc3_rd_board_default[] =
{
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,76,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   /* next ports supporting the '512 ports mode' */
   ,{ PORT_LIST_TYPE_LIST,      {0 +256, 4+256,8+256,12+256,16+256,20+256,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24+256,58+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64+256,76+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};

static PortInitList_STC portInitlist_aldrin2_525MHz_default[] =
{
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,45,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {48,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,74,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};

static PortInitList_STC portInitlist_aldrin2_525MHz_RD[] =
{
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E    }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,76,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {77,77,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E  }
   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};

static PortInitList_STC portInitlist_aldrin2_onEmulator_default[] =
{
/* error on emulator : { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }*/
    { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,78,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   /* next ports supporting the '512 ports mode' */
   ,{ PORT_LIST_TYPE_LIST,      {0 +256, 4+256,8+256,12+256,16+256,20+256,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_INTERVAL,  {24+256,58+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {64+256,78+256,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }

   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};


#endif /*GM_USED*/

#ifdef GM_USED
static boardRevId2PortsInitList_STC bc3_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  572,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  362,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  450,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                               }   /* default port init */
};
static boardRevId2PortsInitList_STC bc3_rd_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                                                              }   /* default port init */
};
static boardRevId2PortsInitList_STC armstrong_rd_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                                                              }   /* default port init */
};

#define aldrin2_prv_boardRevId2PortsInitList               bc3_prv_boardRevId2PortsInitList
#define aldrin2_rd_prv_boardRevId2PortsInitList            bc3_rd_prv_boardRevId2PortsInitList
#define aldrin2_prv_boardRevId2PortsInitList_onEmulator    bc3_prv_boardRevId2PortsInitList

#else /*not GM_USED*/
static boardRevId2PortsInitList_STC bc3_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_default  [0] }
    ,{  572,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_default  [0] }
    ,{  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_default  [0] }
    ,{  362,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  450,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                               }   /* default port init */
};

static boardRevId2PortsInitList_STC bc3_rd_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_rd_board_default[0]}
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                                                              }   /* default port init */
};

static boardRevId2PortsInitList_STC armstrong_rd_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_armstrong_rd[0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                                                              }   /* default port init */
};

static boardRevId2PortsInitList_STC aldrin2_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_bc3_default  [0] }
    ,{  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_aldrin2_525MHz_default [0] }
    ,{  450,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                               }   /* default port init */
};

static boardRevId2PortsInitList_STC aldrin2_rd_prv_boardRevId2PortsInitList[]=
{
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_bc3_default  [0] }
    ,{  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_aldrin2_525MHz_RD [0] }
    ,{  450,   1,  CPSS_PP_SUB_FAMILY_NONE_E,         0,      &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                               }   /* default port init */
};

/* The Aldrin2 Emulator do problems with 100G ports  ... so don't use it */
static boardRevId2PortsInitList_STC aldrin2_prv_boardRevId2PortsInitList_onEmulator[]=
{
    /* clock, revid,               subfamily,          pipeBmp, port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_aldrin2_onEmulator_default  [0] }
    ,{  572,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_aldrin2_onEmulator_default  [0] }
    ,{  600,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_aldrin2_onEmulator_default  [0] }
    ,{  362,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{  450,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_72xGige  [0] }
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                               }   /* default port init */
};
#endif /*not GM_USED*/


#define PORTS_SPEED_LIST_MAC(/*IN*/inPortList,/*OUT*/outPortList,/*OUT*/outListSize) \
    outPortList = &inPortList[0];                                                    \
    outListSize = sizeof(inPortList)/sizeof(inPortList[0])


boardRevId2PortsInitList_STC *appDemo_boardRevId2PortsInitListPtr=NULL;
/* number of elements in appDemo_boardRevId2PortsInitListPtr */
GT_U32                  appDemo_boardRevId2PortsSize = 0;

/**
* @internal bcat2PortInterfaceInitPortInitListGet function
* @endinternal
*
* @brief   Execute predefined ports configuration.
*
* @param[in] boardRevId               - board revision ID
*                                      devFamily - device family
* @param[in] boardType                - board type DB/RD
* @param[in] coreClock                - core clock
*
* @param[out] portInitListPtrPtr       =  (pointer to) port init list
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bcat2PortInterfaceInitPortInitListGet
(
    IN  GT_U8                             devNum,
    IN  GT_U8                             boardRevId,
    IN  GT_U32                            boardType,
    IN  GT_U32                            coreClock,
    OUT PortInitList_STC                **portInitListPtrPtr
)
{
    GT_U32  i;
    boardRevId2PortsInitList_STC  *revId2PortInitListPtr = NULL;
    GT_U32                         listSize = 0;
    boardRevId2PortsInitList_STC  *revId2portInitListPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    switch(pDev->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch(pDev->genInfo.devSubFamily)
            {
                case CPSS_PP_SUB_FAMILY_NONE_E:
                    PORTS_SPEED_LIST_MAC(prv_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,/*OUT*/listSize);
                    break;

                case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
                    switch (pDev->genInfo.devType)
                    {
                        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                            PORTS_SPEED_LIST_MAC(prv_bobk_cetus_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
                            break;

                        case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:

                            if(boardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS)
                            {
                                PORTS_SPEED_LIST_MAC(prv_bobk_caelum_rd_cygnus_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
                            }


                            else if(pDev->genInfo.devType == CPSS_98DX8332_Z0_CNS)
                            {
                                PORTS_SPEED_LIST_MAC(prv_aldrin_Z0_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
                            }

                            else
                            {
                                PORTS_SPEED_LIST_MAC(prv_bobk_caelum_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
                            }

                            break;

                        default:
                        {
                            return GT_NOT_SUPPORTED;
                        }
                    }
                    break;

                default:
                {
                    return GT_NOT_SUPPORTED;
                }
            }
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            if(portInitlist_bc3_72xGige[1].entryType == PORT_LIST_TYPE_LIST &&
               portInitlist_bc3_72xGige[1].portList[0] == APP_INV_PORT_CNS)
            {
                for(i = 0 ; portInitlist_bc3_72xGige[0].portList[i] != APP_INV_PORT_CNS ; i++)
                {
                    /* support port more than 256 */
                    portInitlist_bc3_72xGige[1].portList[i] =
                        portInitlist_bc3_72xGige[0].portList[i] + 256;
                }
                /* must be last */
                portInitlist_bc3_72xGige[1].portList[i] = APP_INV_PORT_CNS;
            }

            PORTS_SPEED_LIST_MAC(bc3_prv_boardRevId2PortsInitList,revId2PortInitListPtr,listSize);

            if(boardType == APP_DEMO_BOBCAT3_BOARD_RD_CNS)
            {
                PORTS_SPEED_LIST_MAC(bc3_rd_prv_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
            }
            if(boardType == APP_DEMO_ARMSTRONG_BOARD_RD_CNS)
            {
                PORTS_SPEED_LIST_MAC(armstrong_rd_prv_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
            }

            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            if(cpssDeviceRunCheck_onEmulator())
            {
                PORTS_SPEED_LIST_MAC(aldrin2_prv_boardRevId2PortsInitList_onEmulator,revId2PortInitListPtr,listSize);
            }
            else
            {
                if(boardType == APP_DEMO_ALDRIN2_BOARD_RD_CNS)
                {
                    PORTS_SPEED_LIST_MAC(aldrin2_rd_prv_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
                }
                if(boardType == APP_DEMO_ALDRIN2_BOARD_DB_CNS)
                {
                    PORTS_SPEED_LIST_MAC(aldrin2_prv_boardRevId2PortsInitList,revId2PortInitListPtr,listSize);
                }
            }

            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            if(boardType == APP_DEMO_ALDRIN_BOARD_RD_CNS)
            {
                PORTS_SPEED_LIST_MAC(prv_aldrinRD_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
            }
            else
            {
                PORTS_SPEED_LIST_MAC(prv_aldrin_boardRevId2PortsInitList,/*OUT*/revId2PortInitListPtr,listSize);
            }
            break;

        default:
        {
            return GT_NOT_SUPPORTED;
        }
    }

    if(appDemo_boardRevId2PortsInitListPtr && appDemo_boardRevId2PortsSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        revId2PortInitListPtr = appDemo_boardRevId2PortsInitListPtr;
        listSize              = appDemo_boardRevId2PortsSize;
    }

    /*
        revId2PortInitListPtr = &prv_boardRevId2PortsInitList[0];
        listSize              = sizeof(prv_boardRevId2PortsInitList)/sizeof(prv_boardRevId2PortsInitList[0]);
    */

    if ((revId2PortInitListPtr == NULL) || (portInitListPtrPtr == NULL))
    {
        return GT_BAD_PTR;
    }
    if (listSize == 0)
    {
        return GT_BAD_PARAM;
    }
    /* set last element to what we search : we find it always !!!*/
    revId2PortInitListPtr[listSize-1].coreClock  = coreClock;
    revId2PortInitListPtr[listSize-1].boardRevId = boardRevId;
    revId2portInitListPtr = &revId2PortInitListPtr[0];
    /* search */
    for (i = 0 ;  ; i++, revId2portInitListPtr++)
    {
        if (revId2portInitListPtr->coreClock == coreClock && revId2portInitListPtr->boardRevId == boardRevId)
        {
            break;
        }
    }
    *portInitListPtrPtr  = revId2portInitListPtr->portsInitListPtr;

    revId2PortInitListPtr[listSize-1].coreClock  = 0; /* restore default value */
    revId2PortInitListPtr[listSize-1].boardRevId = 0;

    return GT_OK;
}



