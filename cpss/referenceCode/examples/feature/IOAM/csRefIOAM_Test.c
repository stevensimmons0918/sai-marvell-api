/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csRefIOAM_Test.c
*
* DESCRIPTION:
*  This file contains test scenarios for the IOAM feature, using the APIs in csRefIOAM.h.h
*
*         Test-1
*           1.1. configure switch as IOAM node, role of node (Ingress/Transit/Egress) depends on MAC DA of incomming packet
*           1.2. Transmit one non-IOAM packet to port 1 as ingress packet to Ingress node
*           1.3. Check that packet is received on port 2 and on no other ports, verify its IOAM and tunnel fields
*
*         Test-2
*           2.1. Same configuration as in test 1..
*           2.2.Transmit one IOAM packet with 1 record to port 1 as ingress packet to Transit node
*           2.3. Check that packet is received on port 2 and on no other ports, verify its IOAM (two records) and tunnel (MAC DA changed) fields
*
*         Test-3....
*           3.1. Same configuration as in test 1..
*           3.2.Transmit one IOAM packet with 2 records to port 1 as ingress packet to Egress node
*           3.3. Check that packet received on port 3 and on port 2, verify IOAM (three records) and tunnel (IPv6 GRE header added) fields on packet received on port 3
*         Test-4....
*         Test-5....
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/


#include "csRefIOAM.h"
#include "../../infrastructure/csRefSysConfig/csRefGlobalSystemInfo.h"

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>


/***********************************************/
/*         IOAM related DB initialization  */
/***********************************************/


/**
* @internal csRefIoamNodeConfig function
* @endinternal
*
* @brief   The function creates configuration for IOAM node. the switch can serve as either ingress, transit or egress node, nodes are recognized by
*             MAC DA of incoming packets
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          - device number
* @param[in] ingressPort        port number where IOAM packet enters
* @param[in] egressPort         port number where IOAM packet egresses
* @param[in] analyzerTxPort   port number where IOAM packet is copied to a monitoring interface when switch works as an egress node
* @param[in] traceType        - trace bitmap in IOAM header - defines wht type of information will be added by the node to the IOAM header
*
* @retval GT_OK                                    - on success
* @retval GT_BAD_PARAM                      - on wrong parameters
* @retval GT_OUT_OF_RANGE                 - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                                  - otherwise
*
* @note  demostrates configuration for IOAM using PHA; Same ingress port serves for any IOAM node role
*          use like: csRefIoamNodeConfig 0,0,36,32,0xfa00
*
*/
GT_STATUS csRefIoamNodeConfig
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM ingressPort,
    IN GT_PHYSICAL_PORT_NUM egressPort,
    IN GT_PHYSICAL_PORT_NUM analyzerTxPort,
    IN GT_U32               traceTypeBmp
)
{
  GT_STATUS rc=GT_OK;

  /* eArch element DB initializatin. */
  rc = csRefIoamNodeConfigCreate(devNum, ingressPort, egressPort, analyzerTxPort, traceTypeBmp);
  if(rc != GT_OK)
    return rc;

  return rc;
}

/**
* @internal csRefIoamNodeConfigClear function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefIoamNodeConfigCreate .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                                   - on success
* @retval GT_BAD_PARAM                     - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                                 - otherwise
*
* @note
*
*/
GT_STATUS csRefIoamNodeConfigClear
(
  IN GT_U8         devNum
)
{
  GT_STATUS rc = GT_OK;

  /* Clear MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefIoamNodeConfigDelete(devNum);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}

