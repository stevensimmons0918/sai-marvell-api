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
* @file sfdb.h
*
* @brief This is a external API definition for SFDB module of SKernel.
*
* @version   9
********************************************************************************
*/
#ifndef __sfdbh
#define __sfdbh

#include <common/Utils/FrameInfo/sframeInfoAddr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum SFDB_UPDATE_MSG_TYPE
 *
 * @brief Enum for FDB update message type
*/
typedef enum{

    /** New address */
    SFDB_UPDATE_MSG_NA_E = 0,

    /** Query Address */
    SFDB_UPDATE_MSG_QA_E,

    /** Query Respond */
    SFDB_UPDATE_MSG_QR_E,

    /** Aged Out Address */
    SFDB_UPDATE_MSG_AA_E,

    /** Transplanted Address */
    SFDB_UPDATE_MSG_TA_E,

    /** FDB Upload message */
    SFDB_UPDATE_MSG_FU_E,

    /** Index Query Request and Index Query Respond */
    SFDB_UPDATE_MSG_QI_E,

    /** FDB Hash Request */
    SFDB_UPDATE_MSG_HR_E,

} SFDB_UPDATE_MSG_TYPE;

/**
* @internal sfdbMsgProcess function
* @endinternal
*
* @brief   Process FDB update message.
*
* @param[in] deviceObj                - pointer to device object.
* @param[in] fdbMsgPtr                - pointer to device object.
*/
void sfdbMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U8                 * fdbMsgPtr
);

/**
* @internal sfdbMsg2Mac function
* @endinternal
*
* @brief   Get mac address from message.
*
* @param[in] msgPtr                   - pointer to first word of message.
*
* @param[out] macAddrPtr               - pointer to the allocated mac address.
*/

void sfdbMsg2Mac
(
    IN  GT_U32            *  msgPtr,
    OUT SGT_MAC_ADDR_TYP  *  macAddrPtr
);
/**
* @internal sfdbMsg2Vid function
* @endinternal
*
* @brief   Get VID from message.
*
* @param[in] msgPtr                   - pointer to first word of message.
*/

void sfdbMsg2Vid
(
    IN  GT_U32            *  msgPtr,
    OUT GT_U16            *  vidPtr
);

/**
* @internal sfdbMacTableTriggerAction function
* @endinternal
*
* @brief   Process triggered MAC table entries.
*/
void sfdbMacTableTriggerAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * fdbMsgPtr
);

/**
* @internal sfdbMacTableAutomaticAging function
* @endinternal
*
* @brief   Process triggered MAC table entries.
*/
void sfdbMacTableAutomaticAging
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * data_PTR
);
/**
* @internal sfdbMacTableAging function
* @endinternal
*
* @brief   Age out MAC table entries.
*/
void sfdbMacTableAging(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
);

/**
* @internal sfdbMacTableUploadAction function
* @endinternal
*
* @brief   FDB upload action
*/
void sfdbMacTableUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * tblActPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sfdbh */



