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
* @file simLogInfoTypeTcam.c
*
* @brief simulation logger tcam functions
*
* @version   2
********************************************************************************
*/

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypeTcam.h>

#include <asicSimulation/SKernel/skernel.h> /* ASSERT_PTR */

#define kDisplayWidth 32

/* working buffer for pBinFill */
static GT_CHAR bufferBinaryStr[kDisplayWidth+1] = {0};

/* func for converting int to binary string */
static GT_CHAR *pBinFill
(
    IN  GT_U32 x,
    OUT GT_CHAR *so
)
{
    GT_CHAR s[kDisplayWidth+1];
    GT_32   i = kDisplayWidth;

    /* terminate string */
    s[i--] = 0;

    do
    {
        /* fill in array from right to left */
        s[i--] = (x & 1) ? '1':'0';
        x >>= 1;
    }
    while(x > 0);

    while(i >= 0)
    {
        s[i--] = '0';
    }

    sprintf(so, "%s", s);
    return so;
}

/**
* @internal simLogTcamTTNotMatch function
* @endinternal
*
* @brief   log tcam key info
*
* @param[in] devObjPtr                - device object pointer
* @param[in] ttSearchKey16Bits        - tcam key (16 bits)
* @param[in] ttSearchKey32Bits        - tcam key (32 bits)
* @param[in] xdataPtr                 - pointer to routing TCAM data X entry
* @param[in] xctrlPtr                 - pointer to routing TCAM ctrl X entry
* @param[in] ydataPtr                 - pointer to routing TCAM data mask Y entry
* @param[in] yctrlPtr                 - pointer to routing TCAM ctrl mask Y entry
*                                       None.
*/
GT_VOID simLogTcamTTNotMatch
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 ttSearchKey16Bits,
    IN GT_U32                 ttSearchKey32Bits,
    IN GT_U32                *xdataPtr,
    IN GT_U32                *ydataPtr,
    IN GT_U32                *xctrlPtr,
    IN GT_U32                *yctrlPtr
)
{
    DECLARE_FUNC_NAME(simLogTcamTTNotMatch);

    GT_U32  tempVal = 0;
    ASSERT_PTR(devObjPtr);

    /* check data entry */
    tempVal = (~ydataPtr[0] & ttSearchKey32Bits) | (~xdataPtr[0] & ~ttSearchKey32Bits);
    if( !(tempVal == 0xFFFFFFFF) )
    {
        __LOG_TCAM(("Data entry not matched. "));
        simLogTcamBitsCausedNoMatchInTheEntry(devObjPtr, tempVal);
    }

    /* check control entry */
    tempVal = (((~yctrlPtr[0] & ttSearchKey16Bits) | (~xctrlPtr[0] & ~ttSearchKey16Bits)) & 0xFFFF);
    if( !(tempVal == 0xFFFF) )
    {
        __LOG_TCAM(("Control entry not matched. "));
        simLogTcamBitsCausedNoMatchInTheEntry(devObjPtr, tempVal);
    }
}

/**
* @internal simLogTcamTTKey function
* @endinternal
*
* @brief   log tcam key info
*
* @param[in] devObjPtr                - device object pointer
* @param[in] ttSearchKey16Bits        - tcam key (16 bits)
* @param[in] ttSearchKey32Bits        - tcam key (32 bits)
*                                       None.
*/
GT_VOID simLogTcamTTKey
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 ttSearchKey16Bits,
    IN GT_U32                 ttSearchKey32Bits
)
{
    DECLARE_FUNC_NAME(simLogTcamTTKey);

    ASSERT_PTR(devObjPtr);

    __LOG_TCAM(("Key 16 bits: 0x%08X\n", ttSearchKey16Bits));
    __LOG_TCAM(("Key 32 bits: 0x%08X\n", ttSearchKey32Bits));
}

/**
* @internal simLogTcamBitsCausedNoMatchInTheEntry function
* @endinternal
*
* @brief   log tcam bits caused no match in the entry
*
* @param[in] devObjPtr                - device object pointer
* @param[in] bits                     -  caused no match in the entry
*                                       None.
*/
GT_VOID simLogTcamBitsCausedNoMatchInTheEntry
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 bits
)
{
    DECLARE_FUNC_NAME(simLogTcamBitsCausedNoMatchInTheEntry);

    ASSERT_PTR(devObjPtr);

    __LOG_TCAM(("Bits caused no match in the entry:\n"));
    __LOG_TCAM(("0x%08X = 0b%s\n", ~bits, pBinFill(~bits, bufferBinaryStr)));
}

