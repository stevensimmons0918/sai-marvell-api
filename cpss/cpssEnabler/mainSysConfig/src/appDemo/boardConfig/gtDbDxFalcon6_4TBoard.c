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
* @file gtDbDxFalcon6_4TBoard.c
*
* @brief Initialization functions for the Falcon - SIP6 - 6.4T TH board.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/gtDbDxFalcon.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>

#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0 {{7,1,6,0,5,3,4,2}}
#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1 {{0,6,1,7,2,4,3,5}}


/* Muxing settings are the same for all ports */
CPSS_PORT_MAC_TO_SERDES_STC  falcon_6_4T_MacToSerdesMap[FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE] =
{
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /* 0-7*/    /*Raven 0 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /* 8-15*/   /*Raven 0 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*16-23*/   /*Raven 1 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*24-31*/   /*Raven 1 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*32-39*/   /*Raven 2 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*40-47*/   /*Raven 2 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*48-55*/   /*Raven 3 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*56-63*/   /*Raven 3 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*64-71*/   /*Raven 4 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*72-79*/   /*Raven 4 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*80-87*/   /*Raven 5 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*88-95*/   /*Raven 5 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*96-103*/  /*Raven 6 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*104-111*/ /*Raven 6 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*112-119*/ /*Raven 7 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1             /*120-127*/ /*Raven 7 */
};

APPDEMO_SERDES_LANE_POLARITY_STC  falcon_6_4T_PolarityArray[FALCON_6_4T_POLARITY_ARR_SIZE] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_TRUE ,   GT_FALSE  },
    { 1,    GT_TRUE ,   GT_FALSE  },
    { 2,    GT_FALSE,   GT_FALSE  },
    { 3,    GT_TRUE ,   GT_TRUE   },
    { 4,    GT_TRUE ,   GT_TRUE   },
    { 5,    GT_TRUE ,   GT_FALSE  },
    { 6,    GT_TRUE ,   GT_TRUE   },
    { 7,    GT_TRUE ,   GT_FALSE  },
    { 8,    GT_FALSE,   GT_TRUE   },
    { 9,    GT_FALSE,   GT_FALSE  },
    { 10,   GT_FALSE,   GT_TRUE   },
    { 11,   GT_TRUE ,   GT_FALSE  },
    { 12,   GT_FALSE,   GT_TRUE   },
    { 13,   GT_FALSE,   GT_TRUE   },
    { 14,   GT_FALSE,   GT_FALSE  },
    { 15,   GT_FALSE,   GT_TRUE   },
    { 16,   GT_FALSE,   GT_TRUE   },
    { 17,   GT_FALSE,   GT_FALSE  },
    { 18,   GT_FALSE,   GT_TRUE   },
    { 19,   GT_TRUE ,   GT_TRUE   },
    { 20,   GT_TRUE ,   GT_FALSE  },
    { 21,   GT_FALSE,   GT_FALSE  },
    { 22,   GT_TRUE ,   GT_FALSE  },
    { 23,   GT_FALSE,   GT_FALSE  },
    { 24,   GT_FALSE,   GT_FALSE  },
    { 25,   GT_FALSE,   GT_TRUE   },
    { 26,   GT_TRUE ,   GT_FALSE  },
    { 27,   GT_FALSE,   GT_TRUE   },
    { 28,   GT_FALSE,   GT_TRUE   },
    { 29,   GT_FALSE,   GT_TRUE   },
    { 30,   GT_FALSE,   GT_FALSE  },
    { 31,   GT_FALSE,   GT_FALSE  },
    { 32,   GT_FALSE,   GT_FALSE  },
    { 33,   GT_TRUE ,   GT_FALSE  },
    { 34,   GT_FALSE,   GT_FALSE  },
    { 35,   GT_TRUE ,   GT_TRUE   },
    { 36,   GT_TRUE ,   GT_FALSE  },
    { 37,   GT_TRUE ,   GT_FALSE  },
    { 38,   GT_FALSE,   GT_FALSE  },
    { 39,   GT_FALSE,   GT_FALSE  },
    { 40,   GT_FALSE,   GT_FALSE  },
    { 41,   GT_TRUE ,   GT_TRUE   },
    { 42,   GT_TRUE ,   GT_FALSE  },
    { 43,   GT_FALSE,   GT_TRUE   },
    { 44,   GT_FALSE,   GT_FALSE  },
    { 45,   GT_TRUE ,   GT_FALSE  },
    { 46,   GT_TRUE ,   GT_TRUE   },
    { 47,   GT_TRUE ,   GT_TRUE   },
    { 48,   GT_FALSE,   GT_TRUE   },
    { 49,   GT_TRUE ,   GT_FALSE  },
    { 50,   GT_FALSE,   GT_TRUE   },
    { 51,   GT_FALSE,   GT_TRUE   },
    { 52,   GT_FALSE,   GT_TRUE   },
    { 53,   GT_TRUE ,   GT_FALSE  },
    { 54,   GT_TRUE ,   GT_TRUE   },
    { 55,   GT_FALSE,   GT_FALSE  },
    { 56,   GT_FALSE,   GT_FALSE  },
    { 57,   GT_FALSE,   GT_TRUE   },
    { 58,   GT_FALSE,   GT_FALSE  },
    { 59,   GT_FALSE,   GT_TRUE   },
    { 60,   GT_FALSE,   GT_TRUE   },
    { 61,   GT_FALSE,   GT_TRUE   },
    { 62,   GT_TRUE ,   GT_FALSE  },
    { 63,   GT_TRUE ,   GT_TRUE   },
    { 64,   GT_TRUE ,   GT_TRUE   },
    { 65,   GT_TRUE ,   GT_FALSE  },
    { 66,   GT_TRUE ,   GT_TRUE   },
    { 67,   GT_TRUE ,   GT_TRUE   },
    { 68,   GT_TRUE ,   GT_TRUE   },
    { 69,   GT_TRUE ,   GT_FALSE  },
    { 70,   GT_FALSE,   GT_TRUE   },
    { 71,   GT_FALSE,   GT_FALSE  },
    { 72,   GT_TRUE ,   GT_FALSE  },
    { 73,   GT_FALSE,   GT_TRUE   },
    { 74,   GT_TRUE ,   GT_FALSE  },
    { 75,   GT_TRUE ,   GT_TRUE   },
    { 76,   GT_TRUE ,   GT_TRUE   },
    { 77,   GT_FALSE,   GT_TRUE   },
    { 78,   GT_FALSE,   GT_FALSE  },
    { 79,   GT_TRUE ,   GT_TRUE   },
    { 80,   GT_TRUE ,   GT_FALSE  },
    { 81,   GT_FALSE,   GT_FALSE  },
    { 82,   GT_FALSE,   GT_FALSE  },
    { 83,   GT_TRUE ,   GT_TRUE   },
    { 84,   GT_TRUE ,   GT_FALSE  },
    { 85,   GT_FALSE,   GT_TRUE   },
    { 86,   GT_FALSE,   GT_FALSE  },
    { 87,   GT_FALSE,   GT_FALSE  },
    { 88,   GT_TRUE ,   GT_FALSE  },
    { 89,   GT_FALSE,   GT_FALSE  },
    { 90,   GT_TRUE ,   GT_FALSE  },
    { 91,   GT_FALSE,   GT_FALSE  },
    { 92,   GT_FALSE,   GT_TRUE   },
    { 93,   GT_FALSE,   GT_FALSE  },
    { 94,   GT_TRUE ,   GT_FALSE  },
    { 95,   GT_TRUE ,   GT_FALSE  },
    { 96,   GT_TRUE ,   GT_TRUE   },
    { 97,   GT_TRUE ,   GT_FALSE  },
    { 98,   GT_FALSE,   GT_TRUE   },
    { 99,   GT_TRUE ,   GT_TRUE   },
    { 100,  GT_TRUE ,   GT_TRUE   },
    { 101,  GT_TRUE ,   GT_FALSE  },
    { 102,  GT_TRUE ,   GT_FALSE  },
    { 103,  GT_TRUE ,   GT_FALSE  },
    { 104,  GT_TRUE ,   GT_FALSE  },
    { 105,  GT_TRUE ,   GT_TRUE   },
    { 106,  GT_TRUE ,   GT_FALSE  },
    { 107,  GT_FALSE,   GT_TRUE   },
    { 108,  GT_FALSE,   GT_FALSE  },
    { 109,  GT_TRUE ,   GT_FALSE  },
    { 110,  GT_FALSE,   GT_FALSE  },
    { 111,  GT_TRUE ,   GT_FALSE  },
    { 112,  GT_TRUE ,   GT_FALSE  },
    { 113,  GT_TRUE ,   GT_TRUE   },
    { 114,  GT_FALSE,   GT_FALSE  },
    { 115,  GT_FALSE,   GT_FALSE  },
    { 116,  GT_TRUE ,   GT_FALSE  },
    { 117,  GT_TRUE ,   GT_FALSE  },
    { 118,  GT_TRUE ,   GT_TRUE   },
    { 119,  GT_TRUE ,   GT_FALSE  },
    { 120,  GT_FALSE,   GT_FALSE  },
    { 121,  GT_FALSE,   GT_TRUE   },
    { 122,  GT_TRUE,    GT_FALSE  },
    { 123,  GT_FALSE,   GT_TRUE   },
    { 124,  GT_FALSE,   GT_TRUE   },
    { 125,  GT_FALSE,   GT_TRUE   },
    { 126,  GT_FALSE,   GT_FALSE  },
    { 127,  GT_FALSE,   GT_TRUE   },
    { 128,  GT_FALSE,   GT_TRUE   },  /* CPU0 */
    { 135,  GT_TRUE ,   GT_FALSE  },  /* CPU1 */
    { 131,  GT_FALSE,   GT_TRUE   },  /* CPU2 */
    { 132,  GT_TRUE ,   GT_TRUE   }   /* CPU3 */
};
