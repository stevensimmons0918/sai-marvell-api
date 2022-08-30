/* log.c
 *
 * Log implementation for specific environment
 */

/*****************************************************************************
* Copyright (c) 2008-2019 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

#define LOG_SEVERITY_MAX  LOG_SEVERITY_NO_OUTPUT

/* Logging API */
#include <Kit/Log/incl/log.h>            /* the API to implement */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*----------------------------------------------------------------------------
 * Log_HexDump
 *
 * This function logs Hex Dump of a Buffer
 *
 * szPrefix
 *     Prefix to be printed on every row.
 *
 * PrintOffset
 *     Offset value that is printed at the start of every row. Can be used
 *     when the byte printed are located at some offset in another buffer.
 *
 * Buffer_p
 *     Pointer to the start of the array of bytes to hex dump.
 *
 * ByteCount
 *     Number of bytes to include in the hex dump from Buffer_p.
 *
 * Return Value
 *     None.
 */
void
Log_HexDump(
        const char * szPrefix_p,
        const unsigned int PrintOffset,
        const uint8_t * Buffer_p,
        const unsigned int ByteCount)
{
    unsigned int i;

    for(i = 0; i < ByteCount; i += 16)
    {
        unsigned int j, Limit;

        /* if we do not have enough data for a full line */
        if (i + 16 > ByteCount)
            Limit = ByteCount - i;
        else
            Limit = 16;

        Log_FormattedMessage("%s %08d:", szPrefix_p, PrintOffset + i);

        for (j = 0; j < Limit; j++)
            Log_FormattedMessage(" %02X", Buffer_p[i+j]);

        Log_FormattedMessage("\n");
    } /* for */
}


/*----------------------------------------------------------------------------
 * Log_HexDump32
 *
 * This function logs Hex Dump of an array of 32-bit words
 *
 * szPrefix
 *     Prefix to be printed on every row.
 *
 * PrintOffset
 *     Offset value that is printed at the start of every row. Can be used
 *     when the byte printed are located at some offset in another buffer.
 *
 * Buffer_p
 *     Pointer to the start of the array of 32-bit words to hex dump.
 *
 * Word32Count
 *     Number of 32-bit words to include in the hex dump from Buffer_p.
 *
 * Return Value
 *     None.
 */
void
Log_HexDump32(
        const char * szPrefix_p,
        const unsigned int PrintOffset,
        const uint32_t * Buffer_p,
        const unsigned int Word32Count)
{
    unsigned int i;

    for(i = 0; i < Word32Count; i += 4)
    {
        unsigned int j, Limit;

        /* if we do not have enough data for a full line */
        if (i + 4 > Word32Count)
            Limit = Word32Count - i;
        else
            Limit = 4;

        Log_FormattedMessage("%s %08d:", szPrefix_p, PrintOffset + i*4);

        for (j = 0; j < Limit; j++)
            Log_FormattedMessage(" %08X", Buffer_p[i+j]);

        Log_FormattedMessage("\n");
    } /* for */
}

/* end of file log.c */
