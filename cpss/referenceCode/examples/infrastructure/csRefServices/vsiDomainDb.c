/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* vsiDomainDb.c
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

#include "vsiDomainDb.h"
#include "../../infrastructure/csRefServices/eArchElementsDb.h"

#include <gtOs/gtOsMem.h>

#include <cpss/generic/cpssTypes.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>


/***********************************************/
/*         Feature relate code                                          */
/***********************************************/

/* Allow maximum of 100 of pair entries in the list. */
#define VPN_VSI_MAX_MLL_SIZE_CNS  100

static GT_BOOL vsiDbInitDone = GT_FALSE;

/****** Structure to store infromation for clean up **/

typedef struct{
    GT_U32    vsiId;  /* virtual Segment ID, taken from packet's vxLan/NVGRE header. */
    GT_U32    evidx;   /* Flooding evidx of the VSI domain. Device use to calculate the first entry of the L2MLL. */
}EVLAN_INFOMATION;


/****************************************************************************/
/***************************** Function implementation ****************************/
/****************************************************************************/

static GT_STATUS vsiDomainCreate
(
  IN GT_U8  devNum,
  IN GT_U32 vlanServiceId,
  IN GT_U16 evlanId,
  IN GT_U16 floodingEvidx
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_PORTS_BMP_STC                    portsMembers, portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC           vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC        lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC             mllPairEntry;
    GT_HW_DEV_NUM                         hwDevNum;
    GT_U32                                vsiDropMllEntry,maxVidxIndex;

    if(vsiDbInitDone == GT_FALSE)
       return GT_NOT_INITIALIZED;

    /* Create evlan entry for the vsi */
    osMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    osMemSet(&portsMembers, 0, sizeof(portsMembers));
    osMemSet(&portsTagging, 0, sizeof(portsTagging));
    osMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    vlanInfo.naMsgToCpuEn = GT_TRUE;
    vlanInfo.floodVidxMode   = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.floodVidx = floodingEvidx;
    vlanInfo.fidValue  = evlanId; /* Must set this value. */
    rc = cpssDxChBrgVlanEntryWrite(devNum, evlanId, &portsMembers, &portsTagging, &vlanInfo, &portsTaggingCmd);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* configure flooding evidx and set mllPointer to an empy MLL */
    /* Note that this element will always exist in the list as the first element with NULL port in the it's firstMllNode */
    rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_L2MLL_E, VPN_INDEX_DB_OPERATION_ALLOC_E, &vsiDropMllEntry);
    if(rc != GT_OK)
    {
      return rc;
    }

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if(rc != GT_OK)
    {
      return rc;
    }

    osMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));
    mllPairEntry.firstMllNode.egressInterface.type  = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum  = CPSS_NULL_PORT_NUM_CNS;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = hwDevNum;
    mllPairEntry.firstMllNode.last = GT_TRUE;
    rc = cpssDxChL2MllPairWrite(devNum, vsiDropMllEntry, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
    if(rc != GT_OK)
    {
      return rc;
    }

    /* Get the maximal VIDX value that refers to a port distribution list(VIDX)
        all eVidx above it are translated to an l2Mll entry as l2MllEntryIndex = eVidx - (maxVidxIndex+1) */
    rc = cpssDxChL2MllLookupMaxVidxIndexGet( devNum, &maxVidxIndex);
    if(rc != GT_OK)
    {
      return rc;
    }
    osMemSet(&lttEntry, 0, sizeof(lttEntry));
    lttEntry.mllPointer = vsiDropMllEntry;
    lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    rc = cpssDxChL2MllLttEntrySet(devNum,
                                      (GT_U32)(floodingEvidx - (maxVidxIndex+1)),/* Evidx to LTT translation is LTT index = Evidx - (MaxVidxIndex +1)*/
                                      &lttEntry);
    if(rc != GT_OK)
    {
      return rc;
    }

    /* Set service id */
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(devNum, evlanId, vlanServiceId);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

static GT_STATUS vsiDomainDelete
(
  IN GT_U8  devNum,
  IN GT_U16 evlanId,
  IN GT_U16 floodingEvidx
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC        lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC             mllPairEntry, mllPairEntryClean;
    GT_U32                                mllEntryIndex, mllEntryCount;
    GT_U32                                maxVidxIndex;
    EVLAN_INFOMATION                     *userInfoPtr;

    if(vsiDbInitDone == GT_FALSE)
       return GT_NOT_INITIALIZED;

   /* First clean evlan entry, stop flooding */
   rc = cpssDxChBrgVlanEntryInvalidate(devNum, evlanId);
   if(rc != GT_OK)
      return rc;

    rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EVID_E, evlanId, (void **)(&userInfoPtr));
    if(rc != GT_OK)
        return rc;

    /* Get the maximal VIDX value that refers to a port distribution list(VIDX)
       all eVidx above it are translated to an l2Mll entry as l2MllEntryIndex = eVidx - (maxVidxIndex+1) */
    rc = cpssDxChL2MllLookupMaxVidxIndexGet( devNum, &maxVidxIndex);
    if(rc != GT_OK)
      return rc;


    /* Read ltt entry from HW. */
    rc = cpssDxChL2MllLttEntryGet(devNum,
                                  floodingEvidx - (maxVidxIndex+1),
                                  &lttEntry);
    if(rc != GT_OK)
      return rc;

   /* Clean the entire link list. Stop flooding. */
    mllEntryIndex = lttEntry.mllPointer;
    mllEntryCount = 0;
    osMemSet(&mllPairEntryClean, 0, sizeof(mllPairEntryClean));
    while(mllEntryCount < VPN_VSI_MAX_MLL_SIZE_CNS)
    {
      /* Read entry, to get next pointer in the L2 link list  */
      rc = cpssDxChL2MllPairRead(devNum, mllEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
      if(rc != GT_OK)
        return rc;

      /* Clean entry in HW. Disconnect element from link list in HW. */
      rc = cpssDxChL2MllPairWrite(devNum, mllEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntryClean);
      if(rc != GT_OK)
        return rc;

      /* Return index to DB. */
      rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_L2MLL_E, VPN_INDEX_DB_OPERATION_FREE_E, &mllEntryIndex);
      if(rc != GT_OK)
        return rc;

      /* Link list cleaned and removed */
      if((mllPairEntry.firstMllNode.last == GT_TRUE) || (mllPairEntry.secondMllNode.last == GT_TRUE))
        break;

      /* Next pair */
      mllEntryIndex = mllPairEntry.nextPointer;
      mllEntryCount++;
    }

    /* Check corruption in DB. */
    if(mllEntryCount == VPN_VSI_MAX_MLL_SIZE_CNS)
      return GT_BAD_STATE;

    return rc;
}

static GT_STATUS addRemoveEvidxEport
(
  IN GT_U8                    devNum,
  IN GT_U32                   evidx,
  IN VPN_VSI_DB_OPERATION_ENT operation,
  IN GT_U32                   eport
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC        mllPairEntry, newMllPairEntry;
    GT_HW_DEV_NUM                    hwDevNum;
    GT_U32                           mllIndex, newMllIndex, mllIndexToDelete = 0xFFFFFFFF;
    GT_U32                           maxVidxIndex, mllCount;


    /* Get the maximal VIDX value that refers to a port distribution list(VIDX)
       all eVidx above it are translated to an l2Mll Ltt Entry Index.
       l2MllLttEntryIndex = eVidx - (maxVidxIndex+1) */
    rc = cpssDxChL2MllLookupMaxVidxIndexGet(devNum, &maxVidxIndex);
    if(rc != GT_OK)
    {
      return rc;
    }

    /* Get Ltt entry from HW. */
    rc = cpssDxChL2MllLttEntryGet(devNum,
                                  evidx - (maxVidxIndex+1),/* Evidx to LTT translation is LTT index = Evidx - (MaxVidxIndex +1)*/
                                  &lttEntry);
    if(rc != GT_OK)
    {
      return rc;
    }

    /* Scan the link list, and get index of last entry in the list. */
    mllIndex = lttEntry.mllPointer;
    mllCount = 0;
    while( mllCount< VPN_VSI_MAX_MLL_SIZE_CNS)
    {
      rc = cpssDxChL2MllPairRead(devNum, mllIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
      if(rc != GT_OK)
      {
        return rc;
      }

      /* Check if eport exists in mll and keep this index for remove operation */
      if((mllPairEntry.firstMllNode.egressInterface.devPort.portNum == eport) ||
         (mllPairEntry.secondMllNode.egressInterface.devPort.portNum == eport))
         mllIndexToDelete = mllIndex;

      if((mllPairEntry.firstMllNode.last == GT_TRUE) || (mllPairEntry.secondMllNode.last == GT_TRUE))
        break;

      mllIndex = mllPairEntry.nextPointer;
      mllCount++;
    }
    /* Corruption in DB. */
    if(mllCount == VPN_VSI_MAX_MLL_SIZE_CNS)
      return GT_BAD_STATE;

    switch(operation)
    {
       case VPN_VSI_DB_OPERATION_ADD_EPORT_E:
         rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
         if(rc != GT_OK)
         {
           return rc;
         }

         /* If first node is last then use second for given eoprt, else allocate new mll pair */
         if(mllPairEntry.firstMllNode.last)
         {
             osMemSet(&(mllPairEntry.secondMllNode), 0, sizeof(mllPairEntry.secondMllNode));
             mllPairEntry.secondMllNode.egressInterface.type             = CPSS_INTERFACE_PORT_E;
             mllPairEntry.secondMllNode.egressInterface.devPort.portNum  = eport;
             mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = hwDevNum;
             mllPairEntry.secondMllNode.last = GT_TRUE;
             mllPairEntry.firstMllNode.last = GT_FALSE;
             rc = cpssDxChL2MllPairWrite(devNum, mllIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
             if(rc != GT_OK)
             {
               return rc;
             }
         }
         else /* Second is last, therefore allocate new mll, and point to it. */
         {
             rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_L2MLL_E, VPN_INDEX_DB_OPERATION_ALLOC_E, &newMllIndex);
             if(rc != GT_OK)
             {
               return rc;
             }

             /* Write the new MLL pair */
             osMemSet(&newMllPairEntry, 0, sizeof(newMllPairEntry));
             newMllPairEntry.firstMllNode.egressInterface.type             = CPSS_INTERFACE_PORT_E;
             newMllPairEntry.firstMllNode.egressInterface.devPort.portNum  = eport;
             newMllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = hwDevNum;
             newMllPairEntry.firstMllNode.last = GT_TRUE;
             rc = cpssDxChL2MllPairWrite(devNum, newMllIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &newMllPairEntry);
             if(rc != GT_OK)
             {
               return rc;
             }

             /* Update last mll pair in the list to point the new one.  */
             mllPairEntry.secondMllNode.last = GT_FALSE;
             mllPairEntry.nextPointer = newMllIndex;
             rc = cpssDxChL2MllPairWrite(devNum, mllIndex, CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E, &mllPairEntry);
             if(rc != GT_OK)
             {
               return rc;
             }
         }
       break;

       case VPN_VSI_DB_OPERATION_DEL_EPORT_E:
          return GT_NOT_IMPLEMENTED;

          if(mllIndexToDelete == 0xFFFFFFFF)
             return GT_BAD_PARAM;
       break;

       default:
         return GT_BAD_PARAM;
    }

    return rc;
}

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
*
*/
GT_STATUS csRefInfraVsiDomainManage
(
  IN GT_U8                    devNum,
  IN GT_U16                   assignedEvlanId,  /* eVlan entry representing the VSI. */
  IN VPN_VSI_DB_OPERATION_ENT op,
  IN GT_U32                   vsiId,
  IN GT_PORT_NUM              assignedEportNum /* ePort to add to vsi domain or remove from vsiDomain. */
)
{
    GT_STATUS         rc = GT_OK;
    GT_U32            floodingEvidx;
    EVLAN_INFOMATION *userInfoPtr;

    if(vsiDbInitDone == GT_FALSE)
       return GT_NOT_INITIALIZED;

    switch(op)
    {
      case VPN_VSI_DB_OPERATION_CREATE_E:

        /* If eVlan allocated and userInfo not NULL, then VSI created. Return GT_OK.
           If eVlan not allocated reurn error, else create VSI domain.*/
        rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EVID_E, assignedEvlanId, (void **)(&userInfoPtr));
        if((rc == GT_OK) && (userInfoPtr != NULL))
          return GT_OK;
        else if(rc != GT_OK)
          return rc;

        /* Create empty VSI domain entity */
        /* allocate flooding-evidx. Allocate evidx that can be mapped to l2mll */
        rc  = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVIDX_E, VPN_INDEX_DB_OPERATION_ALLOC_E, &floodingEvidx);
        if(rc != GT_OK)
        {
          return rc;
        }

        /* Create VSI with evid and floodingEvidx */
        rc = vsiDomainCreate(devNum, vsiId,  assignedEvlanId, floodingEvidx);
        if(rc != GT_OK)
        {
          return rc;
        }

        /* Keep VSI-id and floodingEvidx in the user info of the assigned eVlanId. */
        userInfoPtr = osMalloc(sizeof(EVLAN_INFOMATION));
        if(userInfoPtr == NULL)
          return GT_OUT_OF_CPU_MEM;

        userInfoPtr->vsiId = vsiId;
        userInfoPtr->evidx = floodingEvidx;
        rc = csRefInfraIndexDbUserInfoSet(VPN_EARCH_DB_TYPE_EVID_E,
                                          assignedEvlanId,(void *)userInfoPtr);
        if(rc != GT_OK)
          return rc;
      break;

      case VPN_VSI_DB_OPERATION_DELETE_E:

        /* If eVlan allocated and userInfo not NULL, then VSI created. Return GT_OK.
           If eVlan not allocated reurn error, else create VSI domain.*/
        rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EVID_E, assignedEvlanId, (void **)(&userInfoPtr));
        if((rc != GT_OK) || (userInfoPtr == NULL))
          return GT_BAD_STATE; /* Corruption in DB. */

        /* Delete VSI with evid and floodingEvidx (delete L2-MLL entirely).  */
        rc = vsiDomainDelete(devNum, assignedEvlanId, userInfoPtr->evidx);
        if(rc != GT_OK)
          return rc;

        /* Get flooding-evidx of the assignedEvlanId. */
        rc  = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVIDX_E, VPN_INDEX_DB_OPERATION_FREE_E, &(userInfoPtr->evidx));
        if(rc != GT_OK)
          return rc;

        /* Free user info. */
        osFree(userInfoPtr);
      break;

      case VPN_VSI_DB_OPERATION_ADD_EPORT_E: /* Add eport to VSI */

        /* Get user info of eVlan. */
        userInfoPtr = NULL;
        rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EVID_E, assignedEvlanId, (void **)(&userInfoPtr));
        if((rc != GT_OK) || (userInfoPtr == NULL))
          return GT_NO_SUCH;

        rc = addRemoveEvidxEport(devNum, userInfoPtr->evidx,VPN_VSI_DB_OPERATION_ADD_EPORT_E, assignedEportNum);
        if(rc != GT_OK)
        {
          return rc;
        }
      break;

      case VPN_VSI_DB_OPERATION_DEL_EPORT_E:  /* Remove eport from VSI */

        /* Get user info of eVlan. */
        userInfoPtr = NULL;
        rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EVID_E, assignedEvlanId, (void **)(&userInfoPtr));
        if((rc != GT_OK) || (userInfoPtr == NULL))
          return GT_BAD_PARAM;

        rc = addRemoveEvidxEport(devNum, userInfoPtr->evidx, VPN_VSI_DB_OPERATION_DEL_EPORT_E, assignedEportNum);
        if(rc != GT_OK)
        {
          return rc;
        }
      break;

      default:
        return GT_BAD_PARAM;
    }

    return rc;
}


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
)
{
  GT_UNUSED_PARAM(devNum);
  if(initialize == vsiDbInitDone)
    return GT_OK;

  vsiDbInitDone = initialize;

  return GT_OK;
}
