/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file PP-98EX5520.c
*
* @brief PP-98EX5520 Packet Processor information.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

/* List of Tx queues to work in generator mode */
#ifdef ASIC_SIMULATION
static CPSS_APP_PLATFORM_SDMA_QUEUE_STC txGenQueues[] = { {0,3,1,144}, {0,6,1,144}, {15,6,1,3032}, {15,7,1,144} };
#else
static CPSS_APP_PLATFORM_SDMA_QUEUE_STC txGenQueues[] = { {0,5,1,3032}, {0,6,1,144} };
#endif

CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC falcon_Belly2Belly_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_TRUE ,   GT_FALSE },
    { 1,    GT_FALSE,   GT_FALSE },
    { 2,    GT_FALSE,   GT_TRUE  },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_FALSE },
    { 5,    GT_FALSE,   GT_FALSE },
    { 6,    GT_FALSE,   GT_FALSE },
    { 7,    GT_FALSE,   GT_FALSE },
    { 8,    GT_FALSE,   GT_FALSE },
    { 9,    GT_FALSE,   GT_FALSE },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_FALSE,   GT_TRUE  },
    { 12,   GT_FALSE,   GT_FALSE },
    { 13,   GT_FALSE,   GT_FALSE },
    { 14,   GT_FALSE,   GT_FALSE },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_FALSE,   GT_FALSE },
    { 17,   GT_FALSE,   GT_FALSE },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_FALSE,   GT_FALSE },
    { 21,   GT_FALSE,   GT_FALSE },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_FALSE },
    { 26,   GT_FALSE,   GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_FALSE,   GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_FALSE },
    { 33,   GT_FALSE,   GT_FALSE },
    { 34,   GT_FALSE,   GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_FALSE,   GT_TRUE  },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_TRUE  },
    { 42,   GT_FALSE,   GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_FALSE,   GT_FALSE },
    { 45,   GT_FALSE,   GT_FALSE },
    { 46,   GT_FALSE,   GT_FALSE },
    { 47,   GT_FALSE,   GT_FALSE },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_FALSE,   GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_FALSE,   GT_FALSE },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_FALSE,   GT_FALSE },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_FALSE,   GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_FALSE,   GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_FALSE,   GT_FALSE },
    { 64,   GT_FALSE,   GT_TRUE  },
    { 65,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 67,   GT_FALSE,   GT_FALSE },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_FALSE,   GT_TRUE  },
    { 71,   GT_FALSE,   GT_FALSE },
    { 72,   GT_FALSE,   GT_FALSE },
    { 73,   GT_FALSE,   GT_FALSE },
    { 74,   GT_FALSE,   GT_TRUE  },
    { 75,   GT_FALSE,   GT_TRUE  },
    { 76,   GT_FALSE,   GT_TRUE  },
    { 77,   GT_FALSE,   GT_TRUE  },
    { 78,   GT_FALSE,   GT_TRUE  },
    { 79,   GT_FALSE,   GT_TRUE  },
    { 80,   GT_FALSE,   GT_TRUE  },
    { 81,   GT_FALSE,   GT_TRUE  },
    { 82,   GT_FALSE,   GT_FALSE },
    { 83,   GT_FALSE,   GT_FALSE },
    { 84,   GT_FALSE,   GT_FALSE },
    { 85,   GT_FALSE,   GT_TRUE  },
    { 86,   GT_FALSE,   GT_FALSE },
    { 87,   GT_FALSE,   GT_TRUE  },
    { 88,   GT_FALSE,   GT_FALSE },
    { 89,   GT_FALSE,   GT_FALSE },
    { 90,   GT_FALSE,   GT_TRUE  },
    { 91,   GT_FALSE,   GT_TRUE  },
    { 92,   GT_FALSE,   GT_TRUE  },
    { 93,   GT_FALSE,   GT_TRUE  },
    { 94,   GT_FALSE,   GT_FALSE },
    { 95,   GT_FALSE,   GT_FALSE },
    { 96,   GT_FALSE,   GT_TRUE  },
    { 97,   GT_FALSE,   GT_TRUE  },
    { 98,   GT_FALSE,   GT_TRUE  },
    { 99,   GT_FALSE,   GT_TRUE  },
    { 100,  GT_FALSE,   GT_TRUE  },
    { 101,  GT_FALSE,   GT_FALSE },
    { 102,  GT_FALSE,   GT_TRUE  },
    { 103,  GT_FALSE,   GT_TRUE  },
    { 104,  GT_FALSE,   GT_TRUE  },
    { 105,  GT_FALSE,   GT_FALSE },
    { 106,  GT_FALSE,   GT_FALSE },
    { 107,  GT_FALSE,   GT_TRUE  },
    { 108,  GT_FALSE,   GT_TRUE  },
    { 109,  GT_FALSE,   GT_TRUE  },
    { 110,  GT_FALSE,   GT_TRUE  },
    { 111,  GT_FALSE,   GT_TRUE  },
    { 112,  GT_FALSE,   GT_TRUE  },
    { 113,  GT_FALSE,   GT_TRUE  },
    { 114,  GT_FALSE,   GT_TRUE  },
    { 115,  GT_FALSE,   GT_TRUE  },
    { 116,  GT_FALSE,   GT_TRUE  },
    { 117,  GT_FALSE,   GT_TRUE  },
    { 118,  GT_FALSE,   GT_TRUE  },
    { 119,  GT_FALSE,   GT_TRUE  },
    { 120,  GT_FALSE,   GT_TRUE  },
    { 121,  GT_FALSE,   GT_TRUE  },
    { 122,  GT_FALSE,   GT_TRUE  },
    { 123,  GT_FALSE,   GT_TRUE  },
    { 124,  GT_FALSE,   GT_TRUE  },
    { 125,  GT_FALSE,   GT_FALSE },
    { 126,  GT_FALSE,   GT_TRUE  },
    { 127,  GT_FALSE,   GT_FALSE },
    { 128,  GT_FALSE,   GT_TRUE  },
    { 129,  GT_FALSE,   GT_TRUE  },
    { 130,  GT_FALSE,   GT_TRUE  },
    { 131,  GT_FALSE,   GT_FALSE },
    { 132,  GT_FALSE,   GT_TRUE  },
    { 133,  GT_FALSE,   GT_TRUE  },
    { 134,  GT_FALSE,   GT_TRUE  },
    { 135,  GT_FALSE,   GT_FALSE },
    { 136,  GT_FALSE,   GT_TRUE  },
    { 137,  GT_FALSE,   GT_TRUE  },
    { 138,  GT_FALSE,   GT_TRUE  },
    { 139,  GT_FALSE,   GT_TRUE  },
    { 140,  GT_FALSE,   GT_FALSE },
    { 141,  GT_FALSE,   GT_TRUE  },
    { 142,  GT_FALSE,   GT_TRUE  },
    { 143,  GT_FALSE,   GT_TRUE  },
    { 144,  GT_FALSE,   GT_TRUE  },
    { 145,  GT_FALSE,   GT_TRUE  },
    { 146,  GT_FALSE,   GT_FALSE },
    { 147,  GT_FALSE,   GT_TRUE  },
    { 148,  GT_FALSE,   GT_TRUE  },
    { 149,  GT_FALSE,   GT_TRUE  },
    { 150,  GT_FALSE,   GT_FALSE },
    { 151,  GT_FALSE,   GT_TRUE  },
    { 152,  GT_FALSE,   GT_FALSE },
    { 153,  GT_FALSE,   GT_FALSE },
    { 154,  GT_FALSE,   GT_TRUE  },
    { 155,  GT_FALSE,   GT_TRUE  },
    { 156,  GT_FALSE,   GT_TRUE  },
    { 157,  GT_FALSE,   GT_TRUE  },
    { 158,  GT_FALSE,   GT_FALSE },
    { 159,  GT_FALSE,   GT_FALSE },
    { 160,  GT_FALSE,   GT_TRUE  },
    { 161,  GT_FALSE,   GT_TRUE  },
    { 162,  GT_FALSE,   GT_TRUE  },
    { 163,  GT_FALSE,   GT_TRUE  },
    { 164,  GT_FALSE,   GT_TRUE  },
    { 165,  GT_FALSE,   GT_FALSE },
    { 166,  GT_FALSE,   GT_TRUE  },
    { 167,  GT_FALSE,   GT_TRUE  },
    { 168,  GT_FALSE,   GT_TRUE  },
    { 169,  GT_FALSE,   GT_FALSE },
    { 170,  GT_FALSE,   GT_FALSE },
    { 171,  GT_FALSE,   GT_TRUE  },
    { 172,  GT_FALSE,   GT_TRUE  },
    { 173,  GT_FALSE,   GT_TRUE  },
    { 174,  GT_FALSE,   GT_TRUE  },
    { 175,  GT_FALSE,   GT_TRUE  },
    { 176,  GT_FALSE,   GT_TRUE  },
    { 177,  GT_FALSE,   GT_TRUE  },
    { 178,  GT_FALSE,   GT_TRUE  },
    { 179,  GT_FALSE,   GT_TRUE  },
    { 180,  GT_FALSE,   GT_TRUE  },
    { 181,  GT_FALSE,   GT_TRUE  },
    { 182,  GT_FALSE,   GT_TRUE  },
    { 183,  GT_FALSE,   GT_TRUE  },
    { 184,  GT_FALSE,   GT_TRUE  },
    { 185,  GT_FALSE,   GT_TRUE  },
    { 186,  GT_FALSE,   GT_TRUE  },
    { 187,  GT_FALSE,   GT_TRUE  },
    { 188,  GT_FALSE,   GT_TRUE  },
    { 189,  GT_FALSE,   GT_TRUE  },
    { 190,  GT_FALSE,   GT_TRUE  },
    { 191,  GT_FALSE,   GT_FALSE },
    { 192,  GT_FALSE,   GT_FALSE },
    { 193,  GT_FALSE,   GT_FALSE },
    { 194,  GT_FALSE,   GT_TRUE  },
    { 195,  GT_FALSE,   GT_FALSE },
    { 196,  GT_FALSE,   GT_FALSE },
    { 197,  GT_FALSE,   GT_FALSE },
    { 198,  GT_FALSE,   GT_FALSE },
    { 199,  GT_FALSE,   GT_FALSE },
    { 200,  GT_TRUE,    GT_TRUE  },
    { 201,  GT_FALSE,   GT_TRUE  },
    { 202,  GT_FALSE,   GT_FALSE },
    { 203,  GT_FALSE,   GT_TRUE  },
    { 204,  GT_FALSE,   GT_FALSE },
    { 205,  GT_FALSE,   GT_FALSE },
    { 206,  GT_FALSE,   GT_FALSE },
    { 207,  GT_FALSE,   GT_FALSE },
    { 208,  GT_FALSE,   GT_FALSE },
    { 209,  GT_FALSE,   GT_TRUE  },
    { 210,  GT_FALSE,   GT_FALSE },
    { 211,  GT_FALSE,   GT_FALSE },
    { 212,  GT_FALSE,   GT_FALSE },
    { 213,  GT_FALSE,   GT_FALSE },
    { 214,  GT_FALSE,   GT_FALSE },
    { 215,  GT_FALSE,   GT_TRUE  },
    { 216,  GT_FALSE,   GT_TRUE  },
    { 217,  GT_FALSE,   GT_TRUE  },
    { 218,  GT_FALSE,   GT_TRUE  },
    { 219,  GT_FALSE,   GT_FALSE },
    { 220,  GT_FALSE,   GT_TRUE  },
    { 221,  GT_FALSE,   GT_FALSE },
    { 222,  GT_FALSE,   GT_FALSE },
    { 223,  GT_FALSE,   GT_FALSE },
    { 224,  GT_FALSE,   GT_TRUE  },
    { 225,  GT_FALSE,   GT_TRUE  },
    { 226,  GT_FALSE,   GT_FALSE },
    { 227,  GT_FALSE,   GT_TRUE  },
    { 228,  GT_FALSE,   GT_TRUE  },
    { 229,  GT_FALSE,   GT_FALSE },
    { 230,  GT_FALSE,   GT_FALSE },
    { 231,  GT_FALSE,   GT_TRUE  },
    { 232,  GT_FALSE,   GT_FALSE },
    { 233,  GT_FALSE,   GT_TRUE  },
    { 234,  GT_FALSE,   GT_TRUE  },
    { 235,  GT_FALSE,   GT_FALSE },
    { 236,  GT_FALSE,   GT_TRUE  },
    { 237,  GT_FALSE,   GT_FALSE },
    { 238,  GT_FALSE,   GT_FALSE },
    { 239,  GT_FALSE,   GT_FALSE },
    { 240,  GT_FALSE,   GT_FALSE },
    { 241,  GT_FALSE,   GT_FALSE },
    { 242,  GT_FALSE,   GT_TRUE  },
    { 243,  GT_FALSE,   GT_TRUE  },
    { 244,  GT_FALSE,   GT_FALSE },
    { 245,  GT_FALSE,   GT_FALSE },
    { 246,  GT_FALSE,   GT_FALSE },
    { 247,  GT_FALSE,   GT_FALSE },
    { 248,  GT_FALSE,   GT_FALSE },
    { 249,  GT_FALSE,   GT_FALSE },
    { 250,  GT_FALSE,   GT_FALSE },
    { 251,  GT_FALSE,   GT_TRUE  },
    { 252,  GT_FALSE,   GT_FALSE },
    { 253,  GT_FALSE,   GT_TRUE  },
    { 254,  GT_FALSE,   GT_FALSE },
    { 255,  GT_FALSE,   GT_FALSE },
    { 257,  GT_FALSE,   GT_FALSE },
    { 258,  GT_FALSE,   GT_FALSE }
};

static CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC falcon_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_TRUE  },
    { 1,    GT_FALSE,   GT_FALSE },
    { 2,    GT_FALSE,   GT_FALSE },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_FALSE },
    { 5,    GT_FALSE,   GT_FALSE },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_FALSE,   GT_FALSE },
    { 8,    GT_FALSE,   GT_FALSE },
    { 9,    GT_TRUE,    GT_FALSE },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_FALSE,   GT_FALSE },
    { 12,   GT_FALSE,   GT_TRUE  },
    { 13,   GT_FALSE,   GT_FALSE },
    { 14,   GT_FALSE,   GT_FALSE },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_FALSE,   GT_TRUE  },
    { 17,   GT_FALSE,   GT_TRUE  },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_FALSE,   GT_FALSE },
    { 21,   GT_FALSE,   GT_FALSE },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_FALSE,   GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_FALSE,   GT_FALSE },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_FALSE,   GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_FALSE,   GT_FALSE },
    { 34,   GT_FALSE,   GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_FALSE,   GT_FALSE },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_FALSE },
    { 42,   GT_FALSE,   GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_FALSE,   GT_FALSE },
    { 45,   GT_FALSE,   GT_FALSE },
    { 46,   GT_FALSE,   GT_FALSE },
    { 47,   GT_FALSE,   GT_TRUE  },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_FALSE,   GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_FALSE,   GT_FALSE },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_FALSE,   GT_FALSE },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_FALSE,   GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_FALSE,   GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_FALSE,   GT_FALSE },
    { 64,   GT_FALSE,   GT_FALSE },
    { 65,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 67,   GT_FALSE,   GT_TRUE  },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_FALSE,   GT_FALSE },
    { 71,   GT_FALSE,   GT_TRUE  },
    { 72,   GT_FALSE,   GT_TRUE  },
    { 73,   GT_FALSE,   GT_FALSE },
    { 74,   GT_FALSE,   GT_TRUE  },
    { 75,   GT_FALSE,   GT_FALSE },
    { 76,   GT_FALSE,   GT_TRUE  },
    { 77,   GT_FALSE,   GT_TRUE  },
    { 78,   GT_FALSE,   GT_TRUE  },
    { 79,   GT_FALSE,   GT_TRUE  },
    { 80,   GT_FALSE,   GT_TRUE  },
    { 81,   GT_FALSE,   GT_TRUE  },
    { 82,   GT_FALSE,   GT_FALSE },
    { 83,   GT_FALSE,   GT_FALSE },
    { 84,   GT_FALSE,   GT_TRUE  },
    { 85,   GT_FALSE,   GT_FALSE },
    { 86,   GT_FALSE,   GT_FALSE },
    { 87,   GT_FALSE,   GT_TRUE  },
    { 88,   GT_FALSE,   GT_TRUE  },
    { 89,   GT_FALSE,   GT_FALSE },
    { 90,   GT_FALSE,   GT_TRUE  },
    { 91,   GT_FALSE,   GT_FALSE },
    { 92,   GT_FALSE,   GT_TRUE  },
    { 93,   GT_FALSE,   GT_FALSE },
    { 94,   GT_FALSE,   GT_TRUE  },
    { 95,   GT_FALSE,   GT_FALSE },
    { 96,   GT_FALSE,   GT_TRUE  },
    { 97,   GT_FALSE,   GT_TRUE  },
    { 98,   GT_FALSE,   GT_TRUE  },
    { 99,   GT_FALSE,   GT_TRUE  },
    { 100,  GT_FALSE,   GT_TRUE  },
    { 101,  GT_FALSE,   GT_TRUE  },
    { 102,  GT_FALSE,   GT_FALSE },
    { 103,  GT_FALSE,   GT_TRUE  },
    { 104,  GT_FALSE,   GT_TRUE  },
    { 105,  GT_FALSE,   GT_FALSE },
    { 106,  GT_FALSE,   GT_TRUE  },
    { 107,  GT_FALSE,   GT_TRUE  },
    { 108,  GT_FALSE,   GT_TRUE  },
    { 109,  GT_FALSE,   GT_TRUE  },
    { 110,  GT_FALSE,   GT_TRUE  },
    { 111,  GT_FALSE,   GT_FALSE },
    { 112,  GT_FALSE,   GT_TRUE  },
    { 113,  GT_FALSE,   GT_TRUE  },
    { 114,  GT_FALSE,   GT_TRUE  },
    { 115,  GT_FALSE,   GT_TRUE  },
    { 116,  GT_FALSE,   GT_TRUE  },
    { 117,  GT_FALSE,   GT_TRUE  },
    { 118,  GT_FALSE,   GT_TRUE  },
    { 119,  GT_FALSE,   GT_TRUE  },
    { 120,  GT_FALSE,   GT_TRUE  },
    { 121,  GT_FALSE,   GT_TRUE  },
    { 122,  GT_FALSE,   GT_TRUE  },
    { 123,  GT_FALSE,   GT_TRUE  },
    { 124,  GT_FALSE,   GT_TRUE  },
    { 125,  GT_FALSE,   GT_FALSE },
    { 126,  GT_FALSE,   GT_TRUE  },
    { 127,  GT_FALSE,   GT_FALSE },
    { 128,  GT_FALSE,   GT_FALSE },
    { 129,  GT_FALSE,   GT_TRUE  },
    { 130,  GT_FALSE,   GT_TRUE  },
    { 131,  GT_FALSE,   GT_TRUE  },
    { 132,  GT_FALSE,   GT_TRUE  },
    { 133,  GT_FALSE,   GT_TRUE  },
    { 134,  GT_FALSE,   GT_TRUE  },
    { 135,  GT_FALSE,   GT_FALSE },
    { 136,  GT_FALSE,   GT_TRUE  },
    { 137,  GT_FALSE,   GT_TRUE  },
    { 138,  GT_FALSE,   GT_FALSE },
    { 139,  GT_FALSE,   GT_TRUE  },
    { 140,  GT_FALSE,   GT_TRUE  },
    { 141,  GT_FALSE,   GT_TRUE  },
    { 142,  GT_FALSE,   GT_TRUE  },
    { 143,  GT_FALSE,   GT_TRUE  },
    { 144,  GT_FALSE,   GT_TRUE  },
    { 145,  GT_FALSE,   GT_TRUE  },
    { 146,  GT_FALSE,   GT_FALSE },
    { 147,  GT_FALSE,   GT_TRUE  },
    { 148,  GT_FALSE,   GT_TRUE  },
    { 149,  GT_FALSE,   GT_FALSE },
    { 150,  GT_FALSE,   GT_TRUE  },
    { 151,  GT_FALSE,   GT_TRUE  },
    { 152,  GT_FALSE,   GT_TRUE  },
    { 153,  GT_FALSE,   GT_FALSE },
    { 154,  GT_FALSE,   GT_TRUE  },
    { 155,  GT_FALSE,   GT_FALSE },
    { 156,  GT_FALSE,   GT_TRUE  },
    { 157,  GT_FALSE,   GT_FALSE },
    { 158,  GT_FALSE,   GT_TRUE  },
    { 159,  GT_FALSE,   GT_FALSE },
    { 160,  GT_FALSE,   GT_TRUE  },
    { 161,  GT_FALSE,   GT_TRUE  },
    { 162,  GT_FALSE,   GT_TRUE  },
    { 163,  GT_FALSE,   GT_TRUE  },
    { 164,  GT_FALSE,   GT_TRUE  },
    { 165,  GT_FALSE,   GT_TRUE  },
    { 166,  GT_FALSE,   GT_FALSE },
    { 167,  GT_FALSE,   GT_TRUE  },
    { 168,  GT_FALSE,   GT_TRUE  },
    { 169,  GT_FALSE,   GT_FALSE },
    { 170,  GT_FALSE,   GT_TRUE  },
    { 171,  GT_FALSE,   GT_TRUE  },
    { 172,  GT_FALSE,   GT_TRUE  },
    { 173,  GT_FALSE,   GT_TRUE  },
    { 174,  GT_FALSE,   GT_TRUE  },
    { 175,  GT_FALSE,   GT_FALSE },
    { 176,  GT_FALSE,   GT_TRUE  },
    { 177,  GT_FALSE,   GT_TRUE  },
    { 178,  GT_FALSE,   GT_TRUE  },
    { 179,  GT_FALSE,   GT_TRUE  },
    { 180,  GT_FALSE,   GT_TRUE  },
    { 181,  GT_FALSE,   GT_TRUE  },
    { 182,  GT_FALSE,   GT_TRUE  },
    { 183,  GT_FALSE,   GT_TRUE  },
    { 184,  GT_FALSE,   GT_TRUE  },
    { 185,  GT_FALSE,   GT_TRUE  },
    { 186,  GT_FALSE,   GT_TRUE  },
    { 187,  GT_FALSE,   GT_TRUE  },
    { 188,  GT_FALSE,   GT_TRUE  },
    { 189,  GT_FALSE,   GT_TRUE  },
    { 190,  GT_FALSE,   GT_TRUE  },
    { 191,  GT_FALSE,   GT_FALSE },
    { 192,  GT_FALSE,   GT_FALSE },
    { 193,  GT_FALSE,   GT_FALSE },
    { 194,  GT_FALSE,   GT_TRUE  },
    { 195,  GT_FALSE,   GT_FALSE },
    { 196,  GT_FALSE,   GT_FALSE },
    { 197,  GT_FALSE,   GT_FALSE },
    { 198,  GT_FALSE,   GT_FALSE },
    { 199,  GT_FALSE,   GT_FALSE },
    { 200,  GT_FALSE,   GT_TRUE  },
    { 201,  GT_FALSE,   GT_TRUE  },
    { 202,  GT_FALSE,   GT_FALSE },
    { 203,  GT_FALSE,   GT_FALSE },
    { 204,  GT_FALSE,   GT_TRUE  },
    { 205,  GT_FALSE,   GT_FALSE },
    { 206,  GT_FALSE,   GT_FALSE },
    { 207,  GT_FALSE,   GT_FALSE },
    { 208,  GT_FALSE,   GT_FALSE },
    { 209,  GT_FALSE,   GT_TRUE  },
    { 210,  GT_FALSE,   GT_FALSE },
    { 211,  GT_FALSE,   GT_FALSE },
    { 212,  GT_FALSE,   GT_TRUE  },
    { 213,  GT_FALSE,   GT_FALSE },
    { 214,  GT_FALSE,   GT_FALSE },
    { 215,  GT_FALSE,   GT_FALSE },
    { 216,  GT_FALSE,   GT_TRUE  },
    { 217,  GT_FALSE,   GT_TRUE  },
    { 218,  GT_FALSE,   GT_TRUE  },
    { 219,  GT_FALSE,   GT_TRUE  },
    { 220,  GT_FALSE,   GT_FALSE },
    { 221,  GT_FALSE,   GT_FALSE },
    { 222,  GT_FALSE,   GT_FALSE },
    { 223,  GT_FALSE,   GT_FALSE },
    { 224,  GT_FALSE,   GT_FALSE },
    { 225,  GT_FALSE,   GT_TRUE  },
    { 226,  GT_FALSE,   GT_FALSE },
    { 227,  GT_FALSE,   GT_TRUE  },
    { 228,  GT_FALSE,   GT_FALSE },
    { 229,  GT_FALSE,   GT_TRUE  },
    { 230,  GT_FALSE,   GT_TRUE  },
    { 231,  GT_FALSE,   GT_FALSE },
    { 232,  GT_TRUE,    GT_TRUE  },
    { 233,  GT_FALSE,   GT_TRUE  },
    { 234,  GT_FALSE,   GT_FALSE },
    { 235,  GT_FALSE,   GT_FALSE },
    { 236,  GT_FALSE,   GT_FALSE },
    { 237,  GT_FALSE,   GT_FALSE },
    { 238,  GT_FALSE,   GT_FALSE },
    { 239,  GT_FALSE,   GT_TRUE  },
    { 240,  GT_FALSE,   GT_TRUE  },
    { 241,  GT_FALSE,   GT_TRUE  },
    { 242,  GT_FALSE,   GT_TRUE  },
    { 243,  GT_FALSE,   GT_TRUE  },
    { 244,  GT_FALSE,   GT_FALSE },
    { 245,  GT_FALSE,   GT_FALSE },
    { 246,  GT_FALSE,   GT_TRUE  },
    { 247,  GT_FALSE,   GT_TRUE  },
    { 248,  GT_FALSE,   GT_FALSE },
    { 249,  GT_TRUE,    GT_FALSE },
    { 250,  GT_FALSE,   GT_TRUE  },
    { 251,  GT_FALSE,   GT_TRUE  },
    { 252,  GT_FALSE,   GT_TRUE  },
    { 253,  GT_FALSE,   GT_FALSE },
    { 254,  GT_FALSE,   GT_TRUE  },
    { 255,  GT_FALSE,   GT_TRUE  },
    { 257,  GT_FALSE,   GT_TRUE  },
    { 258,  GT_TRUE,    GT_TRUE  }
};

static CPSS_PORT_MAC_TO_SERDES_STC  falcon_Belly2Belly_MacToSerdesMap[] =
{
   {{0,7,2,4,1,5,3,6}},  /* 0-7*/    /*Raven 0 */
   {{0,5,1,7,3,2,4,6}},  /* 8-15*/   /*Raven 0 */
   {{1,5,0,7,4,2,3,6}},  /*16-23*/   /*Raven 1 */
   {{0,6,1,7,2,3,4,5}},  /*24-31*/   /*Raven 1 */
   {{0,6,2,7,1,5,4,3}},  /*32-39*/   /*Raven 2 */
   {{0,5,1,6,2,7,4,3}},  /*40-47*/   /*Raven 2 */
   {{2,6,1,3,0,4,5,7}},  /*48-55*/   /*Raven 3 */
   {{1,6,2,7,0,4,3,5}},  /*56-63*/   /*Raven 3 */
   {{0,7,2,4,1,5,3,6}},  /*64-71*/   /*Raven 4 */
   {{0,5,1,7,3,2,4,6}},  /*72-79*/   /*Raven 4 */
   {{1,5,0,7,4,2,3,6}},  /*80-87*/   /*Raven 5 */
   {{0,6,1,7,2,3,4,5}},  /*88-95*/   /*Raven 5 */
   {{0,6,2,7,1,5,4,3}},  /*96-103*/  /*Raven 6 */
   {{0,5,1,6,2,7,4,3}},  /*104-111*/ /*Raven 6 */
   {{2,6,1,3,0,4,5,7}},  /*112-119*/ /*Raven 7 */
   {{1,6,2,7,0,4,3,5}},  /*120-127*/ /*Raven 7 */
   {{0,7,2,4,1,5,3,6}},  /*128-135*/ /*Raven 8 */
   {{0,5,1,7,3,2,4,6}},  /*136-143*/ /*Raven 8 */
   {{1,5,0,7,4,2,3,6}},  /*144-151*/ /*Raven 9 */
   {{0,6,1,7,2,3,4,5}},  /*152-159*/ /*Raven 9 */
   {{0,6,2,7,1,5,4,3}},  /*160-167*/ /*Raven 10*/
   {{0,5,1,6,2,7,4,3}},  /*168-175*/ /*Raven 10*/
   {{2,6,1,3,0,4,5,7}},  /*176-183*/ /*Raven 11*/
   {{1,6,2,7,0,4,3,5}},  /*184-191*/ /*Raven 11*/
   {{0,7,2,4,1,5,3,6}},  /*192-199*/ /*Raven 12*/
   {{0,5,1,7,3,2,4,6}},  /*200-207*/ /*Raven 12*/
   {{1,5,0,7,4,2,3,6}},  /*208-215*/ /*Raven 13*/
   {{0,6,1,7,2,3,4,5}},  /*216-223*/ /*Raven 13*/
   {{0,6,2,7,1,5,4,3}},  /*224-231*/ /*Raven 14*/
   {{0,5,1,6,2,7,4,3}},  /*232-239*/ /*Raven 14*/
   {{2,6,1,3,0,4,5,7}},  /*240-247*/ /*Raven 15*/
   {{1,6,2,7,0,4,3,5}},  /*248-255*/ /*Raven 15*/
};

/** Macro for PP Phase 1 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x0,\
    _SM_(appAddrCompletionRegionsBmp) 0x0,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 128,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_512_PHY \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x0,\
    _SM_(appAddrCompletionRegionsBmp) 0x0,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 512,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE \
    _SM_(apEnable                   ) GT_TRUE,\
    _SM_(numOfDataIntegrityElements ) 0x0

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_DISABLE \
    _SM_(apEnable                   ) GT_FALSE,\
    _SM_(numOfDataIntegrityElements ) 0x0

/** Macro for PP Phase 2 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS \
    _SM_(newDevNum                  ) 32, /* to be modified */ \
    _SM_(auDescNum                  ) 2048,\
    _SM_(auMessageLength            ) CPSS_AU_MESSAGE_LENGTH_8_WORDS_E,\
    _SM_(useDoubleAuq               ) GT_FALSE,\
    _SM_(useSecondaryAuq            ) GT_FALSE,\
    _SM_(fuDescNum                  ) 2048,\
    _SM_(fuqUseSeparate             ) GT_TRUE,\
    _SM_(noTraffic2CPU              ) GT_FALSE,\
    _SM_(cpuPortMode                ) CPSS_NET_CPU_PORT_MODE_SDMA_E,\
    _SM_(useMultiNetIfSdma          ) GT_TRUE,\
    _SM_(netifSdmaPortGroupId       ) 0,\
    _SM_(txDescNum                  ) 80, /* To resolve memory exhaust error in Falcon_12_8_B2B,
                                             the value has been changed to 80 from 250 */\
    _SM_(rxDescNum                  ) 64,\
    _SM_(rxAllocMethod              ) CPSS_RX_BUFF_STATIC_ALLOC_E,\
    _SM_(rxBuffersInCachedMem       ) GT_FALSE,\
    _SM_(rxHeaderOffset             ) 0,\
    _SM_(rxBufferPercentage         ) {13,13,13,13,12,12,12,12},\
    _SM_(txGenQueueList             ) txGenQueues,\
    _SM_(txGenQueueNum              ) sizeof(txGenQueues)/sizeof(txGenQueues[0]),\
    /* NetIf Lib Init Params*/\
    _SM_(miiTxDescNum               ) 0,\
    _SM_(miiTxBufBlockSize          ) 0,\
    _SM_(miiRxBufSize               ) 0,\
    _SM_(miiRxBufBlockSize          ) 0,\
    _SM_(miiRxHeaderOffset          ) 0,\
    _SM_(miiRxBufferPercentage      ) {13,13,13,13,12,12,12,12}

/** Macro for PP after Phase 2 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS \
    /* Led Init*/\
    _SM_(ledOrganize                ) CPSS_LED_ORDER_MODE_BY_CLASS_E,\
    _SM_(ledClockFrequency          ) CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E,\
    _SM_(invertEnable               ) GT_TRUE,\
    _SM_(pulseStretch               ) CPSS_LED_PULSE_STRETCH_1_E,\
    _SM_(blink0Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink0DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(blink1Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink1DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(disableOnLinkDown          ) GT_FALSE,\
    _SM_(clkInvert                  ) GT_FALSE,\
    _SM_(class5select               ) CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E,\
    _SM_(class13select              ) CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E,\
    _SM_(ledStart                   ) 64,\
    _SM_(ledEnd                     ) 64,\
    _SM_(ledPositionTable           ) NULL,\
    /*Led Class manipulation config*/\
    _SM_(invertionEnable            ) GT_FALSE,\
    _SM_(blinkSelect                ) CPSS_LED_BLINK_SELECT_0_E,\
    _SM_(forceEnable                ) GT_FALSE,\
    _SM_(forceData                  ) 0,\
    _SM_(pulseStretchEnable         ) GT_FALSE

/** Macro for PP Logical Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS \
    /*Logical Init*/\
    _SM_(routingMode                ) CPSS_DXCH_TCAM_ROUTER_BASED_E,\
    _SM_(maxNumOfPbrEntries         ) 8192,\
    _SM_(lpmMemoryMode              ) CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,\
    _SM_(sharedTableMode            ) CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,\
    /*General Init*/\
    _SM_(ctrlMacLearn               ) GT_TRUE,\
    _SM_(flowControlDisable         ) GT_TRUE,\
    _SM_(modeFcHol                  ) GT_FALSE,\
    _SM_(mirrorAnalyzerPortNum      ) 0,\
    /*Cascade Init*/\
    _SM_(cascadePtr                 ) NULL

/** Macro for PP Lib Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS \
    _SM_(initBridge                 ) GT_TRUE,\
    _SM_(initIp                     ) GT_TRUE,\
    _SM_(initMirror                 ) GT_TRUE,\
    _SM_(initNetIf                  ) GT_TRUE,\
    _SM_(initPhy                    ) GT_TRUE,\
    _SM_(initPort                   ) GT_TRUE,\
    _SM_(initTrunk                  ) GT_TRUE,\
    _SM_(initPcl                    ) GT_TRUE,\
    _SM_(initTcam                   ) GT_TRUE,\
    _SM_(initPolicer                ) GT_TRUE,\
    _SM_(initPha                    ) GT_TRUE,\
    /* Ip Lib Init Params*/\
    _SM_(blocksAllocationMethod     ) CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E,\
    _SM_(usePolicyBasedRouting      ) GT_FALSE,\
    _SM_(maxNumOfIpv4Prefixes       ) 1000,\
    _SM_(maxNumOfIpv4McEntries      ) 100,\
    _SM_(maxNumOfIpv6Prefixes       ) 100,\
    _SM_(lpmDbFirstTcamLine         ) 100,\
    _SM_(lpmDbLastTcamLine          ) 1003,\
    _SM_(lpmDbSupportIpv4           ) GT_TRUE,\
    _SM_(lpmDbSupportIpv6           ) GT_TRUE,\
    _SM_(lpmDbPartitionEnable       ) GT_TRUE,\
    /* Trunk Lib Init Params*/\
    _SM_(numOfTrunks                ) 0x1000,\
    _SM_(trunkMembersMode           ) CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E,\
    /* Pha Lib Init Params*/\
    _SM_(pha_packetOrderChangeEnable) GT_FALSE,\
    /* Bridge Lib Init Params*/\
    _SM_(policerMruSupported        ) GT_TRUE

/** Macro for PP Phase 1 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_256_PHY \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x0,\
    _SM_(appAddrCompletionRegionsBmp) 0x0,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 256,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0

/** Macro for PP Logical Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_6_4_DEFAULTS \
    /*Logical Init*/\
    _SM_(routingMode                ) CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E, \
    _SM_(maxNumOfPbrEntries         ) 8192,\
    _SM_(lpmMemoryMode              ) CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,\
    _SM_(sharedTableMode            ) CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,\
    /*General Init*/\
    _SM_(ctrlMacLearn               ) GT_TRUE,\
    _SM_(flowControlDisable         ) GT_TRUE,\
    _SM_(modeFcHol                  ) GT_FALSE,\
    _SM_(mirrorAnalyzerPortNum      ) 0,\
    /*Cascade Init*/\
    _SM_(cascadePtr                 ) NULL

/** Macro for PP serdes polarity Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(polarityPtr                ) falcon_DB_PolarityArray, \
    _SM_(polarityArrSize            ) sizeof(falcon_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC)

/** Macro for PP serdes polarity Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_BELLY2BELLY_DEFAULTS \
    /*Serdes Params*/\
    _SM_(polarityPtr                ) falcon_Belly2Belly_PolarityArray, \
    _SM_(polarityArrSize            ) sizeof(falcon_Belly2Belly_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC)

/** Macro for PP serdes mux Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(serdesMapPtr               ) NULL, \
    _SM_(serdesMapArrSize           ) 0

/** Macro for PP serdes mux Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_BELLY2BELLY_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(serdesMapPtr               ) falcon_Belly2Belly_MacToSerdesMap, \
    _SM_(serdesMapArrSize           ) sizeof(falcon_Belly2Belly_MacToSerdesMap)/sizeof(CPSS_PORT_MAC_TO_SERDES_STC)

/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_FALSE

/** Macro for MPD init **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS \
    /*Serdes Params*/\
    _SM_(initMpd                ) GT_FALSE

/** Macro for belly2belly handling disable **/
#define CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS \
    _SM_(belly2belly                ) GT_FALSE

/** Macro for belly2belly handling enable **/
#define CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_ENABLE_DEFAULTS \
    _SM_(belly2belly                ) GT_TRUE

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_FALCON_12_8_R0_1_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_FALCON_12_8_R0_1_BELLY2BELLY_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_512_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_BELLY2BELLY_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_BELLY2BELLY_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_ENABLE_DEFAULTS,
};

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_FALCON_12_8_Z2_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_512_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_DISABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_FALCON_6_4_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_256_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_6_4_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};
