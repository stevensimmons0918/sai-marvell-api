/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csExactMatch_Test.c
*
* DESCRIPTION:
*  This file contains test scenarios for the Exact Match feature, using the APIs in csExactMatch.h
*  In all tests EM and TTI use same matching rules for simple packet forwarding, but forward packts to different interfaces.
*  TTI first lookup is EM client.  CNC counters are used to show which rule was matched.
*
*         Test-1
*           1.1. configure EM using EM Manager and give preference to EM action
*           1.2. Transmit one Ethernet packet to ingress port
*           1.3. Check that packet is received on egress Port for EM and on no other ports, see that right CNC counter is incremented
*           1.4. Clear configuration
*
*         Test-2
*           2.1. configure EM using EM low level API's and give preference to EM action
*           2.2. Transmit one Ethernet packet to ingress port
*           2.3. Check that packet is received on egress Port for EM and on no other ports, see that right CNC counter is incremented
*           2.4. Clear configuration
*
*         Test-3....
*           3.1. configure EM using EM low level API's and give preference to TTI action
*           3.2. Transmit one Ethernet packet to ingress port
*           3.3. Check that packet is received on egress Port for TTI and on no other ports, see that right CNC counter is incremented
*           3.4. Clear configuration
*         Test-4....
*         Test-5....
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/


#include "../../infrastructure/csRefSysConfig/csRefGlobalSystemInfo.h"

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include "csExactMatch.h"


/***********************************************/
/*         Exact Match Reference Code initialization  */
/***********************************************/


/**
* @internal csRefEmRedirectTtiEmConfig function
* @endinternal
*
* @brief   In this example packet matched by its ingress port is forwarded to one of two egress ports depending
*             if the match was done by TTI or EM. Different CNC counters are also connected to rules.
*             Traffic classification done on ingress port only, any valid ethernet packet can be used in test.
*             This function is a wrapper for csRefEmRedirectTtiEmConfigCreate
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] ingressPort         port number where sample packet enters
* @param[in] egressPortTTI - where packet is forwarded by TTI rule
* @param[in] egressPortEM  - where packet is forwarded by EM rule
* @param[in] emOverTtiEn  - if true, EM match takes precedence over TTI match
* @param[in] useEmManager - if true, EM is configured by EM Manager API's, if not by low-level API's
*
* @retval GT_OK                                - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note  use like:
*            shell-execute csRefEmRedirectTtiEmConfig 0,0,32,36,1,1
*            to change EM share of Shared Tables call
*            shell-execute appDemoDbEntryAdd "sharedTableMode" <MODE>
*            before cpssInitSystem
*
*/

GT_STATUS csRefEmRedirectTtiEmConfig
(
    IN  GT_U8   devNum,
    IN  GT_U32  ingressPort,
    IN  GT_U32  egressPortTTI,
    IN  GT_U32  egressPortEM,
    IN  GT_BOOL emOverTtiEn,
    IN  GT_BOOL useEmManager
)
{
  GT_STATUS rc=GT_OK;

  rc = csRefEmRedirectTtiEmConfigCreate(devNum, ingressPort, egressPortTTI, egressPortEM, emOverTtiEn, useEmManager);
  if(rc != GT_OK)
    return rc;

  return rc;
}


/**
* @internal csRefEmRedirectTtiEmConfigClear function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefEmRedirectTtiEmConfigCreate.
*             This function is a wrapper for csRefEmRedirectTtiEmConfigDelete
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefEmRedirectTtiEmConfigClear
(
  IN GT_U8         devNum
)
{
    GT_STATUS rc = GT_OK;

    /* Clear MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
    rc = csRefEmRedirectTtiEmConfigDelete(devNum);
    if(rc != GT_OK)
      return rc;

    return GT_OK;
}



