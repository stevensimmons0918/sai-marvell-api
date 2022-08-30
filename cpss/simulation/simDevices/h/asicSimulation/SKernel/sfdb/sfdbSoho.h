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
* @file sfdbSoho.h
*
* @brief This is a external API definition for SFDB module of SKernel Soho.
*
* @version   3
********************************************************************************
*/
#ifndef __sfdbsoho
#define __sfdbsoho

#include <asicSimulation/SKernel/suserframes/snetSoho.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal sfdbSohoMsgProcess function
* @endinternal
*
* @brief   Process FDB update message.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbMsgPtr                - pointer to device object.
*/
GT_VOID sfdbSohoMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * fdbMsgPtr
);

/**
* @internal sfdbSohoAtuEntryAddress function
* @endinternal
*
* @brief   Get ATU database entry in the SRAM
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macAddrPtr               - pointer to 48-bit MAC address
* @param[in] dbNum                    - Vlan database number
*
* @retval GT_OK                    - ATU database entry found
* @retval GT_NOT_FOUND             - ATU database entry not found
*/
GT_STATUS sfdbSohoAtuEntryAddress
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_U8 * macAddrPtr,
    IN GT_U32 dbNum,
    OUT GT_U32 * address
);

/**
* @internal sfdbSohoFreeBinGet function
* @endinternal
*
* @brief   Check if all bins in the bucket are static
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macAddrPtr               - pointer to 48-bit MAC address
* @param[in] dbNum                    - vlan database number
* @param[in] regAddrPtr               - pointer to free bin address
*
* @retval GT_OK                    - all entries static
* @retval GT_NOT_FOUND             - not all entries static
*/
GT_STATUS sfdbSohoFreeBinGet
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_U8 * macAddrPtr,
    IN GT_U32 dbNum,
    OUT GT_U32 * regAddrPtr
);

/**
* @internal sfdbSohoMacTableAging function
* @endinternal
*
* @brief   Age out MAC table entries.
*/

void sfdbSohoMacTableAging
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sfdbsoho */



