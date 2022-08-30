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
* @file snet.c
*
* @brief This is a external API definition for snet module of SKernel.
*
* @version   54
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/suserframes/snetSoho.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/smem/smemPhy.h>
#include <asicSimulation/SKernel/smem/smemMacsec.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/suserframes/snetGm.h>

/**
* @internal snetProcessInit function
* @endinternal
*
* @brief   Init module.
*
* @param[in] deviceObjPtr             - pointer to device object.
*/
void snetProcessInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_BIT  initDone = 0;
    if (!deviceObjPtr)
    {
        skernelFatalError("snetProcessInit : illegal pointer\n");
    }

    if(deviceObjPtr->isWmDxDevice)
    {
    }
    else
    {
        /* check device type (including FA) and call device/FA specific init */
        switch(deviceObjPtr->deviceFamily)
        {
            case SKERNEL_SOHO_FAMILY:
                snetSohoProcessInit(deviceObjPtr);
                initDone = 1;
                break;
            case SKERNEL_LION_PORT_GROUP_SHARED_FAMILY:
            case SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY:
            case SKERNEL_LION3_PORT_GROUP_SHARED_FAMILY:
            case SKERNEL_COM_MODULE_FAMILY:
            case SKERNEL_PHY_SHELL_FAMILY:
            case SKERNEL_PUMA3_SHARED_FAMILY:
                /* no direct packet processing */
                initDone = 1;
                break;
            case SKERNEL_PHY_CORE_FAMILY:
                smemPhyProcessInit(deviceObjPtr);
                initDone = 1;
                break;
            case SKERNEL_MACSEC_FAMILY:
                smemMacsecProcessInit(deviceObjPtr);
                initDone = 1;
                break;
            case SKERNEL_PIPE_FAMILY:
                break;
            default:
                skernelFatalError(" smemInit: not valid mode[%d]",
                                    deviceObjPtr->deviceFamily);
            break;
        }
    }

    if(initDone == 0)
    {
        /* the 'regular' devices split to 2 types :
            1. the non GM device,
            2. the GM devices
        */
        if(deviceObjPtr->gmDeviceType == GOLDEN_MODEL)
        {
            /* the device is 'GM device' */
            snetGmProcessInit(deviceObjPtr);
        }
        else
        {
            /* the device is 'not GM device' */
            snetChtProcessInit(deviceObjPtr);
        }
    }


}

/**
* @internal snetFrameProcess function
* @endinternal
*
* @brief   Process the frame, get and do actions for a frame
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] bufferId                 - frame data buffer Id
* @param[in] srcPort                  - source port number
*/
void snetFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
)
{
    SNET_DEV_FRAME_PROC_FUN  frameProcFunc;

    frameProcFunc = devObjPtr->devFrameProcFuncPtr;
    if (frameProcFunc)
    {
        frameProcFunc(devObjPtr, bufferId, srcPort);
    }
}

/**
* @internal snetCncFastDumpUploadAction function
* @endinternal
*
* @brief   Process upload CNC block demanded by CPU
*/
GT_VOID snetCncFastDumpUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                 * cncTrigPtr
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devCncFastDumpFuncPtr == NULL)
        return ;

    deviceObjPtr->devCncFastDumpFuncPtr(deviceObjPtr, cncTrigPtr);
}

/**
* @internal snetLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*
* @param[in] deviceObjPtr             - pointer to device object.
* @param[in] port                     -  number.
* @param[in] linkState                - link state (0 - down, 1 - up)
*/
void snetLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT      *     deviceObjPtr,
    IN GT_U32                           port,
    IN GT_U32                           linkState
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devPortLinkUpdateFuncPtr == NULL)
        return ;

    /* register the notification in the LOGGER engine*/
    simLogLinkStateNotify(deviceObjPtr,port,linkState);

    deviceObjPtr->devPortLinkUpdateFuncPtr(deviceObjPtr, port, linkState);
}

/**
* @internal snetFrameParsing function
* @endinternal
*
* @brief   Parsing the frame, get information from frame and fill descriptor
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] descrPtr
*/
void snetFrameParsing
(
    IN SKERNEL_DEVICE_OBJECT        *     devObjPtr,
    INOUT SKERNEL_FRAME_DESCR_STC   *     descrPtr
)
{
    DECLARE_FUNC_NAME(snetFrameParsing);

    SBUF_BUF_STC   * frameBufPtr;

    ASSERT_PTR(devObjPtr);
    ASSERT_PTR(descrPtr);

    frameBufPtr = descrPtr->frameBuf;
    /* Set byte count from actual buffer's length */
    __LOG(("Set byte count from actual buffer's length"));
    descrPtr->byteCount = (GT_U16)frameBufPtr->actualDataSize;
    /* Set destination MAC pointer */
    __LOG(("Set destination MAC pointer"));
    descrPtr->dstMacPtr = frameBufPtr->actualDataPtr;
    /* Fill MAC data type of descriptor */
    __LOG(("Fill MAC data type of descriptor"));
    if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr)) {
        descrPtr->macDaType = SKERNEL_BROADCAST_MAC_E;
    }
    else
    if (SGT_MAC_ADDR_IS_MCST(descrPtr->dstMacPtr)) {
        descrPtr->macDaType = SKERNEL_MULTICAST_MAC_E;
    }
    else
    {
        descrPtr->macDaType = SKERNEL_UNICAST_MAC_E;
    }

    /* the broadcast ARP will be checked when parsing the frame's protocol */
    __LOG(("the broadcast ARP will be checked when parsing the frame's protocol"));

    return;
}
#define     SNET_TAG_PROTOCOL_ID_CNS            0x8100
/**
* @internal snetTagDataGet function
* @endinternal
*
* @brief   Get VLAN tag info
*/
void snetTagDataGet
(
    IN  GT_U8   vpt,
    IN  GT_U16  vid,
    IN  GT_BOOL littleEndianOrder,
    OUT GT_U8   tagData[] /* 4 bytes */
)
{
    ASSERT_PTR(tagData);

    if (littleEndianOrder) {
        /* form ieee802.1q tag - little endian order */
        tagData[3] = SNET_TAG_PROTOCOL_ID_CNS >> 8;
        tagData[2] = SNET_TAG_PROTOCOL_ID_CNS & 0xff;

        /* lsb of tag control */
        tagData[1] = ((vid & 0xf00) >> 8) | ((vpt & 0x7) << 5);
        tagData[0] = vid & 0xff; /* msb of tag control */
    }
    else /* big endian */
    {
        /* form ieee802.1q tag - BIG endian order */
        tagData[0] = SNET_TAG_PROTOCOL_ID_CNS >> 8;
        tagData[1] = SNET_TAG_PROTOCOL_ID_CNS & 0xff;

        /* msb of tag control */
        tagData[2] = ((vid & 0xf00) >> 8) | ((vpt & 0x7) << 5);
        tagData[3] = vid & 0xff; /* lsb of tag control */
    }

    return;
}


/**
* @internal snetFromCpuDmaProcess function
* @endinternal
*
* @brief   Process transmitted SDMA queue frames
*/
GT_VOID snetFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT      *     devObjPtr,
    SBUF_BUF_ID                         bufferId
)
{
    SNET_DEV_FROM_CPU_DMA_PROC_FUN  fromCpuDmaProcFuncPtr;

    fromCpuDmaProcFuncPtr = devObjPtr->devFromCpuDmaFuncPtr;
    if (fromCpuDmaProcFuncPtr)
    {
        fromCpuDmaProcFuncPtr(devObjPtr, bufferId);
    }
}

/**
* @internal snetFromEmbeddedCpuProcess function
* @endinternal
*
* @brief   Process transmitted frames from the Embedded CPU to the PP
*/
GT_VOID snetFromEmbeddedCpuProcess
(
    IN SKERNEL_DEVICE_OBJECT      *     devObjPtr,
    SBUF_BUF_ID                         bufferId
)
{
    if (devObjPtr->devFromEmbeddedCpuFuncPtr)
    {
        devObjPtr->devFromEmbeddedCpuFuncPtr(devObjPtr, bufferId);
    }
}

/**
* @internal snetFieldBeValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit
*           which is multiple of 8. Input and output are in big endian format.
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (0..31)
*/
GT_U32  snetFieldBeValueGet(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
)
{
    GT_U32  actualStartWord = startBit >> 5;/*/32*/
    GT_U32  actualStartBit  = ((startBit & 0x1f));
    GT_U32  actualEndBit;
    GT_U32  actualValue = 0;
    GT_U32  workValue = 0;
    GT_U32  numBitsFirst;
    GT_U32  numBitsLeft;

    ASSERT_PTR(startMemPtr);

    if (numBits > 32)
    {
        skernelFatalError(" snetFieldValueGet: oversize numBits[%d] > 32 \n", numBits);
    }

    if ((actualStartBit + numBits) <= 32)
    {
        numBitsFirst = numBits;
        numBitsLeft  = 0;
    }
    else
    {
        numBitsFirst = 32 - actualStartBit;
        numBitsLeft  = numBits - numBitsFirst;
    }

    actualEndBit = 32 - actualStartBit;
    actualStartBit = (numBitsFirst < actualEndBit) ? (actualEndBit - numBitsFirst) : 0;

    actualValue = SMEM_U32_GET_FIELD(
        startMemPtr[actualStartWord], actualStartBit, numBitsFirst);

    if (numBitsLeft > 0)
    {
        /* retrieve the rest of the value from the second word */
        workValue = SMEM_U32_GET_FIELD(
            startMemPtr[actualStartWord + 1], 32-numBitsLeft, numBitsLeft);

        /* place it to the high bits of the result */
        actualValue = (actualValue << numBitsLeft) | workValue;
    }

    return actualValue;
}

/**
* @internal snetFieldValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (0..31)
*/
GT_U32  snetFieldValueGet(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
)
{
    GT_U32  actualStartWord = startBit >> 5;/*/32*/
    GT_U32  actualStartBit  = startBit & 0x1f;/*%32*/
    GT_U32  actualValue;
    GT_U32  workValue;
    GT_U32  numBitsFirst;
    GT_U32  numBitsLeft;

    ASSERT_PTR(startMemPtr);

    if (numBits > 32)
    {
        skernelFatalError(" snetFieldValueGet: oversize numBits[%d] > 32 \n", numBits);
    }

    if ((actualStartBit + numBits) <= 32)
    {
        numBitsFirst = numBits;
        numBitsLeft  = 0;
    }
    else
    {
        numBitsFirst = 32 - actualStartBit;
        numBitsLeft  = numBits - numBitsFirst;
    }

    actualValue = SMEM_U32_GET_FIELD(
        startMemPtr[actualStartWord], actualStartBit, numBitsFirst);

    if (numBitsLeft > 0)
    {
        /* retrieve the rest of the value from the second word */
        workValue = SMEM_U32_GET_FIELD(
            startMemPtr[actualStartWord + 1], 0, numBitsLeft);

        /* place it to the high bits of the result */
        actualValue |= (workValue << numBitsFirst);
    }

    return actualValue;
}


/**
* @internal snetFieldValueSet function
* @endinternal
*
* @brief   set the value to field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (0..31)
* @param[in] value                    -  to write to
*/
void  snetFieldValueSet(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  value
)
{
    GT_U32  actualStartWord = startBit >> 5;/*/32*/
    GT_U32  actualStartBit  = startBit & 0x1f;/*%32*/
    GT_U32  numBitsFirst;
    GT_U32  numBitsLeft;

    ASSERT_PTR(startMemPtr);

    if (numBits > 32)
    {
        skernelFatalError(" snetFieldValueSet: oversize numBits[%d] > 32 \n", numBits);
    }

    if ((actualStartBit + numBits) <= 32)
    {
        numBitsFirst = numBits;
        numBitsLeft  = 0;
    }
    else
    {
        numBitsFirst = 32 - actualStartBit;
        numBitsLeft  = numBits - numBitsFirst;
    }

    SMEM_U32_SET_FIELD(
        startMemPtr[actualStartWord], actualStartBit, numBitsFirst,value);

    if (numBitsLeft > 0)
    {
        /* place rest of value to the high bits of the result */
        SMEM_U32_SET_FIELD(
            startMemPtr[actualStartWord + 1], 0, numBitsLeft,(value>>numBitsFirst));
    }

    return ;
}

/**
* @internal ipV4CheckSumCalc function
* @endinternal
*
* @brief   Perform ones-complement sum , and ones-complement on the final sum-word.
*         The function can be used to make checksum for various protocols.
* @param[in] bytesPtr                 - pointer to IP header.
* @param[in] numBytes                 - IP header length.
*
* @note 1. If there's a field CHECKSUM within the input-buffer
*       it supposed to be zero before calling this function.
*       2. The input buffer is supposed to be in network byte-order.
*
*/
GT_U32 ipV4CheckSumCalc
(
    IN GT_U8 *bytesPtr,
    IN GT_U16 numBytes
)
{
    GT_U32 vResult;
    GT_U32 sum;
    GT_U32 byteIndex;

    sum = 0;

    /* add up all of the 16 bit quantities */
    for (byteIndex = 0 ; byteIndex < numBytes ;  byteIndex += 2)
    {
        sum += (bytesPtr[byteIndex   + 0] << 8) |
                bytesPtr[byteIndex   + 1] ;
    }

    if (numBytes & 1)
    {
        /* an odd number of bytes */
        sum += (bytesPtr[byteIndex   + 0] << 8);
    }

    /* sum together the two 16 bits sections*/
    vResult = (GT_U16)(sum >> 16) + (GT_U16)sum;

    /* vResult can be bigger than a word (example : 0003 +
    *  fffe = 0001 0001), so sum its two words again.
    */
    if (vResult & 0x10000)
    {
        vResult -= 0x0ffff;
    }

    return (GT_U16)(~vResult);
}




/**
* @internal snetFieldFromEntry_GT_U32_Get function
* @endinternal
*
* @brief   Get GT_U32 value of a field from the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
*/
GT_U32 snetFieldFromEntry_GT_U32_Get(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           fieldIndex
)
{
    GT_U32  value;

    ASSERT_PTR(fieldsInfoArr);

    value = snetFieldValueGet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits);

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("table[%s] entryIndex[%d] fieldName[%s] value[0x%x]",
                  tableName ? tableName : "unknown",
                  entryIndex,
                  fieldsNamesArr ? fieldsNamesArr[fieldIndex] : "unknown",
                  value));


    return value;
}

/**
* @internal snetFieldFromEntry_subField_Get function
* @endinternal
*
* @brief   Get sub field (offset and num of bits) from a 'parent' field from the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] parentFieldIndex         - the index of the 'parent' field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
* @param[in] subFieldOffset           - bit offset from start of the parent field.
* @param[in] subFieldNumOfBits        - number of bits of the sub field.
*/
GT_U32 snetFieldFromEntry_subField_Get(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           parentFieldIndex,
    IN GT_U32                           subFieldOffset,
    IN GT_U32                           subFieldNumOfBits
)
{
    GT_U32  value;

    ASSERT_PTR(fieldsInfoArr);

    value = snetFieldValueGet(entryPtr,fieldsInfoArr[parentFieldIndex].startBit + subFieldOffset, subFieldNumOfBits);

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("table[%s] entryIndex[%d] fieldName[%s] subFieldOffset[%d] subFieldNumOfBits[%d] value[0x%x]",
                  tableName ? tableName : "unknown",
                  entryIndex,
                  fieldsNamesArr ? fieldsNamesArr[parentFieldIndex] : "unknown",
                  subFieldOffset,
                  subFieldNumOfBits,
                  value));


    return value;
}


/**
* @internal snetFieldFromEntry_Any_Get function
* @endinternal
*
* @brief   Get (any length) value of a field from the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
*
* @param[out] valueArr[]               - the array of GT_U32 that hold the value of the field.
*                                      RETURN:
*                                      None
*                                      COMMENTS:
*/
void snetFieldFromEntry_Any_Get(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          valueArr[]
)
{
    GT_U32  numOfWords;
    GT_U32  ii;
    GT_U32  subFieldOffset;
    GT_U32  subFieldNumOfBits;

    ASSERT_PTR(fieldsInfoArr);
    ASSERT_PTR(valueArr);

    numOfWords = CONVERT_BITS_TO_WORDS_MAC(fieldsInfoArr[fieldIndex].numOfBits);

    if(numOfWords <= 1)
    {
        valueArr[0] = snetFieldFromEntry_GT_U32_Get(devObjPtr,entryPtr,tableName,entryIndex,fieldsInfoArr,fieldsNamesArr,fieldIndex);
        return;
    }

    /* break the field into 'sub fields' of 32 bits
       to use snetFieldFromEntry_subField_Get */

    subFieldOffset = 0;
    subFieldNumOfBits = 32;
    for(ii = 0; ii < (numOfWords - 1); ii++,subFieldOffset += 32)
    {
        valueArr[ii] = snetFieldFromEntry_subField_Get(devObjPtr,entryPtr,tableName,
                entryIndex,fieldsInfoArr,fieldsNamesArr,
                fieldIndex,subFieldOffset,subFieldNumOfBits);
    }

    subFieldNumOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;/* %32 */
    if(subFieldNumOfBits == 0)
    {
        /* the last word is 32 bits */
        subFieldNumOfBits = 32;
    }

    /* get the last word */
    valueArr[ii] = snetFieldFromEntry_subField_Get(devObjPtr,entryPtr,tableName,
            entryIndex,fieldsInfoArr,fieldsNamesArr,
            fieldIndex,subFieldOffset,subFieldNumOfBits);

    return;
}


/**
* @internal snetFieldFromEntry_GT_U32_Set function
* @endinternal
*
* @brief   Set GT_U32 value into a field in the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
* @param[in] value                    - the  to set to the field (the  is 'masked'
*                                      according to the actual length of the field )
*/
void snetFieldFromEntry_GT_U32_Set(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           value
)
{
    GT_U32  actualUsedValue;
    ASSERT_PTR(fieldsInfoArr);

    snetFieldValueSet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits,value);
    actualUsedValue = SMEM_BIT_MASK(fieldsInfoArr[fieldIndex].numOfBits) & value;

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("table[%s] entryIndex[%d] fieldName[%s] value[0x%x]",
                  tableName ? tableName : "unknown",
                  entryIndex,
                  fieldsNamesArr ? fieldsNamesArr[fieldIndex] : "unknown",
                  actualUsedValue));


    return ;
}

/**
* @internal snetFieldFromEntry_subField_Set function
* @endinternal
*
* @brief   Set value to sub field (offset and num of bits) from a 'parent' field from the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] parentFieldIndex         - the index of the 'parent' field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
* @param[in] subFieldOffset           - bit offset from start of the parent field.
* @param[in] subFieldNumOfBits        - number of bits of the sub field.
* @param[in] value                    - the  to set to the sub field (the  is 'masked'
*                                      according to subFieldNumOfBits )
*/
void snetFieldFromEntry_subField_Set(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           parentFieldIndex,
    IN GT_U32                           subFieldOffset,
    IN GT_U32                           subFieldNumOfBits,
    IN GT_U32                           value
)
{
    ASSERT_PTR(fieldsInfoArr);

    snetFieldValueSet(entryPtr,fieldsInfoArr[parentFieldIndex].startBit + subFieldOffset,subFieldNumOfBits,value);

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("table[%s] entryIndex[%d] fieldName[%s] subFieldOffset[%d] subFieldNumOfBits[%d] value[0x%x]",
                  tableName ? tableName : "unknown",
                  entryIndex,
                  fieldsNamesArr ? fieldsNamesArr[parentFieldIndex] : "unknown",
                  subFieldOffset,
                  subFieldNumOfBits,
                  value));


    return ;
}

/**
* @internal snetFieldFromEntry_Any_Set function
* @endinternal
*
* @brief   Set (any length) value of a field from the table entry.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] entryIndex               - index of the entry in the table            --> for dump to LOG purpose only.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[] and in fieldsNamesArr[])
* @param[in] valueArr[]               - the array of GT_U32 that hold the value of the field.
*/
void snetFieldFromEntry_Any_Set(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN GT_U32                           *entryPtr,
    IN char *                           tableName,
    IN GT_U32                           entryIndex,
    IN SNET_ENTRY_FORMAT_TABLE_STC      fieldsInfoArr[],
    IN char *                           fieldsNamesArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           valueArr[]
)
{
    GT_U32  numOfWords;
    GT_U32  ii;
    GT_U32  subFieldOffset;
    GT_U32  subFieldNumOfBits;

    ASSERT_PTR(fieldsInfoArr);
    ASSERT_PTR(valueArr);

    numOfWords = CONVERT_BITS_TO_WORDS_MAC(fieldsInfoArr[fieldIndex].numOfBits);

    if(numOfWords <= 1)
    {
        snetFieldFromEntry_GT_U32_Set(devObjPtr,entryPtr,tableName,entryIndex,fieldsInfoArr,fieldsNamesArr,fieldIndex,valueArr[0]);
        return;
    }

    /* break the field into 'sub fields' of 32 bits
       to use snetFieldFromEntry_subField_Set */

    subFieldOffset = 0;
    subFieldNumOfBits = 32;
    for(ii = 0; ii < (numOfWords - 1); ii++,subFieldOffset += 32)
    {
        snetFieldFromEntry_subField_Set(devObjPtr,entryPtr,tableName,
                entryIndex,fieldsInfoArr,fieldsNamesArr,
                fieldIndex,subFieldOffset,subFieldNumOfBits,
                valueArr[ii]);
    }

    subFieldNumOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;/* %32 */
    if(subFieldNumOfBits == 0)
    {
        /* the last word is 32 bits */
        subFieldNumOfBits = 32;
    }

    /* set the last word */
    snetFieldFromEntry_subField_Set(devObjPtr,entryPtr,tableName,
            entryIndex,fieldsInfoArr,fieldsNamesArr,
            fieldIndex,subFieldOffset,subFieldNumOfBits,
            valueArr[ii]);

    return;
}

typedef struct{
    char *                           tableName;      /*table name (string)       --> can be NULL  --> for dump to LOG purpose only.*/
    GT_U32                           numOfFields;    /*the number of elements in in fieldsInfoArr[] and in fieldsNamesArr[].       */
    SNET_ENTRY_FORMAT_TABLE_STC     *fieldsInfoPtr;  /*array of fields info                                                        */
    char *                          *fieldsNamesPtr; /*array of fields names     --> can be NULL  --> for dump to LOG purpose only.*/
    GT_U32                           numBitsUsed;    /* number of bits in the entry */
}TABLE_AND_FIELDS_INFO_STC;

#define MAX_TABLES_CNS   512
static TABLE_AND_FIELDS_INFO_STC  tablesInfoArr[MAX_TABLES_CNS];
static GT_U32   lastTableIndex = 0;

/**
* @internal snetFillFieldsStartBitInfo function
* @endinternal
*
* @brief   Fill during init the 'start bit' of the fields in the table format.
*
* @param[in] devObjPtr                - pointer to device object. --> can be NULL --> for dump to LOG purpose only.
* @param[in] tableName                - table name (string)       --> can be NULL  --> for dump to LOG purpose only.
* @param[in] numOfFields              - the number of elements in in fieldsInfoArr[] and in fieldsNamesArr[].
* @param[in,out] fieldsInfoArr[]          - array of fields info
* @param[in] fieldsNamesArr[]         - array of fields names     --> can be NULL  --> for dump to LOG purpose only.
*/
void snetFillFieldsStartBitInfo(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN char *                           tableName,
    IN GT_U32                           numOfFields,
    INOUT SNET_ENTRY_FORMAT_TABLE_STC   fieldsInfoArr[],
    IN char *                           fieldsNamesArr[]
)
{
    GT_U32                      ii;
    SNET_ENTRY_FORMAT_TABLE_STC *currentFieldInfoPtr;
    SNET_ENTRY_FORMAT_TABLE_STC *prevFieldInfoPtr;
    GT_U32                      prevIndex;

    ASSERT_PTR(fieldsInfoArr);

    /* look for this table in the DB (by it's name) */
    for( ii = 0 ; ii < lastTableIndex ; ii++)
    {
        if(0 == strcmp(tablesInfoArr[ii].tableName,tableName))
        {
            /* table already registered */
            /* table names should be unique (currently not supported 'per device') */
            return;
        }
    }

    if(lastTableIndex == (MAX_TABLES_CNS - 1))
    {
        skernelFatalError("snetFillFieldsStartBitInfo : reached max number of tables ... need to enlarge MAX_TABLES_CNS \n");
        return;
    }


    simLogMessage(SIM_LOG_FUNC_NAME_MAC(snetFillFieldsStartBitInfo), devObjPtr, SIM_LOG_INFO_TYPE_DEVICE_E,
                  "table[%s] , numOfFields[%d]",
                  tableName ? tableName : "unknown",
                  numOfFields);


    for( ii = 0 ; ii < numOfFields ; ii++)
    {
        currentFieldInfoPtr = &fieldsInfoArr[ii];

        prevIndex = currentFieldInfoPtr->previousFieldType;

        if(currentFieldInfoPtr->startBit == FIELD_SET_IN_RUNTIME_CNS)
        {
            if(ii == 0)
            {
                /* first field got no options other then to start in bit 0 */
                currentFieldInfoPtr->startBit = 0;
            }
            else /* use the previous field info */
            {
                if(prevIndex == FIELD_CONSECUTIVE_CNS)
                {
                    /* this field is consecutive to the previous field */
                    prevIndex = ii-1;
                }
                else
                {
                    /* this field come after other previous field */
                }

                prevFieldInfoPtr = &fieldsInfoArr[prevIndex];
                currentFieldInfoPtr->startBit = prevFieldInfoPtr->startBit + prevFieldInfoPtr->numOfBits;
            }
        }
        else
        {
            /* no need to calculate the start bit -- it is FORCED by the entry format */
        }

        simLogMessage(SIM_LOG_FUNC_NAME_MAC(snetFillFieldsStartBitInfo), devObjPtr, SIM_LOG_INFO_TYPE_DEVICE_E,
                      "index[%d] fieldName[%s] startBit[%d] numOfBits[%d]",
                      ii,
                      fieldsNamesArr ? fieldsNamesArr[ii] : "unknown",
                      currentFieldInfoPtr->startBit,
                      currentFieldInfoPtr->numOfBits
                      );

        if(prevIndex != FIELD_CONSECUTIVE_CNS)
        {
            prevFieldInfoPtr = &fieldsInfoArr[prevIndex];

            simLogMessage(SIM_LOG_FUNC_NAME_MAC(snetFillFieldsStartBitInfo), devObjPtr, SIM_LOG_INFO_TYPE_DEVICE_E,
                          "     previous_index[%d] previous_fieldName[%s] previous_startBit[%d] previous_numOfBits[%d]",
                          prevIndex,
                          fieldsNamesArr ? fieldsNamesArr[prevIndex] : "unknown",
                          prevFieldInfoPtr->startBit,
                          prevFieldInfoPtr->numOfBits
                          );

        }
    }

    /* save the table into DB */
    tablesInfoArr[lastTableIndex].tableName      = tableName;
    tablesInfoArr[lastTableIndex].numOfFields    = numOfFields;
    tablesInfoArr[lastTableIndex].fieldsInfoPtr  = fieldsInfoArr;
    tablesInfoArr[lastTableIndex].fieldsNamesPtr = fieldsNamesArr;

    tablesInfoArr[lastTableIndex].numBitsUsed = 0;
    for( ii = 0 ; ii < numOfFields ; ii++)
    {
        currentFieldInfoPtr = &fieldsInfoArr[ii];

        if(tablesInfoArr[lastTableIndex].numBitsUsed <
            (currentFieldInfoPtr->startBit + currentFieldInfoPtr->numOfBits))
        {
            tablesInfoArr[lastTableIndex].numBitsUsed =
                (currentFieldInfoPtr->startBit + currentFieldInfoPtr->numOfBits);
        }
    }

    lastTableIndex ++;
}

static GT_U32 numOfCharsInPrefixNameGet(
    IN TABLE_AND_FIELDS_INFO_STC   *currentTableInfoPtr/* current table info */
)
{
    GT_U32  ii,jj;
    static char tempFieldsPrefix[256];
    GT_U32 numOfCharsInPrefixName = (GT_U32)strlen(currentTableInfoPtr->fieldsNamesPtr[0]);

    for( ii = 1 ; ii < currentTableInfoPtr->numOfFields ; ii++)
    {
        for(jj = numOfCharsInPrefixName ; jj > 5 ; jj--)
        {
            if(0 == strncmp(currentTableInfoPtr->fieldsNamesPtr[0] ,
                            currentTableInfoPtr->fieldsNamesPtr[ii] ,
                            jj))
            {
                /* fount new shorter prefix */
                break;
            }
        }

        if(jj == 5)
        {
            /* no need to bother and look for prefix ... it is not exists */
            numOfCharsInPrefixName = 0;
            break;
        }
        else
        {
            numOfCharsInPrefixName = jj;
        }
    }

    if(numOfCharsInPrefixName >= 256)
    {
        numOfCharsInPrefixName = 255;
    }

    strncpy(tempFieldsPrefix,
           currentTableInfoPtr->fieldsNamesPtr[0],
           numOfCharsInPrefixName);

    tempFieldsPrefix[numOfCharsInPrefixName] = 0;/* string terminator*/

    simGeneralPrintf(" removing [%s] prefix from fields names \n",
        tempFieldsPrefix);

    return numOfCharsInPrefixName;
}



/**
* @internal snetPrintFieldsInfo function
* @endinternal
*
* @brief   print the info about the fields in the table format.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tableName                - table name (string)
*                                      NOTE:
*                                      1. can be 'prefix of name' for multi tables !!!
*                                      2. when NULL .. print ALL tables !!!
*/
void snetPrintFieldsInfo(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN char *                           tableName
)
{
    GT_U32      ii;
    TABLE_AND_FIELDS_INFO_STC   *currentTableInfoPtr;/* current table info */
    SNET_ENTRY_FORMAT_TABLE_STC *currentFieldInfoPtr;/* current field info */
    char                        *currentFieldNamePtr;/* current field name */
    GT_U32      lenOfTableName;
    GT_U32      currentTableIndex;
    GT_U32      numTablesPrinted = 0;
    GT_U32      numOfCharsInPrefixName;/* the names of the fields are most likely to have 'same prefix' .
                                        calculate the 'prefix length' */

    devObjPtr = devObjPtr;/*currently not used*/

    if(tableName == NULL)
    {
        lenOfTableName = 0;
        simGeneralPrintf("snetPrintFieldsInfo : print ALL tables \n");
    }
    else
    {
        simGeneralPrintf("snetPrintFieldsInfo : print table(s) [%s] \n", tableName);
        lenOfTableName = (GT_U32)strlen(tableName);
    }

    currentTableIndex = 0 ;

    /* this while allow */
    while(currentTableIndex < lastTableIndex)
    {
        /* look for this table in the DB (by it's name) */
        for( /*continue*/ ; currentTableIndex < lastTableIndex ; currentTableIndex++)
        {
            if(tableName == NULL)
            {
                /* print all tables */
                break;
            }

            if(0 == strncmp(tablesInfoArr[currentTableIndex].tableName,tableName,lenOfTableName))
            {
                /* table found */
                /* table names should be unique (currently not supported 'per device') */
                break;
            }
        }

        if(currentTableIndex == lastTableIndex)
        {
            break;
        }

        currentTableInfoPtr = &tablesInfoArr[currentTableIndex];
        /* update the 'current index' for the next use */
        currentTableIndex++;

        if(numTablesPrinted)
        {
            simGeneralPrintf("--- end table --- [%d] \n", numTablesPrinted);
            simGeneralPrintf("\n\n\n\n");
            simGeneralPrintf("printing table [%d] \n", (numTablesPrinted + 1));
        }

        simGeneralPrintf("table[%s] with numBitsUsed[%d] \n",
                      currentTableInfoPtr->tableName,
                      currentTableInfoPtr->numBitsUsed);

        numOfCharsInPrefixName = numOfCharsInPrefixNameGet(currentTableInfoPtr);

        simGeneralPrintf("index  startBit  numOfBits  fieldName \n");

        currentFieldInfoPtr = &currentTableInfoPtr->fieldsInfoPtr[0];
        for( ii = 0 ; ii < currentTableInfoPtr->numOfFields ; ii++,
            currentFieldInfoPtr++)
        {
            currentFieldNamePtr = currentTableInfoPtr->fieldsNamesPtr[ii];
            if(currentFieldInfoPtr->numOfBits)
            {
                simGeneralPrintf("%d \t %d \t %d \t %s \n",
                          ii,
                          currentFieldInfoPtr->startBit,
                          currentFieldInfoPtr->numOfBits,
                          &currentFieldNamePtr[numOfCharsInPrefixName]
                          );
            }
            else /* 0 bits state only field name */
            {
                simGeneralPrintf("%d \t -- \t 0 \t %s \n",
                          ii,
                          &currentFieldNamePtr[numOfCharsInPrefixName]
                          );
            }
        }

        simGeneralPrintf(" --- end fields ---\n");

        numTablesPrinted++;
    }

    if(numTablesPrinted == 0)
    {
        simGeneralPrintf("table[%s] not found \n",
            tableName ? tableName : "NULL");
    }
    else if(numTablesPrinted == 1)
    {
        simGeneralPrintf(" --- end table (single table) ---\n");
    }
    else
    {
        simGeneralPrintf("--- end table --- [%d] \n", numTablesPrinted);
        simGeneralPrintf("\n\n\n\n");
        simGeneralPrintf("printed [%d] tables \n", numTablesPrinted);
    }

    return;
}

/**
* @internal snetPrintFieldsInfo_debug function
* @endinternal
*
* @brief   debug function to print the table(s) fields format.
*
* @param[in] devNum                   - device number as stated in the INI file.
* @param[in] tableName                - table name (string)
*                                      NOTE:
*                                      1. can be 'prefix of name' for multi tables !!!
*                                      2. when NULL .. print ALL tables !!!
*                                       None
*/
void snetPrintFieldsInfo_debug(
    IN GT_U32                           devNum,
    IN char *                           tableName
)
{
    SKERNEL_DEVICE_OBJECT* deviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_DEVICE_OBJECT* currDeviceObjPtr;
    GT_U32  dev;

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        simGeneralPrintf(" multi-core device [%d] \n",devNum);
        for(dev = 0 ; dev < deviceObjPtr->numOfCoreDevs ; dev++)
        {
            currDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[dev].devObjPtr;
            snetPrintFieldsInfo(currDeviceObjPtr,tableName);
        }
    }
    else
    {
        snetPrintFieldsInfo(deviceObjPtr,tableName);
    }
}

static void printFullMemEntry(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN TABLE_AND_FIELDS_INFO_STC        *currentTableInfoPtr,
    IN GT_U32                           *entryPtr
)
{
    GT_U32  ii,jj;
    static GT_U32  valueArr[128];
    GT_U32  numOfWords;
    char*   valueFormats[] = {
         "[%d]\t\t"                                  /*0 - for up to 8  bits field */
        ,"[0x%4.4x]\t"                             /*1 - for up to 16 bits field */
        ,"[0x%6.6x]\t"                             /*2 - for up to 24 bits field */
        ,"[0x%8.8x]\t"                               /*3 - for up to 32 bits field */
        ,"[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] \t " /*4 - for MAC_ADDR field */
        ,"[%s]\t\t"                                  /*5 - for '1 bit' field */
        };
    char*   currentValueFormatPtr;
    GT_U32      numOfCharsInPrefixName;/* the names of the fields are most likely to have 'same prefix' .
                                        calculate the 'prefix length' */

    numOfCharsInPrefixName = numOfCharsInPrefixNameGet(currentTableInfoPtr);

    for(ii = 0 ; ii < currentTableInfoPtr->numOfFields; ii++)
    {
        snetFieldFromEntry_Any_Get(devObjPtr,entryPtr,
            currentTableInfoPtr->tableName,
            0 , /*entryIndex -- unknown*/
            currentTableInfoPtr->fieldsInfoPtr,
            currentTableInfoPtr->fieldsNamesPtr,
            ii,/*fieldIndex*/
            valueArr);

        numOfWords = (currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits + 31) / 32;

        if(numOfWords == 1)
        {
            if(currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits == 1)
            {
                currentValueFormatPtr = /* ON/OFF field */ valueFormats[5];

                /*print the field value according to it's format */
                simGeneralPrintf(currentValueFormatPtr ,
                    valueArr[0] ? "ON" : "OFF");
            }
            else
            {
                if(currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits <= 8)
                {
                    currentValueFormatPtr = /* decimal value */ valueFormats[0];
                }
                else
                if(currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits <= 16)
                {
                    currentValueFormatPtr = /* hex value */ valueFormats[1];
                }
                else
                if(currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits <= 24)
                {
                    currentValueFormatPtr = /* hex value */ valueFormats[2];
                }
                else/* up to 32*/
                {
                    currentValueFormatPtr = /* hex value */ valueFormats[3];
                }

                /*print the field value according to it's format */
                simGeneralPrintf(currentValueFormatPtr ,
                    valueArr[0]);
            }
        }
        else
        if(currentTableInfoPtr->fieldsInfoPtr[ii].numOfBits == 48)
        {
            currentValueFormatPtr = /* mac address */ valueFormats[4];
            /* mac address */
            simGeneralPrintf(currentValueFormatPtr
                ,(GT_U8)(valueArr[1] >>  8)
                ,(GT_U8)(valueArr[1] >>  0)
                ,(GT_U8)(valueArr[0] >> 24)
                ,(GT_U8)(valueArr[0] >> 16)
                ,(GT_U8)(valueArr[0] >>  8)
                ,(GT_U8)(valueArr[0] >>  0)
                );
        }
        else
        {
            currentValueFormatPtr = /* hex value */ valueFormats[3];

            for(jj = 0 ; jj < numOfWords ; jj++)
            {
                /*print the field value according to it's format */
                simGeneralPrintf(currentValueFormatPtr ,
                    valueArr[jj]);
            }
        }
        simGeneralPrintf("[%s] \n",
            &((currentTableInfoPtr->fieldsNamesPtr[ii])[numOfCharsInPrefixName]));
    }

    simGeneralPrintf("\n\n\n\n");

}

/**
* @internal snetPrintFieldsInfoForSpecificMemoryEntry_debug function
* @endinternal
*
* @brief   debug function to print the content of specific entry in memory according
*         to known format.
* @param[in] devNum                   - device number as stated in the INI file.
* @param[in] tableName                - table name (string)
*                                      NOTE:
*                                      1. can be 'prefix of name' for multi tables !!!
*                                      2. when NULL .. print ALL tables !!!
* @param[in] startAddress             - start address of the entry in the memory
*                                       None
*/
void snetPrintFieldsInfoForSpecificMemoryEntry_debug(
    IN GT_U32                           devNum,
    IN char *                           tableName,
    IN GT_U32                           startAddress
)
{
    SKERNEL_DEVICE_OBJECT* deviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_DEVICE_OBJECT* currDeviceObjPtr;
    GT_U32  dev;
    GT_U32 ii;
    GT_U32  *entryPtr;

    if(tableName == NULL)
    {
        simGeneralPrintf(" table name 'NULL' \n");
        return ;
    }

    /* look for this table in the DB (by it's name) */
    for( ii = 0 ; ii < lastTableIndex ; ii++)
    {
        if(0 == strcmp(tablesInfoArr[ii].tableName,tableName))
        {
            /* table already registered */
            /* table names should be unique (currently not supported 'per device') */
            break;
        }
    }

    if(ii == lastTableIndex)
    {
        simGeneralPrintf(" table name [%s] not found  \n",devNum);
        return ;
    }

    simGeneralPrintf("print from table [%s] entry starts at address [0x%8.8x] with numBitsUsed[%d] \n",
        tablesInfoArr[ii].tableName,
        startAddress,
        tablesInfoArr[ii].numBitsUsed);

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        simGeneralPrintf(" multi-core device [%d] \n",devNum);
        for(dev = 0 ; dev < deviceObjPtr->numOfCoreDevs ; dev++)
        {
            currDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[dev].devObjPtr;

            if(GT_FALSE ==
                smemIsDeviceMemoryOwner(currDeviceObjPtr,startAddress))
            {
                simGeneralPrintf("skip core[%d] , because not owner of this memory \n" , dev);
                /* the device is not the owner of this memory */
                continue;
            }

            simGeneralPrintf("Print the format for core[%d] \n" , dev);

            entryPtr = smemMemGet(currDeviceObjPtr , startAddress);

            printFullMemEntry(currDeviceObjPtr,&tablesInfoArr[ii],entryPtr);
        }
    }
    else
    {
        entryPtr = smemMemGet(deviceObjPtr , startAddress);
        printFullMemEntry(deviceObjPtr,&tablesInfoArr[ii],entryPtr);
    }
}
