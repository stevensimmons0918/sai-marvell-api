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
* @file snetFalconTcam.h
*
* @brief This is a external API definition for SIP6 Tcam
*
* @version   1
********************************************************************************
*/
#ifndef __snetFalconTcamh
#define __snetFalconTcamh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetFalconTcamLookup function
* @endinternal
*
* @brief   do lookup in tcam for given key
*          and fill the results array
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamClient               - tcam client
* @param[in] emProfileAccessIndex     - index to the EM Profile table
* @param[in] tcamProfileId            - TCAM Profile id
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
* @param[out] resultArr               - the index hits
* @param[out] resultHitDoneInTcamArr  - the hits location.
*                                       GT_TRUE for TCAM hits
*                                       GT_FALSE for non TCAM
*                                       hits
*/
GT_U32 snetFalconTcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                    emProfileAccessIndex,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS],
    OUT GT_BOOL                   resultHitDoneInTcamArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
);

/**
* @internal snetFalconTcamPclLookup function
* @endinternal
*
* @brief   do pcl lookup in tcam
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] iPclTcamClient           - tcam client
* @param[in] keyIndex                 - index to the PCL Profile
*                                       table
* @param[in] u32keyArrayPtr           - key array (GT_U32)
* @param[in] keyFormat                - format of the key
*
* @param[out] matchIndexPtr           - the index hits
* @param[out] matchDoneInTcamPtr      - the hits location.
*                                       GT_TRUE for TCAM hits
*                                       GT_FALSE for non TCAM
*                                       hits
*/
GT_VOID snetFalconTcamPclLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      iPclTcamClient,
    IN  GT_U32                    keyIndex,
    IN  GT_U32                   *u32keyArrayPtr,
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT GT_U32                   *matchIndexPtr,
    OUT GT_BOOL                  *matchDoneInTcamPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetLion3Tcamh */

