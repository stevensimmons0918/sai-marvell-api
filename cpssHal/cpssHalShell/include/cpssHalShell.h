/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitializeLuaCli.h
*
* DESCRIPTION:
*       Services initialization
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __INCLUDE_MRVL_HAL_SHELL_API_H
#define __INCLUDE_MRVL_HAL_SHELL_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef SHELL_ENABLE

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/*******************************************************************************
* cpssHalInitializeCmdInit
*
* DESCRIPTION:
*
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
GT_STATUS cpssHalInitializeCmdInitApi(
    void
);


/*******************************************************************************
* cpssHalCmdShellEnable
*
* DESCRIPTION:
*
* INPUTS:
*       GT_BOOL enable
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
GT_STATUS cpssHalCmdShellEnable(GT_BOOL enable);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MRVLHALINITIALIZELUACLI_H_ */
