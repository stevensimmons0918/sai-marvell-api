/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefIOAM.h
*
* DESCRIPTION:
*  This files provide APIs to configure IOAM nodes - ingress, transit and egress and also the configuration for monitoring port on egress node.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef _IOAM_h


#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>

/**
* @internal csRefIoamSetIngressMacDa function
* @endinternal
*
* @brief   sets MAC DA exspected of non-IOAM packet ingressing to Ingress Node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetIngressMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
);


/**
* @internal csRefIoamSetTransitMacDa function
* @endinternal
*
* @brief    sets MAC DA exspected of IOAM packet that ingresses to Transit node and also the TS MAC DA for IOAM packet that egresses Ingress node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetTransitMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
);

/**
* @internal csRefIoamSetEgressMacDa function
* @endinternal
*
* @brief   sets  MAC DA exspected of an IOAM packet that ingresses to Egress node and also the ARP MAC DA for IOAM packet that egresses Transit node,
    where we change MAC DA in additon to adding a record to IOAM header
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetEgressMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
);

/**
* @internal csRefIoamSetMonitorMacDa function
* @endinternal
*
* @brief   sets MAC DA assigned to packet mirrored in GRE tunnel to a monitor from the Egress node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetMonitorMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
);


/**
* @internal csRefIoamNodeConfigCreate function
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
* @param[in] txAnalyzerPort   port number where IOAM packet is copied to a monitoring interface when switch workes as an egress node
* @param[in] traceTypeBmp - trace bitmap in IOAM header - defines wht type of information will be added by the node to the IOAM header
*
* @retval GT_OK                                    - on success
* @retval GT_BAD_PARAM                      - on wrong parameters
* @retval GT_OUT_OF_RANGE                 - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                                  - otherwise
*
* @note  demostrates configuration for IOAM using PHA; Same ingress port serves for any IOAM node role
*          use like: csRefIoamNodeConfigCreate 0,0,36,32,0xfa00
*/
GT_STATUS csRefIoamNodeConfigCreate
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM ingressPort,
    IN GT_PHYSICAL_PORT_NUM egressPort,
    IN GT_PHYSICAL_PORT_NUM txAnalyzerPort,
    IN GT_U32               traceTypeBmp
);



/**
* @internal csRefIoamNodeConfigDelete function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefIoamNodeConfigCreate .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefIoamNodeConfigDelete
(
  IN GT_U8         devNum
);

/**
* @internal csRefIoamParser function
* @endinternal
*
* @brief   parses IOAM packet and prints its most interesting fields.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pktString           - device number
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/

GT_STATUS csRefIoamParser
(
  IN char pktString[1024]
);


#endif
