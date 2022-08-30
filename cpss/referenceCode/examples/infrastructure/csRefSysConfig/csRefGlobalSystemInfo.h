/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefGlobalSystemInfo.h
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
#ifndef __csRefGlobalSystemInfo_h
#define __csRefGlobalSystemInfo_h


#include <cpss/generic/cpssTypes.h>

/***********************************************/
/*         Type Definitions  */
/***********************************************/

#define CSREF_MAX_NUMER_OF_PORTS_CNS  10 /* Maximum number of port can be used in the test. */

/**
* @struct CSREF_DEVICE_INFO_STC
*
* @brief This struct defines device specific information,
* that required by various of test scenarios..
*/
typedef struct{

  /** device number.
  *  (APPLICABLE RANGES: 0..31)
  */
  GT_U8       deviceNumber;
  /** @brief List of ports that are .
  *  that connected to a TG ports.
  *  numberOfPorts - number of valid port numbers in the list
  */
  GT_PORT_NUM portNumberArray[CSREF_MAX_NUMER_OF_PORTS_CNS];
  GT_U32      numberOfPorts;

}CSREF_DEVICE_INFO_STC;



/**
* @struct CSREF_GLOBAL_SYSTEM_INFO_STC
*
* @brief This struct defines global system infomation,
* that required by various of test scenarios.
*/
typedef struct{
  /**
  * @brief systemInfo - system global information.
  * This information required by various of test scenarios.
  */
  CSREF_DEVICE_INFO_STC  deviceInfo;
}CSREF_GLOBAL_SYSTEM_INFO_STC;


/***********************************************/
/*         Global variables decleration                                    */
/***********************************************/


extern CSREF_GLOBAL_SYSTEM_INFO_STC csRefSystemInfo;



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
);

#endif
