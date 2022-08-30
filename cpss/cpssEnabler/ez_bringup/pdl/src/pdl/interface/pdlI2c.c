/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlI2c.c
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - Button related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlI2CDebug.h>
#include <pdl/interface/pdlGpioDebug.h>
#ifdef LINUX_HW
#include    <linux/i2c.h>
#include    <linux/i2c-dev.h>
#include    <fcntl.h>
#include    <sys/ioctl.h>
/*#include    <stropts.h>*/
#include    <unistd.h>
extern int usleep(unsigned int);
#endif
#include <pdl/lib/pdlLib.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   Number of pdl i 2c interfaces */
static UINT_32                  prvPdlI2cInterfaceCount = 0;          /* counter used as an ID for new interfaces */
/** @brief   The pdl i 2c database */
static PDL_DB_PRV_STC           prvPdlI2cDb;
/** @brief   The pdl i 2c bus fd arr[ pdl i2c bus identifier max] */
#ifdef LINUX_HW
static int                      pdlI2cBusFdArr[PDL_I2C_BUS_ID_MAX];
#endif

/**
 * @brief   Zero-based index of the 2c access size enum to name[ pdl interface i 2 c access size
 *          count]
 */

static PDL_I2C_ACCESS_SIZE_ENUM_STR_STC i2cAccessSizeEnumToName[PDL_INTERFACE_I2C_ACCESS_SIZE_COUNT] = {
    {PDL_I2C_ACCESS_SIZE_ONE_BYTE_VALUE, PDL_INTERFACE_I2C_ACCESS_SIZE_1_BYTE_E},
    {PDL_I2C_ACCESS_SIZE_TWO_BYTE_VALUE, PDL_INTERFACE_I2C_ACCESS_SIZE_2_BYTE_E},
    {PDL_I2C_ACCESS_SIZE_FOUR_BYTE_VALUE, PDL_INTERFACE_I2C_ACCESS_SIZE_4_BYTE_E}
};

/** @brief   True if pdl I2C bus identifier is used[ pdl I2C bus identifier max] */
static BOOLEAN                  pdlI2cBusIdIsUsed[PDL_I2C_BUS_ID_MAX+1] = {0};
#ifdef LINUX_HW
/** @brief   The pdl I2C number of retries */
static int                      pdlI2cNumOfRetries = 2;
#endif /*LINUX_HW*/

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/
/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cTransmit
*
* DESCRIPTION:   transmit i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cTransmit ( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, IN UINT_8 * buffer )
 *
 * @brief   Prv pdl i 2c transmit
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cTransmit (
    IN UINT_8     slave_address,   /* the target device slave address on I2C bus */
    IN UINT_32    buffer_size,     /* buffer length */
    IN UINT_8     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
    IN UINT_8   * buffer           /* transmited buffer */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
        int try, ret;
        struct i2c_rdwr_ioctl_data i2c_data;
        struct i2c_msg msg;
        unsigned char buf[PDL_I2C_MAX_MSG_LEN];
        PDL_STATUS              pdlStatus;
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (buffer_size > PDL_I2C_MAX_MSG_LEN -1 ) {
            return PDL_I2C_ERROR;
    }
    memcpy(&buf, buffer, buffer_size);
    /*
        * write operation
        */
    i2c_data.msgs = &msg;
    i2c_data.nmsgs = 1;                     /* use one message structure */

    i2c_data.msgs[0].addr = slave_address;
    i2c_data.msgs[0].flags = 0;             /* don't need flags */
    i2c_data.msgs[0].len = buffer_size;
    i2c_data.msgs[0].buf = (__u8 *)buf;

    /*
        * ioctl() processes read & write.
        * Operation is determined by flags field of i2c_msg
        */
    try = 0;
    do {
        ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
        pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
        if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
        try++;
        usleep (20000);
    } while (1);
    if (pdlStatus!=PDL_OK) {
        prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, buffer[0], TRUE);
    }
    return pdlStatus;

#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cTransmit */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cReceive
*
* DESCRIPTION:   receive i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cReceive ( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, OUT UINT_8 * buffer )
 *
 * @brief   Prv pdl i 2c receive
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cReceive (
    IN  UINT_8     slave_address,   /* the target device slave address on I2C bus */
    IN  UINT_32    buffer_size,     /* buffer length */
    IN  UINT_8     bus_id,          /* the I2C bus id */
    OUT UINT_8   * buffer           /* received buffer */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
        int try, ret;
        struct i2c_rdwr_ioctl_data i2c_data;
        struct i2c_msg msg[1];
        PDL_STATUS              pdlStatus;
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    i2c_data.msgs = msg;
    i2c_data.nmsgs = 1;    /* two i2c_msg */

    i2c_data.msgs[0].addr = slave_address;
    i2c_data.msgs[0].flags = I2C_M_RD;      /* read command */
    i2c_data.msgs[0].len = buffer_size;
    i2c_data.msgs[0].buf = (__u8 *)buffer;


    try = 0;
    do {
        ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
        pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
        if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
        try++;
        usleep (20000);
    } while (1);
    if (pdlStatus!=PDL_OK) {
        prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, buffer[0], FALSE);
    }
    return pdlStatus;
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cReceive */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cReceiveWithOffset
*
* DESCRIPTION:   receive i2c value with offset
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cReceiveWithOffset( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, IN UINT_32 offset_type, IN UINT_32 offset, OUT UINT_8 * buffer )
 *
 * @brief   Prv pdl i 2c receive with offset
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param           offset_type     Type of the offset.
 * @param           offset          The offset.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS  prvPdlI2cReceiveWithOffset(
  IN  UINT_8                                    slave_address,   /* the target device slave address on I2C bus */
  IN  UINT_32                                   buffer_size,     /* buffer length */
  IN  UINT_8                                    bus_id,          /* the I2C bus id */
  IN  UINT_32                                   offset_type,     /* receive mode */
  IN  UINT_32                                   offset,          /* internal offset to read from */
  IN  PDL_INTERFACE_I2C_TRANSACTION_TYPE_ENT    transaction_type,
  OUT UINT_8                                  * buffer           /* received buffer */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
        int try, ret, msg_index = 0, offset_len = 1;
        struct i2c_rdwr_ioctl_data i2c_data;
        struct i2c_msg msg[2];
        char tmp[2];
        PDL_STATUS              pdlStatus;
        UINT_8                  offsetByte;
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    if (transaction_type == PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA) {
        /* If offset type is 8 or 16 bits,
        * two messages are required within this transaction.
        * First determine offset address
        * Second - receive data.
        */
        i2c_data.msgs = msg;

        i2c_data.msgs[msg_index].addr = slave_address;
        i2c_data.nmsgs = 1;                   /* single i2c_msg */

        switch(offset_type) {
        case 0:
                break;
        case PDL_I2C_OFFSET_2_BYTE_SIZE:
                /* "Write" part of the transaction */
                tmp[1] = (char)((offset >> 8) & 0xFF);
                offset_len = 2;
                /* fall through */
        case PDL_I2C_OFFSET_1_BYTE_SIZE:
                i2c_data.nmsgs = 2;                   /* two i2c_msg */
                i2c_data.msgs[msg_index].flags = 0;   /* No flags for write command */
                i2c_data.msgs[msg_index].len = offset_len;   /* only one byte */
                i2c_data.msgs[msg_index++].buf =  (__u8 *)tmp;
                tmp[0] = (char)(offset & 0xFF);
                break;
        default:
            return PDL_BAD_PARAM;
        }
        /*
            * "Read" part of the transaction
            */
        i2c_data.msgs[msg_index].addr = slave_address;
        i2c_data.msgs[msg_index].flags = I2C_M_RD;  /* read command */
        i2c_data.msgs[msg_index].len = buffer_size;
        i2c_data.msgs[msg_index].buf = (__u8 *)buffer;
        try = 0;
        do {
            ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
            pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
            if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
            try++;
            usleep (20000);
        } while (1);
        if (pdlStatus!=PDL_OK) {
            prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, (UINT_8)offset, FALSE);
        }
    }
    else {
        offsetByte = (UINT_8) offset;
        /* prvPdlI2cTransmit() already handles failure case */
        pdlStatus = prvPdlI2cTransmit (slave_address, offset_type, bus_id, &offsetByte);
        PDL_CHECK_STATUS(pdlStatus);
        /* prvPdlI2cReceive() already handles failure case */
        pdlStatus = prvPdlI2cReceive(slave_address, buffer_size, bus_id, buffer);
    }
    return pdlStatus;
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cReceiveWithOffset */


/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConfigureMuxes
*
* DESCRIPTION:   configure all muxes in I2C interface
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConfigureMuxes ( IN PDL_INTERFACE_PRV_I2C_DB_STC * i2cPtr )
 *
 * @brief   Prv pdl i 2 c configure muxes
 *
 * @param [in,out]  i2cPtr  If non-null, zero-based index of the 2c pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConfigureMuxes (
    IN  PDL_INTERFACE_PRV_I2C_DB_STC          * i2cPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_8                                      offset_and_data_buf[20];
    UINT_8                                    * offset_and_data_buf_PTR;
    UINT_32                                     i;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_MUX_STC                     * muxInfoPtr;
    PDL_INTERFACE_PRV_I2C_MUX_KEY_STC           muxKey;
    UINT_32                                     oldData = 0, writeValue;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* configure all muxes */
    if (i2cPtr->publicInfo.numberOfMuxes > 0) {
        for (i = 1; i < i2cPtr->publicInfo.numberOfMuxes; i++) {
            muxKey.muxNum = i;
            pdlStatus  = prvPdlDbFind (&i2cPtr->muxDb, (void*) &muxKey, (void**) &muxInfoPtr);
            PDL_CHECK_STATUS (pdlStatus);
            if (muxInfoPtr->interfaceType == PDL_INTERFACE_TYPE_I2C_E) {
                writeValue = muxInfoPtr->interfaceInfo.i2cMux.value;
                /* read old data in offset if mask is not max value */
                if (muxInfoPtr->interfaceInfo.i2cMux.mask != MAX_UINT_32) {
                    pdlStatus = prvPdlI2cReceiveWithOffset (muxInfoPtr->interfaceInfo.i2cMux.baseAddress, muxInfoPtr->interfaceInfo.i2cMux.accessSize, (UINT_8)muxInfoPtr->interfaceInfo.i2cMux.busId, PDL_I2C_OFFSET_SIZE, muxInfoPtr->interfaceInfo.i2cMux.offset, muxInfoPtr->interfaceInfo.i2cMux.transactionType, (UINT_8*) & oldData);
                    PDL_CHECK_STATUS(pdlStatus);
                    writeValue = (~muxInfoPtr->interfaceInfo.i2cMux.mask & oldData) | (muxInfoPtr->interfaceInfo.i2cMux.mask & muxInfoPtr->interfaceInfo.i2cMux.value);
                }

                /* point to head of buffer */
                offset_and_data_buf_PTR = &offset_and_data_buf[0];
                /* copy offset */
                memcpy(offset_and_data_buf_PTR, &muxInfoPtr->interfaceInfo.i2cMux.offset, PDL_I2C_OFFSET_SIZE);
                /* point past the offset in the buffer */
                offset_and_data_buf_PTR += PDL_I2C_OFFSET_SIZE;
                /* copy data */
                memcpy(offset_and_data_buf_PTR, &writeValue, muxInfoPtr->interfaceInfo.i2cMux.accessSize);
                pdlStatus = prvPdlI2cTransmit (muxInfoPtr->interfaceInfo.i2cMux.baseAddress, muxInfoPtr->interfaceInfo.i2cMux.accessSize + PDL_I2C_OFFSET_SIZE, (UINT_8)muxInfoPtr->interfaceInfo.i2cMux.busId, (UINT_8* )&offset_and_data_buf[0]);
                PDL_CHECK_STATUS(pdlStatus);
                PDL_LIB_DEBUG_MAC(prvPdlI2CInterfaceMuxDebugFlag)(__FUNCTION__, __LINE__, "%s - Set I2C Mux address[0x%x] busId[%d] offset[0x%x] data[0x%x]", __FUNCTION__, muxInfoPtr->interfaceInfo.i2cMux.baseAddress, muxInfoPtr->interfaceInfo.i2cMux.busId, muxInfoPtr->interfaceInfo.i2cMux.offset, writeValue);
            }
            else if (muxInfoPtr->interfaceType == PDL_INTERFACE_TYPE_GPIO_E) {
                prvPdlGpioPinHwSetValue ((GT_U8)muxInfoPtr->interfaceInfo.gpioInterface.dev, muxInfoPtr->interfaceInfo.gpioInterface.pinNumber, PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, muxInfoPtr->interfaceInfo.gpioInterface.pushValue);
            }
        }
    }
    return PDL_OK;
}

/*$ END OF prvPdlI2CConfigureMuxes */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CFindNthBitInData
*
* DESCRIPTION:   Finds the nth bit set in data if none, return FALSE
*
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CFindNthBitInData ( IN UINT_32 data, IN UINT_32 bit, OUT UINT_32 * bitNumPtr )
 *
 * @brief   Prv pdl i 2 c find nth bit in data
 *
 * @param           data        The data.
 * @param           bit         The bit.
 * @param [in,out]  bitNumPtr   If non-null, the bit number pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CFindNthBitInData (
    IN  UINT_32                                 data,
    IN  UINT_32                                 bit,
    OUT UINT_32                               * bitNumPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                 i;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (bitNumPtr == NULL) {
        return PDL_BAD_PTR;
    }
    bit++;
    for (i = 0; i < sizeof (data) * 8; i++) {
        if (data & 1) {
            bit--;
            if (bit == 0)
                break;
        }
        data = data >> 1;
    }
    if (bit == 0) {
        *bitNumPtr = i;
        return PDL_OK;
    }

    return PDL_NOT_FOUND;
}

/*$ END OF prvPdlI2CFindNthBitInData */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConvertRel2Abs
*
* DESCRIPTION:   convert relative data to absolute data using i2c interface mask
*                examples: mask is 0XC0 and data is 3 - absData = 0xC0
*                          mask is 0xC0 and data is 2 - absData = 0x80
*                          mask is 0xC0 and data is 1 - absData = 0x40
*                          mask is 0xC0 and data is 0 - absData = 0
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2Abs ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData, OUT UINT_32 * absDataPtr )
 *
 * @brief   Prv pdl i 2 c convert relative 2 abs
 *
 * @param           interfaceId Identifier for the interface.
 * @param           relData     Information describing the relative.
 * @param [in,out]  absDataPtr  If non-null, the abs data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertRel2Abs (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 relData,
    OUT UINT_32                               * absDataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, temp;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (absDataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);

    for (currentBit = 0; currentBit < sizeof (relData) * 8; currentBit++) {
        if (relData & (1 << currentBit)) {
            /* find the matching bit in mask */
            pdlStatus = prvPdlI2CFindNthBitInData (i2cPtr->publicInfo.mask, currentBit, &temp);
            if (pdlStatus == PDL_OK) {
                out |= 1 << temp;
            }
            else {
                return pdlStatus;
            }
        }
    }
    *absDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertRel2Abs */

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2AbsWithMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData,IN  UINT_32 mask ,OUT UINT_32 * absDataPtr )
 *
 * @brief  brief   Prv pdl i 2 c convert relative 2 abs according to mask
 *
 * @param           interfaceId Identifier for the interface.
 * @param           relData     Information describing the relative.
 * @param           mask        mask needed.
 * @param [in,out]  absDataPtr  If non-null, the abs data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertRel2AbsWithMask (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 relData,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * absDataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, temp;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (absDataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);

    for (currentBit = 0; currentBit < sizeof (relData) * 8; currentBit++) {
        if (relData & (1 << currentBit)) {
            /* find the matching bit in mask */
            pdlStatus = prvPdlI2CFindNthBitInData (mask, currentBit, &temp);
            if (pdlStatus == PDL_OK) {
                out |= 1 << temp;
            }
            else {
                return pdlStatus;
            }
        }
    }
    *absDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertRel2Abs */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConvertAbs2Rel
*
* DESCRIPTION:   convert absolute data to relative data using i2c interface mask
*                examples: mask is 0XC0 and value is 0xC0 - 3 is returned
*                          mask is 0xC0 and value is 0x80 - 2 is returned
*                          mask is 0xC0 and value is 0x40 - 1 is returned
*
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2Rel ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 absData, OUT UINT_32 * relDataPtr )
 *
 * @brief   Prv pdl i 2 c convert abs 2 relative
 *
 * @param           interfaceId Identifier for the interface.
 * @param           absData     Information describing the abs.
 * @param [in,out]  relDataPtr  If non-null, the relative data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertAbs2Rel (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 absData,
    OUT UINT_32                               * relDataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, count = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);

    if (relDataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* absData doesn't match absMask */
    if ((absData & ~i2cPtr->publicInfo.mask) != 0) {
        return PDL_OUT_OF_RANGE;
    }
    for (currentBit = 0; currentBit < sizeof (absData) * 8; currentBit++) {
        if (absData & (1 << currentBit)) {
            /* find the matching bit in mask */
            out |= 1 << count;
        }
        if (i2cPtr->publicInfo.mask & (1 << currentBit)) {
            count++;
        }
    }
    *relDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertAbs2Rel */

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2RelWithMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 absData,IN  UINT_32 mask ,OUT UINT_32 * relDataPtr )
 *
 * @brief   Prv pdl i 2 c convert abs 2 relative according to mask
 *
 * @param           interfaceId Identifier for the interface.
 * @param           absData     Information describing the abs.
 * @param           mask        mask needed.
 * @param [in,out]  relDataPtr  If non-null, the relative data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertAbs2RelWithMask (
    IN  UINT_32                                 absData,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * relDataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, count = 0;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (relDataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* absData doesn't match absMask */
    if ((absData & ~mask) != 0) {
        return PDL_OUT_OF_RANGE;
    }
    for (currentBit = 0; currentBit < sizeof (absData) * 8; currentBit++) {
        if (absData & (1 << currentBit)) {
            /* find the matching bit in mask */
            out |= 1 << count;
        }
        if (mask & (1 << currentBit)) {
            count++;
        }
    }
    *relDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertAbs2Rel */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CHwGetValue
*
* DESCRIPTION:   get i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl i 2 c hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CHwGetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    OUT UINT_32                               * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes (i2cPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);

    /* perform i2c read */
    pdlStatus = prvPdlI2cReceiveWithOffset (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);
    *dataPtr &= i2cPtr->publicInfo.mask;
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - Reading i2c value from address[0x%x] busId[%d] offset[0x%x] mask[0x%x] data[0x%x]\n", __FUNCTION__, i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.mask, *dataPtr);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}

/*$ END OF prvPdlI2CHwGetValue */

/**
 * @fn  PDL_STATUS pdlI2CHwGetValueWithOffsetMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 offset, IN UINT_32 mask, OUT UINT_32 * dataPtr )
 *
 * @brief   get i2c value, offset & mask are given as parameter (SHOULD be used only by fan drivers)
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      offset
 * @param           mask        mask
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwGetValueWithOffsetMask (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 offset,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes (i2cPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);

    /* perform i2c read */
    pdlStatus = prvPdlI2cReceiveWithOffset (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, offset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);
    *dataPtr &= mask;
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - Reading i2c value from address[0x%x] busId[%d] offset[0x%x] mask[0x%x] data[0x%x]\n", __FUNCTION__, i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, offset, mask, *dataPtr);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}

/*$ END OF pdlI2CHwGetValueWithOffsetMask */


/* ***************************************************************************
* FUNCTION NAME: pdlI2CHwGetBufferWithOffset
*
* DESCRIPTION:   get i2c buffer the size of length with given offset (used for SFP EEPROM read)
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlI2CHwGetBufferWithOffset ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr )
 *
 * @brief   Pdl i 2 c hardware get buffer with offset
 *
 * @param           interfaceId Identifier for the interface.
 * @param           i2cAddress  I2c address to read from can only be 0x50 or 0x51
 * @param           offset      The offset.
 * @param           length      The length.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwGetBufferWithOffset (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  i2cAddress,
    IN  UINT_16                                 offset,
    IN  UINT_32                                 length,
    OUT void                                  * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_8                                    * dataWritePtr;
    UINT_16                                     relOffset;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (i2cAddress != 0x50 && i2cAddress != 0x51 && i2cAddress != 0x56) {
        return PDL_BAD_PARAM;
    }
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes (i2cPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);

    /* perform i2c read */
    relOffset = offset;
    dataWritePtr = (UINT_8 *)dataPtr;

    /*
    * Read from I2C the whole required length.
    * Basically, this code is called for devices which supports such reads (SFP) or
    * others (CPLD, FAN Controller, RPS) which read only one byte from specific offset.
    */
    pdlStatus = prvPdlI2cReceiveWithOffset (i2cAddress, length, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, relOffset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataWritePtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}

/*$ END OF pdlI2CHwGetBufferWithOffset */


/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CHwSetValue
*
* DESCRIPTION:   set i2c value with mask, value outside of mask will not be written
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 data )
 *
 * @brief   Pdl i 2 c hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CHwSetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     oldData = 0, writeValue;
    UINT_8                                      offsetAndDataBuf[20];
    UINT_8                                    * offsetAndDataBufPtr;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes (i2cPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);

    pdlStatus = prvPdlI2cReceiveWithOffset (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.transactionType, (UINT_8*) & oldData);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }

    PDL_CHECK_STATUS (pdlStatus);
    writeValue = (~i2cPtr->publicInfo.mask & oldData) | (i2cPtr->publicInfo.mask & data);

    /* perform i2c write */
    /* point to head of buffer */
    offsetAndDataBufPtr = &offsetAndDataBuf[0];
    /* copy offset */
    memcpy(offsetAndDataBufPtr, &i2cPtr->publicInfo.offset, PDL_I2C_OFFSET_SIZE);
    /* point past the offset in the buffer */
    offsetAndDataBufPtr += PDL_I2C_OFFSET_SIZE;
    /* copy data */
    memcpy(offsetAndDataBufPtr, &writeValue, i2cPtr->publicInfo.accessSize);
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - Writing i2c value[0x%x] to address[0x%x] busId[%d] offset[0x%x] mask[0x%x]\n", __FUNCTION__, i2cPtr->publicInfo.mask & data, i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.mask);
    pdlStatus = prvPdlI2cTransmit (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize + PDL_I2C_OFFSET_SIZE, (UINT_8)i2cPtr->publicInfo.busId, (UINT_8* )&offsetAndDataBuf[0]);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return pdlStatus;
}

/*$ END OF prvPdlI2CHwSetValue */

/**
 * @fn  PDL_STATUS pdlI2CHwSetValueWithOffsetMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 offset, IN UINT_32 mask, IN UINT_32 data )
 *
 * @brief   set i2c value, offset & mask are given as parameter (SHOULD be used only by fan drivers)
 *
 * @param   interfaceId Identifier for the interface.
 * @param   offset      offset
 * @param   mask        mask
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwSetValueWithOffsetMask (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 offset,
    IN  UINT_32                                 mask,
    IN  UINT_32                                 data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                     oldData = 0, writeValue;
    UINT_8                                      offsetAndDataBuf[20];
    UINT_8                                    * offsetAndDataBufPtr;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes (i2cPtr);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS (pdlStatus);

    pdlStatus = prvPdlI2cReceiveWithOffset (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, offset, i2cPtr->publicInfo.transactionType, (UINT_8*) & oldData);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }

    PDL_CHECK_STATUS (pdlStatus);
    writeValue = (~mask & oldData) | (mask & data);

    /* perform i2c write */
    /* point to head of buffer */
    offsetAndDataBufPtr = &offsetAndDataBuf[0];
    /* copy offset */
    memcpy(offsetAndDataBufPtr, &offset, PDL_I2C_OFFSET_SIZE);
    /* point past the offset in the buffer */
    offsetAndDataBufPtr += PDL_I2C_OFFSET_SIZE;
    /* copy data */
    memcpy(offsetAndDataBufPtr, &writeValue, i2cPtr->publicInfo.accessSize);
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__,"%s - Writing i2c value[0x%x] to address[0x%x] busId[%d] offset[0x%x] mask[0x%x]\n", __FUNCTION__, mask & data, i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, offset, mask);
    pdlStatus = prvPdlI2cTransmit (i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize + PDL_I2C_OFFSET_SIZE, (UINT_8)i2cPtr->publicInfo.busId, (UINT_8* )&offsetAndDataBuf[0]);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return pdlStatus;
}

/*$ END OF pdlI2CHwSetValueWithOffsetMask */

/* ***************************************************************************
* FUNCTION NAME: PdlI2CDebugDbGetAttributes
*
* DESCRIPTION:  get I2C attributes
*               return all I2C relevant information for given interfaceId
*               based on information from XML
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlI2CDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_I2C_STC * attributesPtr )
 *
 * @brief   Pdl i 2 c debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_I2C_STC              * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy (attributesPtr, &i2cPtr->publicInfo, sizeof (PDL_INTERFACE_I2C_STC));
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  i2cPtr->publicInfo.accessSize);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.busId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  i2cPtr->publicInfo.i2cAddress);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.mask);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.numberOfMuxes);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_16, i2cPtr->publicInfo.offset);
    return PDL_OK;
}
/*$ END OF PdlI2CDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: PdlI2CDebugDbSetAttributes
*
* DESCRIPTION:  set I2C attributes
*               update I2C relevant information for given interfaceId
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlI2CDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_I2C_STC * attributesPtr )
 *
 * @brief   Pdl i 2 c debug database set attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_I2C_STC              * attributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_BUS_ID) {
        i2cPtr->publicInfo.busId = attributesPtr->busId;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ADDRESS) {
        i2cPtr->publicInfo.i2cAddress = attributesPtr->i2cAddress;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ACCESS_SIZE) {
        i2cPtr->publicInfo.accessSize = attributesPtr->accessSize;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_OFFSET) {
        i2cPtr->publicInfo.offset = attributesPtr->offset;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_MASK) {
        i2cPtr->publicInfo.mask = attributesPtr->mask;
    }
    return PDL_OK;
}
/*$ END OF PdlI2CDebugDbSetAttributes */

/* ***************************************************************************
* FUNCTION NAME: PdlI2CDebugDbGetMuxAttributes
*
* DESCRIPTION:  get I2C Mux attributes
*               based on information from XML
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlI2CDebugDbGetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxNumber, OUT PDL_INTERFACE_MUX_STC * muxAttributesPtr )
 *
 * @brief   Pdl i 2 c debug database get multiplexer attributes
 *
 * @param           interfaceId         Identifier for the interface.
 * @param           muxNumber           The multiplexer number.
 * @param [in,out]  muxAttributesPtr    If non-null, the multiplexer attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbGetMuxAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    OUT PDL_INTERFACE_MUX_STC             * muxAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    PDL_INTERFACE_MUX_STC                     * muxPtr;
    PDL_INTERFACE_PRV_I2C_MUX_KEY_STC           muxKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (muxAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    muxKey.muxNum = muxNumber;
    pdlStatus  = prvPdlDbFind (&i2cPtr->muxDb, (void*) &muxKey, (void**) &muxPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (muxAttributesPtr, muxPtr, sizeof (PDL_INTERFACE_MUX_STC));
    return PDL_OK;
}
/*$ END OF PdlI2CDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: PdlI2CDebugDbSetMuxAttributes
*
* DESCRIPTION:  set I2C Mux attributes
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlI2CDebugDbSetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxNumber, IN PDL_INTERFACE_MUX_STC * muxAttributesPtr )
 *
 * @brief   Pdl i 2 c debug database set multiplexer attributes
 *
 * @param           interfaceId         Identifier for the interface.
 * @param           muxNumber           The multiplexer number.
 * @param [in,out]  muxAttributesPtr    If non-null, the multiplexer attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlI2CDebugDbSetMuxAttributes (
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    IN  UINT_32                             attributesMask,
    IN  PDL_INTERFACE_MUX_STC             * muxAttributesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    PDL_INTERFACE_MUX_STC                     * muxPtr;
    PDL_INTERFACE_PRV_I2C_MUX_KEY_STC           muxKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS (pdlStatus);
    if (muxAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    muxKey.muxNum = muxNumber;
    pdlStatus  = prvPdlDbFind (&i2cPtr->muxDb, (void*) &muxKey, (void**) &muxPtr);
    PDL_CHECK_STATUS (pdlStatus);

    if (muxPtr->interfaceType == PDL_INTERFACE_TYPE_GPIO_E) {
        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_DEVICE) {
            muxPtr->interfaceInfo.gpioInterface.dev = muxAttributesPtr->interfaceInfo.gpioInterface.dev;
        }
        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_PIN_NUMBER) {
            muxPtr->interfaceInfo.gpioInterface.pinNumber = muxAttributesPtr->interfaceInfo.gpioInterface.pinNumber;
        }
        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_VALUE) {
            muxPtr->interfaceInfo.gpioInterface.pushValue = muxAttributesPtr->interfaceInfo.gpioInterface.pushValue;
        }
    }
    else {
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ADDRESS) {
            muxPtr->interfaceInfo.i2cMux.baseAddress = muxAttributesPtr->interfaceInfo.i2cMux.baseAddress;
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_OFFSET) {
            muxPtr->interfaceInfo.i2cMux.offset = muxAttributesPtr->interfaceInfo.i2cMux.offset;
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_MASK) {
            muxPtr->interfaceInfo.i2cMux.mask = muxAttributesPtr->interfaceInfo.i2cMux.mask;
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_VALUE) {
            muxPtr->interfaceInfo.i2cMux.value = muxAttributesPtr->interfaceInfo.i2cMux.value;
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_BUS_ID) {
            muxPtr->interfaceInfo.i2cMux.busId = muxAttributesPtr->interfaceInfo.i2cMux.busId;
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ACCESS_SIZE) {
            muxPtr->interfaceInfo.i2cMux.accessSize = muxAttributesPtr->interfaceInfo.i2cMux.accessSize;
        }
    }
    return PDL_OK;
}
/*$ END OF PdlI2CDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cBusMarkUsed
*
* DESCRIPTION:  indicate bus_id is being used
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cBusMarkUsed ( IN UINT_32 busId )
 *
 * @brief   Prv pdl i 2c bus mark used
 *
 * @param   busId   Identifier for the bus.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cBusMarkUsed (
    IN  UINT_32          busId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (busId > PDL_I2C_BUS_ID_MAX) {
        return PDL_OUT_OF_RANGE;
    }
    pdlI2cBusIdIsUsed [busId] = TRUE;
    return PDL_OK;
}
/*$ END OF prvPdlI2cBusMarkUsed */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cBusIsUsed
*
* DESCRIPTION:  return if bus_id is being used
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  BOOLEAN prvPdlI2cBusIsUsed ( IN UINT_32 busId )
 *
 * @brief   Prv pdl i 2c bus is used
 *
 * @param   busId   Identifier for the bus.
 *
 * @return  True if it succeeds, false if it fails.
 */

BOOLEAN prvPdlI2cBusIsUsed (
    IN  UINT_32          busId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (busId > PDL_I2C_BUS_ID_MAX) {
        return PDL_OUT_OF_RANGE;
    }
    return pdlI2cBusIdIsUsed[busId];
}
/*$ END OF prvPdlI2cBusMarkUsed */


/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cXmlIsI2c
*
* DESCRIPTION:  returns TRUE if xmlId is pointing to a I2C interface
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cXmlIsI2c ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT BOOLEAN * isI2c )
 *
 * @brief   Prv pdl i 2c XML is i 2c
 *
 * @param           xmlId   Identifier for the XML.
 * @param [in,out]  isI2c   If non-null, true if is i 2c, false if not.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cXmlIsI2c (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT BOOLEAN                               * isI2c
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char     pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32  maxSize = PDL_XML_MAX_TAG_LEN;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (isI2c == NULL) {
        return PDL_BAD_PTR;
    }
    xmlParserGetName(xmlId, &maxSize, &pdlTagStr[0]);
    if (strcmp (pdlTagStr, PDL_I2C_TAG_NAME) == 0) {
        *isI2c = TRUE;
    }
    else {
        *isI2c = FALSE;
    }

    return PDL_OK;
}
/*$ END OF prvPdlI2cXmlIsI2c */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cAccessSizeToEnum
*
* DESCRIPTION:   Get i2c interface access size
*                Will be used by XML module
*
* RETURNS:       accessSize or PDL_ERROR in case access size doesn't match any of predefined access sizes
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cAccessSizeToEnum ( IN char * name, OUT PDL_INTERFACE_I2C_ACCESS_SIZE_ENT * accessSizePtr )
 *
 * @brief   Prv pdl i 2c access size to enum
 *
 * @param [in,out]  name            If non-null, the name.
 * @param [in,out]  accessSizePtr   If non-null, the access size pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cAccessSizeToEnum (
    IN  char                                        * name,
    OUT PDL_INTERFACE_I2C_ACCESS_SIZE_ENT           * accessSizePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             i;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (accessSizePtr == NULL) {
        return PDL_BAD_PTR;
    }
    for (i = 0; i < PDL_INTERFACE_I2C_ACCESS_SIZE_COUNT; i++) {
        if (strcmp (name,i2cAccessSizeEnumToName[i].name) == 0) {
            *accessSizePtr = i2cAccessSizeEnumToName[i].accessSize;
            return PDL_OK;
        }
    }
    return PDL_ERROR;
}

/*$ END OF prvPdlI2cAccessSizeToEnum */


/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cXmlMuxParser
*
* DESCRIPTION:  parse XML instance of I2c Mux
*
* PARAMETERS:
*
* REMARKS:      Used by pdlI2cXmlParser
*
*
* XML structure:
*
*       <mux>
*           <mux-number> 1 </mux-number>
*           <address> 0x12 </address>
*           <offset> 5 </offset>
*           <mask> 0x07 </mask>
*           <value> 0x02 </value>
*       </mux>

*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cXmlMuxParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_I2C_MUX_STC     * muxInfoPTR
 *
 * @brief   Prv pdl i2c XML multiplexer parser
 *
 * @param           xmlId           Identifier for the XML.
 * @param [out]     muxInfoPTR      If non-null, the I2C multiplexer information structure
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cXmlMuxParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP  xmlId,
    OUT PDL_INTERFACE_I2C_MUX_STC     * muxInfoPTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                               pdlValueStr[PDL_XML_MAX_TAG_LEN] = {0};
    char                               pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                             mask = 0, maxSize;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlMuxChildId;
    XML_PARSER_RET_CODE_TYP             xmlStatus;
    PDL_STATUS                          pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (muxInfoPTR == NULL) {
        return PDL_ERROR;
    }

    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlMuxChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        maxSize = PDL_XML_MAX_TAG_LEN;
        xmlParserGetName(xmlMuxChildId, &maxSize, &pdlTagStr[0]);
        if (strcmp (pdlTagStr, PDL_I2C_MUX_I2C_ADDRESS_TAG_NAME) == 0) {
            maxSize = sizeof (UINT_8);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &muxInfoPTR->baseAddress);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_I2C_MASK_TAG_NAME) == 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &muxInfoPTR->mask);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 1;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_I2C_OFFSET_TAG_NAME) == 0) {
            maxSize = sizeof (UINT_16);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &muxInfoPTR->offset);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 2;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_I2C_VALUE_TAG_NAME) == 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &muxInfoPTR->value);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 3;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_I2C_BUS_ID_NAME) == 0) {
            maxSize = sizeof (UINT_32);
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &muxInfoPTR->busId);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 4;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_ACCESS_NAME) == 0) {
            maxSize = PDL_XML_MAX_TAG_LEN;
            xmlStatus = xmlParserGetValue (xmlMuxChildId, &maxSize, &pdlValueStr[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = prvPdlI2cAccessSizeToEnum (&pdlValueStr[0], &muxInfoPTR->accessSize);
            PDL_CHECK_STATUS (pdlStatus);
            mask |= 1 << 5;
        }
        else if (strcmp (pdlTagStr, PDL_I2C_MUX_TRANSACTION_TYPE_NAME) == 0) {
            /* TBD - need to support transaction type */
            muxInfoPTR->transactionType = PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA;
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlMuxChildId, &xmlMuxChildId);
    }
    if (mask == 0x3F) {
        return PDL_OK;
    }
    else {
        return PDL_XML_PARSE_ERROR;
    }
}

/*$ END OF prvPdlI2cXmlMuxParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cXmlParser
*
* DESCRIPTION:  parse XML instance of I2C interface, insert it into DB and return a reference to it
*
* PARAMETERS:
*
* REMARKS:      I2C muxes can only be of one type, either i2c or gpios
*               gpios muxes order of configuration has no relevance
*
* XML structure:
*
* I2C interface with I2C muxes
* <i2c-interface>
*   <bus-id> 0 </bus-id>
*   <i2c-mux>
*       <mux>
*           <mux-number> 1 </mux-number>
*           <address> 0x12 </address>
*           <offset> 5 </offset>
*           <mask> 0x07 </mask>
*           <value> 0x02 </value>
*       </mux>
*       <mux>
*           <mux-number> 2 </mux-number>
*           <address> 0x10 </address>
*           <offset> 6 </offset>
*           <mask> 0x34 </mask>
*           <value> 0x04 </value>
*       </mux>
*   </i2c-mux>
*   <address> 6 </address>
*   <offset> 0 </offset>
*   <mask> 0xc0 </mask>
*   <access> one-byte </value>
* </i2c-interface>
*
*
*
* I2C interface with GPIO muxes
* <i2c-interface>
*   <bus-id> 0 </bus-id>
*   <gpio-mux>
*       <device-number> 0 </device-number>
*       <pin>
*           <pin-number> 5 </pin-number>
*           <push-value> 0 </push-value>
*       </pin>
*       <pin>
*           <pin-number> 6 </pin-number>
*           <push-value> 1 </push-value>
*       </pin>
*   </gpio-mux>
*   <address> 6 </address>
*   <offset> 0 </offset>
*   <mask> 0xc0 </mask>
*   <access> one-byte </value>
* </i2c-interface>

*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cXmlParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Prv pdl i 2c XML parser
 *
 * @param           xmlId           Identifier for the XML.
 * @param [in,out]  interfaceIdPtr  If non-null, the interface identifier pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cXmlParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP          xmlId,
    OUT PDL_INTERFACE_TYP                     * interfaceIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                pdlValueStr[PDL_XML_MAX_TAG_LEN] = {0};
    UINT_32                             mask = 0, maxSize, muxNum = 1, muxValue, muxPinNumber, muxDevNumber;
    PDL_STATUS                          pdlStatus, pdlStatus2;
    XML_PARSER_RET_CODE_TYP             xmlStatus, xmlStatus2;
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlI2cChildId, xmlMuxId;
    PDL_INTERFACE_PRV_KEY_STC           i2cKey;
    PDL_INTERFACE_PRV_I2C_DB_STC        i2cInfo;
    PDL_INTERFACE_PRV_I2C_DB_STC      * i2cPtr = NULL;
    PDL_INTERFACE_I2C_ACCESS_SIZE_ENT   accessSize;
    PDL_INTERFACE_PRV_I2C_MUX_KEY_STC   muxKey;
    PDL_INTERFACE_MUX_STC               muxInfo;
    PDL_INTERFACE_MUX_STC             * muxPtr;
    PDL_DB_PRV_ATTRIBUTES_STC           dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* xmliD not pointing to not i2c-interface*/
    if (xmlParserIsEqualName (xmlId, "i2c-read-write-address") != XML_PARSER_RET_CODE_OK) {
        return PDL_ERROR;
    }

    memset (&i2cInfo, 0, sizeof (i2cInfo));
    /* create mux list */
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_I2C_MUX_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_MUX_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &i2cInfo.muxDb);
    PDL_CHECK_STATUS(pdlStatus);

    /* set default value for transaction type */
    i2cInfo.publicInfo.transactionType = PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA;
    /* go over all I2C child tags */
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlI2cChildId);
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlI2cChildId, "i2c-bus-id") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(i2cInfo.publicInfo.busId);
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &i2cInfo.publicInfo.busId);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1;
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "i2c-address") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(i2cInfo.publicInfo.i2cAddress);
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &i2cInfo.publicInfo.i2cAddress);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 1;
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "i2c-offset") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(i2cInfo.publicInfo.offset);
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &i2cInfo.publicInfo.offset);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 2;
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "i2c-mask") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(i2cInfo.publicInfo.mask);
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &i2cInfo.publicInfo.mask);
            XML_CHECK_STATUS(xmlStatus);
            mask |= 1 << 3;
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "i2c-access") == XML_PARSER_RET_CODE_OK) {
            maxSize = PDL_XML_MAX_TAG_LEN;
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &pdlValueStr[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = prvPdlI2cAccessSizeToEnum (&pdlValueStr[0], &accessSize);
            PDL_CHECK_STATUS (pdlStatus);
            i2cInfo.publicInfo.accessSize = accessSize;
            mask |= 1 << 4;
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "i2c-transaction-type") == XML_PARSER_RET_CODE_OK) {
            maxSize = PDL_XML_MAX_TAG_LEN;
            xmlStatus = xmlParserGetValue (xmlI2cChildId, &maxSize, &pdlValueStr[0]);
            XML_CHECK_STATUS(xmlStatus);
            pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_I2C_TRANSACTION_TYPE_E, pdlValueStr, (UINT_32*) &i2cInfo.publicInfo.transactionType);
            PDL_CHECK_STATUS (pdlStatus);
        }
        else if (xmlParserIsEqualName (xmlI2cChildId, "is-mux-used") == XML_PARSER_RET_CODE_OK) {
            /* skip this tag */
        }
        /* <muxes> */
        else if (xmlParserIsEqualName (xmlI2cChildId, "mux-list") == XML_PARSER_RET_CODE_OK) {
            xmlStatus2 = xmlParserGetFirstChild (xmlI2cChildId, &xmlMuxId);
            while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus);
                if (xmlParserIsEqualName (xmlMuxId, "mux-number") == XML_PARSER_RET_CODE_OK) {
                    /* do nothing, use internal mux counter */
                }
                /* <i2c-mux> */
                else if (xmlParserIsEqualName (xmlMuxId, "mux-i2c-info") == XML_PARSER_RET_CODE_OK) {
                    memset (&muxInfo, 0, sizeof (muxInfo));
                    pdlStatus2 = prvPdlI2cXmlMuxParser (xmlMuxId, &muxInfo.interfaceInfo.i2cMux);
                    PDL_CHECK_STATUS (pdlStatus2);
                    /* muxNum has to be actual (mux number + 1) since it's being used for loop iteration in prvPdlI2CConfigureMuxes with i < numberOfMuxes */
                    muxKey.muxNum = muxNum++;
                    muxInfo.interfaceType = PDL_INTERFACE_TYPE_I2C_E;
                    pdlStatus2 = prvPdlDbAdd (&i2cInfo.muxDb, (void*) &muxKey, (void*) &muxInfo, (void**) &muxPtr);
                    PDL_CHECK_STATUS (pdlStatus2);
                    mask |= 1 << 5;
                }
                /* <gpio-mux> */
                else if (xmlParserIsEqualName (xmlMuxId, "mux-gpio-info") == XML_PARSER_RET_CODE_OK) {
                    pdlStatus2 = prvPdlGpioPinXmlParser (xmlMuxId, &muxDevNumber,  &muxPinNumber, &muxValue);
                    PDL_CHECK_STATUS (pdlStatus2);
                    memset (&muxInfo, 0, sizeof (muxInfo));
                    muxKey.muxNum = muxNum++;
                    muxInfo.interfaceType = PDL_INTERFACE_TYPE_GPIO_E;
                    muxInfo.interfaceInfo.gpioInterface.dev = (UINT_8)muxDevNumber;
                    muxInfo.interfaceInfo.gpioInterface.pinNumber = (UINT_8)muxPinNumber;
                    muxInfo.interfaceInfo.gpioInterface.pushValue = (UINT_8)muxValue;
                    pdlStatus2 = prvPdlDbAdd (&i2cInfo.muxDb, (void*) &muxKey, (void*) &muxInfo, (void**) &muxPtr);
                    PDL_CHECK_STATUS (pdlStatus2);
                    mask |= 1 << 6;
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling(xmlMuxId, &xmlMuxId);
            }
        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlI2cChildId, &xmlI2cChildId);
    }
    /* all values MUST be set, only GPIO or i2C mux can be set */
    if (mask != 0x1F && mask != 0x3F && mask != 0x5F) {
        return PDL_XML_PARSE_ERROR;
    }
    i2cKey.interfaceId = prvPdlI2cInterfaceCount++;
    i2cInfo.publicInfo.numberOfMuxes = muxNum;
    pdlStatus = prvPdlDbAdd (&prvPdlI2cDb, (void*) &i2cKey, (void*) &i2cInfo, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = i2cKey.interfaceId;
    prvPdlI2cBusMarkUsed(i2cInfo.publicInfo.busId);

    return PDL_OK;
}

/*$ END OF prvPdlI2cXmlParser */

/**
 * @fn  PDL_STATUS prvPdlI2cCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of i2c interfaces
 *
 * @param [out] countPtr Number of i2c interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlI2cCountGet (
    OUT UINT_32 * countPtr
)
{
/****************************************************************************/
/*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/****************************************************************************/

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (countPtr == NULL)
        return PDL_BAD_PTR;

    *countPtr = prvPdlI2cInterfaceCount;

    return PDL_OK;
}

/****************************************************************************
* FUNCTION NAME: prvPdlI2cBusFdInit
*
* DESCRIPTION:   Initialize I2C Bus file descriptors
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cBusFdInit ( IN void )
 *
 * @brief   Prv pdl i 2c bus fd initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cBusFdInit (
    IN void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
      UINT_32                   i;
      char                      i2cDevName[20];
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
#ifdef LINUX_HW
    for (i = 0; i < PDL_I2C_BUS_ID_MAX; i++) {
        if (prvPdlI2cBusIsUsed(i) == TRUE) {
            PRV_IDBG_PDL_LOGGER_SNPRINTF_MAC(i2cDevName, sizeof(i2cDevName)-1,"%s-%d", PDL_I2C_PREFIX_DEV_NAME, i);
            pdlI2cBusFdArr[i] = open(i2cDevName, O_RDWR);
            if (pdlI2cBusFdArr[i] == -1)
                return PDL_I2C_ERROR;
        }
    }
#endif
    return PDL_OK;
}

/*$ END OF prvPdlI2cBusFdInit */

/**
 * @fn  PDL_STATUS prvPdlI2cInit ( IN void )
 *
 * @brief   Pdl i2c initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cInit (
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_I2C_DB_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &prvPdlI2cDb);
    PDL_CHECK_STATUS (pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlI2cInit */


/*$ END OF prvPdlGpioInit */

/**
 * @fn  PDL_STATUS prvPdlI2cDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC      * i2cPtr = NULL;
    PDL_INTERFACE_PRV_KEY_STC           i2cKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlI2cInterfaceCount = 0;
    /* first destroy all mux dbs created inside i2c interface entries */
    pdlStatus = prvPdlDbGetNextKey (&prvPdlI2cDb, NULL, (void*) &i2cKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbDestroy(&i2cPtr->muxDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&prvPdlI2cDb, (void*) &i2cKey, (void*) &i2cKey);
    }

    /* destroy i2c interface db */
    if (prvPdlI2cDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&prvPdlI2cDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}

/*$ END OF prvPdlI2cDestroy */
