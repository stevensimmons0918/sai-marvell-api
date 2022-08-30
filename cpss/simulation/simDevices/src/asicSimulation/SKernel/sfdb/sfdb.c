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
* @file sfdb.c
*
* @brief This is a SFDB module of SKernel.
*
* @version   16
********************************************************************************
*/

#include <os/simTypes.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/sfdb/sfdbSoho.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>

/**
* @internal sfdbMsgProcess function
* @endinternal
*
* @brief   Process FDB update message.
*/
void sfdbMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * fdbMsgPtr
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devFdbMsgProcFuncPtr == NULL)
        return ;

    deviceObjPtr->devFdbMsgProcFuncPtr(deviceObjPtr, fdbMsgPtr);
}
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
)
{
    macAddrPtr->bytes[5] = (GT_U8)((msgPtr[0] >> 16) & 0xff);
    macAddrPtr->bytes[4] = (GT_U8)((msgPtr[0] >> 24) & 0xff);
    macAddrPtr->bytes[3] = (GT_U8)(msgPtr[1] & 0xff);
    macAddrPtr->bytes[2] = (GT_U8)((msgPtr[1] >> 8) & 0xff);
    macAddrPtr->bytes[1] = (GT_U8)((msgPtr[1] >> 16) & 0xff);
    macAddrPtr->bytes[0] = (GT_U8)((msgPtr[1] >> 24) & 0xff);
}

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
)
{
    *vidPtr = (GT_U16)(msgPtr[2] & 0xfff);
}


/**
* @internal sfdbMacTableAging function
* @endinternal
*
* @brief   Age out MAC table entries.
*/
void sfdbMacTableAging(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    if (deviceObjPtr->devMacTblAgingProcFuncPtr != NULL)
       deviceObjPtr->devMacTblAgingProcFuncPtr(deviceObjPtr);
}


/**
* @internal sfdbMacTableTriggerAction function
* @endinternal
*
* @brief   Process triggered MAC table entries.
*/
void sfdbMacTableTriggerAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * tblActPtr
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devMacTblTrigActFuncPtr == NULL)
        return ;

    deviceObjPtr->devMacTblTrigActFuncPtr(deviceObjPtr, tblActPtr);
}

/**
* @internal sfdbMacTableUploadAction function
* @endinternal
*
* @brief   Process triggered MAC table entries.
*/
void sfdbMacTableUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * tblActPtr
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devMacTblUploadProcFuncPtr == NULL)
        return ;

    deviceObjPtr->devMacTblUploadProcFuncPtr(deviceObjPtr, tblActPtr);
}

/**
* @internal sfdbMacTableAutomaticAging function
* @endinternal
*
* @brief   Process automatic aging for the fdb table.
*/
void sfdbMacTableAutomaticAging
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * data_PTR
)
{
    ASSERT_PTR(deviceObjPtr);

    if (deviceObjPtr->devMacEntriesAutoAgingFuncPtr == NULL)
        return ;

    deviceObjPtr->devMacEntriesAutoAgingFuncPtr(deviceObjPtr, data_PTR);
}


