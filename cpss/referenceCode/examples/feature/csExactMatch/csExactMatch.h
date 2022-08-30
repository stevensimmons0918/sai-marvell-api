/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csExactMatch.h
*
* DESCRIPTION:
*  This files provide APIs to configure EM - low level based or EM Manager based also the configuration of TTI engine, to which 
*  EM lookup serves as a client.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef _EM_h


#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>


/**
* @internal csRefEmRedirectTtiEmConfigCreate function
* @endinternal
*
* @brief   In this example packet matched by its ingress port is forwarded to one of two egress ports depending 
*             if match was done by TTI or EM. Different CNC counters are also connected to rules.
*             Traffic classification done on ingress port only, any valid ethernet packet can be used in test.
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
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note  use like:
*            shell-execute csRefEmRedirectTtiEmConfigCreate 0,0,32,36,1,1
*            to change EM share of Shared Tables call
*            shell-execute appDemoDbEntryAdd "sharedTableMode" <MODE>
*            before cpssInitSystem
*
*/
GT_STATUS csRefEmRedirectTtiEmConfigCreate
(
    IN  GT_U8   devNum,
    IN  GT_U32  ingressPort,
    IN  GT_U32  egressPortTTI,
    IN  GT_U32  egressPortEM,
    IN  GT_BOOL emOverTtiEn,
    IN  GT_BOOL useEmManager
);


/**
* @internal csRefEmRedirectTtiEmConfigDelete function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefEmRedirectTtiEmConfigCreate.
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

GT_STATUS csRefEmRedirectTtiEmConfigDelete
(
    IN  GT_U8 devNum
);


#endif
