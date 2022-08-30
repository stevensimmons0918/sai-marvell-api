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
* @file snetFalconExactMatch.h
*
* @brief This is a external API definition for SIP6 Exact Match
*
* @version   1
********************************************************************************
*/
#ifndef __snetFalconExactMatchh
#define __snetFalconExactMatchh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* default entry match indicator */
#define SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS          (0x7ffffff0)
#define SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP1_CNS          (0x7ffffff1)


/* max number of parallel lookups (hitNum) */
#define SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS                     2

/* max number of exact match Clients */
#define SIP6_EXACT_MATCH_MAX_CLIENT_CNS                           2

/* max number of banks in EXACT_MATCH */
#define SIP6_EXACT_MATCH_MAX_NUM_BANKS_CNS                       16

/* size of EXACT_MATCH KEY in bytes   */
#define SIP6_EXACT_MATCH_MAX_KEY_SIZE_BYTE_CNS                   47

/* size of EXACT_MATCH KEY in bits   */
#define SIP6_EXACT_MATCH_MAX_KEY_SIZE_BIT_CNS                   376

/* size of EXACT_MATCH full action in words   */
#define SIP6_EXACT_MATCH_FULL_ACTION_SIZE_WORD_CNS                8

/* size of EXACT_MATCH KEY in words  */
#define SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS                   12

/* size of key field in exact match key entry in bit    */
#define SIP6_EXACT_MATCH_KEY_ONLY_FIELD_SIZE_CNS               112


/* Enum values represent byte size of each key */
typedef enum{
    SIP6_EXACT_MATCH_KEY_SIZE_5B_E  = 0,     /* 5 bytes  */
    SIP6_EXACT_MATCH_KEY_SIZE_19B_E = 1,     /* 19 bytes */
    SIP6_EXACT_MATCH_KEY_SIZE_33B_E = 2,     /* 33 bytes */
    SIP6_EXACT_MATCH_KEY_SIZE_47B_E = 3,     /* 47 bytes */

    SIP6_EXACT_MATCH_KEY_SIZE_LAST_E         /* last value */
}SIP6_EXACT_MATCH_KEY_SIZE_ENT;


typedef struct {
    GT_U32 valid            ;       /*1 bit */
    GT_U32 entry_type       ;       /*1 bit - 0  EXACT_MATCH Key and Action
                                    *         1  EXACT_MATCH Key Only       */

    /*if entry is key and action*/
    GT_U32 age              ;       /* 1 bit  */
    GT_U32 key_size         ;       /* 2 bits */
    GT_U32 lookup_number    ;       /* 1 bit  */
    GT_U32 key_31_0         ;
    GT_U32 key_39_32        ;       /* 40 bit */
    GT_U32 action_31_0      ;
    GT_U32 action_63_32     ;
    GT_U32 action_67_64     ;       /* 68 bit */

    /*if entry is key only */
    GT_U32 keyOnly_31_0       ;
    GT_U32 keyOnly_63_32      ;
    GT_U32 keyOnly_95_64      ;
    GT_U32 keyOnly_111_96     ;       /* 112 bit*/

}SNET_SIP6_EXACT_MATCH_ENTRY_INFO;

/* Converts key size to number of Bytes  */
#define SMEM_FALCON_EXACT_MATCH_KEY_SIZE_TO_BYTE_MAC(keySize)       \
    (                                                      \
     (keySize) == 0x0 /*SIP6_EXACT_MATCH_KEY_SIZE_5B_E */  ? 5  :   \
     (keySize) == 0x1 /*SIP6_EXACT_MATCH_KEY_SIZE_19B_E*/  ? 19 :   \
     (keySize) == 0x2 /*SIP6_EXACT_MATCH_KEY_SIZE_33B_E*/  ? 33 :   \
     (keySize) == 0x3 /*SIP6_EXACT_MATCH_KEY_SIZE_47B_E*/  ? 47 :   \
     0)

/**
* @internal snetExactMatchTablesFormatInit function
* @endinternal
*
* @brief   init the format of Exact Match tables.
*
* @param[in] devObjPtr       - pointer to device object.
*/
void snetExactMatchTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

/**
* @internal snetFalconExactMatchProfileIdGet function
* @endinternal
*
* @brief   Gets Exact Match Lookup Profile ID1/ID2 for TTI
*          PCL and EPCL packet type.
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] Client                         - tcam client (TTI,PCL,EPCL)
* @param[in] index                          - keyType in case of TTI
*                                             or index for EM Profile-ID mapping table
*                                             in case of PCL/EPCL
* @param[out] exactMatchProfileIdArr        - Exact Match Lookup
*                                             Profile ID Array
*/
GT_VOID snetFalconExactMatchProfileIdGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP5_TCAM_CLIENT_ENT               Client,
    IN  GT_U32                             index,
    OUT GT_U32                             exactMatchProfileIdArr[]
);

/**
* @internal snetFalconExactMatchProfileTableControlGet function
* @endinternal
*
* @brief   return Profile Table Control fields according to the ProfileID
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] exactMatchProfileId      - index to the exact match
*                                       profiles table
* @param[out]keySizePtr               - pointer to key size
* @param[out]keyStartPtr              - pointer to start of EM
*                                       Search key in the TCAM key
* @param[out]enableDefaultPtr         - pointer to enable
*                                       default action use,
*                                       in case no match in EM
*                                       lookup or in TCAM lookup
*/
GT_U32 snetFalconExactMatchProfileTableControlGet
(
    IN  SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN  GT_U32                            exactMacthProfileId,
    OUT SIP6_EXACT_MATCH_KEY_SIZE_ENT    *keySizePtr,
    OUT GT_U32                           *keyStartPtr,
    OUT GT_U32                           *enableDefaultPtr
);

/**
* @internal snetFalconExactMatchLookupByGroupId function
* @endinternal
*
* @brief  do lookup in Exact Match for given key and fill the results array
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] groupId                    - Exact match client group Id
* @param[in] tcamClient                 - Exact match client
* @param[in] keyArrayPtr                - key array (size up to 47 bytes)
* @param[in] keySize                    - size of the key number of hits found
* @param[in] exactMatchProfileIdArr     - Exact Match lookup profile ID Array
* @param[out] exactMatchClientMatchArr  - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                 - Exact Match hits
*/
GT_U32 snetFalconExactMatchLookupByGroupId
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                    groupId,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    IN  GT_U32                    exactMatchProfileIdArr[],
    OUT GT_BOOL                   exactMatchClientMatchArr[],
    OUT GT_U32                    resultArr[]
);

/**
* @internal snetFalconExactMacthLookup function
* @endinternal
*
* @brief   do lookup in Exact match for given key and
*          fill the results array
*
* @param[in] devObjPtr                 - (pointer to) the device object
* @param[in] tcamClient                - Exact match client
* @param[in] keyArrayPtr               - key array (size up to 47
*                                        bytes)
* @param[in] keySize                   - size of the key
*                                        number of hits found
* @param[in] exactMatchProfileIdArr    - Exact Match lookup
*                                        profile ID array
* @param[out] exactMatchClientMatchArr - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                - Exact Match hits
*
*/
GT_U32 snetFalconExactMacthLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    IN  GT_U32                    exactMatchProfileIdArr[],
    OUT GT_BOOL                   exactMatchClientMatchArr[],
    OUT GT_U32                    resultArr[]
);

/**
* @internal snetFalconExactMatchActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in] devObjPtr                       - pointer to device object.
* @param[in] descrPtr                        - pointer to frame descriptor.
* @param[in] matchIndex                      - index to the action table .
* @param[in] Client                          - tcam client (TTI,PCL,EPCL)
* @param[in] exactMatchProfileIdTableIndex   - keyType in case of TTI or
*                                              index for Exact match Profile-ID mapping table
*                                              in case of PCL/EPCL
* @param[out]actionDataPtr                   - (pointer to) action data
*/
GT_VOID snetFalconExactMatchActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                               matchIndex,
    IN SIP5_TCAM_CLIENT_ENT                 client,
    IN GT_U32                               exactMatchProfileIdTableIndex,
    OUT GT_U32                             *actionDataPtr
);

/**
* @internal snetFalconExactMatchParseEntry function
* @endinternal
*
* @brief   Parses Exact match entry to the struct
*
* @param[in] devObjPtr                    - (pointer to) the device object
* @param[in] exactMatchEntryPtr           - (pointer to) Exact match entry
* @param[in] entryIndex                   -  entry index (hashed index)
* @param[out] keySize                     - (pointer to) Exact match entry parsed into fields
*
*/
GT_VOID snetFalconExactMatchParseEntry
(
    IN    SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN    GT_U32                               *exactMatchEntryPtr,
    IN    GT_U32                                entryIndex,
    OUT   SNET_SIP6_EXACT_MATCH_ENTRY_INFO     *exactMatchEntryInfoPtr
);

/**
* @internal snetFalconTtiGetIndexForKeyType function
* @endinternal
*
* @brief   Gets the index used in the HW for a specific key type.
*
* @param[in] keyType                   - the key type
* @param[out] indexPtr                 - (pointer to) the index of the key type
*
*/
GT_VOID snetFalconTtiGetIndexForKeyType
(
    IN  SKERNEL_CHT3_TTI_KEY_TYPE_ENT       keyType,
    OUT GT_U32                              *indexPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetFalconExactMatchh */

