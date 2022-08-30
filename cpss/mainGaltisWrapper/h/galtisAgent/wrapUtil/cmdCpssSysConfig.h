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

/*******************************************************************************
* cmdSysConfig.h
*
* DESCRIPTION:
*       System configuration and initialization control.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 25 $
*
*******************************************************************************/
#ifndef __cmdSysConfigh
#define __cmdSysConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/cmdDb/cmdBase.h>

/*******************************************************************************
 * susconf related definitions
 ******************************************************************************/
/* Max number of chars in string fields in CMD_SYSCONF_INFO */
#define MAX_SYSCONF_STR_LEN 30

#define TAPI_TYPE_ID    12      /* For father Type  */
#define TAPI_ID         0       /* For id           */
#define TAPI_TYPE       0x101   /* For deviceId     */

#define UNIT_TYPE_ID    13      /* For father Type  */
#define UNIT_TYPE       0x102   /* For deviceId     */
#define UNIT_SON_TYPE   0       /* For sonType      */


#define PP_TYPE_ID      14      /* For father type  */
#define PP_SON_TYPE     0       /* For sonType      */


#define PHY_SON_TYPE    22      /* For sonType      */

#define IS_MARVELL_PHY(phyId)                                   \
        ((phyId >> 10) == 0x5043)

#define GET_PHY_ID(phyId)                                       \
        (phyId >> 4)

/**
* @struct CMD_SYSCONF_INFO
 *
 * @brief Holds information regarding all elements in system for the
 * sysconf usage.
*/
typedef struct{

    /** The id of the device. */
    GT_U32 id;

    GT_U8 revision[MAX_SYSCONF_STR_LEN];

    GT_U8 subRev[MAX_SYSCONF_STR_LEN];

    /** Device type. */
    GT_U32 deviceId;

    /** Id field of the device's father. */
    GT_U32 fatherId;

    /** Type of the father. */
    GT_U32 fatherType;

    /** Which son is this. */
    GT_U32 sonIndex;

    /** Type of this device. */
    GT_U32 sonType;

} CMD_SYSCONF_INFO;


/**
* @internal mngInitSystem function
* @endinternal
*
* @brief   Initialize the system according to the pp configuration paramters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note
*       GalTis:
*       Command - mngInitSystem
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngInitSystem(void);


/**
* @internal mngGetSysFirstElement function
* @endinternal
*
* @brief   Returns the first element of the sysconf.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note
*       Galtis:
*       Command - mngGetSysFirstElement
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngGetSysFirstElement(void);

/**
* @internal mngGetSysNextElement function
* @endinternal
*
* @brief   Returns the next element of the sysconf.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note
*       Galtis:
*       Command - mngGetSysNextElement
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngGetSysNextElement(void);


/**
* @internal sys function
* @endinternal
*
* @brief   Prints the sysconf elements.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note
*       Galtis:
*       Command - sys
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS sys(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdSysConfigh */


