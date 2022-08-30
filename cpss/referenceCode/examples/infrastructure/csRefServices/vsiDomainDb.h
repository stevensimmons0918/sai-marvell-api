/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* vsiDomainDb.h
*
* DESCRIPTION:
*  similar to file prvTgfBrgVplsBasicTest.c but with next changes:
*       1. supports 3 modes :
*           a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest.c
*           b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
*           c. 'tag mode' + 'add double vlan tag'
*
*       2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
*           tunnels is without vlan tags.
*       3. in TAG mode the ethernet packets come with one tag but considered untagged
*           when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __vsiDomainDb_h
#define __vsiDomainDb_h


#include <cpss/generic/cpssTypes.h>

/***********************************************/
/*         Definitions for entry management purposes only */
/***********************************************/

typedef enum
{
   VPN_VSI_DB_OPERATION_CREATE_E,    /* Create new VSI instance */
   VPN_VSI_DB_OPERATION_DELETE_E,    /* Delete VSI instance */
   VPN_VSI_DB_OPERATION_ADD_EPORT_E, /* Add ePort to VSI  */
   VPN_VSI_DB_OPERATION_DEL_EPORT_E, /* Delete ePort from VSI */
   VPN_VSI_DB_OPERATION_SET_E,       /* Set element info */
   VPN_VSI_DB_OPERATION_GET_E        /* Get element info */
}VPN_VSI_DB_OPERATION_ENT;


/***********************************************/
/*         Feature relate code                                          */
/***********************************************/


/**
* @internal csRefInfraVsiDomainManage function
* @endinternal
*
* @brief   This API used to manage the VSI in the device.
*            Create and remove VSI domain : assign eVlan to represent the domain.
*            Configure empty flooding eVIDX for the domain.
*            Add and remove port from VSI domain.
*            Allocate an MLL entry to add ePort to the flooding domain of the VSI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum    - device number
* @param[in] assignedEvlanId   - port number
* @param[in] op           - packet assigned vlan id.
* @param[in] vsiId   - packet assigned vlan id.
* @param[in] assignedEportNum   - Packet's source mac address
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS csRefInfraVsiDomainManage
(
  IN GT_U8                    devNum,
  IN GT_U16                   assignedEvlanId,  /* eVlan entry representing the VSI. */
  IN VPN_VSI_DB_OPERATION_ENT op,
  IN GT_U32                   vsiId,
  IN GT_PORT_NUM              assignedEportNum /* ePort to add to vsi domain or remove from vsiDomain. */
);


/**
* @internal csRefInfraVsiDbInit function
* @endinternal
*
* @brief   Initialize or Reset VSI manage DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum    - device number
* @param[in] initialize    - init or reset vsi DB.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*
*/
GT_STATUS csRefInfraVsiDbInit
(
  IN  GT_U8    devNum,
  IN  GT_BOOL  initialize
);


#endif
