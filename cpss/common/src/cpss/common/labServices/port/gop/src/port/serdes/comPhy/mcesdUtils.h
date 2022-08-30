/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
* mcesdUtils.h
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
#ifndef MCESD_UTILS_H
#define MCESD_UTILS_H

#define MAX_LINE_LEN 80

/**
@brief  Returns the version number of this API.

@param[out] major - major version number
@param[out] minor - minor version number
@param[out] buildID - if non-released software, this number will be non-zero indicating the interim version between releases

@note The non-zero build ID is used for test and preview releases.  General release build ID is 0.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_VOID mcesdGetAPIVersion
(
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *buildID
);

/**
@brief  Return if MCU is active

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  mcuStatus0FieldPtr - pointer to field to MCU status 0
@param[in]  laneSelFieldPtr - pointer to field to lane select
@param[in]  laneCount - number of lanes to check

@param[out] mcuActive - True if MCU is active

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS mcesdGetMCUActive
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR mcuStatus0FieldPtr,
    IN MCESD_FIELD_PTR laneSelFieldPtr,
    IN MCESD_U8 laneCount,
    OUT MCESD_BOOL *mcuActive
);

/**
@brief  Return Major Minor Revision of IP

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] ipMajorRev - Major revision of IP
@param[out] ipMinorRev - Minor revision of IP

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS mcesdGetIPRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *ipMajorRev,
    OUT MCESD_U8 *ipMinorRev
);

#ifdef MCESD_DEBUG
/**
@brief  Debug Printing

@param[in]  stream - file stream
@param[in]  debug_level - control of how much is printed
@param[in]  format - string
@param[in]  ... - ...

@note Set global MCESD_debug_level to MCESD_DBG_OFF_LVL to turn off all printing, or MCESD_DBG_ERR_LVL to print only errors, or MCESD_DBG_ALL_LVL for both errors and informational messages.
@note This can be done during run time or the global default can be changed at compile time to change the run-time behavior.
@note Undefine MCESD_DEBUG to remove all printing code (may need some modifications on some compilers).

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
void mcesdDbgPrint
(
    FILE *stream,
    MCESD_DBG_LEVEL debug_level,
    char *format,
    ...
);
#endif

MCESD_32 ConvertSignedMagnitudeToI32
(
    IN MCESD_U32 sm,
    IN MCESD_U16 smBits
);

MCESD_32 ConvertTwosComplementToI32
(
    IN MCESD_U32 tc,
    IN MCESD_U16 tcBits
);

MCESD_STATUS LoadFwDataFileToBuffer
(
    IN const char *fileName,
    IN MCESD_U32 *bufferPtr,        /* buffer pointer to store code */
    IN MCESD_U32 bufferSizeDW,      /* buffer size in DWORDS */
    OUT MCESD_U32 *actualSizeDW,    /* actual file size in DWORDS */
    OUT MCESD_U16 *errCode
);

MCESD_STATUS PatternStringToU8Array
(
    IN const char *hexString,
    OUT MCESD_U8 *u8Array
);

MCESD_STATUS GenerateStringFromU8Array
(
    IN MCESD_U8 *u8Array,
    OUT char *hexString
);

MCESD_U32 ConvertU32ToGrayCode
(
    IN MCESD_U32 raw
);

MCESD_U32 ConvertGrayCodeToU32
(
    IN MCESD_U32 grayCode
);

MCESD_STATUS calculateChecksum
(
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    OUT MCESD_U32 *checksum
);

#endif /* defined MCESD_UTILS_H */
