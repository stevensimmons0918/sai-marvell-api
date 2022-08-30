/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefGlobalSystemInfo.c
*
* DESCRIPTION:
*       Global system infomation that used by tests files.
*       The global infomation set by test global environment, for usage in the tests.
*       Such infomation can device numebr and it's ports list that are connected to TG or in loopback.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "csRefGlobalSystemInfo.h"

#include <cpss/generic/cpssTypes.h>

#include <gtOs/gtOsMem.h>

/***********************************************/
/*         Global variables definition                                    */
/***********************************************/


CSREF_GLOBAL_SYSTEM_INFO_STC csRefSystemInfo;


/***********************************************/
/*         Function definition                                          */
/***********************************************/

/**
* @internal csRefGlobalSystemInfoSet function
* @endinternal
*
* @brief   Set the global infomation to external variable.
*            Tests will use these variable in their test.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] systemInfo - system infomation
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM        - on wrong parameters
* @retval GT_OUT_OF_RANGE   - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS csRefGlobalSystemInfoSet
(
  IN CSREF_GLOBAL_SYSTEM_INFO_STC *systemInfo
)
{

  if(systemInfo == NULL)
    return GT_BAD_PARAM;

  if(systemInfo->deviceInfo.numberOfPorts > CSREF_MAX_NUMER_OF_PORTS_CNS)
      return GT_BAD_PARAM;

  osMemCpy(&csRefSystemInfo, systemInfo, sizeof(csRefSystemInfo));

  return GT_OK;
}


GT_STATUS csRefGlobalSystemInfoSetPort
(
    GT_U8       devNum,
    GT_PORT_NUM portNumber,
    GT_BOOL     reset
)
{
  static GT_BOOL firstTime = GT_TRUE;
  GT_STATUS rc = GT_OK;

  if(reset == GT_TRUE)
    firstTime = GT_TRUE;

  if(firstTime == GT_TRUE)
  {
    osMemSet(&csRefSystemInfo, 0, sizeof(csRefSystemInfo));
    firstTime = GT_FALSE;
  }

  if(csRefSystemInfo.deviceInfo.numberOfPorts >= CSREF_MAX_NUMER_OF_PORTS_CNS)
    return GT_BAD_PARAM;

  csRefSystemInfo.deviceInfo.portNumberArray[csRefSystemInfo.deviceInfo.numberOfPorts++]=portNumber;
  csRefSystemInfo.deviceInfo.deviceNumber = devNum;

  return rc;
}

