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
* @file snetCheetah3MacLookup.h
*
* @brief deceleration next for cheetah3(plus) :
* 1. Ingress MAC SA Lookup -
* The Ingress MAC SA is performed in a hash based lookup into
* a dedicated 32K MAC Address table.
* 2. Egress MAC DA Lookup .
*
*
*
*
* Note : This lookup is independent of the Bridge engine FDB lookup.
*
*
* @version   1
********************************************************************************
*/
#ifndef __snetCheetah3MacLookuph
#define __snetCheetah3MacLookuph


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetCht3IngressMacSa function
* @endinternal
*
* @brief   Ingress MAC SA LookUp ,VLAN assignment , RSSI
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
GT_VOID snetCht3IngressMacSa(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetCht3EgressMacDa function
* @endinternal
*
* @brief   Egress MAC DA LookUp
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
GT_VOID snetCht3EgressMacDa(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetah3MacLookuph */

