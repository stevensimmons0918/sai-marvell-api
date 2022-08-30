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
* @file snet.h
*
* @brief This is a external API definition for SMem module of SKernel.
*
* @version   37
********************************************************************************
*/
#ifndef __sneth
#define __sneth


#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BITS_IN_BYTE 8

/* Convert tick clocks to seconds and nanoseconds */
#define SNET_TOD_CLOCK_FORMAT_MAC(clock, sec, nanosec) \
{ \
    sec = (clock) / 1000; \
    nanosec = ((clock) % 1000) * 1000000; \
}

#define SNET_GET_TIME_IN_MICROSEC_MAC()    \
    (1000 * (SIM_OS_MAC(simOsTickGet)()))

/* convert the clock that is in milliseconds , to 'time stamp' that is 2 bits seconds and 30 bits 'nano' */
#define SNET_CONVERT_MILLISEC_TO_TIME_STAMP_MAC(clock)    \
   (((clock)/1000) << 30)/*seconds*/ | (0x3FFFFFFF & (((clock)%1000)*1000000)) /* nanos*/

/*
 * Typedef: struct STRUCT_TX_DESC
 *
 * Description: Includes the PP Tx descriptor fields, to be used for handling
 *              packet transmits.
 *
 * Fields:
 *      word1           - First word of the Tx Descriptor.
 *      word2           - Second word of the Tx Descriptor.
 *      buffPointer     - The physical data-buffer address.
 *      nextDescPointer - The physical address of the next Tx descriptor.
 *
 */
typedef struct _txDesc
{
    /*volatile*/ GT_U32         word1;
    /*volatile*/ GT_U32         word2;

    /*volatile*/ GT_U32         buffPointer;
    /*volatile*/ GT_U32         nextDescPointer;
}SNET_STRUCT_TX_DESC;

/****** SDMA *********************************************/
/* Get / Set the Own bit field of the tx descriptor.    */
#define TX_DESC_GET_OWN_BIT(txDesc)                         \
            (((txDesc)->word1) >> 31)
#define TX_DESC_SET_OWN_BIT(txDesc,val)                     \
            (SMEM_U32_SET_FIELD((txDesc)->word1, 31, 1, val))

/* Get / Set the First bit field of the tx descriptor.  */
#define TX_DESC_GET_FIRST_BIT(txDesc)                       \
            ((((txDesc)->word1) >> 21) & 0x1)
#define TX_DESC_SET_FIRST_BIT(txDesc,val)                   \
            (SMEM_U32_SET_FIELD((txDesc)->word1, 21, 1, val))

/* If set, CRC of the packet should be recalculated on packet transmission */
#define TX_DESC_GET_RECALC_CRC_BIT(txDesc)                  \
            ((((txDesc)->word1) >> 12) & 0x1)

/* Get / Set the Last bit field of the tx descriptor.   */
#define TX_DESC_GET_LAST_BIT(txDesc)                        \
            ((((txDesc)->word1)>> 20) & 0x1)
#define TX_DESC_SET_LAST_BIT(txDesc,val)                    \
            (SMEM_U32_SET_FIELD((txDesc)->word1, 20, 1, val))

/* Get / Set the Int bit field of the tx descriptor.    */
#define TX_DESC_GET_INT_BIT(txDesc)                         \
            (((txDesc)->word1 >> 23) & 0x1)

#define TX_HEADER_SIZE      (16)  /* Bytes    */

/* Tx descriptor related macros.    */
#define TX_DESC_CPU_OWN     (0)
#define TX_DESC_DMA_OWN     (1)

/* Number of Transmit Queues in ASIC */
#define TRANS_QUEUE_MAX_NUMBER                       8

/* get the byte order that the PP should sent to Host CPU
NOTE : the field called 'Rx' but it is from HOST point of view
       so it is actually has impact on simulation that do 'tx to cpu'
*/
#define TX_BYTE_ORDER_MAC(devObjPtr) \
    (devObjPtr->rxByteSwap ? SCIB_DMA_WORDS : SCIB_DMA_BYTE_STREAM)

/* get the byte order that the PP should receive from the Host CPU
NOTE : the field called 'Tx' but it is from HOST point of view
       so it is actually has impact on simulation that do 'rx from cpu'
*/
#define RX_BYTE_ORDER_MAC(devObjPtr) \
    (devObjPtr->txByteSwap ? SCIB_DMA_WORDS : SCIB_DMA_BYTE_STREAM)


#define     SNET_FRAMES_1024_TO_MAX_OCTETS           1024
#define     SNET_FRAMES_512_TO_1023_OCTETS           512
#define     SNET_FRAMES_256_TO_511_OCTETS            256
#define     SNET_FRAMES_128_TO_255_OCTETS            128
#define     SNET_FRAMES_65_TO_127_OCTETS             65
#define     SNET_FRAMES_64_OCTETS                    64

#define SNET_GET_NUM_OCTETS_IN_FRAME(size) \
    (size >= SNET_FRAMES_1024_TO_MAX_OCTETS) ? SNET_FRAMES_1024_TO_MAX_OCTETS : \
    (size >= SNET_FRAMES_512_TO_1023_OCTETS) ? SNET_FRAMES_512_TO_1023_OCTETS : \
    (size >= SNET_FRAMES_256_TO_511_OCTETS) ? SNET_FRAMES_256_TO_511_OCTETS : \
    (size >= SNET_FRAMES_128_TO_255_OCTETS) ? SNET_FRAMES_128_TO_255_OCTETS : \
    (size >= SNET_FRAMES_65_TO_127_OCTETS) ? SNET_FRAMES_65_TO_127_OCTETS : \
    (size >= SNET_FRAMES_64_OCTETS) ? SNET_FRAMES_64_OCTETS : 0

/* Threshold between 1024to1518 and 1519toMax MIB counters is variable: */
#define SNET_MIB_CNT_THRESHOLD_1518  1518
#define SNET_MIB_CNT_THRESHOLD_1522  1522

typedef enum {
    SNET_SFLOW_RX = 0,
    SNET_SFLOW_TX
} SNET_SFLOW_DIRECTION_ENT;

/* apply mask on value and key , check if equal */
#define SNET_CHT_MASK_CHECK(value,mask,key) \
    (((value)&(mask)) == ((key)&(mask)))

/* build GT_U32 from pointer to 4 bytes */
#define SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(bytesPtr)\
    ((GT_U32)(((bytesPtr)[0] << 24) | ((bytesPtr)[1]) << 16 | ((bytesPtr)[2]) << 8 | ((bytesPtr)[3] << 0)))

/* build GT_U16 from pointer to 2 bytes */
#define SNET_BUILD_HALF_WORD_FROM_BYTES_PTR_MAC(bytesPtr)\
    ((GT_U16)(((bytesPtr)[0] << 8) | ((bytesPtr)[1]) << 0))

/* Retrieve DSA tag from frame data buffer */
#define DSA_TAG(_frame_data_ptr)\
    (GT_U32)((_frame_data_ptr)[12] << 24 | (_frame_data_ptr)[13] << 16 |\
             (_frame_data_ptr)[14] << 8  | (_frame_data_ptr)[15])


/* build GT_U32 from pointer to 4 bytes */
#define SNET_BUILD_BYTES_FROM_WORD_MAC(word,bytesPtr)\
    bytesPtr[0] = (GT_U8)SMEM_U32_GET_FIELD(word,24,8);\
    bytesPtr[1] = (GT_U8)SMEM_U32_GET_FIELD(word,16,8);\
    bytesPtr[2] = (GT_U8)SMEM_U32_GET_FIELD(word, 8,8);\
    bytesPtr[3] = (GT_U8)SMEM_U32_GET_FIELD(word, 0,8)

#define SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descr, offset) \
                (((descr)->startFramePtr[(offset)] << 8) | \
                 ((descr)->startFramePtr[(offset) + 1]))

#define SNET_GET_PCKT_TAG_VLAN_ID_MAC(descr, offset) \
                (((descr)->startFramePtr[(offset) + 2] << 8) | \
                 ((descr)->startFramePtr[(offset) + 3])) & 0xfff

#define SNET_GET_PCKT_TAG_UP_MAC(descr, offset) \
                ((descr)->startFramePtr[(offset) + 2]) >> 5

#define SNET_GET_PCKT_TAG_CFI_DEI_MAC(descr, offset) \
                (((descr)->startFramePtr[(offset) + 2] >> 4)) & 0x1

#define SNET_GET_PCKT_6B_TAG_TCI_GET_LOW_16_MAC(descr, offset)  (((descr)->startFramePtr[(offset) + 6] << 8)  |       \
                                                                 ((descr)->startFramePtr[(offset) + 7]))
#define SNET_GET_PCKT_6B_TAG_TCI_GET_HIGH_32_MAC(descr, offset) (((descr)->startFramePtr[(offset) + 2] << 24) |       \
                                                                 ((descr)->startFramePtr[(offset) + 3] << 16) |       \
                                                                 ((descr)->startFramePtr[(offset) + 4] <<  8) |       \
                                                                 ((descr)->startFramePtr[(offset) + 5]))

/* indication to set SNET_ENTRY_FORMAT_TABLE_STC::startBit during run time */
#define FIELD_SET_IN_RUNTIME_CNS   0xFFFFFFFF
/* indication in SNET_ENTRY_FORMAT_TABLE_STC::previousFieldType that 'current'
field is consecutive to the previous field */
#define FIELD_CONSECUTIVE_CNS      0xFFFFFFFF

#define STR(strname)    \
    #strname

/*
 * Typedef: struct SNET_ENTRY_FORMAT_TABLE_STC
 *
 * Description: A structure to hold info about field in entry of table
 *
 * Fields:
 *      startBit  - start bit of the field. filled in runtime when value != FIELD_SET_IN_RUNTIME_CNS
 *      numOfBits - number of bits in the field
 *      previousFieldType - 'point' to the previous field for calculation of startBit.
 *                          used when != FIELD_CONSECUTIVE_CNS
 *
 */
typedef struct SNET_ENTRY_FORMAT_TABLE_STCT{
    GT_U32      startBit;
    GT_U32      numOfBits;
    GT_U32      previousFieldType;
}SNET_ENTRY_FORMAT_TABLE_STC;

/* macro to fill instance of SNET_ENTRY_FORMAT_TABLE_STC with value that good for 'standard' field.
'standard' field is a field that comes after the field that is defined before it in the array of fields

the macro gets the <numOfBits> of the current field.
*/
#define STANDARD_FIELD_MAC(numOfBits)     \
        {FIELD_SET_IN_RUNTIME_CNS,        \
         numOfBits,                       \
         FIELD_CONSECUTIVE_CNS}

/* macro to set <startBit> and <numOfBits>
the macro gets the <numOfBits> of the current field.
*/
#define EXPLICIT_FIELD_MAC(startBit,numOfBits)     \
        {startBit,        \
         numOfBits,                       \
         0/*don't care*/}


/**
* @internal SNET_IPCL_LOOKUP_ENT function
* @endinternal
*
*/
typedef enum{
    SNET_IPCL_LOOKUP_0_0_E = 0,
    SNET_IPCL_LOOKUP_0_1_E,
    SNET_IPCL_LOOKUP_1_E
}SNET_IPCL_LOOKUP_ENT;

/* Packet Type field: bits 46:47 in USXGMII_PCH<Extension> */
typedef enum {

    /* packet information */
    SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_ETHERNET_WITH_PCH_E,

    /* packet information */
    SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_ETHERNET_WITHOUT_PCH_E,

    /* contains status data for a port - no packet data */
    SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_IDLE_E,

    /* reserved */
    SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_RESERVED_E

} SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_ENT;

/* Extention Type field: bits 40:41 in USXGMII_PCH<Extension> */
typedef enum {
    /* Ignore extention field */
    SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_IGNORE_E,

    /* Extention field contains 8-bit reserved + 32-bit Timestamp */
    SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_PTP_E,

    /* Externtion field: for devices that support Preemption;
       otherwise reserved. It also contains fields related to PTP timestamp */
    SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_PREEMPTION_PTP_E,

    /* Reserved */
    SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_RESERVED_E

} SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_ENT;

/* USXGMII PCH PTP information */
typedef struct
{
    /* Packet Type field: bits 46:47 */
    SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_ENT packetType;

    /* USXGMII PHY channel id field: bits 42:45; port0-port15 */
    GT_U32 subPortId;

    /* Extention Type field: bits 40:41 */
    SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_ENT extentionType;

    /* Ingress Timestamp field: (2 bits seconds , 30 bits nanoSeconds) */
    SNET_TOD_TIMER_STC timestamp;

    /* Signature associated with the egress PTP timestamp to be done in the PHY */
    SNET_PTP_PHY_INFO_STC signature;

    /* When enabled PHY adds egress PTP timestamp to the assicated frame */
    GT_BOOL ptpTimestampEnable;

    /* 2-bit frame counter: bits 34-35 */
    GT_U32  frameCount;

}SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC;

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
);

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
);


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
);

/**
* @internal snetFrameParsing function
* @endinternal
*
* @brief   Parsing the frame, get information from frame and fill descriptor
*/
void snetFrameParsing
(
    IN SKERNEL_DEVICE_OBJECT        *     deviceObjPtr,
    INOUT SKERNEL_FRAME_DESCR_STC   *     descrPrt
);

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
);

/**
* @internal snetFromCpuDmaProcess function
* @endinternal
*
* @brief   Process transmitted SDMA queue frames
*/
GT_VOID snetFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT      *     deviceObjPtr,
    IN SBUF_BUF_ID                      bufferId
);

/**
* @internal snetFromEmbeddedCpuProcess function
* @endinternal
*
* @brief   Process transmitted frames from the Embedded CPU to the PP
*/
GT_VOID snetFromEmbeddedCpuProcess
(
    IN SKERNEL_DEVICE_OBJECT      *     devObjPtr,
    IN SBUF_BUF_ID                      bufferId
);


/**
* @internal snetCncFastDumpUploadAction function
* @endinternal
*
* @brief   Process upload CNC block by CPU demand
*/
GT_VOID snetCncFastDumpUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                * cncTrigPtr
);

/**
* @internal snetUtilGetContinuesValue function
* @endinternal
*
* @brief   the function get value of next continues fields .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] startAddress             - need to be register address %4 == 0.
* @param[in] sizeofValue              - how many bits this continues field is
* @param[in] indexOfField             - what is the index of the field
*/
extern GT_U32 snetUtilGetContinuesValue(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 startAddress ,
    IN GT_U32 sizeofValue,
    IN GT_U32 indexOfField
);

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
);

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
);

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
);


/**
* @internal ipV4CheckSumCalc function
* @endinternal
*
* @brief   Perform ones-complement sum , and ones-complement on the final sum-word.
*         The function can be used to make checksum for various protocols.
*
* @note 1. If there's a field CHECKSUM within the input-buffer
*       it supposed to be zero before calling this function.
*       2. The input buffer is supposed to be in network byte-order.
*
*/
extern GT_U32 ipV4CheckSumCalc
(
    IN GT_U8 *pMsg,
    IN GT_U16 lMsg
);

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
);

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
);

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
);


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
);

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
);

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
);



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
);

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
);

/* common macro to set an entry field of size <= 32 bits */
#define SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,_tableName) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                  \
        _memPtr,                                               \
        _devObjPtr->tableFormatInfo[_tableName].formatNamePtr, \
        _index,/* the index */                                 \
        _devObjPtr->tableFormatInfo[_tableName].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[_tableName].fieldsNamePtr, \
        fieldName,_value)

/* common macro to get an entry field of size <= 32 bits */
#define SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,_tableName) \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                           \
        _memPtr,                                                        \
        _devObjPtr->tableFormatInfo[_tableName].formatNamePtr,                              \
        _index,/* the index */                                          \
        _devObjPtr->tableFormatInfo[_tableName].fieldsInfoPtr,\
        _devObjPtr->tableFormatInfo[_tableName].fieldsNamePtr,       \
        fieldName)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sneth */

