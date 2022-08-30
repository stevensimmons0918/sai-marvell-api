/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
* mcesdUtils.c
*
* DESCRIPTION:
*       Collection of Utility functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <serdes/mcesd/mcesdTop.h>
#include <serdes/mcesd/mcesdApiTypes.h>
#include <serdes/mcesd/mcesdUtils.h>

MCESD_VOID mcesdGetAPIVersion
(
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *buildID
)
{
    *major = MCESD_API_MAJOR_VERSION;
    *minor = MCESD_API_MINOR_VERSION;
    *buildID = MCESD_API_BUILD_ID;
}

MCESD_STATUS mcesdGetMCUActive
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR mcuStatus0FieldPtr,
    IN MCESD_FIELD_PTR laneSelFieldPtr,
    IN MCESD_U8 laneCount,
    OUT MCESD_BOOL *mcuActive
)
{
    MCESD_U32 regValue, combinedValue;
    MCESD_U8 lane;
    /* Check to see if MCU is active */
    for (lane = 0; lane < laneCount; lane++)
    {
        devPtr->fmcesdReadReg(devPtr, laneSelFieldPtr->reg, &regValue);

        /* Modify the register value with the desired field value */
        combinedValue = (regValue & laneSelFieldPtr->retainMask) | (lane << laneSelFieldPtr->loBit);

        devPtr->fmcesdWriteReg(devPtr, laneSelFieldPtr->reg, combinedValue);

        devPtr->fmcesdReadReg(devPtr, mcuStatus0FieldPtr->reg, &regValue);

        if (0 != regValue)
        {
            *mcuActive = MCESD_TRUE;
            return MCESD_OK;
        }
    }
    *mcuActive = MCESD_FALSE;
    return MCESD_OK;
}

MCESD_STATUS mcesdGetIPRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *ipMajorRev,
    OUT MCESD_U8 *ipMinorRev
)
{
    *ipMajorRev = devPtr->ipMajorRev; 
    *ipMinorRev = devPtr->ipMinorRev;
    return MCESD_OK;
}

#ifdef MCESD_DEBUG

/* Set default debug level */
MCESD_DBG_LEVEL mcesd_debug_level = MCESD_DBG_ALL_LVL;

#ifdef _WINDOWS
void mcesdDbgPrint
(
    FILE *stream, 
    MCESD_DBG_LEVEL debug_level, 
    char* format,
    ...
)
{
    va_list argP;
    char dbgStr[1000] = "";
    (void)stream;

    if (debug_level)
    {
        if (debug_level <= mcesd_debug_level)
        {
            va_start(argP, format);

            vsprintf(dbgStr, format, argP);

            OutputDebugString( dbgStr );

            va_end(argP);
        }
    }
}
#endif

#else 

/* All printing code removed */

#endif /* MCESD_DEBUG */

MCESD_32 ConvertSignedMagnitudeToI32
(
    IN MCESD_U32 sm,
    IN MCESD_U16 smBits
)
{
    if ((sm >> (smBits - 1)) == 0)
        return (MCESD_32) sm;
    else
    {
        MCESD_32 value = (MCESD_32) (sm & ((1 << (smBits - 1)) - 1));
        return -value;
    }
}

MCESD_32 ConvertTwosComplementToI32
(
    IN MCESD_U32 tc,
    IN MCESD_U16 tcBits
)
{
    if ((tc >> (tcBits - 1)) == 0)
        return (MCESD_32)tc;
    else
    {
        MCESD_U32 inverted = (~tc) + 1;
        MCESD_32 value = (MCESD_32)(inverted & ((1 << (tcBits - 1)) - 1));
        return -value;
    }
}

MCESD_STATUS LoadFwDataFileToBuffer
(
    IN const char *fileName,
    IN MCESD_U32 *bufferPtr,        /* buffer pointer to store code */
    IN MCESD_U32 bufferSizeDW,      /* buffer size in DWORDS */
    OUT MCESD_U32 *actualSizeDW,    /* actual file size in DWORDS */
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 lineIndex = 0;
    char line[MAX_LINE_LEN];
    FILE *codeFile;

    *errCode = 0;
    *actualSizeDW = 0;

    codeFile = fopen(fileName, "r");
    if (codeFile == NULL)
    {
        *errCode = MCESD_IMAGE_FILE_DOESNT_EXIST;
        return MCESD_FAIL;
    }

    while (fgets(line, MAX_LINE_LEN, codeFile) != NULL)
    {
        char *endPtr;

        if (lineIndex >= bufferSizeDW)
        {
            *errCode = MCESD_IMAGE_FILE_EXCEEDS_BUFFER;
            fclose(codeFile); /* KW */
            return MCESD_FAIL;
        }
            
        bufferPtr[lineIndex++] = (MCESD_U32) strtoul(line, &endPtr, 16);
    }

    *actualSizeDW = lineIndex;
    fclose(codeFile); /* KW */
    return MCESD_OK;
}

MCESD_STATUS PatternStringToU8Array
(
    IN const char *hexString,
    OUT MCESD_U8 *u8Array
)
{
    char normalizedHexString[21] = "00000000000000000000";
    int nibbles = strlen(hexString);
    int normalizedStart = 20 - nibbles;
    int i;

    if (nibbles > 20)
        return MCESD_FAIL; /* Pattern String exceeds 80 bit maximum */

    for (i = 0; i < nibbles; i++)
        normalizedHexString[normalizedStart + i] = hexString[i];

    for (i = 0; i < 10; i++)
    {
        char *endPtr;
        char byteString[3];
        const char *bytePtr = &(normalizedHexString[i*2]);

        strncpy(byteString, bytePtr, 2);
        byteString[2] = '\0';
        u8Array[ i ] = (MCESD_U8) strtol(byteString, &endPtr, 16);
    }

    return MCESD_OK;
}

MCESD_STATUS GenerateStringFromU8Array
(
    IN MCESD_U8 *u8Array,
    OUT char *hexString
)
{
    char* hexArray = "0123456789ABCDEF";
    MCESD_32 i;
    for (i = 0; i < 10; i++)
    {
        hexString[i*2] = hexArray[u8Array[i] >> 4];
        hexString[i * 2 + 1] = hexArray[u8Array[i] & 0xF];
    }
    hexString[20] = '\0';
    return MCESD_OK;
}

MCESD_U32 ConvertU32ToGrayCode
(
    IN MCESD_U32 raw
)
{
    return raw ^ (raw >> 1);
}

MCESD_U32 ConvertGrayCodeToU32
(
    IN MCESD_U32 grayCode
)
{
    grayCode ^= grayCode >> 16;
    grayCode ^= grayCode >> 8;
    grayCode ^= grayCode >> 4;
    grayCode ^= grayCode >> 2;
    grayCode ^= grayCode >> 1;
    return grayCode;
}

MCESD_STATUS mtdCalculateChecksum
(
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    OUT MCESD_U32 *checksum
)
{
    MCESD_U32 index;

    *checksum = 0;
    for (index = 0; index < codeSize; index++)
        *checksum += code[index];

    return MCESD_OK;
}
