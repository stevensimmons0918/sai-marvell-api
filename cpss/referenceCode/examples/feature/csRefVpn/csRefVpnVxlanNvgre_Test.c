/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csRefVpnVxlanNvgre_Test.c
*
* DESCRIPTION:
*  This file contains test scenarios for the vxLan, using the APIs in vpnVxlanNvgre.h
*
*         Test-1
*           1.1. configure UNI interface on physical port
*           1.2. configure NNI on different physical port
*           1.3. Send Ethernet packet, from TG to UNI port
*                 Verify packet egress NNI port, with IPv6 vxLAN encapsulation
*           1.4. Send IPv6 vxLan packet encapsulating Ethernet packet, from TG to NNI port.
*                 Verify packet egress UNI port, after removing vxLAN encapsulation.
*
*         Test-2
*           2.1. Same UNI NNI configuration as in test 2..
*           2.2. Addtional configuration for triggering router:
*                             - By default LPM-DB created, VRF created and routing enabled globally.
*                             - Enable routing on UNI port, NNI port and VSI.
*                             - Configure MAC2ME
*           2.3. Send Ethernet packet, from TG to UNI port
*                 Verify packet egress NNI port, with IPv6 vxLAN encapsulation
*           2.4. Send IPv6 vxLan packet encapsulating Ethernet packet, from TG to NNI port.
*                 Verify packet egress UNI port, after removing vxLAN encapsulation.
*
*         Test-3....
*         Test-4....
*         Test-5....
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/


#include "csRefVpnVxlanNvgre_Test.h"
#include "csRefVpnVxlanNvgre.h"
#include "../../infrastructure/csRefSysConfig/csRefGlobalSystemInfo.h"
#include "../../infrastructure/csRefServices/eArchElementsDb.h"
#include "../../infrastructure/csRefServices/vsiDomainDb.h"

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>

/******** External appDemo API ***********/

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

/***********************************************/
/*         vxLAN related DB initialization  */
/***********************************************/
#define VPN_TS_FIRST_ENTRY_CNS    0
#define VPN_TS_NUM_OF_ENTRIES_CNS 20
#define VPN_TS_ENTRY_SIZE_CNS     2 /* Num of lines in entry. */

#define VPN_TTI_NUM_OF_ENTRIES_CNS 50
#define VPN_TTI_ENTRY_SIZE_CNS   3 /* Num of lines in entry. */
#define VPN_PCL_NUM_OF_ENTRIES_CNS 30
#define VPN_PCL_ENTRY_SIZE_CNS   3 /* Num of lines in entry. */


#define VPN_EVID_FIRST_ENTRY_CNS      4096
#define VPN_EVID_NUM_OF_ENTRIES_CNS   20

#define VPN_EVIDX_FIRST_ENTRY_CNS     VPN_EVID_FIRST_ENTRY_CNS
#define VPN_EVIDX_NUM_OF_ENTRIES_CNS  VPN_EVID_NUM_OF_ENTRIES_CNS

#define VPN_EPORT_FIRST_ENTRY_CNS     3000
#define VPN_EPORT_NUM_OF_ENTRIES_CNS  VPN_EVID_NUM_OF_ENTRIES_CNS

#define VPN_L2MLL_FIRST_ENTRY_CNS     28
#define VPN_L2MLL_NUM_OF_ENTRIES_CNS  40


static GT_BOOL prvVxLanRelateDbInitDone =GT_FALSE;

static GT_STATUS prvVxLanRelateDBInit
(
  IN  GT_U8   devNum
)
{
  GT_STATUS rc=GT_OK;
  GT_U32    tcamEntryBase;

  if(prvVxLanRelateDbInitDone == GT_TRUE)
    return GT_OK;

  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_TS_E,  VPN_TS_ENTRY_SIZE_CNS ,VPN_TS_FIRST_ENTRY_CNS, VPN_TS_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  /* This value configured in cpssInitSystem flow, function prvTcamLibInit*/
  tcamEntryBase = appDemoDxChTcamTtiBaseIndexGet(devNum, 0); /* get base index for TTI first hit lookup. */
  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_TTI_E, VPN_TTI_ENTRY_SIZE_CNS, tcamEntryBase, VPN_TTI_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  tcamEntryBase = appDemoDxChTcamIpclBaseIndexGet(devNum, 0); /* get base index for PCL first lookup. */
  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_PCL_E, VPN_PCL_ENTRY_SIZE_CNS, tcamEntryBase, VPN_PCL_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_EVID_E, 1, VPN_EVID_FIRST_ENTRY_CNS, VPN_EVID_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_EVIDX_E, 1, VPN_EVIDX_FIRST_ENTRY_CNS, VPN_EVIDX_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_EPORT_E, 1, VPN_EPORT_FIRST_ENTRY_CNS, VPN_EVID_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbInit(VPN_EARCH_DB_TYPE_L2MLL_E, 1, VPN_L2MLL_FIRST_ENTRY_CNS, VPN_L2MLL_NUM_OF_ENTRIES_CNS);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraVsiDbInit(devNum,GT_TRUE);
  if(rc != GT_OK)
    return rc;

  prvVxLanRelateDbInitDone = GT_TRUE;
  return GT_OK;
}

static GT_STATUS prvVxLanRelateDBReset
(
  IN  GT_U8   devNum
)
{
  GT_STATUS rc=GT_OK;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TS_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  /* This value configured in cpssInitSystem flow, function prvTcamLibInit*/
  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TTI_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_PCL_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVID_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVIDX_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EPORT_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_L2MLL_E, VPN_INDEX_DB_OPERATION_RESET_E, NULL);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraVsiDbInit(devNum, GT_FALSE);
  if(rc != GT_OK)
    return rc;

  prvVxLanRelateDbInitDone = GT_FALSE;
  return GT_OK;
}

/***********************************************/
/*         END of vxLAN related DB initialization  */
/***********************************************/



/***********************************************/
/*         local service function and variable definition  */
/***********************************************/

/* Keep infomation for clean up */
#define CSREF_VPN_MAX_NUMBER_OF_EPORTS_CNS  30
static GT_PORT_NUM  vpnAassignEportsArray[CSREF_VPN_MAX_NUMBER_OF_EPORTS_CNS], assignEportIndex = 0;

#define CSREF_VPN_MAX_NUMBER_OF_EVLANS_CNS  5
static GT_U16  vpnAssingedEvlanArray[CSREF_VPN_MAX_NUMBER_OF_EVLANS_CNS], assignEvlanIndex = 0;

static GT_U32  prvDomainVsiId = 100;
static CSREF_TEST_UNI_ATTRIBUTE_STC  *prvUniInterfacesArray;
static CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC *prvNniInterfacesArray;
static GT_ETHERADDR prvMac2meAddress = {{0x0,0x0,0x10,0x10,0x10,0x10}};



/* Configure eVlan to represent the VSI domain, in the device. */
static GT_STATUS prvVxLanIPv6VpnVsiCreate
(
  IN GT_U8        devNum,
  IN GT_U32       serviceId,
  OUT GT_U32     *eVlanAssigned /* Representing the VSI */
)
{
  GT_STATUS rc=GT_OK;
  GT_U32          eVlanId, dummy = 0xFFFFFF;

  if(assignEvlanIndex >= CSREF_VPN_MAX_NUMBER_OF_EVLANS_CNS)
    return GT_NO_RESOURCE;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVID_E, VPN_INDEX_DB_OPERATION_ALLOC_E, &eVlanId);
  if(rc != GT_OK)
    return rc;

  /* Create VSI domain with allocated eVlanAssigned. */
  rc = csRefInfraVsiDomainManage(devNum, eVlanId, VPN_VSI_DB_OPERATION_CREATE_E, serviceId, dummy);
  if(rc != GT_OK)
    return rc;

  vpnAssingedEvlanArray[assignEvlanIndex++] = eVlanId;
  *eVlanAssigned = eVlanId;

  return GT_OK;
}


/* Configure UNI or NNI interface flow.*/
static GT_STATUS prvVxLanIPv6VpnInterfaceCreate
(
  GT_U8         devNum,
  GT_U32        configEntryIndex, /* Entry index in local DB */
  GT_U32        eVlanAssigned, /* Representing the VSI */
  GT_U32        serviceId,
  GT_BOOL       isUniInterface,
  GT_BOOL       innerPacketTagged
)
{
  GT_STATUS     rc;
  GT_PORT_NUM   portNum;
  GT_U32        dummy=0xFFFFFF;
  GT_PORT_NUM   assignEportNum;

  if(assignEportIndex >= CSREF_VPN_MAX_NUMBER_OF_EPORTS_CNS)
    return GT_NO_RESOURCE;

  /* Allocate ePort for the interface */
  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EPORT_E, VPN_INDEX_DB_OPERATION_ALLOC_E, &assignEportNum);
  if(rc != GT_OK)
  {
    return rc;
  }

  /* Create interface */
  if(isUniInterface == GT_TRUE)
  {
      GT_U32        vlanId        = prvUniInterfacesArray[configEntryIndex].vlanId;
      GT_ETHERADDR  srcMacAddress = prvUniInterfacesArray[configEntryIndex].srcMacAddress;
      portNum = prvUniInterfacesArray[configEntryIndex].portNum;

      rc = csRefVpnUniInterfaceCreate(devNum, portNum, vlanId, &srcMacAddress, serviceId, eVlanAssigned, assignEportNum);
      if(rc != GT_OK)
      {
        return rc;
      }
  }
  else /* NNI interface */
  {
      GT_U8           *localIp       = prvNniInterfacesArray[configEntryIndex].localIpAddress;
      GT_U8           *remoteIp      = prvNniInterfacesArray[configEntryIndex].remoteIpAddress;
      GT_ETHERADDR     nhMacAddress = prvNniInterfacesArray[configEntryIndex].nhMacAddress;
      portNum =  prvNniInterfacesArray[configEntryIndex].portNum;

      rc = csRefVpnVxlanNniInterfaceCreate(devNum, portNum, localIp, remoteIp,
                                           CPSS_IP_PROTOCOL_IPV6_E, serviceId,
                                           eVlanAssigned, assignEportNum,
                                           innerPacketTagged, &nhMacAddress);
      if(rc != GT_OK)
      {
        csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EPORT_E, VPN_INDEX_DB_OPERATION_FREE_E, &assignEportNum);
        return rc;
      }
  }

  /* Add created interface to VSI domain. */
  rc = csRefInfraVsiDomainManage(devNum, eVlanAssigned, VPN_VSI_DB_OPERATION_ADD_EPORT_E, dummy, assignEportNum);
  if(rc != GT_OK)
    return rc;

  vpnAassignEportsArray[assignEportIndex++] = assignEportNum;

  return GT_OK;
}


/****** Phase-1 UNI/NNI configuration information *******/
/* Configure 1 UNI and 1 NNI .*/

#define NEXT_HOP_MAC_ADDRESS_NNI_IF0 0x00,0xB0,0x00,0x20,0x20,0x00
#define NEXT_HOP_MAC_ADDRESS_NNI_IF1 0x00,0xC0,0x00,0x30,0x30,0x00

static CSREF_TEST_UNI_ATTRIBUTE_STC phase1_UNI_array[] =
{
   {0xFFFF, 100, {{0x0,0x0,0x0,0x0,0x0,0x0}}} /* all 0 MAC address means ignore SA */
  ,{0xFFFF, 0, {{0x0,0x0,0x0,0x0,0x0,0x0}}}
};

static CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC phase1_NNI_array[] =
{
   { 0xFFFF,
     {0,0,0x11,0x11,0,0,0x22,0x22,0,0,0xAB,0xCD,0,0,0xDD,0xDD},
     {0,0,0x11,0x11,0,0,0x22,0x22,0,0,0x11,0x11,0,0,0x22,0x22},
     {{NEXT_HOP_MAC_ADDRESS_NNI_IF0}}
   }
  ,{ 0xFFFF,
     {0,0,0x11,0x11,0,0,0x22,0x22,0,0,0xAB,0xCD,0,0,0xDD,0xDD},
     {0,0,0x11,0x11,0,0,0x22,0x22,0,0,0x88,0x88,0,0,0xAA,0xAA},
     {{NEXT_HOP_MAC_ADDRESS_NNI_IF1}}
   }
  ,{ 0xFFFF,
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
      {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
     {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}
   }
};

static GT_STATUS vxLanIPv6Phase_1_Sequence
(
  GT_U8         devNum,
  GT_U32        serviceId
)
{
  GT_STATUS    rc=GT_OK;
  GT_U32       eVlanAssigned;

  /* Point to relevant information array */
  prvUniInterfacesArray = phase1_UNI_array;
  prvNniInterfacesArray = phase1_NNI_array;
/* TBD
  if((csRefSystemInfo.deviceInfo.numberOfPorts == 0) ||
     (csRefSystemInfo.deviceInfo.numberOfPorts >= CSREF_MAX_NUMER_OF_PORTS_CNS))
    return GT_BAD_SIZE;
*/
  /* Set actual port number in array. */
  if(phase1_UNI_array[0].portNum == 0xFFFF)
    phase1_UNI_array[0].portNum = csRefSystemInfo.deviceInfo.portNumberArray[0];
  if(phase1_NNI_array[0].portNum == 0xFFFF)
    phase1_NNI_array[0].portNum = csRefSystemInfo.deviceInfo.portNumberArray[1];
  if(phase1_NNI_array[1].portNum == 0xFFFF)
    phase1_NNI_array[1].portNum = csRefSystemInfo.deviceInfo.portNumberArray[2];

  /* Create VSI domain, with empty ePort list. */
  rc = prvVxLanIPv6VpnVsiCreate(devNum, serviceId, &eVlanAssigned);
  if(rc != GT_OK)
    return rc;

  /* Create UNI eport and add it to VSI domain. */
  rc = prvVxLanIPv6VpnInterfaceCreate(devNum, 0, eVlanAssigned, serviceId, GT_TRUE, GT_FALSE);
  if(rc != GT_OK)
    return rc;

  /* Create NNI eport and add it to VSI domain. */
  rc = prvVxLanIPv6VpnInterfaceCreate(devNum, 0, eVlanAssigned, serviceId, GT_FALSE, GT_FALSE);
  if(rc != GT_OK)
    return rc;


  /* Create NNI eport and add it to VSI domain. */
  rc = prvVxLanIPv6VpnInterfaceCreate(devNum, 1, eVlanAssigned, serviceId, GT_FALSE, GT_TRUE);

  /* Set MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefVpnMac2MeSet(devNum, &prvMac2meAddress);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}


/****** Phase-2 UNI/NNI configuration information *******/
/* Add 1 UNI and 1 NNI .*/

static CSREF_TEST_UNI_ATTRIBUTE_STC phase2_UNI_array[] =
{
   {11, 110, {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
   {0xFFFF, 0xFFFF, {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}}
};

static CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC phase2_NNI_array[] =
{
   {12, {0xfc,0,2,0,2,2,2,2 ,2,2,2,2,2,2,2,1},
        {0xfc,0,2,0,2,1,1,1 ,1,1,1,1,1,1,1,1},
        {{0x0, 0x0, 0x0, 0xB0, 0xB0, 0x0}}}
};

static GT_STATUS vxLanIPv6Phase_2_Sequence
(
  GT_U8         devNum,
  GT_U32        serviceId
)
{
  GT_STATUS rc=GT_OK;
  GT_U32       eVlanAssigned;

  /* Point to relevant array of information */
  prvUniInterfacesArray = phase2_UNI_array;
  prvNniInterfacesArray = phase2_NNI_array;


  /* Create VSI domain, with empty ePort list. */
  rc = prvVxLanIPv6VpnVsiCreate(devNum, serviceId, &eVlanAssigned);
  if(rc != GT_OK)
    return rc;

  /* Create UNI eport and add it to VSI domain. */
  rc = prvVxLanIPv6VpnInterfaceCreate(devNum, 0, eVlanAssigned, serviceId, GT_TRUE, 0);
  if(rc != GT_OK)
    return rc;

  /* Set MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefVpnMac2MeSet(devNum, &prvMac2meAddress);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}



/**
* @internal csRefVpnVxLanTestConfigure function
* @endinternal
*
* @brief   This test function uses the Vxlan API to configure the device for the various of test scenario.
*            Phase1: Configure one NNI and one UNI in one segment L2-VPN
*            Phase2: Add NNI and UNI and check flooding behavior.
*            Phase3: Same configuration in phase1, different segment id and different port.
*                        Show that routing/bridging is only within the segment
*            Phase4: Routing of vxLAN passenger packet after TT.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] phaseNumber           - test phase number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfig
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   phaseNumber
)
{
  GT_STATUS rc=GT_OK;

  /* eArch element DB initializatin. */
  rc = prvVxLanRelateDBInit(devNum);
  if(rc != GT_OK)
    return rc;

  /* Per phase single function for configuration*/
  if(phaseNumber == 1)
  {
    rc = vxLanIPv6Phase_1_Sequence(devNum, prvDomainVsiId);
  }
  else if(phaseNumber == 2)
  {
    rc = vxLanIPv6Phase_2_Sequence(devNum, prvDomainVsiId);
  }
  else
    return GT_BAD_PARAM;

  return rc;
}

/**
* @internal csRefVpnVxLanConfigClear function
* @endinternal
*
* @brief   This function clean all configuration done by csRefVpnVxLanTestConfigure
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum     - device number
* @param[in] resetDb    - Reset DB or not
*                         For robustness purposes
*                         don't reset DB between phases
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfigClear
(
  IN GT_U8         devNum,
  IN GT_BOOL       resetDb 
)
{
  GT_STATUS rc = GT_OK;

  /* Clear MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefVpnMac2MeDelete(devNum);
  if(rc != GT_OK)
    return rc;

  /* Remove reference to ePorts, from VSI domain L2-MLL, and delete the eVlan.  */
  for( ; assignEvlanIndex > 0 ; assignEvlanIndex--)
  {
    rc = csRefInfraVsiDomainManage(devNum, vpnAssingedEvlanArray[assignEvlanIndex-1], VPN_VSI_DB_OPERATION_DELETE_E,
                                        0xFFFFFF/*dummy*/, 0xFFFFFF/*dummy*/);
    if(rc != GT_OK)
       return rc;

    rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EVID_E, VPN_INDEX_DB_OPERATION_FREE_E,
                                (GT_U32*)(&vpnAssingedEvlanArray[assignEvlanIndex-1]));
    if(rc != GT_OK)
       return rc;
  }

  /* Remove UNI/NNI interface configuration. */
  for( ; assignEportIndex>0  ; assignEportIndex--)
  {
    rc = csRefVpnInterfaceDelete(devNum, vpnAassignEportsArray[assignEportIndex-1]);
    if(rc != GT_OK)
      return rc;

    rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_EPORT_E, VPN_INDEX_DB_OPERATION_FREE_E,
                                &(vpnAassignEportsArray[assignEportIndex-1]));
    if(rc != GT_OK)
      return rc;
  }

  /* Must be at the end. */
  /* eArch element DB reset. */
  if(resetDb == GT_TRUE)
    rc = prvVxLanRelateDBReset(devNum);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}

/**
* @internal csRefVpnVxLanTestInfoSet function
* @endinternal
*
* @brief   This test function override the default vxLan setting.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                        - device number
* @param[in] domainVsiId                   - domain VSI id
* @param[in] mac2meAddressPtr              - pointer to man2me
* @param[in] uniAttributeOvveridePtr       - pointer to first elements in uni attribute array
* @param[in] numberOfUniAttributeEelements - number of elements in uni array
* @param[in] nniAttributeOvveridePtr       - pointer to first elements in nni attribute array
* @param[in] numberOfNniAttributeEelements - number of elements in nni array 
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter 
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanTestInfoSet
(
  IN GT_U8                               devNum,
  IN GT_U32                              domainVsiId,
  IN GT_ETHERADDR                       *mac2meAddressPtr,
  IN CSREF_TEST_UNI_ATTRIBUTE_STC       *uniAttributeOvveridePtr,
  IN GT_U32                              numberOfUniAttributeEelements,
  IN CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC  *nniAttributeOvveridePtr,
  IN GT_U32                              numberOfNniAttributeEelements  
)
{
  GT_U32 i;

  GT_UNUSED_PARAM(devNum);

  /* Override VSI domain */
  prvDomainVsiId = domainVsiId;

  /* Override mac2me if not all 0 mac address */
  for(i = 0; i < 6 ; i++)
    if(mac2meAddressPtr->arEther[i] != 0)
       break;
  if(i < 6)
    prvMac2meAddress  = *mac2meAddressPtr;
  
  for(i=0 ; i<numberOfUniAttributeEelements ; i++)
    phase1_UNI_array[i] = uniAttributeOvveridePtr[i];

  for(i=0 ; i<numberOfNniAttributeEelements ; i++)
    phase1_NNI_array[i] = nniAttributeOvveridePtr[i];

  return GT_OK;
}
