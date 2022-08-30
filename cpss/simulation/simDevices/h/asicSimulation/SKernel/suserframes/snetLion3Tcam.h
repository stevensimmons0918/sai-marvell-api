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
* @file snetLion3Tcam.h
*
* @brief This is a external API definition for SIP5 Tcam
*
* @version   9
********************************************************************************
*/
#ifndef __snetLion3Tcamh
#define __snetLion3Tcamh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* max number of parallel lookups (hitNum) */
#define SIP5_TCAM_MAX_NUM_OF_HITS_CNS               4

/* number of relevant banks in each floor */
#define SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS         12

/* number of X lines in the bank */
#define SIP5_TCAM_NUM_OF_X_LINES_IN_BANK_CNS       256

/* max size of key array (bytes) */
#define SIP5_TCAM_MAX_SIZE_OF_KEY_ARRAY_CNS        84


typedef enum{
    SIP5_TCAM_CLIENT_TTI_E      ,     /* TTI           */
    SIP5_TCAM_CLIENT_IPCL0_E    ,     /* Ingress Pcl 0 */
    SIP5_TCAM_CLIENT_IPCL1_E    ,     /* Ingress Pcl 1 */
    SIP5_TCAM_CLIENT_IPCL2_E    ,     /* Ingress Pcl 2 */
    SIP5_TCAM_CLIENT_EPCL_E     ,     /* Egress Pcl    */

    SIP5_TCAM_CLIENT_LAST_E           /* last value */
}SIP5_TCAM_CLIENT_ENT;


/* Enum values represent byte size of each key */
typedef enum{
    SIP5_TCAM_KEY_SIZE_10B_E = 1,     /* 10 bytes */
    SIP5_TCAM_KEY_SIZE_20B_E = 2,     /* 20 bytes */
    SIP5_TCAM_KEY_SIZE_30B_E = 3,     /* 30 bytes */
    SIP5_TCAM_KEY_SIZE_40B_E = 4,     /* 40 bytes */
    SIP5_TCAM_KEY_SIZE_50B_E = 5,     /* 50 bytes */
    SIP5_TCAM_KEY_SIZE_60B_E = 6,     /* 60 bytes */
    SIP5_TCAM_KEY_SIZE_80B_E = 8,     /* 80 bytes */

    SIP5_TCAM_KEY_SIZE_LAST_E         /* last value */
}SIP5_TCAM_KEY_SIZE_ENT;


/**
* @internal sip5TcamLookupByGroupId function
* @endinternal
*
* @brief   sip5 function that do lookup in tcam for given key and fill the
*         results array
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] groupId                  - tcam client group Id
* @param[in] tcamProfileId            - tcam profile Id (relevant to SIP6_10 devices)
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
*/
GT_U32 sip5TcamLookupByGroupId
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                    groupId,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
);

/**
* @internal sip5TcamLookup function
* @endinternal
*
* @brief   sip5 function that do lookup in tcam for given key and fill the
*         results array
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamClient               - tcam client
* @param[in] tcamProfileId            - tcam profile Id (relevant to SIP6_10 devices)
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
*/
GT_U32 sip5TcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
);

/**
* @internal snetLion3TcamGetKeySizeBits function
* @endinternal
*
* @brief   sip5 function that returns size bits (4 bits),
*         these bits must be added to each chunk (bits 0..3)
* @param[in] keySize                  - key size
*                                       sizeBits value
*/
GT_U32 snetLion3TcamGetKeySizeBits
(
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize
);

/**
* @internal snetSip5PclTcamLookup function
* @endinternal
*
* @brief   sip5 function that do pcl lookup in tcam
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] iPclTcamClient           - tcam client
* @param[in] u32keyArrayPtr           key array (GT_U32)
* @param[in] keyFormat                - format of the key
*/
GT_VOID snetSip5PclTcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      iPclTcamClient,
    IN  GT_U32                   *u32keyArrayPtr,
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT GT_U32                   *matchIndexPtr
);

/**
* @internal sip5TcamConvertPclKeyFormatToKeySize function
* @endinternal
*
* @brief   sip5 function that do convertation of old tcam key format to sip5 key size
*
* @param[in] keyFormat                - format of the key
*
* @param[out] sip5KeySizePtr           - sip5 key size
*                                      COMMENTS:
*/
GT_VOID sip5TcamConvertPclKeyFormatToKeySize
(
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT SIP5_TCAM_KEY_SIZE_ENT   *sip5KeySizePtr
);

/**
* @internal snetLion3TcamGetTcamGroupId function
* @endinternal
*
* @brief   sip5 function that returns tcam group id
*
* @param[in]  devObjPtr                - (pointer to) the device object
* @param[in]  tcamClient               - tcam client
* @param[out] groupIdPtr               - (pointer to) group id
*
* @retval GT_OK                    - found,
* @retval GT_NOT_FOUND             - not found
*/
GT_STATUS snetLion3TcamGetTcamGroupId
(
    IN  SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT    tcamClient,
    OUT GT_U32                 *groupIdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetLion3Tcamh */










