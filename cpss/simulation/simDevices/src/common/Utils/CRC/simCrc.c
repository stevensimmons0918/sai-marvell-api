/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file simCrc.c
*
* @brief This file includes function definitions for CRC 8, 16 and 32 bit.
*
* Inorder for the user to be able to use the above functionality, the file
* "common/simCrc.h" must be included.
*
* @version   5
********************************************************************************
*/


/********* Include files ******************************************************/
#include <common/Utils/CRC/simCrc.h>

/********* Defines ************************************************************/

/* x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0 */
#define CRC_32_POLYNOMIAL 0x04c11db7

/* x^16+x^15+x^2+x^0.*/
#define CRC_16_POLYNOMIAL 0x00008005

/* x^8+x^2+x^1+x^0 */
#define CRC_8_POLYNOMIAL  0x00000007

#define CRC_HASH_MAX_BIT

/********************************************************************/
/********************************************************************/
/* next macros done to keep code similar to the verilog code of PP  */

/* macro to get bit from the array of 70 bytes */
#define _D(bit) (bufPtr[(bit)>>3] >> ((bit)&0x7))
/* macro to get bit from CRC value */
#define _C(bit) (crc >> (bit))
/********************************************************************/
/********************************************************************/

/* set bit in target according to bit 0 from source */
#define setBitFromValue(targetVal,bit0Value,bit) \
    (targetVal) |= ((bit0Value) & 1) << (bit)

extern void skernelFatalError
(
    IN char * format, ...
);


/********* Local variables ****************************************************/
static GT_U32 crc32Table[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static GT_U16 crc16Table[256] =
{
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};



static GT_U8 crc8Table[256] =
{
    0x00, 0x07, 0x0e, 0x09,0x1c, 0x1b, 0x12, 0x15,0x38, 0x3f, 0x36, 0x31,
    0x24, 0x23, 0x2a, 0x2d,0x70, 0x77, 0x7e, 0x79,0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41,0x54, 0x53, 0x5a, 0x5d,0xe0, 0xe7, 0xee, 0xe9,
    0xfc, 0xfb, 0xf2, 0xf5,0xd8, 0xdf, 0xd6, 0xd1,0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99,0x8c, 0x8b, 0x82, 0x85,0xa8, 0xaf, 0xa6, 0xa1,
    0xb4, 0xb3, 0xba, 0xbd,0xc7, 0xc0, 0xc9, 0xce,0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6,0xe3, 0xe4, 0xed, 0xea,0xb7, 0xb0, 0xb9, 0xbe,
    0xab, 0xac, 0xa5, 0xa2,0x8f, 0x88, 0x81, 0x86,0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e,0x3b, 0x3c, 0x35, 0x32,0x1f, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0d, 0x0a,0x57, 0x50, 0x59, 0x5e,0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66,0x73, 0x74, 0x7d, 0x7a,0x89, 0x8e, 0x87, 0x80,
    0x95, 0x92, 0x9b, 0x9c,0xb1, 0xb6, 0xbf, 0xb8,0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0,0xe5, 0xe2, 0xeb, 0xec,0xc1, 0xc6, 0xcf, 0xc8,
    0xdd, 0xda, 0xd3, 0xd4,0x69, 0x6e, 0x67, 0x60,0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58,0x4d, 0x4a, 0x43, 0x44,0x19, 0x1e, 0x17, 0x10,
    0x05, 0x02, 0x0b, 0x0c,0x21, 0x26, 0x2f, 0x28,0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47,0x52, 0x55, 0x5c, 0x5b,0x76, 0x71, 0x78, 0x7f,
    0x6a, 0x6d, 0x64, 0x63,0x3e, 0x39, 0x30, 0x37,0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f,0x1a, 0x1d, 0x14, 0x13,0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb,0x96, 0x91, 0x98, 0x9f,0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7,0xc2, 0xc5, 0xcc, 0xcb,0xe6, 0xe1, 0xe8, 0xef,
    0xfa, 0xfd, 0xf4, 0xf3,
};



/********* Public functions ***************************************************/


/**
* @internal simCalcCrc32 function
* @endinternal
*
* @brief   Calculate CRC 32 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xffffffff.
*
*/
GT_U32 simCalcCrc32
(
    IN GT_U32 crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
)
{
    GT_U32 i;           /* Loop counter                      */
    GT_U8  newEntry;    /* The new entry in the lookup table */
    GT_U32 result = 0;  /* The result crc                    */


    for ( i = 0;  i < byteNum;  i++ )
    {
        newEntry = (GT_U8)( ( crc ^ (*buffer) ) & 0xff );
        crc = crc32Table[newEntry] ^ (crc >> 8);
        buffer++;
    }

    crc = crc ^ 0xFFFFFFFF;


    result |= (crc & 0xFF000000) >> 24;
    result |= (crc & 0x00FF0000) >> 8;
    result |= (crc & 0x0000FF00) << 8;
    result |= (crc & 0x000000FF) << 24;

    return result;
}

/**
* @internal simCalcCrc16 function
* @endinternal
*
* @brief   Calculate CRC 16 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xffff.
*
*/
GT_U16 simCalcCrc16
(
    IN GT_U16 crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
)
{
    GT_U32 i;
    GT_U8  newEntry;

    for ( i = 0;  i < byteNum;  i++ )
    {
        newEntry = (GT_U8)( ( (GT_U32) ( crc >> 8) )^ (*buffer) ) & 0xff;
        crc = ( crc << 8 ) ^ crc16Table[newEntry];
        buffer++;
    }

    return crc;

}

/**
* @internal simCalcCrc8 function
* @endinternal
*
* @brief   Calculate CRC 8 bit for input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] buffer                   - pointer to the buffer.
* @param[in] byteNum                  - number of bytes in the buffer.
*                                       32 bit CRC.
*
* @note For calculation a new CRC the value of CRC should be 0xff.
*
*/
GT_U8 simCalcCrc8
(
    IN GT_U8  crc,
    IN GT_U8  *buffer,
    IN GT_U32 byteNum
)
{
    GT_U32 i;

    for ( i = 0;  i < byteNum;  i++ )
    {
        crc = crc8Table[ (crc ^ (*buffer) )];
        buffer++;
    }

    return crc;
}


/********* Private functions **************************************************/



/**
* @internal swapBits function
* @endinternal
*
* @brief   Swap bit 0 for bit 7. bit 1 for bit 6 etc.
*
* @param[in] data                     -  to swap.
* @param[in] num                      - Number of bits to swap.
*                                       The new value
*/
static GT_U32 swapBits
(
    IN GT_U32 data,
    IN GT_U8 num
)
{
    GT_U32 value = 0;
    GT_U32 i;

    /* Swap bit 0 for bit 7  */
    /* bit 1 for bit 6, etc. */
    for(i = 1; i < (GT_U32)(num + 1); i++)
    {
        if( (data & 0x1) != 0)
            value |= 1 << (num - i);
        data >>= 1;
    }

    return (value);
}

/**
* @internal simCreateCrc32Table function
* @endinternal
*
* @brief   Create the remainders look up table to CRC 32
*/
GT_VOID simCreateCrc32Table
(
    GT_VOID
)
{
    GT_U32 i,j;      /* Loop counter */

    for(i = 0; i < 256; i++)
    {
        crc32Table[i]= swapBits(i, 8) << 24;

        for (j = 0; j < 8; j++)
        {
            crc32Table[i] = (crc32Table[i] << 1) ^ (crc32Table[i] & (1 << 31) ?
                                                    CRC_32_POLYNOMIAL : 0);
        }

        crc32Table[i] = swapBits(crc32Table[i], 32);
    }
}

/**
* @internal simCreateCrc16Table function
* @endinternal
*
* @brief   Create the remainders look up table to CRC 16
*/
GT_VOID simCreateCrc16Table
(
    GT_VOID
)
{
    GT_32  i,j;      /* Loop counter */
    GT_U16 crcAccum; /* crc          */

    for ( i = 0;  i < 256;  i++ )
    {
        crcAccum = (GT_U16) (i << 8 );
        for ( j = 0;  j < 8;  j++ )
        {
            if ( (crcAccum & 0x00008000) != 0 )
                   crcAccum = ( crcAccum << 1 ) ^ CRC_16_POLYNOMIAL;
            else
                   crcAccum = ( crcAccum << 1 );
        }
        crc16Table[i] = crcAccum;
    }


}

/**
* @internal simCreateCrc8Table function
* @endinternal
*
* @brief   Create the remainders look up table to CRC 8
*/
GT_VOID simCreateCrc8Table
(
    GT_VOID
)
{

    GT_U32 i,j;      /* Loop counter */
    GT_U8  crcAccum; /* crc          */

    for ( i = 0;  i < 256;  i++ )
    {
        crcAccum = (GT_U8)i;
        for ( j = 0;  j < 8;  j++ )
        {
            if ( (crcAccum & 0x00000080) != 0 )
                   crcAccum = ( crcAccum << 1 ) ^ CRC_8_POLYNOMIAL;
            else
                   crcAccum = ( crcAccum << 1 );
        }
        crc8Table[i] = crcAccum;
    }

}

/**
* @internal simCalcCrc6_280 function
* @endinternal
*
* @brief   Calculate CRC 6 bit for 35 bytes data.
*
* @param[in] crc                      - CRC start value.
*                                      buffer  - pointer to the buffer.
*                                       6 bit CRC.
*/
static GT_32 simCalcCrc6_280
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr
)
{
    GT_U32 newCrc = 0;
    GT_U32 tmpCrc;
    GT_U32 index = 0;/* bit index in final CRC */

    tmpCrc =
        _D(278) ^ _D(277) ^ _D(276) ^ _D(272) ^ _D(270) ^ _D(269) ^
        _D(268) ^ _D(267) ^ _D(264) ^ _D(262) ^ _D(258) ^ _D(257) ^
        _D(252) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(242) ^
        _D(241) ^ _D(239) ^ _D(237) ^ _D(235) ^ _D(234) ^ _D(231) ^
        _D(230) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(224) ^ _D(223) ^
        _D(221) ^ _D(218) ^ _D(215) ^ _D(214) ^ _D(213) ^ _D(209) ^
        _D(207) ^ _D(206) ^ _D(205) ^ _D(204) ^ _D(201) ^ _D(199) ^
        _D(195) ^ _D(194) ^ _D(189) ^ _D(183) ^ _D(182) ^ _D(181) ^
        _D(180) ^ _D(179) ^ _D(178) ^ _D(176) ^ _D(174) ^ _D(172) ^
        _D(171) ^ _D(168) ^ _D(167) ^ _D(165) ^ _D(164) ^ _D(163) ^
        _D(161) ^ _D(160) ^ _D(158) ^ _D(155) ^ _D(152) ^ _D(151) ^
        _D(150) ^ _D(146) ^ _D(144) ^ _D(143) ^ _D(142) ^ _D(141) ^
        _D(138) ^ _D(136) ^ _D(132) ^ _D(131) ^ _D(126) ^ _D(120) ^
        _D(119) ^ _D(118) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(113) ^
        _D(111) ^ _D(109) ^ _D(108) ^ _D(105) ^ _D(104) ^ _D(102) ^
        _D(101) ^ _D(100) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(92) ^ _D(89) ^
        _D(88) ^ _D(87) ^ _D(83) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(78) ^
        _D(75) ^ _D(73) ^ _D(69) ^ _D(68) ^ _D(63) ^ _D(57) ^ _D(56) ^
        _D(55) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(50) ^ _D(48) ^ _D(46) ^
        _D(45) ^ _D(42) ^ _D(41) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(35) ^
        _D(34) ^ _D(32) ^ _D(29) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(20) ^
        _D(18) ^ _D(17) ^ _D(16) ^ _D(15) ^ _D(12) ^ _D(10) ^ _D(6) ^
        _D(5) ^ _D(0) ^ _C( 2) ^ _C( 3) ^ _C( 4);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(276) ^ _D(273) ^ _D(272) ^ _D(271) ^ _D(267) ^
        _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(259) ^ _D(257) ^
        _D(253) ^ _D(252) ^ _D(247) ^ _D(241) ^ _D(240) ^ _D(239) ^
        _D(238) ^ _D(237) ^ _D(236) ^ _D(234) ^ _D(232) ^ _D(230) ^
        _D(229) ^ _D(226) ^ _D(225) ^ _D(223) ^ _D(222) ^ _D(221) ^
        _D(219) ^ _D(218) ^ _D(216) ^ _D(213) ^ _D(210) ^ _D(209) ^
        _D(208) ^ _D(204) ^ _D(202) ^ _D(201) ^ _D(200) ^ _D(199) ^
        _D(196) ^ _D(194) ^ _D(190) ^ _D(189) ^ _D(184) ^ _D(178) ^
        _D(177) ^ _D(176) ^ _D(175) ^ _D(174) ^ _D(173) ^ _D(171) ^
        _D(169) ^ _D(167) ^ _D(166) ^ _D(163) ^ _D(162) ^ _D(160) ^
        _D(159) ^ _D(158) ^ _D(156) ^ _D(155) ^ _D(153) ^ _D(150) ^
        _D(147) ^ _D(146) ^ _D(145) ^ _D(141) ^ _D(139) ^ _D(138) ^
        _D(137) ^ _D(136) ^ _D(133) ^ _D(131) ^ _D(127) ^ _D(126) ^
        _D(121) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^
        _D(110) ^ _D(108) ^ _D(106) ^ _D(104) ^ _D(103) ^ _D(100) ^
        _D(99) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(93) ^ _D(92) ^ _D(90) ^
        _D(87) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(78) ^ _D(76) ^ _D(75) ^
        _D(74) ^ _D(73) ^ _D(70) ^ _D(68) ^ _D(64) ^ _D(63) ^ _D(58) ^
        _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(45) ^
        _D(43) ^ _D(41) ^ _D(40) ^ _D(37) ^ _D(36) ^ _D(34) ^ _D(33) ^
        _D(32) ^ _D(30) ^ _D(29) ^ _D(27) ^ _D(24) ^ _D(21) ^ _D(20) ^
        _D(19) ^ _D(15) ^ _D(13) ^ _D(12) ^ _D(11) ^ _D(10) ^ _D(7) ^
        _D(5) ^ _D(1) ^ _D(0) ^ _C( 2) ^ _C( 5);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(277) ^ _D(274) ^ _D(273) ^ _D(272) ^ _D(268) ^ _D(266) ^
        _D(265) ^ _D(264) ^ _D(263) ^ _D(260) ^ _D(258) ^ _D(254) ^
        _D(253) ^ _D(248) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(239) ^
        _D(238) ^ _D(237) ^ _D(235) ^ _D(233) ^ _D(231) ^ _D(230) ^
        _D(227) ^ _D(226) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(220) ^
        _D(219) ^ _D(217) ^ _D(214) ^ _D(211) ^ _D(210) ^ _D(209) ^
        _D(205) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(200) ^ _D(197) ^
        _D(195) ^ _D(191) ^ _D(190) ^ _D(185) ^ _D(179) ^ _D(178) ^
        _D(177) ^ _D(176) ^ _D(175) ^ _D(174) ^ _D(172) ^ _D(170) ^
        _D(168) ^ _D(167) ^ _D(164) ^ _D(163) ^ _D(161) ^ _D(160) ^
        _D(159) ^ _D(157) ^ _D(156) ^ _D(154) ^ _D(151) ^ _D(148) ^
        _D(147) ^ _D(146) ^ _D(142) ^ _D(140) ^ _D(139) ^ _D(138) ^
        _D(137) ^ _D(134) ^ _D(132) ^ _D(128) ^ _D(127) ^ _D(122) ^
        _D(116) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^
        _D(109) ^ _D(107) ^ _D(105) ^ _D(104) ^ _D(101) ^ _D(100) ^
        _D(98) ^ _D(97) ^ _D(96) ^ _D(94) ^ _D(93) ^ _D(91) ^ _D(88) ^
        _D(85) ^ _D(84) ^ _D(83) ^ _D(79) ^ _D(77) ^ _D(76) ^ _D(75) ^
        _D(74) ^ _D(71) ^ _D(69) ^ _D(65) ^ _D(64) ^ _D(59) ^ _D(53) ^
        _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(46) ^ _D(44) ^
        _D(42) ^ _D(41) ^ _D(38) ^ _D(37) ^ _D(35) ^ _D(34) ^ _D(33) ^
        _D(31) ^ _D(30) ^ _D(28) ^ _D(25) ^ _D(22) ^ _D(21) ^ _D(20) ^
        _D(16) ^ _D(14) ^ _D(13) ^ _D(12) ^ _D(11) ^ _D(8) ^ _D(6) ^
        _D(2) ^ _D(1) ^ _C( 0) ^ _C( 3);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(278) ^ _D(275) ^ _D(274) ^ _D(273) ^ _D(269) ^ _D(267) ^
        _D(266) ^ _D(265) ^ _D(264) ^ _D(261) ^ _D(259) ^ _D(255) ^
        _D(254) ^ _D(249) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(240) ^
        _D(239) ^ _D(238) ^ _D(236) ^ _D(234) ^ _D(232) ^ _D(231) ^
        _D(228) ^ _D(227) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(221) ^
        _D(220) ^ _D(218) ^ _D(215) ^ _D(212) ^ _D(211) ^ _D(210) ^
        _D(206) ^ _D(204) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(198) ^
        _D(196) ^ _D(192) ^ _D(191) ^ _D(186) ^ _D(180) ^ _D(179) ^
        _D(178) ^ _D(177) ^ _D(176) ^ _D(175) ^ _D(173) ^ _D(171) ^
        _D(169) ^ _D(168) ^ _D(165) ^ _D(164) ^ _D(162) ^ _D(161) ^
        _D(160) ^ _D(158) ^ _D(157) ^ _D(155) ^ _D(152) ^ _D(149) ^
        _D(148) ^ _D(147) ^ _D(143) ^ _D(141) ^ _D(140) ^ _D(139) ^
        _D(138) ^ _D(135) ^ _D(133) ^ _D(129) ^ _D(128) ^ _D(123) ^
        _D(117) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(112) ^
        _D(110) ^ _D(108) ^ _D(106) ^ _D(105) ^ _D(102) ^ _D(101) ^
        _D(99) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(94) ^ _D(92) ^ _D(89) ^
        _D(86) ^ _D(85) ^ _D(84) ^ _D(80) ^ _D(78) ^ _D(77) ^ _D(76) ^
        _D(75) ^ _D(72) ^ _D(70) ^ _D(66) ^ _D(65) ^ _D(60) ^ _D(54) ^
        _D(53) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(47) ^ _D(45) ^
        _D(43) ^ _D(42) ^ _D(39) ^ _D(38) ^ _D(36) ^ _D(35) ^ _D(34) ^
        _D(32) ^ _D(31) ^ _D(29) ^ _D(26) ^ _D(23) ^ _D(22) ^ _D(21) ^
        _D(17) ^ _D(15) ^ _D(14) ^ _D(13) ^ _D(12) ^ _D(9) ^ _D(7) ^
        _D(3) ^ _D(2) ^ _C( 0) ^ _C( 1) ^ _C( 4);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(276) ^ _D(275) ^ _D(274) ^ _D(270) ^ _D(268) ^
        _D(267) ^ _D(266) ^ _D(265) ^ _D(262) ^ _D(260) ^ _D(256) ^
        _D(255) ^ _D(250) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^
        _D(240) ^ _D(239) ^ _D(237) ^ _D(235) ^ _D(233) ^ _D(232) ^
        _D(229) ^ _D(228) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(222) ^
        _D(221) ^ _D(219) ^ _D(216) ^ _D(213) ^ _D(212) ^ _D(211) ^
        _D(207) ^ _D(205) ^ _D(204) ^ _D(203) ^ _D(202) ^ _D(199) ^
        _D(197) ^ _D(193) ^ _D(192) ^ _D(187) ^ _D(181) ^ _D(180) ^
        _D(179) ^ _D(178) ^ _D(177) ^ _D(176) ^ _D(174) ^ _D(172) ^
        _D(170) ^ _D(169) ^ _D(166) ^ _D(165) ^ _D(163) ^ _D(162) ^
        _D(161) ^ _D(159) ^ _D(158) ^ _D(156) ^ _D(153) ^ _D(150) ^
        _D(149) ^ _D(148) ^ _D(144) ^ _D(142) ^ _D(141) ^ _D(140) ^
        _D(139) ^ _D(136) ^ _D(134) ^ _D(130) ^ _D(129) ^ _D(124) ^
        _D(118) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(113) ^
        _D(111) ^ _D(109) ^ _D(107) ^ _D(106) ^ _D(103) ^ _D(102) ^
        _D(100) ^ _D(99) ^ _D(98) ^ _D(96) ^ _D(95) ^ _D(93) ^ _D(90) ^
        _D(87) ^ _D(86) ^ _D(85) ^ _D(81) ^ _D(79) ^ _D(78) ^ _D(77) ^
        _D(76) ^ _D(73) ^ _D(71) ^ _D(67) ^ _D(66) ^ _D(61) ^ _D(55) ^
        _D(54) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(48) ^ _D(46) ^
        _D(44) ^ _D(43) ^ _D(40) ^ _D(39) ^ _D(37) ^ _D(36) ^ _D(35) ^
        _D(33) ^ _D(32) ^ _D(30) ^ _D(27) ^ _D(24) ^ _D(23) ^ _D(22) ^
        _D(18) ^ _D(16) ^ _D(15) ^ _D(14) ^ _D(13) ^ _D(10) ^ _D(8) ^
        _D(4) ^ _D(3) ^ _C( 0) ^ _C( 1) ^ _C( 2) ^ _C( 5);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(277) ^ _D(276) ^ _D(275) ^ _D(271) ^ _D(269) ^ _D(268) ^
        _D(267) ^ _D(266) ^ _D(263) ^ _D(261) ^ _D(257) ^ _D(256) ^
        _D(251) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^
        _D(240) ^ _D(238) ^ _D(236) ^ _D(234) ^ _D(233) ^ _D(230) ^
        _D(229) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(223) ^ _D(222) ^
        _D(220) ^ _D(217) ^ _D(214) ^ _D(213) ^ _D(212) ^ _D(208) ^
        _D(206) ^ _D(205) ^ _D(204) ^ _D(203) ^ _D(200) ^ _D(198) ^
        _D(194) ^ _D(193) ^ _D(188) ^ _D(182) ^ _D(181) ^ _D(180) ^
        _D(179) ^ _D(178) ^ _D(177) ^ _D(175) ^ _D(173) ^ _D(171) ^
        _D(170) ^ _D(167) ^ _D(166) ^ _D(164) ^ _D(163) ^ _D(162) ^
        _D(160) ^ _D(159) ^ _D(157) ^ _D(154) ^ _D(151) ^ _D(150) ^
        _D(149) ^ _D(145) ^ _D(143) ^ _D(142) ^ _D(141) ^ _D(140) ^
        _D(137) ^ _D(135) ^ _D(131) ^ _D(130) ^ _D(125) ^ _D(119) ^
        _D(118) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(112) ^
        _D(110) ^ _D(108) ^ _D(107) ^ _D(104) ^ _D(103) ^ _D(101) ^
        _D(100) ^ _D(99) ^ _D(97) ^ _D(96) ^ _D(94) ^ _D(91) ^ _D(88) ^
        _D(87) ^ _D(86) ^ _D(82) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(77) ^
        _D(74) ^ _D(72) ^ _D(68) ^ _D(67) ^ _D(62) ^ _D(56) ^ _D(55) ^
        _D(54) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(49) ^ _D(47) ^ _D(45) ^
        _D(44) ^ _D(41) ^ _D(40) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(34) ^
        _D(33) ^ _D(31) ^ _D(28) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(19) ^
        _D(17) ^ _D(16) ^ _D(15) ^ _D(14) ^ _D(11) ^ _D(9) ^ _D(5) ^
        _D(4) ^ _C( 1) ^ _C( 2) ^ _C( 3);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    return (newCrc & 0x3f);
}

/**
* @internal simCalcHashFor70BytesCrc6 function
* @endinternal
*
* @brief   Calculate CRC 6 bit for 70 bytes input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       6 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc6
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
)
{
    GT_U32 newCrc;

    if(byteNum != 70)
    {
        skernelFatalError("simCalcHashFor70BytesCrc16: number of bytes[%d] != 70 \n", byteNum);

    }

    newCrc = simCalcCrc6_280(crc   , &bufPtr[35]);/*[559:280]*/
    newCrc = simCalcCrc6_280(newCrc, &bufPtr[ 0]);  /*[279:0]*/

    return newCrc;
}

/**
* @internal simCalcCrc16_280 function
* @endinternal
*
* @brief   Calculate CRC 16 bit for 35 bytes data.
*
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the bufPtr.
*                                       16 bit CRC.
*/
static GT_U32 simCalcCrc16_280
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr
)
{
    GT_U32 newCrc = 0;
    GT_U32 tmpCrc;
    GT_U32 index = 0;/* bit index in final CRC */

    tmpCrc =
        _D(279) ^ _D(278) ^ _D(277) ^ _D(276) ^ _D(275) ^ _D(274) ^
        _D(273) ^ _D(272) ^ _D(271) ^ _D(270) ^ _D(268) ^ _D(267) ^
        _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^
        _D(259) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(251) ^
        _D(250) ^ _D(249) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^
        _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(238) ^
        _D(224) ^ _D(223) ^ _D(221) ^ _D(219) ^ _D(217) ^ _D(215) ^
        _D(213) ^ _D(211) ^ _D(207) ^ _D(206) ^ _D(203) ^ _D(202) ^
        _D(199) ^ _D(198) ^ _D(195) ^ _D(193) ^ _D(192) ^ _D(191) ^
        _D(189) ^ _D(188) ^ _D(187) ^ _D(185) ^ _D(184) ^ _D(183) ^
        _D(181) ^ _D(180) ^ _D(175) ^ _D(174) ^ _D(173) ^ _D(172) ^
        _D(167) ^ _D(166) ^ _D(165) ^ _D(163) ^ _D(161) ^ _D(160) ^
        _D(159) ^ _D(158) ^ _D(157) ^ _D(155) ^ _D(153) ^ _D(152) ^
        _D(151) ^ _D(150) ^ _D(147) ^ _D(146) ^ _D(145) ^ _D(144) ^
        _D(143) ^ _D(142) ^ _D(139) ^ _D(138) ^ _D(137) ^ _D(136) ^
        _D(135) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(129) ^
        _D(128) ^ _D(127) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(122) ^
        _D(121) ^ _D(120) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(108) ^
        _D(107) ^ _D(106) ^ _D(105) ^ _D(103) ^ _D(101) ^ _D(99) ^
        _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^
        _D(90) ^ _D(87) ^ _D(86) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^
        _D(79) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(73) ^ _D(72) ^
        _D(71) ^ _D(69) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(64) ^
        _D(63) ^ _D(62) ^ _D(61) ^ _D(60) ^ _D(55) ^ _D(54) ^ _D(53) ^
        _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(46) ^
        _D(45) ^ _D(43) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(37) ^
        _D(36) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(30) ^
        _D(27) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(21) ^
        _D(20) ^ _D(19) ^ _D(18) ^ _D(17) ^ _D(16) ^ _D(15) ^ _D(13) ^
        _D(12) ^ _D(11) ^ _D(10) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(6) ^
        _D(5) ^ _D(4) ^ _D(3) ^ _D(2) ^ _D(1) ^ _D(0) ^ _C(0) ^ _C(1) ^
        _C(3) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(8) ^ _C(9) ^ _C(10) ^ _C(11) ^
        _C(12) ^ _C(13) ^ _C(14) ^ _C(15);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(278) ^ _D(277) ^ _D(276) ^ _D(275) ^ _D(274) ^
        _D(273) ^ _D(272) ^ _D(271) ^ _D(269) ^ _D(268) ^ _D(266) ^
        _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^
        _D(259) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(252) ^ _D(251) ^
        _D(250) ^ _D(249) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^
        _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(239) ^ _D(225) ^
        _D(224) ^ _D(222) ^ _D(220) ^ _D(218) ^ _D(216) ^ _D(214) ^
        _D(212) ^ _D(208) ^ _D(207) ^ _D(204) ^ _D(203) ^ _D(200) ^
        _D(199) ^ _D(196) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(190) ^
        _D(189) ^ _D(188) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(182) ^
        _D(181) ^ _D(176) ^ _D(175) ^ _D(174) ^ _D(173) ^ _D(168) ^
        _D(167) ^ _D(166) ^ _D(164) ^ _D(162) ^ _D(161) ^ _D(160) ^
        _D(159) ^ _D(158) ^ _D(156) ^ _D(154) ^ _D(153) ^ _D(152) ^
        _D(151) ^ _D(148) ^ _D(147) ^ _D(146) ^ _D(145) ^ _D(144) ^
        _D(143) ^ _D(140) ^ _D(139) ^ _D(138) ^ _D(137) ^ _D(136) ^
        _D(134) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(129) ^
        _D(128) ^ _D(126) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(122) ^
        _D(121) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(108) ^
        _D(107) ^ _D(106) ^ _D(104) ^ _D(102) ^ _D(100) ^ _D(98) ^
        _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^
        _D(88) ^ _D(87) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^
        _D(79) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(74) ^ _D(73) ^ _D(72) ^
        _D(70) ^ _D(69) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(64) ^
        _D(63) ^ _D(62) ^ _D(61) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(53) ^
        _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(46) ^
        _D(44) ^ _D(42) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(37) ^
        _D(36) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(28) ^
        _D(27) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(21) ^
        _D(20) ^ _D(19) ^ _D(18) ^ _D(17) ^ _D(16) ^ _D(14) ^ _D(13) ^
        _D(12) ^ _D(11) ^ _D(10) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(6) ^
        _D(5) ^ _D(4) ^ _D(3) ^ _D(2) ^ _D(1) ^ _C(0) ^ _C(1) ^ _C(2) ^
        _C(4) ^ _C(5) ^ _C(7) ^ _C(8) ^ _C(9) ^ _C(10) ^ _C(11) ^
        _C(12) ^ _C(13) ^ _C(14) ^ _C(15);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(271) ^ _D(269) ^ _D(268) ^ _D(266) ^ _D(256) ^ _D(255) ^
        _D(253) ^ _D(252) ^ _D(241) ^ _D(238) ^ _D(226) ^ _D(225) ^
        _D(224) ^ _D(211) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(206) ^
        _D(205) ^ _D(204) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(200) ^
        _D(199) ^ _D(198) ^ _D(197) ^ _D(194) ^ _D(192) ^ _D(190) ^
        _D(188) ^ _D(186) ^ _D(184) ^ _D(182) ^ _D(181) ^ _D(180) ^
        _D(177) ^ _D(176) ^ _D(173) ^ _D(172) ^ _D(169) ^ _D(168) ^
        _D(166) ^ _D(162) ^ _D(158) ^ _D(154) ^ _D(151) ^ _D(150) ^
        _D(149) ^ _D(148) ^ _D(143) ^ _D(142) ^ _D(141) ^ _D(140) ^
        _D(136) ^ _D(134) ^ _D(128) ^ _D(126) ^ _D(121) ^ _D(120) ^
        _D(113) ^ _D(112) ^ _D(106) ^ _D(98) ^ _D(91) ^ _D(90) ^
        _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(84) ^ _D(76) ^
        _D(74) ^ _D(72) ^ _D(70) ^ _D(61) ^ _D(60) ^ _D(57) ^ _D(56) ^
        _D(46) ^ _D(42) ^ _D(31) ^ _D(30) ^ _D(29) ^ _D(28) ^ _D(16) ^
        _D(14) ^ _D(1) ^ _D(0) ^ _C(2) ^ _C(4) ^ _C(5) ^ _C(7);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(272) ^ _D(270) ^ _D(269) ^ _D(267) ^ _D(257) ^ _D(256) ^
        _D(254) ^ _D(253) ^ _D(242) ^ _D(239) ^ _D(227) ^ _D(226) ^
        _D(225) ^ _D(212) ^ _D(210) ^ _D(209) ^ _D(208) ^ _D(207) ^
        _D(206) ^ _D(205) ^ _D(204) ^ _D(203) ^ _D(202) ^ _D(201) ^
        _D(200) ^ _D(199) ^ _D(198) ^ _D(195) ^ _D(193) ^ _D(191) ^
        _D(189) ^ _D(187) ^ _D(185) ^ _D(183) ^ _D(182) ^ _D(181) ^
        _D(178) ^ _D(177) ^ _D(174) ^ _D(173) ^ _D(170) ^ _D(169) ^
        _D(167) ^ _D(163) ^ _D(159) ^ _D(155) ^ _D(152) ^ _D(151) ^
        _D(150) ^ _D(149) ^ _D(144) ^ _D(143) ^ _D(142) ^ _D(141) ^
        _D(137) ^ _D(135) ^ _D(129) ^ _D(127) ^ _D(122) ^ _D(121) ^
        _D(114) ^ _D(113) ^ _D(107) ^ _D(99) ^ _D(92) ^ _D(91) ^
        _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(77) ^
        _D(75) ^ _D(73) ^ _D(71) ^ _D(62) ^ _D(61) ^ _D(58) ^ _D(57) ^
        _D(47) ^ _D(43) ^ _D(32) ^ _D(31) ^ _D(30) ^ _D(29) ^ _D(17) ^
        _D(15) ^ _D(2) ^ _D(1) ^ _C(3) ^ _C(5) ^ _C(6) ^ _C(8);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(273) ^ _D(271) ^ _D(270) ^ _D(268) ^ _D(258) ^ _D(257) ^
            _D(255) ^ _D(254) ^ _D(243) ^ _D(240) ^ _D(228) ^ _D(227) ^
            _D(226) ^ _D(213) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^
            _D(207) ^ _D(206) ^ _D(205) ^ _D(204) ^ _D(203) ^ _D(202) ^
            _D(201) ^ _D(200) ^ _D(199) ^ _D(196) ^ _D(194) ^ _D(192) ^
            _D(190) ^ _D(188) ^ _D(186) ^ _D(184) ^ _D(183) ^ _D(182) ^
            _D(179) ^ _D(178) ^ _D(175) ^ _D(174) ^ _D(171) ^ _D(170) ^
            _D(168) ^ _D(164) ^ _D(160) ^ _D(156) ^ _D(153) ^ _D(152) ^
            _D(151) ^ _D(150) ^ _D(145) ^ _D(144) ^ _D(143) ^ _D(142) ^
            _D(138) ^ _D(136) ^ _D(130) ^ _D(128) ^ _D(123) ^ _D(122) ^
            _D(115) ^ _D(114) ^ _D(108) ^ _D(100) ^ _D(93) ^ _D(92) ^
            _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(78) ^
            _D(76) ^ _D(74) ^ _D(72) ^ _D(63) ^ _D(62) ^ _D(59) ^ _D(58) ^
            _D(48) ^ _D(44) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(30) ^ _D(18) ^
            _D(16) ^ _D(3) ^ _D(2) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(9);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(274) ^ _D(272) ^ _D(271) ^ _D(269) ^ _D(259) ^ _D(258) ^
        _D(256) ^ _D(255) ^ _D(244) ^ _D(241) ^ _D(229) ^ _D(228) ^
        _D(227) ^ _D(214) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^
        _D(208) ^ _D(207) ^ _D(206) ^ _D(205) ^ _D(204) ^ _D(203) ^
        _D(202) ^ _D(201) ^ _D(200) ^ _D(197) ^ _D(195) ^ _D(193) ^
        _D(191) ^ _D(189) ^ _D(187) ^ _D(185) ^ _D(184) ^ _D(183) ^
        _D(180) ^ _D(179) ^ _D(176) ^ _D(175) ^ _D(172) ^ _D(171) ^
        _D(169) ^ _D(165) ^ _D(161) ^ _D(157) ^ _D(154) ^ _D(153) ^
        _D(152) ^ _D(151) ^ _D(146) ^ _D(145) ^ _D(144) ^ _D(143) ^
        _D(139) ^ _D(137) ^ _D(131) ^ _D(129) ^ _D(124) ^ _D(123) ^
        _D(116) ^ _D(115) ^ _D(109) ^ _D(101) ^ _D(94) ^ _D(93) ^
        _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(79) ^
        _D(77) ^ _D(75) ^ _D(73) ^ _D(64) ^ _D(63) ^ _D(60) ^ _D(59) ^
        _D(49) ^ _D(45) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(19) ^
        _D(17) ^ _D(4) ^ _D(3) ^ _C(5) ^ _C(7) ^ _C(8) ^ _C(10);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(275) ^ _D(273) ^ _D(272) ^ _D(270) ^ _D(260) ^ _D(259) ^
        _D(257) ^ _D(256) ^ _D(245) ^ _D(242) ^ _D(230) ^ _D(229) ^
        _D(228) ^ _D(215) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^
        _D(209) ^ _D(208) ^ _D(207) ^ _D(206) ^ _D(205) ^ _D(204) ^
        _D(203) ^ _D(202) ^ _D(201) ^ _D(198) ^ _D(196) ^ _D(194) ^
        _D(192) ^ _D(190) ^ _D(188) ^ _D(186) ^ _D(185) ^ _D(184) ^
        _D(181) ^ _D(180) ^ _D(177) ^ _D(176) ^ _D(173) ^ _D(172) ^
        _D(170) ^ _D(166) ^ _D(162) ^ _D(158) ^ _D(155) ^ _D(154) ^
        _D(153) ^ _D(152) ^ _D(147) ^ _D(146) ^ _D(145) ^ _D(144) ^
        _D(140) ^ _D(138) ^ _D(132) ^ _D(130) ^ _D(125) ^ _D(124) ^
        _D(117) ^ _D(116) ^ _D(110) ^ _D(102) ^ _D(95) ^ _D(94) ^
        _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(80) ^
        _D(78) ^ _D(76) ^ _D(74) ^ _D(65) ^ _D(64) ^ _D(61) ^ _D(60) ^
        _D(50) ^ _D(46) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(20) ^
        _D(18) ^ _D(5) ^ _D(4) ^ _C(6) ^ _C(8) ^ _C(9) ^ _C(11);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(276) ^ _D(274) ^ _D(273) ^ _D(271) ^ _D(261) ^ _D(260) ^
        _D(258) ^ _D(257) ^ _D(246) ^ _D(243) ^ _D(231) ^ _D(230) ^
        _D(229) ^ _D(216) ^ _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^
        _D(210) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(206) ^ _D(205) ^
        _D(204) ^ _D(203) ^ _D(202) ^ _D(199) ^ _D(197) ^ _D(195) ^
        _D(193) ^ _D(191) ^ _D(189) ^ _D(187) ^ _D(186) ^ _D(185) ^
        _D(182) ^ _D(181) ^ _D(178) ^ _D(177) ^ _D(174) ^ _D(173) ^
        _D(171) ^ _D(167) ^ _D(163) ^ _D(159) ^ _D(156) ^ _D(155) ^
        _D(154) ^ _D(153) ^ _D(148) ^ _D(147) ^ _D(146) ^ _D(145) ^
        _D(141) ^ _D(139) ^ _D(133) ^ _D(131) ^ _D(126) ^ _D(125) ^
        _D(118) ^ _D(117) ^ _D(111) ^ _D(103) ^ _D(96) ^ _D(95) ^
        _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(81) ^
        _D(79) ^ _D(77) ^ _D(75) ^ _D(66) ^ _D(65) ^ _D(62) ^ _D(61) ^
        _D(51) ^ _D(47) ^ _D(36) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(21) ^
        _D(19) ^ _D(6) ^ _D(5) ^ _C(7) ^ _C(9) ^ _C(10) ^ _C(12);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(277) ^ _D(275) ^ _D(274) ^ _D(272) ^ _D(262) ^ _D(261) ^
        _D(259) ^ _D(258) ^ _D(247) ^ _D(244) ^ _D(232) ^ _D(231) ^
        _D(230) ^ _D(217) ^ _D(215) ^ _D(214) ^ _D(213) ^ _D(212) ^
        _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(206) ^
        _D(205) ^ _D(204) ^ _D(203) ^ _D(200) ^ _D(198) ^ _D(196) ^
        _D(194) ^ _D(192) ^ _D(190) ^ _D(188) ^ _D(187) ^ _D(186) ^
        _D(183) ^ _D(182) ^ _D(179) ^ _D(178) ^ _D(175) ^ _D(174) ^
        _D(172) ^ _D(168) ^ _D(164) ^ _D(160) ^ _D(157) ^ _D(156) ^
        _D(155) ^ _D(154) ^ _D(149) ^ _D(148) ^ _D(147) ^ _D(146) ^
        _D(142) ^ _D(140) ^ _D(134) ^ _D(132) ^ _D(127) ^ _D(126) ^
        _D(119) ^ _D(118) ^ _D(112) ^ _D(104) ^ _D(97) ^ _D(96) ^
        _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(82) ^
        _D(80) ^ _D(78) ^ _D(76) ^ _D(67) ^ _D(66) ^ _D(63) ^ _D(62) ^
        _D(52) ^ _D(48) ^ _D(37) ^ _D(36) ^ _D(35) ^ _D(34) ^ _D(22) ^
        _D(20) ^ _D(7) ^ _D(6) ^ _C(8) ^ _C(10) ^ _C(11) ^ _C(13);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(278) ^ _D(276) ^ _D(275) ^ _D(273) ^ _D(263) ^ _D(262) ^
        _D(260) ^ _D(259) ^ _D(248) ^ _D(245) ^ _D(233) ^ _D(232) ^
        _D(231) ^ _D(218) ^ _D(216) ^ _D(215) ^ _D(214) ^ _D(213) ^
        _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^ _D(207) ^
        _D(206) ^ _D(205) ^ _D(204) ^ _D(201) ^ _D(199) ^ _D(197) ^
        _D(195) ^ _D(193) ^ _D(191) ^ _D(189) ^ _D(188) ^ _D(187) ^
        _D(184) ^ _D(183) ^ _D(180) ^ _D(179) ^ _D(176) ^ _D(175) ^
        _D(173) ^ _D(169) ^ _D(165) ^ _D(161) ^ _D(158) ^ _D(157) ^
        _D(156) ^ _D(155) ^ _D(150) ^ _D(149) ^ _D(148) ^ _D(147) ^
        _D(143) ^ _D(141) ^ _D(135) ^ _D(133) ^ _D(128) ^ _D(127) ^
        _D(120) ^ _D(119) ^ _D(113) ^ _D(105) ^ _D(98) ^ _D(97) ^
        _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(83) ^
        _D(81) ^ _D(79) ^ _D(77) ^ _D(68) ^ _D(67) ^ _D(64) ^ _D(63) ^
        _D(53) ^ _D(49) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(35) ^ _D(23) ^
        _D(21) ^ _D(8) ^ _D(7) ^ _C(9) ^ _C(11) ^ _C(12) ^ _C(14);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(277) ^ _D(276) ^ _D(274) ^ _D(264) ^ _D(263) ^
        _D(261) ^ _D(260) ^ _D(249) ^ _D(246) ^ _D(234) ^ _D(233) ^
        _D(232) ^ _D(219) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^
        _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^
        _D(207) ^ _D(206) ^ _D(205) ^ _D(202) ^ _D(200) ^ _D(198) ^
        _D(196) ^ _D(194) ^ _D(192) ^ _D(190) ^ _D(189) ^ _D(188) ^
        _D(185) ^ _D(184) ^ _D(181) ^ _D(180) ^ _D(177) ^ _D(176) ^
        _D(174) ^ _D(170) ^ _D(166) ^ _D(162) ^ _D(159) ^ _D(158) ^
        _D(157) ^ _D(156) ^ _D(151) ^ _D(150) ^ _D(149) ^ _D(148) ^
        _D(144) ^ _D(142) ^ _D(136) ^ _D(134) ^ _D(129) ^ _D(128) ^
        _D(121) ^ _D(120) ^ _D(114) ^ _D(106) ^ _D(99) ^ _D(98) ^
        _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(84) ^
        _D(82) ^ _D(80) ^ _D(78) ^ _D(69) ^ _D(68) ^ _D(65) ^ _D(64) ^
        _D(54) ^ _D(50) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(24) ^
        _D(22) ^ _D(9) ^ _D(8) ^ _C(0) ^ _C(10) ^ _C(12) ^ _C(13) ^
        _C(15);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(278) ^ _D(277) ^ _D(275) ^ _D(265) ^ _D(264) ^ _D(262) ^
        _D(261) ^ _D(250) ^ _D(247) ^ _D(235) ^ _D(234) ^ _D(233) ^
        _D(220) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^
        _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^
        _D(207) ^ _D(206) ^ _D(203) ^ _D(201) ^ _D(199) ^ _D(197) ^
        _D(195) ^ _D(193) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(186) ^
        _D(185) ^ _D(182) ^ _D(181) ^ _D(178) ^ _D(177) ^ _D(175) ^
        _D(171) ^ _D(167) ^ _D(163) ^ _D(160) ^ _D(159) ^ _D(158) ^
        _D(157) ^ _D(152) ^ _D(151) ^ _D(150) ^ _D(149) ^ _D(145) ^
        _D(143) ^ _D(137) ^ _D(135) ^ _D(130) ^ _D(129) ^ _D(122) ^
        _D(121) ^ _D(115) ^ _D(107) ^ _D(100) ^ _D(99) ^ _D(98) ^
        _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(85) ^ _D(83) ^
        _D(81) ^ _D(79) ^ _D(70) ^ _D(69) ^ _D(66) ^ _D(65) ^ _D(55) ^
        _D(51) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(25) ^ _D(23) ^
        _D(10) ^ _D(9) ^ _C(0) ^ _C(1) ^ _C(11) ^ _C(13) ^ _C(14);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(278) ^ _D(276) ^ _D(266) ^ _D(265) ^ _D(263) ^
        _D(262) ^ _D(251) ^ _D(248) ^ _D(236) ^ _D(235) ^ _D(234) ^
        _D(221) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^
        _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^
        _D(208) ^ _D(207) ^ _D(204) ^ _D(202) ^ _D(200) ^ _D(198) ^
        _D(196) ^ _D(194) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(187) ^
        _D(186) ^ _D(183) ^ _D(182) ^ _D(179) ^ _D(178) ^ _D(176) ^
        _D(172) ^ _D(168) ^ _D(164) ^ _D(161) ^ _D(160) ^ _D(159) ^
        _D(158) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(150) ^ _D(146) ^
        _D(144) ^ _D(138) ^ _D(136) ^ _D(131) ^ _D(130) ^ _D(123) ^
        _D(122) ^ _D(116) ^ _D(108) ^ _D(101) ^ _D(100) ^ _D(99) ^
        _D(98) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(86) ^ _D(84) ^
        _D(82) ^ _D(80) ^ _D(71) ^ _D(70) ^ _D(67) ^ _D(66) ^ _D(56) ^
        _D(52) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(26) ^ _D(24) ^
        _D(11) ^ _D(10) ^ _C(1) ^ _C(2) ^ _C(12) ^ _C(14) ^ _C(15);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(279) ^ _D(277) ^ _D(267) ^ _D(266) ^ _D(264) ^ _D(263) ^
        _D(252) ^ _D(249) ^ _D(237) ^ _D(236) ^ _D(235) ^ _D(222) ^
        _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^
        _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^
        _D(208) ^ _D(205) ^ _D(203) ^ _D(201) ^ _D(199) ^ _D(197) ^
        _D(195) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(188) ^ _D(187) ^
        _D(184) ^ _D(183) ^ _D(180) ^ _D(179) ^ _D(177) ^ _D(173) ^
        _D(169) ^ _D(165) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(159) ^
        _D(154) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(147) ^ _D(145) ^
        _D(139) ^ _D(137) ^ _D(132) ^ _D(131) ^ _D(124) ^ _D(123) ^
        _D(117) ^ _D(109) ^ _D(102) ^ _D(101) ^ _D(100) ^ _D(99) ^
        _D(98) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(87) ^ _D(85) ^ _D(83) ^
        _D(81) ^ _D(72) ^ _D(71) ^ _D(68) ^ _D(67) ^ _D(57) ^ _D(53) ^
        _D(42) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(27) ^ _D(25) ^ _D(12) ^
        _D(11) ^ _C(0) ^ _C(2) ^ _C(3) ^ _C(13) ^ _C(15);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(278) ^ _D(268) ^ _D(267) ^ _D(265) ^ _D(264) ^ _D(253) ^
        _D(250) ^ _D(238) ^ _D(237) ^ _D(236) ^ _D(223) ^ _D(221) ^
        _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^
        _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^
        _D(206) ^ _D(204) ^ _D(202) ^ _D(200) ^ _D(198) ^ _D(196) ^
        _D(194) ^ _D(193) ^ _D(192) ^ _D(189) ^ _D(188) ^ _D(185) ^
        _D(184) ^ _D(181) ^ _D(180) ^ _D(178) ^ _D(174) ^ _D(170) ^
        _D(166) ^ _D(163) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(155) ^
        _D(154) ^ _D(153) ^ _D(152) ^ _D(148) ^ _D(146) ^ _D(140) ^
        _D(138) ^ _D(133) ^ _D(132) ^ _D(125) ^ _D(124) ^ _D(118) ^
        _D(110) ^ _D(103) ^ _D(102) ^ _D(101) ^ _D(100) ^ _D(99) ^
        _D(98) ^ _D(97) ^ _D(96) ^ _D(88) ^ _D(86) ^ _D(84) ^ _D(82) ^
        _D(73) ^ _D(72) ^ _D(69) ^ _D(68) ^ _D(58) ^ _D(54) ^ _D(43) ^
        _D(42) ^ _D(41) ^ _D(40) ^ _D(28) ^ _D(26) ^ _D(13) ^ _D(12) ^
        _C(0) ^ _C(1) ^ _C(3) ^ _C(4) ^ _C(14);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;

    tmpCrc =
        _D(278) ^ _D(277) ^ _D(276) ^ _D(275) ^ _D(274) ^ _D(273) ^
        _D(272) ^ _D(271) ^ _D(270) ^ _D(269) ^ _D(267) ^ _D(266) ^
        _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^
        _D(258) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(254) ^ _D(250) ^
        _D(249) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(244) ^
        _D(243) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(239) ^ _D(237) ^
        _D(223) ^ _D(222) ^ _D(220) ^ _D(218) ^ _D(216) ^ _D(214) ^
        _D(212) ^ _D(210) ^ _D(206) ^ _D(205) ^ _D(202) ^ _D(201) ^
        _D(198) ^ _D(197) ^ _D(194) ^ _D(192) ^ _D(191) ^ _D(190) ^
        _D(188) ^ _D(187) ^ _D(186) ^ _D(184) ^ _D(183) ^ _D(182) ^
        _D(180) ^ _D(179) ^ _D(174) ^ _D(173) ^ _D(172) ^ _D(171) ^
        _D(166) ^ _D(165) ^ _D(164) ^ _D(162) ^ _D(160) ^ _D(159) ^
        _D(158) ^ _D(157) ^ _D(156) ^ _D(154) ^ _D(152) ^ _D(151) ^
        _D(150) ^ _D(149) ^ _D(146) ^ _D(145) ^ _D(144) ^ _D(143) ^
        _D(142) ^ _D(141) ^ _D(138) ^ _D(137) ^ _D(136) ^ _D(135) ^
        _D(134) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(129) ^ _D(128) ^
        _D(127) ^ _D(126) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^
        _D(120) ^ _D(119) ^ _D(110) ^ _D(109) ^ _D(108) ^ _D(107) ^
        _D(106) ^ _D(105) ^ _D(104) ^ _D(102) ^ _D(100) ^ _D(98) ^
        _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^
        _D(89) ^ _D(86) ^ _D(85) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^
        _D(78) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(72) ^ _D(71) ^
        _D(70) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(64) ^ _D(63) ^
        _D(62) ^ _D(61) ^ _D(60) ^ _D(59) ^ _D(54) ^ _D(53) ^ _D(52) ^
        _D(51) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(46) ^ _D(45) ^
        _D(44) ^ _D(42) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(36) ^
        _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(30) ^ _D(29) ^
        _D(26) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(21) ^ _D(20) ^
        _D(19) ^ _D(18) ^ _D(17) ^ _D(16) ^ _D(15) ^ _D(14) ^ _D(12) ^
        _D(11) ^ _D(10) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(6) ^ _D(5) ^
        _D(4) ^ _D(3) ^ _D(2) ^ _D(1) ^ _D(0) ^ _C(0) ^ _C(2) ^ _C(3) ^
        _C(5) ^ _C(6) ^ _C(7) ^ _C(8) ^ _C(9) ^ _C(10) ^ _C(11) ^
        _C(12) ^ _C(13) ^ _C(14);

    setBitFromValue(newCrc,tmpCrc,index);
    index++;
    newCrc |= (tmpCrc << 15) & 0x8000;

    return newCrc;
}

/**
* @internal simCalcHashFor70BytesCrc16 function
* @endinternal
*
* @brief   Calculate CRC 16 bit for 70 bytes input data.
*
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       16 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc16
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
)
{
    GT_U32 newCrc;

    if(byteNum != 70)
    {
        skernelFatalError("simCalcHashFor70BytesCrc16: number of bytes[%d] != 70 \n", byteNum);

    }

    newCrc = simCalcCrc16_280(crc   , &bufPtr[35]);/*[559:280]*/
    newCrc = simCalcCrc16_280(newCrc, &bufPtr[ 0]);  /*[279:0]*/

    return newCrc;
}

/**
* @internal simCalcHashFor70BytesCrc32 function
* @endinternal
*
* @brief   Calculate CRC 32 bit for 70 bytes input data.
*         CRC-32 hash value calculation is performed using the CRC-32-IEEE 802.3 polynomial
* @param[in] crc                      - CRC start value.
* @param[in] bufPtr                   - pointer to the  (of 70 bytes)
* @param[in] byteNum                  - number of bytes in the bufPtr -- MUST be 70 bytes
*                                       32 bit CRC.
*/
GT_U32 simCalcHashFor70BytesCrc32
(
    IN GT_U32  crc,
    IN GT_U8  *bufPtr,
    IN GT_U32 byteNum
)
{
/***************************************************
	 http://www.easics.com/webtools/crctool
	 polynomial: (0 1 2 4 5 7 8 10 11 12 16 22 23 26 32)
	 data width: 560
	 convention: the first serial bit is d[559)
***************************************************/
	GT_U32 newCrc = 0;
	GT_U32 tmpCrc;
	GT_U32 index = 0;/* bit index in final CRC */

	tmpCrc =
	_D(558) ^ _D(557) ^ _D(556) ^ _D(555) ^ _D(554) ^ _D(551) ^ _D(549) ^ _D(545) ^ _D(542) ^ _D(540) ^ _D(539) ^ _D(535) ^ _D(534) ^ _D(532) ^ _D(531) ^ _D(530) ^_D(529) ^ _D(528) ^ _D(526) ^ _D(525) ^ _D(522) ^ _D(521) ^ _D(519) ^ _D(518) ^ _D(516) ^ _D(514) ^ _D(512) ^ _D(511) ^ _D(510) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(502) ^ _D(501) ^ _D(500) ^ _D(495) ^ _D(494) ^ _D(493) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(486) ^ _D(483) ^ _D(482) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(477) ^ _D(476) ^ _D(472) ^ _D(470) ^ _D(468) ^ _D(465) ^ _D(464) ^ _D(462) ^ _D(461) ^ _D(458) ^ _D(452) ^ _D(450) ^ _D(449) ^ _D(448) ^ _D(444) ^ _D(437) ^ _D(436) ^ _D(434) ^ _D(433) ^ _D(424) ^ _D(422) ^ _D(419) ^ _D(418) ^ _D(416) ^ _D(414) ^ _D(412) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(405) ^ _D(404) ^ _D(400) ^ _D(399) ^ _D(398) ^ _D(396) ^ _D(393) ^ _D(392) ^ _D(391) ^ _D(390) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(381) ^ _D(378) ^ _D(376) ^ _D(374) ^ _D(372) ^ _D(369) ^ _D(368) ^ _D(366) ^ _D(363) ^ _D(362) ^ _D(359) ^ _D(358) ^ _D(357) ^ _D(353) ^ _D(349) ^ _D(348) ^ _D(347) ^ _D(345) ^ _D(344) ^ _D(342) ^ _D(341) ^ _D(339) ^ _D(338) ^ _D(337) ^ _D(335) ^ _D(334) ^ _D(333) ^ _D(328) ^ _D(327) ^ _D(322) ^ _D(321) ^ _D(320) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(315) ^ _D(312) ^ _D(310) ^ _D(309) ^ _D(305) ^ _D(303) ^ _D(302) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(287) ^ _D(286) ^ _D(283) ^ _D(279) ^ _D(277) ^ _D(276) ^ _D(274) ^ _D(273) ^ _D(269) ^ _D(268) ^ _D(265) ^ _D(264) ^ _D(261) ^ _D(259) ^ _D(257) ^ _D(255) ^ _D(252) ^ _D(248) ^ _D(243) ^ _D(237) ^ _D(234) ^ _D(230) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(224) ^ _D(216) ^ _D(214) ^ _D(212) ^ _D(210) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(199) ^ _D(198) ^ _D(197) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(188) ^ _D(186) ^ _D(183) ^ _D(182) ^ _D(172) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(167) ^ _D(166) ^ _D(162) ^ _D(161) ^ _D(158) ^ _D(156) ^ _D(155) ^ _D(151) ^ _D(149) ^ _D(144) ^ _D(143) ^ _D(137) ^ _D(136) ^ _D(135) ^ _D(134) ^ _D(132) ^ _D(128) ^ _D(127) ^ _D(126) ^ _D(125) ^ _D(123) ^ _D(119) ^ _D(118) ^ _D(117) ^ _D(116) ^ _D(114) ^ _D(113) ^ _D(111) ^ _D(110) ^ _D(106) ^ _D(104) ^ _D(103) ^ _D(101) ^ _D(99) ^ _D(98) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(87) ^ _D(85) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(79) ^ _D(73) ^ _D(72) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(63) ^ _D(61) ^ _D(60) ^ _D(58) ^ _D(55) ^ _D(54) ^ _D(53) ^ _D(50) ^ _D(48) ^ _D(47) ^ _D(45) ^ _D(44) ^ _D(37) ^ _D(34) ^ _D(32) ^ _D(31) ^ _D(30) ^ _D(29) ^ _D(28) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(16) ^ _D(12) ^ _D(10) ^ _D(9) ^ _D(6) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(11) ^ _C(12) ^ _C(14) ^ _C(17) ^ _C(21) ^ _C(23) ^ _C(26) ^ _C(27) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
	_D(559) ^ _D(554) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(546) ^ _D(545) ^ _D(543) ^ _D(542) ^ _D(541) ^ _D(539) ^ _D(536) ^ _D(534) ^ _D(533) ^ _D(528) ^_D(527) ^ _D(525) ^ _D(523) ^ _D(521) ^ _D(520) ^ _D(518) ^ _D(517) ^ _D(516) ^ _D(515) ^ _D(514) ^ _D(513) ^ _D(510) ^ _D(509) ^ _D(506) ^ _D(503) ^ _D(500) ^ _D(496) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(484) ^ _D(479) ^ _D(478) ^ _D(476) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(470) ^ _D(469) ^ _D(468) ^ _D(466) ^ _D(464) ^ _D(463) ^ _D(461) ^ _D(459) ^ _D(458) ^ _D(453) ^ _D(452) ^ _D(451) ^ _D(448) ^ _D(445) ^ _D(444) ^ _D(438) ^ _D(436) ^ _D(435) ^ _D(433) ^ _D(425) ^ _D(424) ^ _D(423) ^ _D(422) ^ _D(420) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(413) ^ _D(412) ^ _D(410) ^ _D(407) ^ _D(406) ^ _D(404) ^ _D(401) ^ _D(398) ^ _D(397) ^ _D(396) ^ _D(394) ^ _D(390) ^ _D(389) ^ _D(386) ^ _D(382) ^ _D(381) ^ _D(379) ^ _D(378) ^ _D(377) ^ _D(376) ^ _D(375) ^ _D(374) ^ _D(373) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(367) ^ _D(366) ^ _D(364) ^ _D(362) ^ _D(360) ^ _D(357) ^ _D(354) ^ _D(353) ^ _D(350) ^ _D(347) ^ _D(346) ^ _D(344) ^ _D(343) ^ _D(341) ^ _D(340) ^ _D(337) ^ _D(336) ^ _D(333) ^ _D(329) ^ _D(327) ^ _D(323) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(313) ^ _D(312) ^ _D(311) ^ _D(309) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(302) ^ _D(301) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(289) ^ _D(286) ^ _D(284) ^ _D(283) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(276) ^ _D(275) ^ _D(273) ^ _D(270) ^ _D(268) ^ _D(266) ^ _D(264) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(253) ^ _D(252) ^ _D(249) ^ _D(248) ^ _D(244) ^ _D(243) ^ _D(238) ^ _D(237) ^ _D(235) ^ _D(234) ^ _D(231) ^ _D(230) ^ _D(229) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(207) ^ _D(204) ^ _D(201) ^ _D(200) ^ _D(197) ^ _D(195) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(184) ^ _D(182) ^ _D(173) ^ _D(169) ^ _D(168) ^ _D(166) ^ _D(163) ^ _D(161) ^ _D(159) ^ _D(158) ^ _D(157) ^ _D(155) ^ _D(152) ^ _D(151) ^ _D(150) ^ _D(149) ^ _D(145) ^ _D(143) ^ _D(138) ^ _D(134) ^ _D(133) ^ _D(132) ^ _D(129) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(120) ^ _D(116) ^ _D(115) ^ _D(113) ^ _D(112) ^ _D(110) ^ _D(107) ^ _D(106) ^ _D(105) ^ _D(103) ^ _D(102) ^ _D(101) ^ _D(100) ^ _D(94) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(74) ^ _D(72) ^ _D(69) ^ _D(65) ^ _D(64) ^ _D(63) ^ _D(62) ^ _D(60) ^ _D(59) ^ _D(58) ^ _D(56) ^ _D(53) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(47) ^ _D(46) ^ _D(44) ^ _D(38) ^ _D(37) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(28) ^ _D(27) ^ _D(24) ^ _D(17) ^ _D(16) ^ _D(13) ^ _D(12) ^ _D(11) ^ _D(9) ^ _D(7) ^ _D(6) ^ _D(1) ^ _D(0) ^ _C(0) ^ _C(5) ^ _C(6) ^ _C(8) ^ _C(11) ^ _C(13) ^ _C(14) ^ _C(15) ^ _C(17) ^ _C(18) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(26) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(556) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(550) ^ _D(549) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(543) ^ _D(539) ^ _D(537) ^ _D(532) ^ _D(531) ^ _D(530) ^ _D(525) ^ _D(524) ^ _D(517) ^ _D(515) ^ _D(512) ^ _D(508) ^ _D(506) ^ _D(504) ^ _D(502) ^ _D(500) ^ _D(497) ^ _D(495) ^ _D(494) ^ _D(493) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(483) ^ _D(482) ^ _D(481) ^ _D(476) ^ _D(474) ^ _D(473) ^ _D(471) ^ _D(469) ^ _D(468) ^ _D(467) ^ _D(461) ^ _D(460) ^ _D(459) ^ _D(458) ^ _D(454) ^ _D(453) ^ _D(450) ^ _D(448) ^ _D(446) ^ _D(445) ^ _D(444) ^ _D(439) ^ _D(433) ^ _D(426) ^ _D(425) ^ _D(423) ^ _D(422) ^ _D(421) ^ _D(417) ^ _D(415) ^ _D(413) ^ _D(412) ^ _D(411) ^ _D(409) ^ _D(404) ^ _D(402) ^ _D(400) ^ _D(397) ^ _D(396) ^ _D(395) ^ _D(393) ^ _D(392) ^ _D(388) ^ _D(386) ^ _D(383) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(379) ^ _D(377) ^ _D(375) ^ _D(373) ^ _D(372) ^ _D(371) ^ _D(367) ^ _D(366) ^ _D(365) ^ _D(362) ^ _D(361) ^ _D(359) ^ _D(357) ^ _D(355) ^ _D(354) ^ _D(353) ^ _D(351) ^ _D(349) ^ _D(339) ^ _D(335) ^ _D(333) ^ _D(330) ^ _D(327) ^ _D(324) ^ _D(322) ^ _D(321) ^ _D(320) ^ _D(319) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(309) ^ _D(307) ^ _D(306) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(296) ^ _D(293) ^ _D(291) ^ _D(288) ^ _D(286) ^ _D(285) ^ _D(284) ^ _D(283) ^ _D(281) ^ _D(280) ^ _D(273) ^ _D(271) ^ _D(268) ^ _D(267) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(260) ^ _D(258) ^ _D(256) ^ _D(255) ^ _D(254) ^ _D(253) ^ _D(252) ^ _D(250) ^ _D(249) ^ _D(248) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(239) ^ _D(238) ^ _D(237) ^ _D(236) ^ _D(235) ^ _D(234) ^ _D(232) ^ _D(231) ^ _D(228) ^ _D(225) ^ _D(224) ^ _D(218) ^ _D(217) ^ _D(215) ^ _D(213) ^ _D(210) ^ _D(209) ^ _D(207) ^ _D(205) ^ _D(203) ^ _D(199) ^ _D(197) ^ _D(196) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(189) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(182) ^ _D(174) ^ _D(172) ^ _D(171) ^ _D(166) ^ _D(164) ^ _D(161) ^ _D(160) ^ _D(159) ^ _D(155) ^ _D(153) ^ _D(152) ^ _D(150) ^ _D(149) ^ _D(146) ^ _D(143) ^ _D(139) ^ _D(137) ^ _D(136) ^ _D(133) ^ _D(132) ^ _D(130) ^ _D(128) ^ _D(127) ^ _D(124) ^ _D(123) ^ _D(121) ^ _D(119) ^ _D(118) ^ _D(110) ^ _D(108) ^ _D(107) ^ _D(102) ^ _D(99) ^ _D(98) ^ _D(97) ^ _D(96) ^ _D(94) ^ _D(89) ^ _D(88) ^ _D(85) ^ _D(84) ^ _D(83) ^ _D(80) ^ _D(79) ^ _D(75) ^ _D(72) ^ _D(70) ^ _D(68) ^ _D(67) ^ _D(64) ^ _D(59) ^ _D(58) ^ _D(57) ^ _D(55) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(44) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(35) ^ _D(32) ^ _D(31) ^ _D(30) ^ _D(26) ^ _D(24) ^ _D(18) ^ _D(17) ^ _D(16) ^ _D(14) ^ _D(13) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(6) ^ _D(2) ^ _D(1) ^ _D(0) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(9) ^ _C(11) ^ _C(15) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(21) ^ _C(22) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(557) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(551) ^ _D(550) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(540) ^ _D(538) ^ _D(533) ^ _D(532) ^ _D(531) ^ _D(526) ^ _D(525) ^ _D(518) ^ _D(516) ^ _D(513) ^ _D(509) ^ _D(507) ^ _D(505) ^ _D(503) ^ _D(501) ^ _D(498) ^ _D(496) ^ _D(495) ^ _D(494) ^ _D(493) ^ _D(492) ^ _D(491) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(484) ^ _D(483) ^ _D(482) ^ _D(477) ^ _D(475) ^ _D(474) ^ _D(472) ^ _D(470) ^ _D(469) ^ _D(468) ^ _D(462) ^ _D(461) ^ _D(460) ^ _D(459) ^ _D(455) ^ _D(454) ^ _D(451) ^ _D(449) ^ _D(447) ^ _D(446) ^ _D(445) ^ _D(440) ^ _D(434) ^ _D(427) ^ _D(426) ^ _D(424) ^ _D(423) ^ _D(422) ^ _D(418) ^ _D(416) ^ _D(414) ^ _D(413) ^ _D(412) ^ _D(410) ^ _D(405) ^ _D(403) ^ _D(401) ^ _D(398) ^ _D(397) ^ _D(396) ^ _D(394) ^ _D(393) ^ _D(389) ^ _D(387) ^ _D(384) ^ _D(383) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(378) ^ _D(376) ^ _D(374) ^ _D(373) ^ _D(372) ^ _D(368) ^ _D(367) ^ _D(366) ^ _D(363) ^ _D(362) ^ _D(360) ^ _D(358) ^ _D(356) ^ _D(355) ^ _D(354) ^ _D(352) ^ _D(350) ^ _D(340) ^ _D(336) ^ _D(334) ^ _D(331) ^ _D(328) ^ _D(325) ^ _D(323) ^ _D(322) ^ _D(321) ^ _D(320) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(294) ^ _D(292) ^ _D(289) ^ _D(287) ^ _D(286) ^ _D(285) ^ _D(284) ^ _D(282) ^ _D(281) ^ _D(274) ^ _D(272) ^ _D(269) ^ _D(268) ^ _D(265) ^ _D(264) ^ _D(263) ^ _D(261) ^ _D(259) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(254) ^ _D(253) ^ _D(251) ^ _D(250) ^ _D(249) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(240) ^ _D(239) ^ _D(238) ^ _D(237) ^ _D(236) ^ _D(235) ^ _D(233) ^ _D(232) ^ _D(229) ^ _D(226) ^ _D(225) ^ _D(219) ^ _D(218) ^ _D(216) ^ _D(214) ^ _D(211) ^ _D(210) ^ _D(208) ^ _D(206) ^ _D(204) ^ _D(200) ^ _D(198) ^ _D(197) ^ _D(195) ^ _D(194) ^ _D(193) ^ _D(190) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(183) ^ _D(175) ^ _D(173) ^ _D(172) ^ _D(167) ^ _D(165) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(156) ^ _D(154) ^ _D(153) ^ _D(151) ^ _D(150) ^ _D(147) ^ _D(144) ^ _D(140) ^ _D(138) ^ _D(137) ^ _D(134) ^ _D(133) ^ _D(131) ^ _D(129) ^ _D(128) ^ _D(125) ^ _D(124) ^ _D(122) ^ _D(120) ^ _D(119) ^ _D(111) ^ _D(109) ^ _D(108) ^ _D(103) ^ _D(100) ^ _D(99) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(90) ^ _D(89) ^ _D(86) ^ _D(85) ^ _D(84) ^ _D(81) ^ _D(80) ^ _D(76) ^ _D(73) ^ _D(71) ^ _D(69) ^ _D(68) ^ _D(65) ^ _D(60) ^ _D(59) ^ _D(58) ^ _D(56) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(45) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(27) ^ _D(25) ^ _D(19) ^ _D(18) ^ _D(17) ^ _D(15) ^ _D(14) ^ _D(10) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(3) ^ _D(2) ^ _D(1) ^ _C(3) ^ _C(4) ^ _C(5) ^ _C(10) ^ _C(12) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(22) ^ _C(23) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(29) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(557) ^ _D(552) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(542) ^ _D(541) ^ _D(540) ^ _D(535) ^ _D(533) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(525) ^ _D(522) ^ _D(521) ^ _D(518) ^ _D(517) ^ _D(516) ^ _D(512) ^ _D(511) ^ _D(507) ^ _D(504) ^ _D(501) ^ _D(500) ^ _D(499) ^ _D(497) ^ _D(496) ^ _D(491) ^ _D(490) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(484) ^ _D(482) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(475) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(469) ^ _D(468) ^ _D(465) ^ _D(464) ^ _D(463) ^ _D(460) ^ _D(458) ^ _D(456) ^ _D(455) ^ _D(449) ^ _D(447) ^ _D(446) ^ _D(444) ^ _D(441) ^ _D(437) ^ _D(436) ^ _D(435) ^ _D(434) ^ _D(433) ^ _D(428) ^ _D(427) ^ _D(425) ^ _D(423) ^ _D(422) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(413) ^ _D(412) ^ _D(411) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(402) ^ _D(400) ^ _D(397) ^ _D(396) ^ _D(395) ^ _D(394) ^ _D(393) ^ _D(392) ^ _D(391) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(383) ^ _D(382) ^ _D(379) ^ _D(378) ^ _D(377) ^ _D(376) ^ _D(375) ^ _D(373) ^ _D(372) ^ _D(367) ^ _D(366) ^ _D(364) ^ _D(362) ^ _D(361) ^ _D(358) ^ _D(356) ^ _D(355) ^ _D(351) ^ _D(349) ^ _D(348) ^ _D(347) ^ _D(345) ^ _D(344) ^ _D(342) ^ _D(339) ^ _D(338) ^ _D(334) ^ _D(333) ^ _D(332) ^ _D(329) ^ _D(328) ^ _D(327) ^ _D(326) ^ _D(324) ^ _D(323) ^ _D(320) ^ _D(319) ^ _D(316) ^ _D(312) ^ _D(311) ^ _D(310) ^ _D(308) ^ _D(305) ^ _D(303) ^ _D(301) ^ _D(297) ^ _D(296) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(285) ^ _D(282) ^ _D(279) ^ _D(277) ^ _D(276) ^ _D(275) ^ _D(274) ^ _D(270) ^ _D(268) ^ _D(266) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(256) ^ _D(254) ^ _D(251) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(243) ^ _D(241) ^ _D(240) ^ _D(239) ^ _D(238) ^ _D(236) ^ _D(233) ^ _D(228) ^ _D(224) ^ _D(220) ^ _D(219) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^ _D(211) ^ _D(210) ^ _D(208) ^ _D(205) ^ _D(203) ^ _D(202) ^ _D(197) ^ _D(196) ^ _D(195) ^ _D(193) ^ _D(192) ^ _D(190) ^ _D(189) ^ _D(187) ^ _D(186) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(176) ^ _D(174) ^ _D(173) ^ _D(172) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(163) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(154) ^ _D(152) ^ _D(149) ^ _D(148) ^ _D(145) ^ _D(144) ^ _D(143) ^ _D(141) ^ _D(139) ^ _D(138) ^ _D(137) ^ _D(136) ^ _D(130) ^ _D(129) ^ _D(128) ^ _D(127) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(118) ^ _D(117) ^ _D(116) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(109) ^ _D(106) ^ _D(103) ^ _D(100) ^ _D(97) ^ _D(95) ^ _D(94) ^ _D(91) ^ _D(90) ^ _D(86) ^ _D(84) ^ _D(83) ^ _D(79) ^ _D(77) ^ _D(74) ^ _D(73) ^ _D(70) ^ _D(69) ^ _D(68) ^ _D(67) ^ _D(65) ^ _D(63) ^ _D(59) ^ _D(58) ^ _D(57) ^ _D(50) ^ _D(48) ^ _D(47) ^ _D(46) ^ _D(45) ^ _D(44) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(38) ^ _D(33) ^ _D(31) ^ _D(30) ^ _D(29) ^ _D(25) ^ _D(24) ^ _D(20) ^ _D(19) ^ _D(18) ^ _D(15) ^ _D(12) ^ _D(11) ^ _D(8) ^ _D(6) ^ _D(4) ^ _D(3) ^ _D(2) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(5) ^ _C(7) ^ _C(12) ^ _C(13) ^ _C(14) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(24) ^ _C(29) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(556) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(551) ^ _D(548) ^ _D(547) ^ _D(545) ^ _D(543) ^ _D(541) ^ _D(540) ^ _D(539) ^ _D(536) ^ _D(535) ^ _D(525) ^ _D(523) ^ _D(521) ^ _D(517) ^ _D(516) ^ _D(514) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(498) ^ _D(497) ^ _D(495) ^ _D(494) ^ _D(493) ^ _D(490) ^ _D(489) ^ _D(487) ^ _D(485) ^ _D(478) ^ _D(477) ^ _D(474) ^ _D(473) ^ _D(469) ^ _D(468) ^ _D(466) ^ _D(462) ^ _D(459) ^ _D(458) ^ _D(457) ^ _D(456) ^ _D(452) ^ _D(449) ^ _D(447) ^ _D(445) ^ _D(444) ^ _D(442) ^ _D(438) ^ _D(435) ^ _D(433) ^ _D(429) ^ _D(428) ^ _D(426) ^ _D(423) ^ _D(422) ^ _D(417) ^ _D(413) ^ _D(410) ^ _D(406) ^ _D(405) ^ _D(404) ^ _D(403) ^ _D(401) ^ _D(400) ^ _D(399) ^ _D(397) ^ _D(395) ^ _D(394) ^ _D(391) ^ _D(390) ^ _D(385) ^ _D(384) ^ _D(383) ^ _D(381) ^ _D(380) ^ _D(379) ^ _D(377) ^ _D(373) ^ _D(372) ^ _D(369) ^ _D(367) ^ _D(366) ^ _D(365) ^ _D(358) ^ _D(356) ^ _D(353) ^ _D(352) ^ _D(350) ^ _D(347) ^ _D(346) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(338) ^ _D(337) ^ _D(330) ^ _D(329) ^ _D(325) ^ _D(324) ^ _D(322) ^ _D(319) ^ _D(318) ^ _D(315) ^ _D(313) ^ _D(311) ^ _D(310) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(300) ^ _D(299) ^ _D(296) ^ _D(293) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(287) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(275) ^ _D(274) ^ _D(273) ^ _D(271) ^ _D(268) ^ _D(267) ^ _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(260) ^ _D(251) ^ _D(249) ^ _D(247) ^ _D(246) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(239) ^ _D(230) ^ _D(229) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(221) ^ _D(220) ^ _D(218) ^ _D(217) ^ _D(215) ^ _D(214) ^ _D(211) ^ _D(210) ^ _D(208) ^ _D(207) ^ _D(206) ^ _D(204) ^ _D(202) ^ _D(201) ^ _D(199) ^ _D(196) ^ _D(192) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(182) ^ _D(177) ^ _D(175) ^ _D(174) ^ _D(173) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(164) ^ _D(162) ^ _D(161) ^ _D(159) ^ _D(157) ^ _D(156) ^ _D(153) ^ _D(151) ^ _D(150) ^ _D(146) ^ _D(145) ^ _D(143) ^ _D(142) ^ _D(140) ^ _D(139) ^ _D(138) ^ _D(136) ^ _D(135) ^ _D(134) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(129) ^ _D(127) ^ _D(126) ^ _D(125) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(116) ^ _D(115) ^ _D(112) ^ _D(111) ^ _D(107) ^ _D(106) ^ _D(103) ^ _D(99) ^ _D(97) ^ _D(94) ^ _D(92) ^ _D(91) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(75) ^ _D(74) ^ _D(73) ^ _D(72) ^ _D(71) ^ _D(70) ^ _D(69) ^ _D(67) ^ _D(65) ^ _D(64) ^ _D(63) ^ _D(61) ^ _D(59) ^ _D(55) ^ _D(54) ^ _D(53) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(46) ^ _D(44) ^ _D(42) ^ _D(41) ^ _D(40) ^ _D(39) ^ _D(37) ^ _D(29) ^ _D(28) ^ _D(24) ^ _D(21) ^ _D(20) ^ _D(19) ^ _D(13) ^ _D(10) ^ _D(7) ^ _D(6) ^ _D(5) ^ _D(4) ^ _D(3) ^ _D(1) ^ _D(0) ^ _C(7) ^ _C(8) ^ _C(11) ^ _C(12) ^ _C(13) ^ _C(15) ^ _C(17) ^ _C(19) ^ _C(20) ^ _C(23) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(28) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(556) ^ _D(555) ^ _D(554) ^ _D(552) ^ _D(549) ^ _D(548) ^ _D(546) ^ _D(544) ^ _D(542) ^ _D(541) ^ _D(540) ^ _D(537) ^ _D(536) ^ _D(526) ^ _D(524) ^ _D(522) ^ _D(518) ^ _D(517) ^ _D(515) ^ _D(514) ^ _D(512) ^ _D(511) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(499) ^ _D(498) ^ _D(496) ^ _D(495) ^ _D(494) ^ _D(491) ^ _D(490) ^ _D(488) ^ _D(486) ^ _D(479) ^ _D(478) ^ _D(475) ^ _D(474) ^ _D(470) ^ _D(469) ^ _D(467) ^ _D(463) ^ _D(460) ^ _D(459) ^ _D(458) ^ _D(457) ^ _D(453) ^ _D(450) ^ _D(448) ^ _D(446) ^ _D(445) ^ _D(443) ^ _D(439) ^ _D(436) ^ _D(434) ^ _D(430) ^ _D(429) ^ _D(427) ^ _D(424) ^ _D(423) ^ _D(418) ^ _D(414) ^ _D(411) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(404) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(398) ^ _D(396) ^ _D(395) ^ _D(392) ^ _D(391) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(378) ^ _D(374) ^ _D(373) ^ _D(370) ^ _D(368) ^ _D(367) ^ _D(366) ^ _D(359) ^ _D(357) ^ _D(354) ^ _D(353) ^ _D(351) ^ _D(348) ^ _D(347) ^ _D(345) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(339) ^ _D(338) ^ _D(331) ^ _D(330) ^ _D(326) ^ _D(325) ^ _D(323) ^ _D(320) ^ _D(319) ^ _D(316) ^ _D(314) ^ _D(312) ^ _D(311) ^ _D(307) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(301) ^ _D(300) ^ _D(297) ^ _D(294) ^ _D(293) ^ _D(291) ^ _D(289) ^ _D(288) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(276) ^ _D(275) ^ _D(274) ^ _D(272) ^ _D(269) ^ _D(268) ^ _D(266) ^ _D(265) ^ _D(264) ^ _D(263) ^ _D(261) ^ _D(252) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(231) ^ _D(230) ^ _D(229) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(222) ^ _D(221) ^ _D(219) ^ _D(218) ^ _D(216) ^ _D(215) ^ _D(212) ^ _D(211) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(205) ^ _D(203) ^ _D(202) ^ _D(200) ^ _D(197) ^ _D(193) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(183) ^ _D(178) ^ _D(176) ^ _D(175) ^ _D(174) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(165) ^ _D(163) ^ _D(162) ^ _D(160) ^ _D(158) ^ _D(157) ^ _D(154) ^ _D(152) ^ _D(151) ^ _D(147) ^ _D(146) ^ _D(144) ^ _D(143) ^ _D(141) ^ _D(140) ^ _D(139) ^ _D(137) ^ _D(136) ^ _D(135) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(128) ^ _D(127) ^ _D(126) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(117) ^ _D(116) ^ _D(113) ^ _D(112) ^ _D(108) ^ _D(107) ^ _D(104) ^ _D(100) ^ _D(98) ^ _D(95) ^ _D(93) ^ _D(92) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(73) ^ _D(72) ^ _D(71) ^ _D(70) ^ _D(68) ^ _D(66) ^ _D(65) ^ _D(64) ^ _D(62) ^ _D(60) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(47) ^ _D(45) ^ _D(43) ^ _D(42) ^ _D(41) ^ _D(40) ^ _D(38) ^ _D(30) ^ _D(29) ^ _D(25) ^ _D(22) ^ _D(21) ^ _D(20) ^ _D(14) ^ _D(11) ^ _D(8) ^ _D(7) ^ _D(6) ^ _D(5) ^ _D(4) ^ _D(2) ^ _D(1) ^ _C(8) ^ _C(9) ^ _C(12) ^ _C(13) ^ _C(14) ^ _C(16) ^ _C(18) ^ _C(20) ^ _C(21) ^ _C(24) ^ _C(26) ^ _C(27) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(554) ^ _D(553) ^ _D(551) ^ _D(550) ^ _D(547) ^ _D(543) ^ _D(541) ^ _D(540) ^ _D(539) ^ _D(538) ^ _D(537) ^ _D(535) ^ _D(534) ^ _D(532) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(526) ^ _D(523) ^ _D(522) ^ _D(521) ^ _D(515) ^ _D(514) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(509) ^ _D(506) ^ _D(502) ^ _D(501) ^ _D(499) ^ _D(497) ^ _D(496) ^ _D(494) ^ _D(493) ^ _D(490) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(483) ^ _D(482) ^ _D(481) ^ _D(477) ^ _D(475) ^ _D(472) ^ _D(471) ^ _D(465) ^ _D(462) ^ _D(460) ^ _D(459) ^ _D(454) ^ _D(452) ^ _D(451) ^ _D(450) ^ _D(448) ^ _D(447) ^ _D(446) ^ _D(440) ^ _D(436) ^ _D(435) ^ _D(434) ^ _D(433) ^ _D(431) ^ _D(430) ^ _D(428) ^ _D(425) ^ _D(422) ^ _D(418) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(409) ^ _D(406) ^ _D(404) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(398) ^ _D(397) ^ _D(391) ^ _D(390) ^ _D(388) ^ _D(385) ^ _D(383) ^ _D(382) ^ _D(379) ^ _D(378) ^ _D(376) ^ _D(375) ^ _D(372) ^ _D(371) ^ _D(367) ^ _D(366) ^ _D(363) ^ _D(362) ^ _D(360) ^ _D(359) ^ _D(357) ^ _D(355) ^ _D(354) ^ _D(353) ^ _D(352) ^ _D(347) ^ _D(346) ^ _D(343) ^ _D(341) ^ _D(340) ^ _D(338) ^ _D(337) ^ _D(335) ^ _D(334) ^ _D(333) ^ _D(332) ^ _D(331) ^ _D(328) ^ _D(326) ^ _D(324) ^ _D(322) ^ _D(319) ^ _D(318) ^ _D(313) ^ _D(310) ^ _D(309) ^ _D(308) ^ _D(307) ^ _D(306) ^ _D(303) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(297) ^ _D(296) ^ _D(289) ^ _D(288) ^ _D(287) ^ _D(286) ^ _D(283) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(275) ^ _D(274) ^ _D(270) ^ _D(268) ^ _D(267) ^ _D(266) ^ _D(262) ^ _D(261) ^ _D(259) ^ _D(257) ^ _D(255) ^ _D(253) ^ _D(252) ^ _D(251) ^ _D(249) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(242) ^ _D(241) ^ _D(237) ^ _D(234) ^ _D(232) ^ _D(231) ^ _D(229) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(220) ^ _D(219) ^ _D(217) ^ _D(214) ^ _D(213) ^ _D(207) ^ _D(206) ^ _D(204) ^ _D(202) ^ _D(199) ^ _D(197) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(187) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(179) ^ _D(177) ^ _D(176) ^ _D(175) ^ _D(172) ^ _D(171) ^ _D(168) ^ _D(167) ^ _D(164) ^ _D(163) ^ _D(162) ^ _D(159) ^ _D(156) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(149) ^ _D(148) ^ _D(147) ^ _D(145) ^ _D(143) ^ _D(142) ^ _D(141) ^ _D(140) ^ _D(138) ^ _D(135) ^ _D(133) ^ _D(131) ^ _D(129) ^ _D(126) ^ _D(124) ^ _D(122) ^ _D(119) ^ _D(116) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(108) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(93) ^ _D(87) ^ _D(80) ^ _D(79) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(71) ^ _D(69) ^ _D(68) ^ _D(60) ^ _D(58) ^ _D(57) ^ _D(56) ^ _D(54) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(47) ^ _D(46) ^ _D(45) ^ _D(43) ^ _D(42) ^ _D(41) ^ _D(39) ^ _D(37) ^ _D(34) ^ _D(32) ^ _D(29) ^ _D(28) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(21) ^ _D(16) ^ _D(15) ^ _D(10) ^ _D(8) ^ _D(7) ^ _D(5) ^ _D(3) ^ _D(2) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(9) ^ _C(10) ^ _C(11) ^ _C(12) ^ _C(13) ^ _C(15) ^ _C(19) ^ _C(22) ^ _C(23) ^ _C(25) ^ _C(26) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(556) ^ _D(552) ^ _D(549) ^ _D(548) ^ _D(545) ^ _D(544) ^ _D(541) ^ _D(538) ^ _D(536) ^ _D(534) ^ _D(533) ^ _D(527) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(523) ^ _D(521) ^ _D(519) ^ _D(518) ^ _D(515) ^ _D(508) ^ _D(506) ^ _D(503) ^ _D(501) ^ _D(498) ^ _D(497) ^ _D(493) ^ _D(492) ^ _D(490) ^ _D(487) ^ _D(486) ^ _D(484) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(473) ^ _D(470) ^ _D(468) ^ _D(466) ^ _D(465) ^ _D(464) ^ _D(463) ^ _D(462) ^ _D(460) ^ _D(458) ^ _D(455) ^ _D(453) ^ _D(451) ^ _D(450) ^ _D(447) ^ _D(444) ^ _D(441) ^ _D(435) ^ _D(433) ^ _D(432) ^ _D(431) ^ _D(429) ^ _D(426) ^ _D(424) ^ _D(423) ^ _D(422) ^ _D(418) ^ _D(417) ^ _D(415) ^ _D(414) ^ _D(412) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(396) ^ _D(393) ^ _D(390) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(384) ^ _D(383) ^ _D(381) ^ _D(380) ^ _D(379) ^ _D(378) ^ _D(377) ^ _D(374) ^ _D(373) ^ _D(369) ^ _D(367) ^ _D(366) ^ _D(364) ^ _D(362) ^ _D(361) ^ _D(360) ^ _D(359) ^ _D(357) ^ _D(356) ^ _D(355) ^ _D(354) ^ _D(349) ^ _D(345) ^ _D(337) ^ _D(336) ^ _D(332) ^ _D(329) ^ _D(328) ^ _D(325) ^ _D(323) ^ _D(322) ^ _D(321) ^ _D(318) ^ _D(317) ^ _D(315) ^ _D(314) ^ _D(312) ^ _D(311) ^ _D(308) ^ _D(307) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(301) ^ _D(299) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(292) ^ _D(289) ^ _D(286) ^ _D(284) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(277) ^ _D(275) ^ _D(274) ^ _D(273) ^ _D(271) ^ _D(267) ^ _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(254) ^ _D(253) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(242) ^ _D(238) ^ _D(237) ^ _D(235) ^ _D(234) ^ _D(233) ^ _D(232) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(223) ^ _D(221) ^ _D(220) ^ _D(218) ^ _D(216) ^ _D(215) ^ _D(212) ^ _D(210) ^ _D(209) ^ _D(205) ^ _D(202) ^ _D(201) ^ _D(200) ^ _D(199) ^ _D(197) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(182) ^ _D(180) ^ _D(178) ^ _D(177) ^ _D(176) ^ _D(173) ^ _D(171) ^ _D(170) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(165) ^ _D(164) ^ _D(163) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(155) ^ _D(154) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(150) ^ _D(148) ^ _D(146) ^ _D(142) ^ _D(141) ^ _D(139) ^ _D(137) ^ _D(135) ^ _D(130) ^ _D(128) ^ _D(126) ^ _D(120) ^ _D(119) ^ _D(118) ^ _D(116) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(109) ^ _D(107) ^ _D(105) ^ _D(103) ^ _D(101) ^ _D(97) ^ _D(95) ^ _D(88) ^ _D(87) ^ _D(85) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(73) ^ _D(70) ^ _D(69) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(63) ^ _D(60) ^ _D(59) ^ _D(57) ^ _D(54) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(46) ^ _D(45) ^ _D(43) ^ _D(42) ^ _D(40) ^ _D(38) ^ _D(37) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(28) ^ _D(23) ^ _D(22) ^ _D(17) ^ _D(12) ^ _D(11) ^ _D(10) ^ _D(8) ^ _D(4) ^ _D(3) ^ _D(1) ^ _D(0) ^ _C(5) ^ _C(6) ^ _C(8) ^ _C(10) ^ _C(13) ^ _C(16) ^ _C(17) ^ _C(20) ^ _C(21) ^ _C(24) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(557) ^ _D(553) ^ _D(550) ^ _D(549) ^ _D(546) ^ _D(545) ^ _D(542) ^ _D(539) ^ _D(537) ^ _D(535) ^ _D(534) ^ _D(528) ^ _D(527) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(522) ^ _D(520) ^ _D(519) ^ _D(516) ^ _D(509) ^ _D(507) ^ _D(504) ^ _D(502) ^ _D(499) ^ _D(498) ^ _D(494) ^ _D(493) ^ _D(491) ^ _D(488) ^ _D(487) ^ _D(485) ^ _D(482) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(474) ^ _D(471) ^ _D(469) ^ _D(467) ^ _D(466) ^ _D(465) ^ _D(464) ^ _D(463) ^ _D(461) ^ _D(459) ^ _D(456) ^ _D(454) ^ _D(452) ^ _D(451) ^ _D(448) ^ _D(445) ^ _D(442) ^ _D(436) ^ _D(434) ^ _D(433) ^ _D(432) ^ _D(430) ^ _D(427) ^ _D(425) ^ _D(424) ^ _D(423) ^ _D(419) ^ _D(418) ^ _D(416) ^ _D(415) ^ _D(413) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(404) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(397) ^ _D(394) ^ _D(391) ^ _D(390) ^ _D(389) ^ _D(388) ^ _D(385) ^ _D(384) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(379) ^ _D(378) ^ _D(375) ^ _D(374) ^ _D(370) ^ _D(368) ^ _D(367) ^ _D(365) ^ _D(363) ^ _D(362) ^ _D(361) ^ _D(360) ^ _D(358) ^ _D(357) ^ _D(356) ^ _D(355) ^ _D(350) ^ _D(346) ^ _D(338) ^ _D(337) ^ _D(333) ^ _D(330) ^ _D(329) ^ _D(326) ^ _D(324) ^ _D(323) ^ _D(322) ^ _D(319) ^ _D(318) ^ _D(316) ^ _D(315) ^ _D(313) ^ _D(312) ^ _D(309) ^ _D(308) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(302) ^ _D(300) ^ _D(297) ^ _D(296) ^ _D(295) ^ _D(293) ^ _D(290) ^ _D(287) ^ _D(285) ^ _D(283) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(278) ^ _D(276) ^ _D(275) ^ _D(274) ^ _D(272) ^ _D(268) ^ _D(266) ^ _D(265) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(254) ^ _D(251) ^ _D(249) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(243) ^ _D(239) ^ _D(238) ^ _D(236) ^ _D(235) ^ _D(234) ^ _D(233) ^ _D(229) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(224) ^ _D(222) ^ _D(221) ^ _D(219) ^ _D(217) ^ _D(216) ^ _D(213) ^ _D(211) ^ _D(210) ^ _D(206) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(200) ^ _D(198) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(183) ^ _D(181) ^ _D(179) ^ _D(178) ^ _D(177) ^ _D(174) ^ _D(172) ^ _D(171) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(165) ^ _D(164) ^ _D(163) ^ _D(162) ^ _D(161) ^ _D(159) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(155) ^ _D(154) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(149) ^ _D(147) ^ _D(143) ^ _D(142) ^ _D(140) ^ _D(138) ^ _D(136) ^ _D(131) ^ _D(129) ^ _D(127) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(110) ^ _D(108) ^ _D(106) ^ _D(104) ^ _D(102) ^ _D(98) ^ _D(96) ^ _D(89) ^ _D(88) ^ _D(86) ^ _D(85) ^ _D(84) ^ _D(83) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(74) ^ _D(71) ^ _D(70) ^ _D(69) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(64) ^ _D(61) ^ _D(60) ^ _D(58) ^ _D(55) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(47) ^ _D(46) ^ _D(44) ^ _D(43) ^ _D(41) ^ _D(39) ^ _D(38) ^ _D(36) ^ _D(35) ^ _D(34) ^ _D(33) ^ _D(32) ^ _D(29) ^ _D(24) ^ _D(23) ^ _D(18) ^ _D(13) ^ _D(12) ^ _D(11) ^ _D(9) ^ _D(5) ^ _D(4) ^ _D(2) ^ _D(1) ^ _C(0) ^ _C(6) ^ _C(7) ^ _C(9) ^ _C(11) ^ _C(14) ^ _C(17) ^ _C(18) ^ _C(21) ^ _C(22) ^ _C(25) ^ _C(29) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(557) ^ _D(556) ^ _D(555) ^ _D(550) ^ _D(549) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(543) ^ _D(542) ^ _D(539) ^ _D(538) ^ _D(536) ^ _D(534) ^ _D(532) ^ _D(531) ^ _D(530) ^ _D(527) ^ _D(523) ^ _D(522) ^ _D(520) ^ _D(519) ^ _D(518) ^ _D(517) ^ _D(516) ^ _D(514) ^ _D(512) ^ _D(511) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(503) ^ _D(502) ^ _D(501) ^ _D(499) ^ _D(493) ^ _D(491) ^ _D(490) ^ _D(477) ^ _D(476) ^ _D(475) ^ _D(467) ^ _D(466) ^ _D(461) ^ _D(460) ^ _D(458) ^ _D(457) ^ _D(455) ^ _D(453) ^ _D(450) ^ _D(448) ^ _D(446) ^ _D(444) ^ _D(443) ^ _D(436) ^ _D(435) ^ _D(431) ^ _D(428) ^ _D(426) ^ _D(425) ^ _D(422) ^ _D(420) ^ _D(418) ^ _D(417) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(403) ^ _D(402) ^ _D(400) ^ _D(399) ^ _D(396) ^ _D(395) ^ _D(393) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(385) ^ _D(383) ^ _D(382) ^ _D(380) ^ _D(379) ^ _D(378) ^ _D(375) ^ _D(374) ^ _D(372) ^ _D(371) ^ _D(364) ^ _D(361) ^ _D(356) ^ _D(353) ^ _D(351) ^ _D(349) ^ _D(348) ^ _D(345) ^ _D(344) ^ _D(342) ^ _D(341) ^ _D(337) ^ _D(335) ^ _D(333) ^ _D(331) ^ _D(330) ^ _D(328) ^ _D(325) ^ _D(324) ^ _D(323) ^ _D(322) ^ _D(321) ^ _D(318) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(312) ^ _D(307) ^ _D(306) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(295) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(287) ^ _D(284) ^ _D(282) ^ _D(281) ^ _D(275) ^ _D(274) ^ _D(268) ^ _D(267) ^ _D(266) ^ _D(263) ^ _D(262) ^ _D(260) ^ _D(258) ^ _D(256) ^ _D(250) ^ _D(249) ^ _D(247) ^ _D(244) ^ _D(243) ^ _D(240) ^ _D(239) ^ _D(236) ^ _D(235) ^ _D(229) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(220) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(208) ^ _D(204) ^ _D(198) ^ _D(197) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(187) ^ _D(184) ^ _D(183) ^ _D(180) ^ _D(179) ^ _D(178) ^ _D(175) ^ _D(173) ^ _D(171) ^ _D(168) ^ _D(165) ^ _D(164) ^ _D(163) ^ _D(161) ^ _D(160) ^ _D(159) ^ _D(157) ^ _D(154) ^ _D(153) ^ _D(152) ^ _D(151) ^ _D(150) ^ _D(149) ^ _D(148) ^ _D(141) ^ _D(139) ^ _D(136) ^ _D(135) ^ _D(134) ^ _D(130) ^ _D(127) ^ _D(126) ^ _D(125) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(115) ^ _D(113) ^ _D(110) ^ _D(109) ^ _D(107) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(101) ^ _D(98) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(90) ^ _D(89) ^ _D(86) ^ _D(83) ^ _D(80) ^ _D(78) ^ _D(77) ^ _D(75) ^ _D(73) ^ _D(71) ^ _D(70) ^ _D(69) ^ _D(66) ^ _D(63) ^ _D(62) ^ _D(60) ^ _D(59) ^ _D(58) ^ _D(56) ^ _D(55) ^ _D(52) ^ _D(50) ^ _D(42) ^ _D(40) ^ _D(39) ^ _D(36) ^ _D(35) ^ _D(33) ^ _D(32) ^ _D(31) ^ _D(29) ^ _D(28) ^ _D(26) ^ _D(19) ^ _D(16) ^ _D(14) ^ _D(13) ^ _D(9) ^ _D(5) ^ _D(3) ^ _D(2) ^ _D(0) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(6) ^ _C(8) ^ _C(10) ^ _C(11) ^ _C(14) ^ _C(15) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(21) ^ _C(22) ^ _C(27) ^ _C(28) ^ _C(29) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(555) ^ _D(554) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(543) ^ _D(542) ^ _D(537) ^ _D(534) ^ _D(533) ^ _D(530) ^ _D(529) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(523) ^ _D(522) ^ _D(520) ^ _D(517) ^ _D(516) ^ _D(515) ^ _D(514) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(504) ^ _D(503) ^ _D(501) ^ _D(495) ^ _D(493) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(486) ^ _D(483) ^ _D(482) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(472) ^ _D(470) ^ _D(467) ^ _D(465) ^ _D(464) ^ _D(459) ^ _D(456) ^ _D(454) ^ _D(452) ^ _D(451) ^ _D(450) ^ _D(448) ^ _D(447) ^ _D(445) ^ _D(434) ^ _D(433) ^ _D(432) ^ _D(429) ^ _D(427) ^ _D(426) ^ _D(424) ^ _D(423) ^ _D(422) ^ _D(421) ^ _D(416) ^ _D(414) ^ _D(411) ^ _D(410) ^ _D(407) ^ _D(405) ^ _D(403) ^ _D(401) ^ _D(399) ^ _D(398) ^ _D(397) ^ _D(394) ^ _D(393) ^ _D(392) ^ _D(391) ^ _D(389) ^ _D(387) ^ _D(384) ^ _D(383) ^ _D(380) ^ _D(379) ^ _D(378) ^ _D(375) ^ _D(374) ^ _D(373) ^ _D(369) ^ _D(368) ^ _D(366) ^ _D(365) ^ _D(363) ^ _D(359) ^ _D(358) ^ _D(354) ^ _D(353) ^ _D(352) ^ _D(350) ^ _D(348) ^ _D(347) ^ _D(346) ^ _D(344) ^ _D(343) ^ _D(341) ^ _D(339) ^ _D(337) ^ _D(336) ^ _D(335) ^ _D(333) ^ _D(332) ^ _D(331) ^ _D(329) ^ _D(328) ^ _D(327) ^ _D(326) ^ _D(325) ^ _D(324) ^ _D(323) ^ _D(321) ^ _D(320) ^ _D(318) ^ _D(316) ^ _D(314) ^ _D(313) ^ _D(312) ^ _D(310) ^ _D(309) ^ _D(308) ^ _D(307) ^ _D(305) ^ _D(301) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(291) ^ _D(290) ^ _D(287) ^ _D(286) ^ _D(285) ^ _D(282) ^ _D(279) ^ _D(277) ^ _D(275) ^ _D(274) ^ _D(273) ^ _D(267) ^ _D(265) ^ _D(263) ^ _D(255) ^ _D(252) ^ _D(251) ^ _D(250) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(241) ^ _D(240) ^ _D(236) ^ _D(234) ^ _D(228) ^ _D(225) ^ _D(223) ^ _D(221) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(214) ^ _D(211) ^ _D(208) ^ _D(207) ^ _D(205) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(197) ^ _D(195) ^ _D(190) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(181) ^ _D(180) ^ _D(179) ^ _D(176) ^ _D(174) ^ _D(171) ^ _D(170) ^ _D(167) ^ _D(165) ^ _D(164) ^ _D(160) ^ _D(156) ^ _D(154) ^ _D(153) ^ _D(152) ^ _D(150) ^ _D(144) ^ _D(143) ^ _D(142) ^ _D(140) ^ _D(134) ^ _D(132) ^ _D(131) ^ _D(125) ^ _D(124) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(113) ^ _D(108) ^ _D(107) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(102) ^ _D(101) ^ _D(98) ^ _D(94) ^ _D(91) ^ _D(90) ^ _D(85) ^ _D(83) ^ _D(82) ^ _D(78) ^ _D(76) ^ _D(74) ^ _D(73) ^ _D(71) ^ _D(70) ^ _D(68) ^ _D(66) ^ _D(65) ^ _D(64) ^ _D(59) ^ _D(58) ^ _D(57) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(51) ^ _D(50) ^ _D(48) ^ _D(47) ^ _D(45) ^ _D(44) ^ _D(43) ^ _D(41) ^ _D(40) ^ _D(36) ^ _D(33) ^ _D(31) ^ _D(28) ^ _D(27) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(20) ^ _D(17) ^ _D(16) ^ _D(15) ^ _D(14) ^ _D(12) ^ _D(9) ^ _D(4) ^ _D(3) ^ _D(1) ^ _D(0) ^ _C(1) ^ _C(2) ^ _C(5) ^ _C(6) ^ _C(9) ^ _C(14) ^ _C(15) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(26) ^ _C(27);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(554) ^ _D(550) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(544) ^ _D(543) ^ _D(542) ^ _D(540) ^ _D(539) ^ _D(538) ^ _D(532) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(524) ^ _D(523) ^ _D(522) ^ _D(519) ^ _D(517) ^ _D(515) ^ _D(510) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(504) ^ _D(501) ^ _D(500) ^ _D(496) ^ _D(495) ^ _D(493) ^ _D(492) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(484) ^ _D(477) ^ _D(476) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(470) ^ _D(466) ^ _D(464) ^ _D(462) ^ _D(461) ^ _D(460) ^ _D(458) ^ _D(457) ^ _D(455) ^ _D(453) ^ _D(451) ^ _D(450) ^ _D(446) ^ _D(444) ^ _D(437) ^ _D(436) ^ _D(435) ^ _D(430) ^ _D(428) ^ _D(427) ^ _D(425) ^ _D(423) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(411) ^ _D(409) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(402) ^ _D(396) ^ _D(395) ^ _D(394) ^ _D(391) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(380) ^ _D(379) ^ _D(378) ^ _D(375) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(367) ^ _D(364) ^ _D(363) ^ _D(362) ^ _D(360) ^ _D(358) ^ _D(357) ^ _D(355) ^ _D(354) ^ _D(351) ^ _D(341) ^ _D(340) ^ _D(339) ^ _D(336) ^ _D(335) ^ _D(332) ^ _D(330) ^ _D(329) ^ _D(326) ^ _D(325) ^ _D(324) ^ _D(320) ^ _D(318) ^ _D(314) ^ _D(313) ^ _D(312) ^ _D(311) ^ _D(308) ^ _D(306) ^ _D(305) ^ _D(303) ^ _D(297) ^ _D(291) ^ _D(290) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(277) ^ _D(275) ^ _D(273) ^ _D(269) ^ _D(266) ^ _D(265) ^ _D(261) ^ _D(259) ^ _D(257) ^ _D(256) ^ _D(255) ^ _D(253) ^ _D(251) ^ _D(248) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(235) ^ _D(234) ^ _D(230) ^ _D(229) ^ _D(228) ^ _D(227) ^ _D(222) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^ _D(210) ^ _D(207) ^ _D(206) ^ _D(204) ^ _D(201) ^ _D(199) ^ _D(197) ^ _D(196) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(190) ^ _D(188) ^ _D(187) ^ _D(185) ^ _D(184) ^ _D(181) ^ _D(180) ^ _D(177) ^ _D(175) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(165) ^ _D(162) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(154) ^ _D(153) ^ _D(149) ^ _D(145) ^ _D(141) ^ _D(137) ^ _D(136) ^ _D(134) ^ _D(133) ^ _D(128) ^ _D(127) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(116) ^ _D(113) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(108) ^ _D(105) ^ _D(102) ^ _D(101) ^ _D(98) ^ _D(97) ^ _D(96) ^ _D(94) ^ _D(92) ^ _D(91) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(82) ^ _D(81) ^ _D(77) ^ _D(75) ^ _D(74) ^ _D(73) ^ _D(71) ^ _D(69) ^ _D(68) ^ _D(63) ^ _D(61) ^ _D(59) ^ _D(57) ^ _D(56) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(47) ^ _D(46) ^ _D(42) ^ _D(41) ^ _D(31) ^ _D(30) ^ _D(27) ^ _D(24) ^ _D(21) ^ _D(18) ^ _D(17) ^ _D(15) ^ _D(13) ^ _D(12) ^ _D(9) ^ _D(6) ^ _D(5) ^ _D(4) ^ _D(2) ^ _D(1) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(4) ^ _C(10) ^ _C(11) ^ _C(12) ^ _C(14) ^ _C(15) ^ _C(16) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(22) ^ _C(26) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(555) ^ _D(551) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(545) ^ _D(544) ^ _D(543) ^ _D(541) ^ _D(540) ^ _D(539) ^ _D(533) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(525) ^ _D(524) ^ _D(523) ^ _D(520) ^ _D(518) ^ _D(516) ^ _D(511) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(502) ^ _D(501) ^ _D(497) ^ _D(496) ^ _D(494) ^ _D(493) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(485) ^ _D(478) ^ _D(477) ^ _D(474) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(467) ^ _D(465) ^ _D(463) ^ _D(462) ^ _D(461) ^ _D(459) ^ _D(458) ^ _D(456) ^ _D(454) ^ _D(452) ^ _D(451) ^ _D(447) ^ _D(445) ^ _D(438) ^ _D(437) ^ _D(436) ^ _D(431) ^ _D(429) ^ _D(428) ^ _D(426) ^ _D(424) ^ _D(420) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(412) ^ _D(410) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(403) ^ _D(397) ^ _D(396) ^ _D(395) ^ _D(392) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(381) ^ _D(380) ^ _D(379) ^ _D(376) ^ _D(373) ^ _D(371) ^ _D(369) ^ _D(368) ^ _D(365) ^ _D(364) ^ _D(363) ^ _D(361) ^ _D(359) ^ _D(358) ^ _D(356) ^ _D(355) ^ _D(352) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(337) ^ _D(336) ^ _D(333) ^ _D(331) ^ _D(330) ^ _D(327) ^ _D(326) ^ _D(325) ^ _D(321) ^ _D(319) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(312) ^ _D(309) ^ _D(307) ^ _D(306) ^ _D(304) ^ _D(298) ^ _D(292) ^ _D(291) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(276) ^ _D(274) ^ _D(270) ^ _D(267) ^ _D(266) ^ _D(262) ^ _D(260) ^ _D(258) ^ _D(257) ^ _D(256) ^ _D(254) ^ _D(252) ^ _D(249) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(236) ^ _D(235) ^ _D(231) ^ _D(230) ^ _D(229) ^ _D(228) ^ _D(223) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(211) ^ _D(208) ^ _D(207) ^ _D(205) ^ _D(202) ^ _D(200) ^ _D(198) ^ _D(197) ^ _D(195) ^ _D(194) ^ _D(193) ^ _D(191) ^ _D(189) ^ _D(188) ^ _D(186) ^ _D(185) ^ _D(182) ^ _D(181) ^ _D(178) ^ _D(176) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(166) ^ _D(163) ^ _D(159) ^ _D(158) ^ _D(157) ^ _D(155) ^ _D(154) ^ _D(150) ^ _D(146) ^ _D(142) ^ _D(138) ^ _D(137) ^ _D(135) ^ _D(134) ^ _D(129) ^ _D(128) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(118) ^ _D(117) ^ _D(114) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(106) ^ _D(103) ^ _D(102) ^ _D(99) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(93) ^ _D(92) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(83) ^ _D(82) ^ _D(78) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(72) ^ _D(70) ^ _D(69) ^ _D(64) ^ _D(62) ^ _D(60) ^ _D(58) ^ _D(57) ^ _D(55) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(48) ^ _D(47) ^ _D(43) ^ _D(42) ^ _D(32) ^ _D(31) ^ _D(28) ^ _D(25) ^ _D(22) ^ _D(19) ^ _D(18) ^ _D(16) ^ _D(14) ^ _D(13) ^ _D(10) ^ _D(7) ^ _D(6) ^ _D(5) ^ _D(3) ^ _D(2) ^ _D(1) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(5) ^ _C(11) ^ _C(12) ^ _C(13) ^ _C(15) ^ _C(16) ^ _C(17) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(23) ^ _C(27) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(556) ^ _D(552) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(542) ^ _D(541) ^ _D(540) ^ _D(534) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(521) ^ _D(519) ^ _D(517) ^ _D(512) ^ _D(510) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(503) ^ _D(502) ^ _D(498) ^ _D(497) ^ _D(495) ^ _D(494) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(486) ^ _D(479) ^ _D(478) ^ _D(475) ^ _D(474) ^ _D(473) ^ _D(472) ^ _D(468) ^ _D(466) ^ _D(464) ^ _D(463) ^ _D(462) ^ _D(460) ^ _D(459) ^ _D(457) ^ _D(455) ^ _D(453) ^ _D(452) ^ _D(448) ^ _D(446) ^ _D(439) ^ _D(438) ^ _D(437) ^ _D(432) ^ _D(430) ^ _D(429) ^ _D(427) ^ _D(425) ^ _D(421) ^ _D(420) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(413) ^ _D(411) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(404) ^ _D(398) ^ _D(397) ^ _D(396) ^ _D(393) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(377) ^ _D(374) ^ _D(372) ^ _D(370) ^ _D(369) ^ _D(366) ^ _D(365) ^ _D(364) ^ _D(362) ^ _D(360) ^ _D(359) ^ _D(357) ^ _D(356) ^ _D(353) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(338) ^ _D(337) ^ _D(334) ^ _D(332) ^ _D(331) ^ _D(328) ^ _D(327) ^ _D(326) ^ _D(322) ^ _D(320) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(305) ^ _D(299) ^ _D(293) ^ _D(292) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(277) ^ _D(275) ^ _D(271) ^ _D(268) ^ _D(267) ^ _D(263) ^ _D(261) ^ _D(259) ^ _D(258) ^ _D(257) ^ _D(255) ^ _D(253) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(243) ^ _D(237) ^ _D(236) ^ _D(232) ^ _D(231) ^ _D(230) ^ _D(229) ^ _D(224) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(212) ^ _D(209) ^ _D(208) ^ _D(206) ^ _D(203) ^ _D(201) ^ _D(199) ^ _D(198) ^ _D(196) ^ _D(195) ^ _D(194) ^ _D(192) ^ _D(190) ^ _D(189) ^ _D(187) ^ _D(186) ^ _D(183) ^ _D(182) ^ _D(179) ^ _D(177) ^ _D(172) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(167) ^ _D(164) ^ _D(160) ^ _D(159) ^ _D(158) ^ _D(156) ^ _D(155) ^ _D(151) ^ _D(147) ^ _D(143) ^ _D(139) ^ _D(138) ^ _D(136) ^ _D(135) ^ _D(130) ^ _D(129) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(119) ^ _D(118) ^ _D(115) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(107) ^ _D(104) ^ _D(103) ^ _D(100) ^ _D(99) ^ _D(98) ^ _D(96) ^ _D(94) ^ _D(93) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(84) ^ _D(83) ^ _D(79) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(73) ^ _D(71) ^ _D(70) ^ _D(65) ^ _D(63) ^ _D(61) ^ _D(59) ^ _D(58) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(49) ^ _D(48) ^ _D(44) ^ _D(43) ^ _D(33) ^ _D(32) ^ _D(29) ^ _D(26) ^ _D(23) ^ _D(20) ^ _D(19) ^ _D(17) ^ _D(15) ^ _D(14) ^ _D(11) ^ _D(8) ^ _D(7) ^ _D(6) ^ _D(4) ^ _D(3) ^ _D(2) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(6) ^ _C(12) ^ _C(13) ^ _C(14) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(24) ^ _C(28) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(553) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(543) ^ _D(542) ^ _D(541) ^ _D(535) ^ _D(532) ^ _D(531) ^ _D(530) ^ _D(527) ^ _D(526) ^ _D(525) ^ _D(522) ^ _D(520) ^ _D(518) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(504) ^ _D(503) ^ _D(499) ^ _D(498) ^ _D(496) ^ _D(495) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(487) ^ _D(480) ^ _D(479) ^ _D(476) ^ _D(475) ^ _D(474) ^ _D(473) ^ _D(469) ^ _D(467) ^ _D(465) ^ _D(464) ^ _D(463) ^ _D(461) ^ _D(460) ^ _D(458) ^ _D(456) ^ _D(454) ^ _D(453) ^ _D(449) ^ _D(447) ^ _D(440) ^ _D(439) ^ _D(438) ^ _D(433) ^ _D(431) ^ _D(430) ^ _D(428) ^ _D(426) ^ _D(422) ^ _D(421) ^ _D(420) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(414) ^ _D(412) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(405) ^ _D(399) ^ _D(398) ^ _D(397) ^ _D(394) ^ _D(390) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(383) ^ _D(382) ^ _D(381) ^ _D(378) ^ _D(375) ^ _D(373) ^ _D(371) ^ _D(370) ^ _D(367) ^ _D(366) ^ _D(365) ^ _D(363) ^ _D(361) ^ _D(360) ^ _D(358) ^ _D(357) ^ _D(354) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(339) ^ _D(338) ^ _D(335) ^ _D(333) ^ _D(332) ^ _D(329) ^ _D(328) ^ _D(327) ^ _D(323) ^ _D(321) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(311) ^ _D(309) ^ _D(308) ^ _D(306) ^ _D(300) ^ _D(294) ^ _D(293) ^ _D(283) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(278) ^ _D(276) ^ _D(272) ^ _D(269) ^ _D(268) ^ _D(264) ^ _D(262) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(256) ^ _D(254) ^ _D(251) ^ _D(249) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(244) ^ _D(238) ^ _D(237) ^ _D(233) ^ _D(232) ^ _D(231) ^ _D(230) ^ _D(225) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(213) ^ _D(210) ^ _D(209) ^ _D(207) ^ _D(204) ^ _D(202) ^ _D(200) ^ _D(199) ^ _D(197) ^ _D(196) ^ _D(195) ^ _D(193) ^ _D(191) ^ _D(190) ^ _D(188) ^ _D(187) ^ _D(184) ^ _D(183) ^ _D(180) ^ _D(178) ^ _D(173) ^ _D(172) ^ _D(171) ^ _D(170) ^ _D(168) ^ _D(165) ^ _D(161) ^ _D(160) ^ _D(159) ^ _D(157) ^ _D(156) ^ _D(152) ^ _D(148) ^ _D(144) ^ _D(140) ^ _D(139) ^ _D(137) ^ _D(136) ^ _D(131) ^ _D(130) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(120) ^ _D(119) ^ _D(116) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(108) ^ _D(105) ^ _D(104) ^ _D(101) ^ _D(100) ^ _D(99) ^ _D(97) ^ _D(95) ^ _D(94) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(85) ^ _D(84) ^ _D(80) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(74) ^ _D(72) ^ _D(71) ^ _D(66) ^ _D(64) ^ _D(62) ^ _D(60) ^ _D(59) ^ _D(57) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(50) ^ _D(49) ^ _D(45) ^ _D(44) ^ _D(34) ^ _D(33) ^ _D(30) ^ _D(27) ^ _D(24) ^ _D(21) ^ _D(20) ^ _D(18) ^ _D(16) ^ _D(15) ^ _D(12) ^ _D(9) ^ _D(8) ^ _D(7) ^ _D(5) ^ _D(4) ^ _D(3) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(7) ^ _C(13) ^ _C(14) ^ _C(15) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(25) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(556) ^ _D(555) ^ _D(552) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(543) ^ _D(540) ^ _D(539) ^ _D(536) ^ _D(535) ^ _D(534) ^ _D(533) ^ _D(530) ^ _D(529) ^ _D(527) ^ _D(525) ^ _D(523) ^ _D(522) ^ _D(518) ^ _D(516) ^ _D(509) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(504) ^ _D(502) ^ _D(501) ^ _D(499) ^ _D(497) ^ _D(496) ^ _D(495) ^ _D(494) ^ _D(493) ^ _D(489) ^ _D(486) ^ _D(483) ^ _D(482) ^ _D(479) ^ _D(475) ^ _D(474) ^ _D(472) ^ _D(466) ^ _D(459) ^ _D(458) ^ _D(457) ^ _D(455) ^ _D(454) ^ _D(452) ^ _D(449) ^ _D(444) ^ _D(441) ^ _D(440) ^ _D(439) ^ _D(437) ^ _D(436) ^ _D(433) ^ _D(432) ^ _D(431) ^ _D(429) ^ _D(427) ^ _D(424) ^ _D(423) ^ _D(421) ^ _D(420) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(413) ^ _D(412) ^ _D(411) ^ _D(410) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(404) ^ _D(396) ^ _D(395) ^ _D(393) ^ _D(392) ^ _D(389) ^ _D(387) ^ _D(386) ^ _D(384) ^ _D(383) ^ _D(382) ^ _D(381) ^ _D(379) ^ _D(378) ^ _D(371) ^ _D(369) ^ _D(367) ^ _D(364) ^ _D(363) ^ _D(361) ^ _D(357) ^ _D(355) ^ _D(353) ^ _D(349) ^ _D(348) ^ _D(347) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(338) ^ _D(337) ^ _D(336) ^ _D(335) ^ _D(330) ^ _D(329) ^ _D(327) ^ _D(324) ^ _D(321) ^ _D(320) ^ _D(319) ^ _D(316) ^ _D(307) ^ _D(305) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(296) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(287) ^ _D(286) ^ _D(284) ^ _D(282) ^ _D(281) ^ _D(276) ^ _D(274) ^ _D(270) ^ _D(268) ^ _D(264) ^ _D(263) ^ _D(260) ^ _D(250) ^ _D(249) ^ _D(247) ^ _D(246) ^ _D(245) ^ _D(243) ^ _D(239) ^ _D(238) ^ _D(237) ^ _D(233) ^ _D(232) ^ _D(231) ^ _D(230) ^ _D(228) ^ _D(227) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(216) ^ _D(212) ^ _D(211) ^ _D(209) ^ _D(207) ^ _D(205) ^ _D(202) ^ _D(200) ^ _D(199) ^ _D(196) ^ _D(193) ^ _D(190) ^ _D(189) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(181) ^ _D(179) ^ _D(174) ^ _D(173) ^ _D(170) ^ _D(167) ^ _D(160) ^ _D(157) ^ _D(156) ^ _D(155) ^ _D(153) ^ _D(151) ^ _D(145) ^ _D(144) ^ _D(143) ^ _D(141) ^ _D(140) ^ _D(138) ^ _D(136) ^ _D(135) ^ _D(134) ^ _D(131) ^ _D(128) ^ _D(127) ^ _D(124) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(118) ^ _D(116) ^ _D(115) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(109) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(102) ^ _D(100) ^ _D(99) ^ _D(97) ^ _D(94) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(87) ^ _D(86) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(78) ^ _D(77) ^ _D(75) ^ _D(68) ^ _D(66) ^ _D(57) ^ _D(56) ^ _D(51) ^ _D(48) ^ _D(47) ^ _D(46) ^ _D(44) ^ _D(37) ^ _D(35) ^ _D(32) ^ _D(30) ^ _D(29) ^ _D(26) ^ _D(24) ^ _D(22) ^ _D(21) ^ _D(19) ^ _D(17) ^ _D(13) ^ _D(12) ^ _D(8) ^ _D(5) ^ _D(4) ^ _D(0) ^ _C(1) ^ _C(2) ^ _C(5) ^ _C(6) ^ _C(7) ^ _C(8) ^ _C(11) ^ _C(12) ^ _C(15) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(24) ^ _C(27) ^ _C(28) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(556) ^ _D(553) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(544) ^ _D(541) ^ _D(540) ^ _D(537) ^ _D(536) ^ _D(535) ^ _D(534) ^ _D(531) ^ _D(530) ^ _D(528) ^ _D(526) ^ _D(524) ^ _D(523) ^ _D(519) ^ _D(517) ^ _D(510) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(503) ^ _D(502) ^ _D(500) ^ _D(498) ^ _D(497) ^ _D(496) ^ _D(495) ^ _D(494) ^ _D(490) ^ _D(487) ^ _D(484) ^ _D(483) ^ _D(480) ^ _D(476) ^ _D(475) ^ _D(473) ^ _D(467) ^ _D(460) ^ _D(459) ^ _D(458) ^ _D(456) ^ _D(455) ^ _D(453) ^ _D(450) ^ _D(445) ^ _D(442) ^ _D(441) ^ _D(440) ^ _D(438) ^ _D(437) ^ _D(434) ^ _D(433) ^ _D(432) ^ _D(430) ^ _D(428) ^ _D(425) ^ _D(424) ^ _D(422) ^ _D(421) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(413) ^ _D(412) ^ _D(411) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(397) ^ _D(396) ^ _D(394) ^ _D(393) ^ _D(390) ^ _D(388) ^ _D(387) ^ _D(385) ^ _D(384) ^ _D(383) ^ _D(382) ^ _D(380) ^ _D(379) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(365) ^ _D(364) ^ _D(362) ^ _D(358) ^ _D(356) ^ _D(354) ^ _D(350) ^ _D(349) ^ _D(348) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(339) ^ _D(338) ^ _D(337) ^ _D(336) ^ _D(331) ^ _D(330) ^ _D(328) ^ _D(325) ^ _D(322) ^ _D(321) ^ _D(320) ^ _D(317) ^ _D(308) ^ _D(306) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(293) ^ _D(291) ^ _D(289) ^ _D(288) ^ _D(287) ^ _D(285) ^ _D(283) ^ _D(282) ^ _D(277) ^ _D(275) ^ _D(271) ^ _D(269) ^ _D(265) ^ _D(264) ^ _D(261) ^ _D(251) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(246) ^ _D(244) ^ _D(240) ^ _D(239) ^ _D(238) ^ _D(234) ^ _D(233) ^ _D(232) ^ _D(231) ^ _D(229) ^ _D(228) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(217) ^ _D(213) ^ _D(212) ^ _D(210) ^ _D(208) ^ _D(206) ^ _D(203) ^ _D(201) ^ _D(200) ^ _D(197) ^ _D(194) ^ _D(191) ^ _D(190) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(180) ^ _D(175) ^ _D(174) ^ _D(171) ^ _D(168) ^ _D(161) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(154) ^ _D(152) ^ _D(146) ^ _D(145) ^ _D(144) ^ _D(142) ^ _D(141) ^ _D(139) ^ _D(137) ^ _D(136) ^ _D(135) ^ _D(132) ^ _D(129) ^ _D(128) ^ _D(125) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(116) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(101) ^ _D(100) ^ _D(98) ^ _D(95) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(88) ^ _D(87) ^ _D(85) ^ _D(84) ^ _D(83) ^ _D(79) ^ _D(78) ^ _D(76) ^ _D(69) ^ _D(67) ^ _D(58) ^ _D(57) ^ _D(52) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(45) ^ _D(38) ^ _D(36) ^ _D(33) ^ _D(31) ^ _D(30) ^ _D(27) ^ _D(25) ^ _D(23) ^ _D(22) ^ _D(20) ^ _D(18) ^ _D(14) ^ _D(13) ^ _D(9) ^ _D(6) ^ _D(5) ^ _D(1) ^ _C(0) ^ _C(2) ^ _C(3) ^ _C(6) ^ _C(7) ^ _C(8) ^ _C(9) ^ _C(12) ^ _C(13) ^ _C(16) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(25) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(557) ^ _D(554) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(545) ^ _D(542) ^ _D(541) ^ _D(538) ^ _D(537) ^ _D(536) ^ _D(535) ^ _D(532) ^ _D(531) ^ _D(529) ^ _D(527) ^ _D(525) ^ _D(524) ^ _D(520) ^ _D(518) ^ _D(511) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(504) ^ _D(503) ^ _D(501) ^ _D(499) ^ _D(498) ^ _D(497) ^ _D(496) ^ _D(495) ^ _D(491) ^ _D(488) ^ _D(485) ^ _D(484) ^ _D(481) ^ _D(477) ^ _D(476) ^ _D(474) ^ _D(468) ^ _D(461) ^ _D(460) ^ _D(459) ^ _D(457) ^ _D(456) ^ _D(454) ^ _D(451) ^ _D(446) ^ _D(443) ^ _D(442) ^ _D(441) ^ _D(439) ^ _D(438) ^ _D(435) ^ _D(434) ^ _D(433) ^ _D(431) ^ _D(429) ^ _D(426) ^ _D(425) ^ _D(423) ^ _D(422) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(413) ^ _D(412) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(398) ^ _D(397) ^ _D(395) ^ _D(394) ^ _D(391) ^ _D(389) ^ _D(388) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(383) ^ _D(381) ^ _D(380) ^ _D(373) ^ _D(371) ^ _D(369) ^ _D(366) ^ _D(365) ^ _D(363) ^ _D(359) ^ _D(357) ^ _D(355) ^ _D(351) ^ _D(350) ^ _D(349) ^ _D(345) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(340) ^ _D(339) ^ _D(338) ^ _D(337) ^ _D(332) ^ _D(331) ^ _D(329) ^ _D(326) ^ _D(323) ^ _D(322) ^ _D(321) ^ _D(318) ^ _D(309) ^ _D(307) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(298) ^ _D(294) ^ _D(292) ^ _D(290) ^ _D(289) ^ _D(288) ^ _D(286) ^ _D(284) ^ _D(283) ^ _D(278) ^ _D(276) ^ _D(272) ^ _D(270) ^ _D(266) ^ _D(265) ^ _D(262) ^ _D(252) ^ _D(251) ^ _D(249) ^ _D(248) ^ _D(247) ^ _D(245) ^ _D(241) ^ _D(240) ^ _D(239) ^ _D(235) ^ _D(234) ^ _D(233) ^ _D(232) ^ _D(230) ^ _D(229) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(218) ^ _D(214) ^ _D(213) ^ _D(211) ^ _D(209) ^ _D(207) ^ _D(204) ^ _D(202) ^ _D(201) ^ _D(198) ^ _D(195) ^ _D(192) ^ _D(191) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(183) ^ _D(181) ^ _D(176) ^ _D(175) ^ _D(172) ^ _D(169) ^ _D(162) ^ _D(159) ^ _D(158) ^ _D(157) ^ _D(155) ^ _D(153) ^ _D(147) ^ _D(146) ^ _D(145) ^ _D(143) ^ _D(142) ^ _D(140) ^ _D(138) ^ _D(137) ^ _D(136) ^ _D(133) ^ _D(130) ^ _D(129) ^ _D(126) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(118) ^ _D(117) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(107) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(102) ^ _D(101) ^ _D(99) ^ _D(96) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(89) ^ _D(88) ^ _D(86) ^ _D(85) ^ _D(84) ^ _D(80) ^ _D(79) ^ _D(77) ^ _D(70) ^ _D(68) ^ _D(59) ^ _D(58) ^ _D(53) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(46) ^ _D(39) ^ _D(37) ^ _D(34) ^ _D(32) ^ _D(31) ^ _D(28) ^ _D(26) ^ _D(24) ^ _D(23) ^ _D(21) ^ _D(19) ^ _D(15) ^ _D(14) ^ _D(10) ^ _D(7) ^ _D(6) ^ _D(2) ^ _C(1) ^ _C(3) ^ _C(4) ^ _C(7) ^ _C(8) ^ _C(9) ^ _C(10) ^ _C(13) ^ _C(14) ^ _C(17) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(26) ^ _C(29) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(555) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(546) ^ _D(543) ^ _D(542) ^ _D(539) ^ _D(538) ^ _D(537) ^ _D(536) ^ _D(533) ^ _D(532) ^ _D(530) ^ _D(528) ^ _D(526) ^ _D(525) ^ _D(521) ^ _D(519) ^ _D(512) ^ _D(510) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(505) ^ _D(504) ^ _D(502) ^ _D(500) ^ _D(499) ^ _D(498) ^ _D(497) ^ _D(496) ^ _D(492) ^ _D(489) ^ _D(486) ^ _D(485) ^ _D(482) ^ _D(478) ^ _D(477) ^ _D(475) ^ _D(469) ^ _D(462) ^ _D(461) ^ _D(460) ^ _D(458) ^ _D(457) ^ _D(455) ^ _D(452) ^ _D(447) ^ _D(444) ^ _D(443) ^ _D(442) ^ _D(440) ^ _D(439) ^ _D(436) ^ _D(435) ^ _D(434) ^ _D(432) ^ _D(430) ^ _D(427) ^ _D(426) ^ _D(424) ^ _D(423) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(413) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(399) ^ _D(398) ^ _D(396) ^ _D(395) ^ _D(392) ^ _D(390) ^ _D(389) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(382) ^ _D(381) ^ _D(374) ^ _D(372) ^ _D(370) ^ _D(367) ^ _D(366) ^ _D(364) ^ _D(360) ^ _D(358) ^ _D(356) ^ _D(352) ^ _D(351) ^ _D(350) ^ _D(346) ^ _D(345) ^ _D(344) ^ _D(343) ^ _D(341) ^ _D(340) ^ _D(339) ^ _D(338) ^ _D(333) ^ _D(332) ^ _D(330) ^ _D(327) ^ _D(324) ^ _D(323) ^ _D(322) ^ _D(319) ^ _D(310) ^ _D(308) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(299) ^ _D(295) ^ _D(293) ^ _D(291) ^ _D(290) ^ _D(289) ^ _D(287) ^ _D(285) ^ _D(284) ^ _D(279) ^ _D(277) ^ _D(273) ^ _D(271) ^ _D(267) ^ _D(266) ^ _D(263) ^ _D(253) ^ _D(252) ^ _D(250) ^ _D(249) ^ _D(248) ^ _D(246) ^ _D(242) ^ _D(241) ^ _D(240) ^ _D(236) ^ _D(235) ^ _D(234) ^ _D(233) ^ _D(231) ^ _D(230) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(219) ^ _D(215) ^ _D(214) ^ _D(212) ^ _D(210) ^ _D(208) ^ _D(205) ^ _D(203) ^ _D(202) ^ _D(199) ^ _D(196) ^ _D(193) ^ _D(192) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(182) ^ _D(177) ^ _D(176) ^ _D(173) ^ _D(170) ^ _D(163) ^ _D(160) ^ _D(159) ^ _D(158) ^ _D(156) ^ _D(154) ^ _D(148) ^ _D(147) ^ _D(146) ^ _D(144) ^ _D(143) ^ _D(141) ^ _D(139) ^ _D(138) ^ _D(137) ^ _D(134) ^ _D(131) ^ _D(130) ^ _D(127) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(119) ^ _D(118) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(108) ^ _D(107) ^ _D(106) ^ _D(105) ^ _D(103) ^ _D(102) ^ _D(100) ^ _D(97) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(90) ^ _D(89) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(81) ^ _D(80) ^ _D(78) ^ _D(71) ^ _D(69) ^ _D(60) ^ _D(59) ^ _D(54) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(47) ^ _D(40) ^ _D(38) ^ _D(35) ^ _D(33) ^ _D(32) ^ _D(29) ^ _D(27) ^ _D(25) ^ _D(24) ^ _D(22) ^ _D(20) ^ _D(16) ^ _D(15) ^ _D(11) ^ _D(8) ^ _D(7) ^ _D(3) ^ _C(0) ^ _C(2) ^ _C(4) ^ _C(5) ^ _C(8) ^ _C(9) ^ _C(10) ^ _C(11) ^ _C(14) ^ _C(15) ^ _C(18) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(27) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(556) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(547) ^ _D(544) ^ _D(543) ^ _D(540) ^ _D(539) ^ _D(538) ^ _D(537) ^ _D(534) ^ _D(533) ^ _D(531) ^ _D(529) ^ _D(527) ^ _D(526) ^ _D(522) ^ _D(520) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(509) ^ _D(508) ^ _D(506) ^ _D(505) ^ _D(503) ^ _D(501) ^ _D(500) ^ _D(499) ^ _D(498) ^ _D(497) ^ _D(493) ^ _D(490) ^ _D(487) ^ _D(486) ^ _D(483) ^ _D(479) ^ _D(478) ^ _D(476) ^ _D(470) ^ _D(463) ^ _D(462) ^ _D(461) ^ _D(459) ^ _D(458) ^ _D(456) ^ _D(453) ^ _D(448) ^ _D(445) ^ _D(444) ^ _D(443) ^ _D(441) ^ _D(440) ^ _D(437) ^ _D(436) ^ _D(435) ^ _D(433) ^ _D(431) ^ _D(428) ^ _D(427) ^ _D(425) ^ _D(424) ^ _D(420) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(414) ^ _D(412) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(400) ^ _D(399) ^ _D(397) ^ _D(396) ^ _D(393) ^ _D(391) ^ _D(390) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(383) ^ _D(382) ^ _D(375) ^ _D(373) ^ _D(371) ^ _D(368) ^ _D(367) ^ _D(365) ^ _D(361) ^ _D(359) ^ _D(357) ^ _D(353) ^ _D(352) ^ _D(351) ^ _D(347) ^ _D(346) ^ _D(345) ^ _D(344) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(339) ^ _D(334) ^ _D(333) ^ _D(331) ^ _D(328) ^ _D(325) ^ _D(324) ^ _D(323) ^ _D(320) ^ _D(311) ^ _D(309) ^ _D(307) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(300) ^ _D(296) ^ _D(294) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(288) ^ _D(286) ^ _D(285) ^ _D(280) ^ _D(278) ^ _D(274) ^ _D(272) ^ _D(268) ^ _D(267) ^ _D(264) ^ _D(254) ^ _D(253) ^ _D(251) ^ _D(250) ^ _D(249) ^ _D(247) ^ _D(243) ^ _D(242) ^ _D(241) ^ _D(237) ^ _D(236) ^ _D(235) ^ _D(234) ^ _D(232) ^ _D(231) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(220) ^ _D(216) ^ _D(215) ^ _D(213) ^ _D(211) ^ _D(209) ^ _D(206) ^ _D(204) ^ _D(203) ^ _D(200) ^ _D(197) ^ _D(194) ^ _D(193) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(183) ^ _D(178) ^ _D(177) ^ _D(174) ^ _D(171) ^ _D(164) ^ _D(161) ^ _D(160) ^ _D(159) ^ _D(157) ^ _D(155) ^ _D(149) ^ _D(148) ^ _D(147) ^ _D(145) ^ _D(144) ^ _D(142) ^ _D(140) ^ _D(139) ^ _D(138) ^ _D(135) ^ _D(132) ^ _D(131) ^ _D(128) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(120) ^ _D(119) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(109) ^ _D(108) ^ _D(107) ^ _D(106) ^ _D(104) ^ _D(103) ^ _D(101) ^ _D(98) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(91) ^ _D(90) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(82) ^ _D(81) ^ _D(79) ^ _D(72) ^ _D(70) ^ _D(61) ^ _D(60) ^ _D(55) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(48) ^ _D(41) ^ _D(39) ^ _D(36) ^ _D(34) ^ _D(33) ^ _D(30) ^ _D(28) ^ _D(26) ^ _D(25) ^ _D(23) ^ _D(21) ^ _D(17) ^ _D(16) ^ _D(12) ^ _D(9) ^ _D(8) ^ _D(4) ^ _C(1) ^ _C(3) ^ _C(5) ^ _C(6) ^ _C(9) ^ _C(10) ^ _C(11) ^ _C(12) ^ _C(15) ^ _C(16) ^ _C(19) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(28) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(545) ^ _D(544) ^ _D(541) ^ _D(540) ^ _D(539) ^ _D(538) ^ _D(535) ^ _D(534) ^ _D(532) ^ _D(530) ^ _D(528) ^ _D(527) ^ _D(523) ^ _D(521) ^ _D(514) ^ _D(512) ^ _D(511) ^ _D(510) ^ _D(509) ^ _D(507) ^ _D(506) ^ _D(504) ^ _D(502) ^ _D(501) ^ _D(500) ^ _D(499) ^ _D(498) ^ _D(494) ^ _D(491) ^ _D(488) ^ _D(487) ^ _D(484) ^ _D(480) ^ _D(479) ^ _D(477) ^ _D(471) ^ _D(464) ^ _D(463) ^ _D(462) ^ _D(460) ^ _D(459) ^ _D(457) ^ _D(454) ^ _D(449) ^ _D(446) ^ _D(445) ^ _D(444) ^ _D(442) ^ _D(441) ^ _D(438) ^ _D(437) ^ _D(436) ^ _D(434) ^ _D(432) ^ _D(429) ^ _D(428) ^ _D(426) ^ _D(425) ^ _D(421) ^ _D(420) ^ _D(419) ^ _D(418) ^ _D(417) ^ _D(416) ^ _D(415) ^ _D(413) ^ _D(412) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(401) ^ _D(400) ^ _D(398) ^ _D(397) ^ _D(394) ^ _D(392) ^ _D(391) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(384) ^ _D(383) ^ _D(376) ^ _D(374) ^ _D(372) ^ _D(369) ^ _D(368) ^ _D(366) ^ _D(362) ^ _D(360) ^ _D(358) ^ _D(354) ^ _D(353) ^ _D(352) ^ _D(348) ^ _D(347) ^ _D(346) ^ _D(345) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(335) ^ _D(334) ^ _D(332) ^ _D(329) ^ _D(326) ^ _D(325) ^ _D(324) ^ _D(321) ^ _D(312) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(306) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(297) ^ _D(295) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(289) ^ _D(287) ^ _D(286) ^ _D(281) ^ _D(279) ^ _D(275) ^ _D(273) ^ _D(269) ^ _D(268) ^ _D(265) ^ _D(255) ^ _D(254) ^ _D(252) ^ _D(251) ^ _D(250) ^ _D(248) ^ _D(244) ^ _D(243) ^ _D(242) ^ _D(238) ^ _D(237) ^ _D(236) ^ _D(235) ^ _D(233) ^ _D(232) ^ _D(228) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(221) ^ _D(217) ^ _D(216) ^ _D(214) ^ _D(212) ^ _D(210) ^ _D(207) ^ _D(205) ^ _D(204) ^ _D(201) ^ _D(198) ^ _D(195) ^ _D(194) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(184) ^ _D(179) ^ _D(178) ^ _D(175) ^ _D(172) ^ _D(165) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(158) ^ _D(156) ^ _D(150) ^ _D(149) ^ _D(148) ^ _D(146) ^ _D(145) ^ _D(143) ^ _D(141) ^ _D(140) ^ _D(139) ^ _D(136) ^ _D(133) ^ _D(132) ^ _D(129) ^ _D(126) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(121) ^ _D(120) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(110) ^ _D(109) ^ _D(108) ^ _D(107) ^ _D(105) ^ _D(104) ^ _D(102) ^ _D(99) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(92) ^ _D(91) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(83) ^ _D(82) ^ _D(80) ^ _D(73) ^ _D(71) ^ _D(62) ^ _D(61) ^ _D(56) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(49) ^ _D(42) ^ _D(40) ^ _D(37) ^ _D(35) ^ _D(34) ^ _D(31) ^ _D(29) ^ _D(27) ^ _D(26) ^ _D(24) ^ _D(22) ^ _D(18) ^ _D(17) ^ _D(13) ^ _D(10) ^ _D(9) ^ _D(5) ^ _C(0) ^ _C(2) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(10) ^ _C(11) ^ _C(12) ^ _C(13) ^ _C(16) ^ _C(17) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(553) ^ _D(552) ^ _D(550) ^ _D(546) ^ _D(541) ^ _D(536) ^ _D(534) ^ _D(533) ^ _D(532) ^ _D(530) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(521) ^ _D(519) ^ _D(518) ^ _D(516) ^ _D(515) ^ _D(514) ^ _D(513) ^ _D(506) ^ _D(505) ^ _D(503) ^ _D(499) ^ _D(494) ^ _D(493) ^ _D(491) ^ _D(490) ^ _D(486) ^ _D(485) ^ _D(483) ^ _D(482) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(476) ^ _D(470) ^ _D(468) ^ _D(463) ^ _D(462) ^ _D(460) ^ _D(455) ^ _D(452) ^ _D(449) ^ _D(448) ^ _D(447) ^ _D(446) ^ _D(445) ^ _D(444) ^ _D(443) ^ _D(442) ^ _D(439) ^ _D(438) ^ _D(436) ^ _D(435) ^ _D(434) ^ _D(430) ^ _D(429) ^ _D(427) ^ _D(426) ^ _D(424) ^ _D(421) ^ _D(420) ^ _D(417) ^ _D(413) ^ _D(411) ^ _D(410) ^ _D(409) ^ _D(408) ^ _D(407) ^ _D(405) ^ _D(404) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(396) ^ _D(395) ^ _D(391) ^ _D(389) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(381) ^ _D(378) ^ _D(377) ^ _D(376) ^ _D(375) ^ _D(374) ^ _D(373) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(367) ^ _D(366) ^ _D(362) ^ _D(361) ^ _D(358) ^ _D(357) ^ _D(355) ^ _D(354) ^ _D(346) ^ _D(345) ^ _D(343) ^ _D(339) ^ _D(338) ^ _D(337) ^ _D(336) ^ _D(334) ^ _D(330) ^ _D(328) ^ _D(326) ^ _D(325) ^ _D(321) ^ _D(320) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(315) ^ _D(313) ^ _D(312) ^ _D(311) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(306) ^ _D(304) ^ _D(300) ^ _D(299) ^ _D(297) ^ _D(295) ^ _D(293) ^ _D(286) ^ _D(283) ^ _D(282) ^ _D(280) ^ _D(279) ^ _D(277) ^ _D(273) ^ _D(270) ^ _D(268) ^ _D(266) ^ _D(265) ^ _D(264) ^ _D(261) ^ _D(259) ^ _D(257) ^ _D(256) ^ _D(253) ^ _D(251) ^ _D(249) ^ _D(248) ^ _D(245) ^ _D(244) ^ _D(239) ^ _D(238) ^ _D(236) ^ _D(233) ^ _D(230) ^ _D(229) ^ _D(225) ^ _D(222) ^ _D(218) ^ _D(217) ^ _D(216) ^ _D(215) ^ _D(214) ^ _D(213) ^ _D(212) ^ _D(211) ^ _D(210) ^ _D(209) ^ _D(207) ^ _D(206) ^ _D(205) ^ _D(203) ^ _D(201) ^ _D(198) ^ _D(197) ^ _D(196) ^ _D(195) ^ _D(194) ^ _D(193) ^ _D(189) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(183) ^ _D(182) ^ _D(180) ^ _D(179) ^ _D(176) ^ _D(173) ^ _D(172) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(167) ^ _D(163) ^ _D(159) ^ _D(158) ^ _D(157) ^ _D(156) ^ _D(155) ^ _D(150) ^ _D(147) ^ _D(146) ^ _D(143) ^ _D(142) ^ _D(141) ^ _D(140) ^ _D(136) ^ _D(135) ^ _D(133) ^ _D(132) ^ _D(130) ^ _D(128) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(119) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(109) ^ _D(108) ^ _D(105) ^ _D(104) ^ _D(101) ^ _D(100) ^ _D(99) ^ _D(98) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(85) ^ _D(82) ^ _D(79) ^ _D(74) ^ _D(73) ^ _D(68) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(62) ^ _D(61) ^ _D(60) ^ _D(58) ^ _D(57) ^ _D(55) ^ _D(52) ^ _D(48) ^ _D(47) ^ _D(45) ^ _D(44) ^ _D(43) ^ _D(41) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(35) ^ _D(34) ^ _D(31) ^ _D(29) ^ _D(27) ^ _D(26) ^ _D(24) ^ _D(23) ^ _D(19) ^ _D(18) ^ _D(16) ^ _D(14) ^ _D(12) ^ _D(11) ^ _D(9) ^ _D(0) ^ _C(2) ^ _C(4) ^ _C(5) ^ _C(6) ^ _C(8) ^ _C(13) ^ _C(18) ^ _C(22) ^ _C(24) ^ _C(25) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(557) ^ _D(556) ^ _D(555) ^ _D(553) ^ _D(549) ^ _D(547) ^ _D(545) ^ _D(540) ^ _D(539) ^ _D(537) ^ _D(533) ^ _D(532) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(521) ^ _D(520) ^ _D(518) ^ _D(517) ^ _D(515) ^ _D(512) ^ _D(511) ^ _D(510) ^ _D(508) ^ _D(504) ^ _D(502) ^ _D(501) ^ _D(493) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(484) ^ _D(482) ^ _D(481) ^ _D(478) ^ _D(476) ^ _D(472) ^ _D(471) ^ _D(470) ^ _D(469) ^ _D(468) ^ _D(465) ^ _D(463) ^ _D(462) ^ _D(458) ^ _D(456) ^ _D(453) ^ _D(452) ^ _D(447) ^ _D(446) ^ _D(445) ^ _D(443) ^ _D(440) ^ _D(439) ^ _D(435) ^ _D(434) ^ _D(433) ^ _D(431) ^ _D(430) ^ _D(428) ^ _D(427) ^ _D(425) ^ _D(424) ^ _D(421) ^ _D(419) ^ _D(416) ^ _D(411) ^ _D(410) ^ _D(407) ^ _D(406) ^ _D(404) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(399) ^ _D(398) ^ _D(397) ^ _D(393) ^ _D(391) ^ _D(388) ^ _D(385) ^ _D(382) ^ _D(381) ^ _D(379) ^ _D(377) ^ _D(375) ^ _D(373) ^ _D(372) ^ _D(371) ^ _D(367) ^ _D(366) ^ _D(357) ^ _D(356) ^ _D(355) ^ _D(353) ^ _D(349) ^ _D(348) ^ _D(346) ^ _D(345) ^ _D(342) ^ _D(341) ^ _D(340) ^ _D(334) ^ _D(333) ^ _D(331) ^ _D(329) ^ _D(328) ^ _D(326) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(311) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(303) ^ _D(302) ^ _D(301) ^ _D(299) ^ _D(297) ^ _D(295) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(286) ^ _D(284) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(277) ^ _D(276) ^ _D(273) ^ _D(271) ^ _D(268) ^ _D(267) ^ _D(266) ^ _D(264) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(258) ^ _D(255) ^ _D(254) ^ _D(250) ^ _D(249) ^ _D(248) ^ _D(246) ^ _D(245) ^ _D(243) ^ _D(240) ^ _D(239) ^ _D(231) ^ _D(228) ^ _D(227) ^ _D(224) ^ _D(223) ^ _D(219) ^ _D(218) ^ _D(217) ^ _D(215) ^ _D(213) ^ _D(211) ^ _D(209) ^ _D(206) ^ _D(204) ^ _D(203) ^ _D(201) ^ _D(196) ^ _D(195) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(187) ^ _D(184) ^ _D(182) ^ _D(181) ^ _D(180) ^ _D(177) ^ _D(174) ^ _D(173) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(164) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(159) ^ _D(157) ^ _D(155) ^ _D(149) ^ _D(148) ^ _D(147) ^ _D(142) ^ _D(141) ^ _D(135) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(129) ^ _D(128) ^ _D(127) ^ _D(126) ^ _D(124) ^ _D(122) ^ _D(120) ^ _D(119) ^ _D(118) ^ _D(117) ^ _D(115) ^ _D(113) ^ _D(111) ^ _D(109) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(102) ^ _D(100) ^ _D(98) ^ _D(97) ^ _D(96) ^ _D(93) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(84) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(75) ^ _D(74) ^ _D(73) ^ _D(72) ^ _D(69) ^ _D(65) ^ _D(62) ^ _D(60) ^ _D(59) ^ _D(56) ^ _D(55) ^ _D(54) ^ _D(50) ^ _D(49) ^ _D(47) ^ _D(46) ^ _D(42) ^ _D(39) ^ _D(38) ^ _D(36) ^ _D(35) ^ _D(34) ^ _D(31) ^ _D(29) ^ _D(27) ^ _D(26) ^ _D(20) ^ _D(19) ^ _D(17) ^ _D(16) ^ _D(15) ^ _D(13) ^ _D(9) ^ _D(6) ^ _D(1) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(4) ^ _C(5) ^ _C(9) ^ _C(11) ^ _C(12) ^ _C(17) ^ _C(19) ^ _C(21) ^ _C(25) ^ _C(27) ^ _C(28) ^ _C(29);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(556) ^ _D(554) ^ _D(550) ^ _D(548) ^ _D(546) ^ _D(541) ^ _D(540) ^ _D(538) ^ _D(534) ^ _D(533) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(522) ^ _D(521) ^ _D(519) ^ _D(518) ^ _D(516) ^ _D(513) ^ _D(512) ^ _D(511) ^ _D(509) ^ _D(505) ^ _D(503) ^ _D(502) ^ _D(494) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(485) ^ _D(483) ^ _D(482) ^ _D(479) ^ _D(477) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(470) ^ _D(469) ^ _D(466) ^ _D(464) ^ _D(463) ^ _D(459) ^ _D(457) ^ _D(454) ^ _D(453) ^ _D(448) ^ _D(447) ^ _D(446) ^ _D(444) ^ _D(441) ^ _D(440) ^ _D(436) ^ _D(435) ^ _D(434) ^ _D(432) ^ _D(431) ^ _D(429) ^ _D(428) ^ _D(426) ^ _D(425) ^ _D(422) ^ _D(420) ^ _D(417) ^ _D(412) ^ _D(411) ^ _D(408) ^ _D(407) ^ _D(405) ^ _D(404) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(399) ^ _D(398) ^ _D(394) ^ _D(392) ^ _D(389) ^ _D(386) ^ _D(383) ^ _D(382) ^ _D(380) ^ _D(378) ^ _D(376) ^ _D(374) ^ _D(373) ^ _D(372) ^ _D(368) ^ _D(367) ^ _D(358) ^ _D(357) ^ _D(356) ^ _D(354) ^ _D(350) ^ _D(349) ^ _D(347) ^ _D(346) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(335) ^ _D(334) ^ _D(332) ^ _D(330) ^ _D(329) ^ _D(327) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(312) ^ _D(311) ^ _D(309) ^ _D(308) ^ _D(304) ^ _D(303) ^ _D(302) ^ _D(300) ^ _D(298) ^ _D(296) ^ _D(293) ^ _D(291) ^ _D(289) ^ _D(287) ^ _D(285) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(277) ^ _D(274) ^ _D(272) ^ _D(269) ^ _D(268) ^ _D(267) ^ _D(265) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(259) ^ _D(256) ^ _D(255) ^ _D(251) ^ _D(250) ^ _D(249) ^ _D(247) ^ _D(246) ^ _D(244) ^ _D(241) ^ _D(240) ^ _D(232) ^ _D(229) ^ _D(228) ^ _D(225) ^ _D(224) ^ _D(220) ^ _D(219) ^ _D(218) ^ _D(216) ^ _D(214) ^ _D(212) ^ _D(210) ^ _D(207) ^ _D(205) ^ _D(204) ^ _D(202) ^ _D(197) ^ _D(196) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(188) ^ _D(185) ^ _D(183) ^ _D(182) ^ _D(181) ^ _D(178) ^ _D(175) ^ _D(174) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(165) ^ _D(163) ^ _D(162) ^ _D(161) ^ _D(160) ^ _D(158) ^ _D(156) ^ _D(150) ^ _D(149) ^ _D(148) ^ _D(143) ^ _D(142) ^ _D(136) ^ _D(134) ^ _D(133) ^ _D(132) ^ _D(130) ^ _D(129) ^ _D(128) ^ _D(127) ^ _D(125) ^ _D(123) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(118) ^ _D(116) ^ _D(114) ^ _D(112) ^ _D(110) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(103) ^ _D(101) ^ _D(99) ^ _D(98) ^ _D(97) ^ _D(94) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(85) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(73) ^ _D(70) ^ _D(66) ^ _D(63) ^ _D(61) ^ _D(60) ^ _D(57) ^ _D(56) ^ _D(55) ^ _D(51) ^ _D(50) ^ _D(48) ^ _D(47) ^ _D(43) ^ _D(40) ^ _D(39) ^ _D(37) ^ _D(36) ^ _D(35) ^ _D(32) ^ _D(30) ^ _D(28) ^ _D(27) ^ _D(21) ^ _D(20) ^ _D(18) ^ _D(17) ^ _D(16) ^ _D(14) ^ _D(10) ^ _D(7) ^ _D(2) ^ _D(1) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(5) ^ _C(6) ^ _C(10) ^ _C(12) ^ _C(13) ^ _C(18) ^ _C(20) ^ _C(22) ^ _C(26) ^ _C(28) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(557) ^ _D(555) ^ _D(551) ^ _D(549) ^ _D(547) ^ _D(542) ^ _D(541) ^ _D(539) ^ _D(535) ^ _D(534) ^ _D(532) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(523) ^ _D(522) ^ _D(520) ^ _D(519) ^ _D(517) ^ _D(514) ^ _D(513) ^ _D(512) ^ _D(510) ^ _D(506) ^ _D(504) ^ _D(503) ^ _D(495) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(486) ^ _D(484) ^ _D(483) ^ _D(480) ^ _D(478) ^ _D(474) ^ _D(473) ^ _D(472) ^ _D(471) ^ _D(470) ^ _D(467) ^ _D(465) ^ _D(464) ^ _D(460) ^ _D(458) ^ _D(455) ^ _D(454) ^ _D(449) ^ _D(448) ^ _D(447) ^ _D(445) ^ _D(442) ^ _D(441) ^ _D(437) ^ _D(436) ^ _D(435) ^ _D(433) ^ _D(432) ^ _D(430) ^ _D(429) ^ _D(427) ^ _D(426) ^ _D(423) ^ _D(421) ^ _D(418) ^ _D(413) ^ _D(412) ^ _D(409) ^ _D(408) ^ _D(406) ^ _D(405) ^ _D(404) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(400) ^ _D(399) ^ _D(395) ^ _D(393) ^ _D(390) ^ _D(387) ^ _D(384) ^ _D(383) ^ _D(381) ^ _D(379) ^ _D(377) ^ _D(375) ^ _D(374) ^ _D(373) ^ _D(369) ^ _D(368) ^ _D(359) ^ _D(358) ^ _D(357) ^ _D(355) ^ _D(351) ^ _D(350) ^ _D(348) ^ _D(347) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(336) ^ _D(335) ^ _D(333) ^ _D(331) ^ _D(330) ^ _D(328) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(313) ^ _D(312) ^ _D(310) ^ _D(309) ^ _D(305) ^ _D(304) ^ _D(303) ^ _D(301) ^ _D(299) ^ _D(297) ^ _D(294) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(286) ^ _D(283) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(279) ^ _D(278) ^ _D(275) ^ _D(273) ^ _D(270) ^ _D(269) ^ _D(268) ^ _D(266) ^ _D(264) ^ _D(263) ^ _D(262) ^ _D(261) ^ _D(260) ^ _D(257) ^ _D(256) ^ _D(252) ^ _D(251) ^ _D(250) ^ _D(248) ^ _D(247) ^ _D(245) ^ _D(242) ^ _D(241) ^ _D(233) ^ _D(230) ^ _D(229) ^ _D(226) ^ _D(225) ^ _D(221) ^ _D(220) ^ _D(219) ^ _D(217) ^ _D(215) ^ _D(213) ^ _D(211) ^ _D(208) ^ _D(206) ^ _D(205) ^ _D(203) ^ _D(198) ^ _D(197) ^ _D(195) ^ _D(194) ^ _D(193) ^ _D(189) ^ _D(186) ^ _D(184) ^ _D(183) ^ _D(182) ^ _D(179) ^ _D(176) ^ _D(175) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(166) ^ _D(164) ^ _D(163) ^ _D(162) ^ _D(161) ^ _D(159) ^ _D(157) ^ _D(151) ^ _D(150) ^ _D(149) ^ _D(144) ^ _D(143) ^ _D(137) ^ _D(135) ^ _D(134) ^ _D(133) ^ _D(131) ^ _D(130) ^ _D(129) ^ _D(128) ^ _D(126) ^ _D(124) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(115) ^ _D(113) ^ _D(111) ^ _D(107) ^ _D(106) ^ _D(105) ^ _D(104) ^ _D(102) ^ _D(100) ^ _D(99) ^ _D(98) ^ _D(95) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(87) ^ _D(86) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(74) ^ _D(71) ^ _D(67) ^ _D(64) ^ _D(62) ^ _D(61) ^ _D(58) ^ _D(57) ^ _D(56) ^ _D(52) ^ _D(51) ^ _D(49) ^ _D(48) ^ _D(44) ^ _D(41) ^ _D(40) ^ _D(38) ^ _D(37) ^ _D(36) ^ _D(33) ^ _D(31) ^ _D(29) ^ _D(28) ^ _D(22) ^ _D(21) ^ _D(19) ^ _D(18) ^ _D(17) ^ _D(15) ^ _D(11) ^ _D(8) ^ _D(3) ^ _D(2) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(4) ^ _C(6) ^ _C(7) ^ _C(11) ^ _C(13) ^ _C(14) ^ _C(19) ^ _C(21) ^ _C(23) ^ _C(27) ^ _C(29) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(557) ^ _D(555) ^ _D(554) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(548) ^ _D(545) ^ _D(543) ^ _D(539) ^ _D(536) ^ _D(534) ^ _D(533) ^ _D(529) ^ _D(528) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(523) ^ _D(522) ^ _D(520) ^ _D(519) ^ _D(516) ^ _D(515) ^ _D(513) ^ _D(512) ^ _D(510) ^ _D(508) ^ _D(506) ^ _D(505) ^ _D(504) ^ _D(502) ^ _D(501) ^ _D(500) ^ _D(496) ^ _D(495) ^ _D(494) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(484) ^ _D(483) ^ _D(482) ^ _D(480) ^ _D(477) ^ _D(476) ^ _D(475) ^ _D(474) ^ _D(473) ^ _D(471) ^ _D(470) ^ _D(466) ^ _D(464) ^ _D(462) ^ _D(459) ^ _D(458) ^ _D(456) ^ _D(455) ^ _D(452) ^ _D(446) ^ _D(444) ^ _D(443) ^ _D(442) ^ _D(438) ^ _D(431) ^ _D(430) ^ _D(428) ^ _D(427) ^ _D(418) ^ _D(416) ^ _D(413) ^ _D(412) ^ _D(410) ^ _D(408) ^ _D(406) ^ _D(403) ^ _D(402) ^ _D(401) ^ _D(399) ^ _D(398) ^ _D(394) ^ _D(393) ^ _D(392) ^ _D(390) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(382) ^ _D(381) ^ _D(380) ^ _D(375) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(366) ^ _D(363) ^ _D(362) ^ _D(360) ^ _D(357) ^ _D(356) ^ _D(353) ^ _D(352) ^ _D(351) ^ _D(347) ^ _D(343) ^ _D(342) ^ _D(341) ^ _D(339) ^ _D(338) ^ _D(336) ^ _D(335) ^ _D(333) ^ _D(332) ^ _D(331) ^ _D(329) ^ _D(328) ^ _D(327) ^ _D(322) ^ _D(321) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(312) ^ _D(311) ^ _D(309) ^ _D(306) ^ _D(304) ^ _D(303) ^ _D(299) ^ _D(297) ^ _D(296) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(289) ^ _D(288) ^ _D(286) ^ _D(284) ^ _D(282) ^ _D(281) ^ _D(280) ^ _D(277) ^ _D(273) ^ _D(271) ^ _D(270) ^ _D(268) ^ _D(267) ^ _D(263) ^ _D(262) ^ _D(259) ^ _D(258) ^ _D(255) ^ _D(253) ^ _D(251) ^ _D(249) ^ _D(246) ^ _D(242) ^ _D(237) ^ _D(231) ^ _D(228) ^ _D(224) ^ _D(222) ^ _D(221) ^ _D(220) ^ _D(218) ^ _D(210) ^ _D(208) ^ _D(206) ^ _D(204) ^ _D(203) ^ _D(202) ^ _D(201) ^ _D(197) ^ _D(196) ^ _D(195) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(184) ^ _D(182) ^ _D(180) ^ _D(177) ^ _D(176) ^ _D(166) ^ _D(165) ^ _D(164) ^ _D(163) ^ _D(161) ^ _D(160) ^ _D(156) ^ _D(155) ^ _D(152) ^ _D(150) ^ _D(149) ^ _D(145) ^ _D(143) ^ _D(138) ^ _D(137) ^ _D(131) ^ _D(130) ^ _D(129) ^ _D(128) ^ _D(126) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(119) ^ _D(117) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(110) ^ _D(108) ^ _D(107) ^ _D(105) ^ _D(104) ^ _D(100) ^ _D(98) ^ _D(97) ^ _D(95) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(88) ^ _D(81) ^ _D(79) ^ _D(78) ^ _D(77) ^ _D(76) ^ _D(75) ^ _D(73) ^ _D(67) ^ _D(66) ^ _D(62) ^ _D(61) ^ _D(60) ^ _D(59) ^ _D(57) ^ _D(55) ^ _D(54) ^ _D(52) ^ _D(49) ^ _D(48) ^ _D(47) ^ _D(44) ^ _D(42) ^ _D(41) ^ _D(39) ^ _D(38) ^ _D(31) ^ _D(28) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(20) ^ _D(19) ^ _D(18) ^ _D(10) ^ _D(6) ^ _D(4) ^ _D(3) ^ _D(0) ^ _C(0) ^ _C(1) ^ _C(5) ^ _C(6) ^ _C(8) ^ _C(11) ^ _C(15) ^ _C(17) ^ _C(20) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(26) ^ _C(27) ^ _C(29) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(556) ^ _D(555) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(549) ^ _D(546) ^ _D(544) ^ _D(540) ^ _D(537) ^ _D(535) ^ _D(534) ^ _D(530) ^ _D(529) ^_D(527) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(523) ^ _D(521) ^ _D(520) ^ _D(517) ^ _D(516) ^ _D(514) ^ _D(513) ^ _D(511) ^ _D(509) ^ _D(507) ^ _D(506) ^ _D(505) ^_D(503) ^ _D(502) ^ _D(501) ^ _D(497) ^ _D(496) ^ _D(495) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(484) ^ _D(483) ^ _D(481) ^ _D(478) ^_D(477) ^ _D(476) ^ _D(475) ^ _D(474) ^ _D(472) ^ _D(471) ^ _D(467) ^ _D(465) ^ _D(463) ^ _D(460) ^ _D(459) ^ _D(457) ^ _D(456) ^ _D(453) ^ _D(447) ^ _D(445) ^_D(444) ^ _D(443) ^ _D(439) ^ _D(432) ^ _D(431) ^ _D(429) ^ _D(428) ^ _D(419) ^ _D(417) ^ _D(414) ^ _D(413) ^ _D(411) ^ _D(409) ^ _D(407) ^ _D(404) ^ _D(403) ^_D(402) ^ _D(400) ^ _D(399) ^ _D(395) ^ _D(394) ^ _D(393) ^ _D(391) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(383) ^ _D(382) ^ _D(381) ^ _D(376) ^ _D(373) ^_D(371) ^ _D(369) ^ _D(367) ^ _D(364) ^ _D(363) ^ _D(361) ^ _D(358) ^ _D(357) ^ _D(354) ^ _D(353) ^ _D(352) ^ _D(348) ^ _D(344) ^ _D(343) ^ _D(342) ^ _D(340) ^_D(339) ^ _D(337) ^ _D(336) ^ _D(334) ^ _D(333) ^ _D(332) ^ _D(330) ^ _D(329) ^ _D(328) ^ _D(323) ^ _D(322) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^_D(312) ^ _D(310) ^ _D(307) ^ _D(305) ^ _D(304) ^ _D(300) ^ _D(298) ^ _D(297) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(289) ^ _D(287) ^_D(285) ^ _D(283) ^ _D(282) ^ _D(281) ^ _D(278) ^ _D(274) ^ _D(272) ^ _D(271) ^ _D(269) ^ _D(268) ^ _D(264) ^ _D(263) ^ _D(260) ^ _D(259) ^ _D(256) ^ _D(254) ^_D(252) ^ _D(250) ^ _D(247) ^ _D(243) ^ _D(238) ^ _D(232) ^ _D(229) ^ _D(225) ^ _D(223) ^ _D(222) ^ _D(221) ^ _D(219) ^ _D(211) ^ _D(209) ^ _D(207) ^ _D(205) ^_D(204) ^ _D(203) ^ _D(202) ^ _D(198) ^ _D(197) ^ _D(196) ^ _D(194) ^ _D(193) ^ _D(192) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(185) ^ _D(183) ^ _D(181) ^_D(178) ^ _D(177) ^ _D(167) ^ _D(166) ^ _D(165) ^ _D(164) ^ _D(162) ^ _D(161) ^ _D(157) ^ _D(156) ^ _D(153) ^ _D(151) ^ _D(150) ^ _D(146) ^ _D(144) ^ _D(139) ^_D(138) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(129) ^ _D(127) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(120) ^ _D(118) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(111) ^ _D(109) ^_D(108) ^ _D(106) ^ _D(105) ^ _D(101) ^ _D(99) ^ _D(98) ^ _D(96) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(89) ^ _D(82) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(77) ^_D(76) ^ _D(74) ^ _D(68) ^ _D(67) ^ _D(63) ^ _D(62) ^ _D(61) ^ _D(60) ^ _D(58) ^ _D(56) ^ _D(55) ^ _D(53) ^ _D(50) ^ _D(49) ^ _D(48) ^ _D(45) ^ _D(43) ^ _D(42) ^ _D(40) ^_D(39) ^ _D(32) ^ _D(29) ^ _D(27) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(21) ^ _D(20) ^ _D(19) ^ _D(11) ^ _D(7) ^ _D(5) ^ _D(4) ^ _D(1) ^ _C(1) ^ _C(2) ^ _C(6) ^ _C(7) ^_C(9) ^ _C(12) ^ _C(16) ^ _C(18) ^ _C(21) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(27) ^ _C(28) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(557) ^ _D(556) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(550) ^ _D(547) ^ _D(545) ^ _D(541) ^ _D(538) ^ _D(536) ^ _D(535) ^ _D(531) ^ _D(530) ^ _D(528) ^ _D(527) ^ _D(526) ^ _D(525) ^ _D(524) ^ _D(522) ^ _D(521) ^ _D(518) ^ _D(517) ^ _D(515) ^ _D(514) ^ _D(512) ^ _D(510) ^ _D(508) ^ _D(507) ^ _D(506) ^ _D(504) ^ _D(503) ^ _D(502) ^ _D(498) ^ _D(497) ^ _D(496) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(484) ^ _D(482) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(476) ^ _D(475) ^ _D(473) ^ _D(472) ^ _D(468) ^ _D(466) ^ _D(464) ^ _D(461) ^ _D(460) ^ _D(458) ^ _D(457) ^ _D(454) ^ _D(448) ^ _D(446) ^ _D(445) ^ _D(444) ^ _D(440) ^ _D(433) ^ _D(432) ^ _D(430) ^ _D(429) ^ _D(420) ^ _D(418) ^ _D(415) ^ _D(414) ^ _D(412) ^ _D(410) ^ _D(408) ^ _D(405) ^ _D(404) ^ _D(403) ^ _D(401) ^ _D(400) ^ _D(396) ^ _D(395) ^ _D(394) ^ _D(392) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(386) ^ _D(384) ^ _D(383) ^ _D(382) ^ _D(377) ^ _D(374) ^ _D(372) ^ _D(370) ^ _D(368) ^ _D(365) ^ _D(364) ^ _D(362) ^ _D(359) ^ _D(358) ^ _D(355) ^ _D(354) ^ _D(353) ^ _D(349) ^ _D(345) ^ _D(344) ^ _D(343) ^ _D(341) ^ _D(340) ^ _D(338) ^ _D(337) ^ _D(335) ^ _D(334) ^ _D(333) ^ _D(331) ^ _D(330) ^ _D(329) ^ _D(324) ^ _D(323) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(313) ^ _D(311) ^ _D(308) ^ _D(306) ^ _D(305) ^ _D(301) ^ _D(299) ^ _D(298) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(290) ^ _D(288) ^ _D(286) ^ _D(284) ^ _D(283) ^ _D(282) ^ _D(279) ^ _D(275) ^ _D(273) ^ _D(272) ^ _D(270) ^ _D(269) ^ _D(265) ^ _D(264) ^ _D(261) ^ _D(260) ^ _D(257) ^ _D(255) ^ _D(253) ^ _D(251) ^ _D(248) ^ _D(244) ^ _D(239) ^ _D(233) ^ _D(230) ^ _D(226) ^ _D(224) ^ _D(223) ^ _D(222) ^ _D(220) ^ _D(212) ^ _D(210) ^ _D(208) ^ _D(206) ^ _D(205) ^ _D(204) ^ _D(203) ^ _D(199) ^ _D(198) ^ _D(197) ^ _D(195) ^ _D(194) ^ _D(193) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(186) ^ _D(184) ^ _D(182) ^ _D(179) ^ _D(178) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(165) ^ _D(163) ^ _D(162) ^ _D(158) ^ _D(157) ^ _D(154) ^ _D(152) ^ _D(151) ^ _D(147) ^ _D(145) ^ _D(140) ^ _D(139) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(130) ^ _D(128) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(121) ^ _D(119) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(112) ^ _D(110) ^ _D(109) ^ _D(107) ^ _D(106) ^ _D(102) ^ _D(100) ^ _D(99) ^ _D(97) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(90) ^ _D(83) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(77) ^ _D(75) ^ _D(69) ^ _D(68) ^ _D(64) ^ _D(63) ^ _D(62) ^ _D(61) ^ _D(59) ^ _D(57) ^ _D(56) ^ _D(54) ^ _D(51) ^ _D(50) ^ _D(49) ^ _D(46) ^ _D(44) ^ _D(43) ^ _D(41) ^ _D(40) ^ _D(33) ^ _D(30) ^ _D(28) ^ _D(27) ^ _D(26) ^ _D(25) ^ _D(24) ^ _D(22) ^ _D(21) ^ _D(20) ^ _D(12) ^ _D(8) ^ _D(6) ^ _D(5) ^ _D(2) ^ _C(0) ^ _C(2) ^ _C(3) ^ _C(7) ^ _C(8) ^ _C(10) ^ _C(13) ^ _C(17) ^ _C(19) ^ _C(22) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(28) ^ _C(29) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(558) ^ _D(557) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(551) ^ _D(548) ^ _D(546) ^ _D(542) ^ _D(539) ^ _D(537) ^ _D(536) ^ _D(532) ^ _D(531) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(526) ^ _D(525) ^ _D(523) ^ _D(522) ^ _D(519) ^ _D(518) ^ _D(516) ^ _D(515) ^ _D(513) ^ _D(511) ^ _D(509) ^ _D(508) ^ _D(507) ^ _D(505) ^ _D(504) ^ _D(503) ^ _D(499) ^ _D(498) ^ _D(497) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(485) ^ _D(483) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(476) ^ _D(474) ^ _D(473) ^ _D(469) ^ _D(467) ^ _D(465) ^ _D(462) ^ _D(461) ^ _D(459) ^ _D(458) ^ _D(455) ^ _D(449) ^ _D(447) ^ _D(446) ^ _D(445) ^ _D(441) ^ _D(434) ^ _D(433) ^ _D(431) ^ _D(430) ^ _D(421) ^ _D(419) ^ _D(416) ^ _D(415) ^ _D(413) ^ _D(411) ^ _D(409) ^ _D(406) ^ _D(405) ^ _D(404) ^ _D(402) ^ _D(401) ^ _D(397) ^ _D(396) ^ _D(395) ^ _D(393) ^ _D(390) ^ _D(389) ^ _D(388) ^ _D(387) ^ _D(385) ^ _D(384) ^ _D(383) ^ _D(378) ^ _D(375) ^ _D(373) ^ _D(371) ^ _D(369) ^ _D(366) ^ _D(365) ^ _D(363) ^ _D(360) ^ _D(359) ^ _D(356) ^ _D(355) ^ _D(354) ^ _D(350) ^ _D(346) ^ _D(345) ^ _D(344) ^ _D(342) ^ _D(341) ^ _D(339) ^ _D(338) ^ _D(336) ^ _D(335) ^ _D(334) ^ _D(332) ^ _D(331) ^ _D(330) ^ _D(325) ^ _D(324) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(314) ^ _D(312) ^ _D(309) ^ _D(307) ^ _D(306) ^ _D(302) ^ _D(300) ^ _D(299) ^ _D(297) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(291) ^ _D(289) ^ _D(287) ^ _D(285) ^ _D(284) ^ _D(283) ^ _D(280) ^ _D(276) ^ _D(274) ^ _D(273) ^ _D(271) ^ _D(270) ^ _D(266) ^ _D(265) ^ _D(262) ^ _D(261) ^ _D(258) ^ _D(256) ^ _D(254) ^ _D(252) ^ _D(249) ^ _D(245) ^ _D(240) ^ _D(234) ^ _D(231) ^ _D(227) ^ _D(225) ^ _D(224) ^ _D(223) ^ _D(221) ^ _D(213) ^ _D(211) ^ _D(209) ^ _D(207) ^ _D(206) ^ _D(205) ^ _D(204) ^ _D(200) ^ _D(199) ^ _D(198) ^ _D(196) ^ _D(195) ^ _D(194) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(187) ^ _D(185) ^ _D(183) ^ _D(180) ^ _D(179) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(166) ^ _D(164) ^ _D(163) ^ _D(159) ^ _D(158) ^ _D(155) ^ _D(153) ^ _D(152) ^ _D(148) ^ _D(146) ^ _D(141) ^ _D(140) ^ _D(134) ^ _D(133) ^ _D(132) ^ _D(131) ^ _D(129) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(122) ^ _D(120) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(113) ^ _D(111) ^ _D(110) ^ _D(108) ^ _D(107) ^ _D(103) ^ _D(101) ^ _D(100) ^ _D(98) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(91) ^ _D(84) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(78) ^ _D(76) ^ _D(70) ^ _D(69) ^ _D(65) ^ _D(64) ^ _D(63) ^ _D(62) ^ _D(60) ^ _D(58) ^ _D(57) ^ _D(55) ^ _D(52) ^ _D(51) ^ _D(50) ^ _D(47) ^ _D(45) ^ _D(44) ^ _D(42) ^ _D(41) ^ _D(34) ^ _D(31) ^ _D(29) ^ _D(28) ^ _D(27) ^ _D(26) ^ _D(25) ^ _D(23) ^ _D(22) ^ _D(21) ^ _D(13) ^ _D(9) ^ _D(7) ^ _D(6) ^ _D(3) ^ _C(0) ^ _C(1) ^ _C(3) ^ _C(4) ^ _C(8) ^ _C(9) ^ _C(11) ^ _C(14) ^ _C(18) ^ _C(20) ^ _C(23) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(29) ^ _C(30);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(558) ^ _D(556) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(552) ^ _D(549) ^ _D(547) ^ _D(543) ^ _D(540) ^ _D(538) ^ _D(537) ^ _D(533) ^ _D(532) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(526) ^ _D(524) ^ _D(523) ^ _D(520) ^ _D(519) ^ _D(517) ^ _D(516) ^ _D(514) ^ _D(512) ^ _D(510) ^ _D(509) ^ _D(508) ^ _D(506) ^ _D(505) ^ _D(504) ^ _D(500) ^ _D(499) ^ _D(498) ^ _D(493) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(486) ^ _D(484) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(477) ^ _D(475) ^ _D(474) ^ _D(470) ^ _D(468) ^ _D(466) ^ _D(463) ^ _D(462) ^ _D(460) ^ _D(459) ^ _D(456) ^ _D(450) ^ _D(448) ^ _D(447) ^ _D(446) ^ _D(442) ^ _D(435) ^ _D(434) ^ _D(432) ^ _D(431) ^ _D(422) ^ _D(420) ^ _D(417) ^ _D(416) ^ _D(414) ^ _D(412) ^ _D(410) ^ _D(407) ^ _D(406) ^ _D(405) ^ _D(403) ^ _D(402) ^ _D(398) ^ _D(397) ^ _D(396) ^ _D(394) ^ _D(391) ^ _D(390) ^ _D(389) ^ _D(388) ^ _D(386) ^ _D(385) ^ _D(384) ^ _D(379) ^ _D(376) ^ _D(374) ^ _D(372) ^ _D(370) ^ _D(367) ^ _D(366) ^ _D(364) ^ _D(361) ^ _D(360) ^ _D(357) ^ _D(356) ^ _D(355) ^ _D(351) ^ _D(347) ^ _D(346) ^ _D(345) ^ _D(343) ^ _D(342) ^ _D(340) ^ _D(339) ^ _D(337) ^ _D(336) ^ _D(335) ^ _D(333) ^ _D(332) ^ _D(331) ^ _D(326) ^ _D(325) ^ _D(320) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(315) ^ _D(313) ^ _D(310) ^ _D(308) ^ _D(307) ^ _D(303) ^ _D(301) ^ _D(300) ^ _D(298) ^ _D(297) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(292) ^ _D(290) ^ _D(288) ^ _D(286) ^ _D(285) ^ _D(284) ^ _D(281) ^ _D(277) ^ _D(275) ^ _D(274) ^ _D(272) ^ _D(271) ^ _D(267) ^ _D(266) ^ _D(263) ^ _D(262) ^ _D(259) ^ _D(257) ^ _D(255) ^ _D(253) ^ _D(250) ^ _D(246) ^ _D(241) ^ _D(235) ^ _D(232) ^ _D(228) ^ _D(226) ^ _D(225) ^ _D(224) ^ _D(222) ^ _D(214) ^ _D(212) ^ _D(210) ^ _D(208) ^ _D(207) ^ _D(206) ^ _D(205) ^ _D(201) ^ _D(200) ^ _D(199) ^ _D(197) ^ _D(196) ^ _D(195) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(188) ^ _D(186) ^ _D(184) ^ _D(181) ^ _D(180) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(167) ^ _D(165) ^ _D(164) ^ _D(160) ^ _D(159) ^ _D(156) ^ _D(154) ^ _D(153) ^ _D(149) ^ _D(147) ^ _D(142) ^ _D(141) ^ _D(135) ^ _D(134) ^ _D(133) ^ _D(132) ^ _D(130) ^ _D(126) ^ _D(125) ^ _D(124) ^ _D(123) ^ _D(121) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(114) ^ _D(112) ^ _D(111) ^ _D(109) ^ _D(108) ^ _D(104) ^ _D(102) ^ _D(101) ^ _D(99) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(92) ^ _D(85) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(79) ^ _D(77) ^ _D(71) ^ _D(70) ^ _D(66) ^ _D(65) ^ _D(64) ^ _D(63) ^ _D(61) ^ _D(59) ^ _D(58) ^ _D(56) ^ _D(53) ^ _D(52) ^ _D(51) ^ _D(48) ^ _D(46) ^ _D(45) ^ _D(43) ^ _D(42) ^ _D(35) ^ _D(32) ^ _D(30) ^ _D(29) ^ _D(28) ^ _D(27) ^ _D(26) ^ _D(24) ^ _D(23) ^ _D(22) ^ _D(14) ^ _D(10) ^ _D(8) ^ _D(7) ^ _D(4) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(4) ^ _C(5) ^ _C(9) ^ _C(10) ^ _C(12) ^ _C(15) ^ _C(19) ^ _C(21) ^ _C(24) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(28) ^ _C(30) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;
	tmpCrc =
		_D(559) ^ _D(557) ^ _D(556) ^ _D(555) ^ _D(554) ^ _D(553) ^ _D(550) ^ _D(548) ^ _D(544) ^ _D(541) ^ _D(539) ^ _D(538) ^ _D(534) ^ _D(533) ^ _D(531) ^ _D(530) ^ _D(529) ^ _D(528) ^ _D(527) ^ _D(525) ^ _D(524) ^ _D(521) ^ _D(520) ^ _D(518) ^ _D(517) ^ _D(515) ^ _D(513) ^ _D(511) ^ _D(510) ^ _D(509) ^ _D(507) ^ _D(506) ^ _D(505) ^ _D(501) ^ _D(500) ^ _D(499) ^ _D(494) ^ _D(493) ^ _D(492) ^ _D(491) ^ _D(490) ^ _D(489) ^ _D(488) ^ _D(487) ^ _D(485) ^ _D(482) ^ _D(481) ^ _D(480) ^ _D(479) ^ _D(478) ^ _D(476) ^ _D(475) ^ _D(471) ^ _D(469) ^ _D(467) ^ _D(464) ^ _D(463) ^ _D(461) ^ _D(460) ^ _D(457) ^ _D(451) ^ _D(449) ^ _D(448) ^ _D(447) ^ _D(443) ^ _D(436) ^ _D(435) ^ _D(433) ^ _D(432) ^ _D(423) ^ _D(421) ^ _D(418) ^ _D(417) ^ _D(415) ^ _D(413) ^ _D(411) ^ _D(408) ^ _D(407) ^ _D(406) ^ _D(404) ^ _D(403) ^ _D(399) ^ _D(398) ^ _D(397) ^ _D(395) ^ _D(392) ^ _D(391) ^ _D(390) ^ _D(389) ^ _D(387) ^ _D(386) ^ _D(385) ^ _D(380) ^ _D(377) ^ _D(375) ^ _D(373) ^ _D(371) ^ _D(368) ^ _D(367) ^ _D(365) ^ _D(362) ^ _D(361) ^ _D(358) ^ _D(357) ^ _D(356) ^ _D(352) ^ _D(348) ^ _D(347) ^ _D(346) ^ _D(344) ^ _D(343) ^ _D(341) ^ _D(340) ^ _D(338) ^ _D(337) ^ _D(336) ^ _D(334) ^ _D(333) ^ _D(332) ^ _D(327) ^ _D(326) ^ _D(321) ^ _D(320) ^ _D(319) ^ _D(318) ^ _D(317) ^ _D(316) ^ _D(314) ^ _D(311) ^ _D(309) ^ _D(308) ^ _D(304) ^ _D(302) ^ _D(301) ^ _D(299) ^ _D(298) ^ _D(297) ^ _D(296) ^ _D(295) ^ _D(294) ^ _D(293) ^ _D(291) ^ _D(289) ^ _D(287) ^ _D(286) ^ _D(285) ^ _D(282) ^ _D(278) ^ _D(276) ^ _D(275) ^ _D(273) ^ _D(272) ^ _D(268) ^ _D(267) ^ _D(264) ^ _D(263) ^ _D(260) ^ _D(258) ^ _D(256) ^ _D(254) ^ _D(251) ^ _D(247) ^ _D(242) ^ _D(236) ^ _D(233) ^ _D(229) ^ _D(227) ^ _D(226) ^ _D(225) ^ _D(223) ^ _D(215) ^ _D(213) ^ _D(211) ^ _D(209) ^ _D(208) ^ _D(207) ^ _D(206) ^ _D(202) ^ _D(201) ^ _D(200) ^ _D(198) ^ _D(197) ^ _D(196) ^ _D(193) ^ _D(192) ^ _D(191) ^ _D(190) ^ _D(189) ^ _D(187) ^ _D(185) ^ _D(182) ^ _D(181) ^ _D(171) ^ _D(170) ^ _D(169) ^ _D(168) ^ _D(166) ^ _D(165) ^ _D(161) ^ _D(160) ^ _D(157) ^ _D(155) ^ _D(154) ^ _D(150) ^ _D(148) ^ _D(143) ^ _D(142) ^ _D(136) ^ _D(135) ^ _D(134) ^ _D(133) ^ _D(131) ^ _D(127) ^ _D(126) ^ _D(125) ^ _D(124) ^ _D(122) ^ _D(118) ^ _D(117) ^ _D(116) ^ _D(115) ^ _D(113) ^ _D(112) ^ _D(110) ^ _D(109) ^ _D(105) ^ _D(103) ^ _D(102) ^ _D(100) ^ _D(98) ^ _D(97) ^ _D(96) ^ _D(95) ^ _D(94) ^ _D(93) ^ _D(86) ^ _D(84) ^ _D(83) ^ _D(82) ^ _D(81) ^ _D(80) ^ _D(78) ^ _D(72) ^ _D(71) ^ _D(67) ^ _D(66) ^ _D(65) ^ _D(64) ^ _D(62) ^ _D(60) ^ _D(59) ^ _D(57) ^ _D(54) ^ _D(53) ^ _D(52) ^ _D(49) ^ _D(47) ^ _D(46) ^ _D(44) ^ _D(43) ^ _D(36) ^ _D(33) ^ _D(31) ^ _D(30) ^ _D(29) ^ _D(28) ^ _D(27) ^ _D(25) ^ _D(24) ^ _D(23) ^ _D(15) ^ _D(11) ^ _D(9) ^ _D(8) ^ _D(5) ^ _C(0) ^ _C(1) ^ _C(2) ^ _C(3) ^ _C(5) ^ _C(6) ^ _C(10) ^ _C(11) ^ _C(13) ^ _C(16) ^ _C(20) ^ _C(22) ^ _C(25) ^ _C(26) ^ _C(27) ^ _C(28) ^ _C(29) ^ _C(31);

	setBitFromValue(newCrc,tmpCrc,index);
	index++;

	return newCrc;
}


