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
/**
********************************************************************************
* @file simOsSlan.h
* @version   2
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsSlanh
#define __simOsSlanh

#include <os/simTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal simOsSlanBind function
* @endinternal
*
* @brief   Binds a slan to client.
*
* @param[in] slanNamePtr              - pointer to slan name.
* @param[in] clientNamePtr            - pointer to client name.
* @param[in] funcPtr                  - pointer to function
*                                       Pointer to void
*/
extern SIM_OS_SLAN_ID simOsSlanBind (
    char                            *slanNamePtr,
    char                            *clientNamePtr,
    void                            *usrInfoPtr,
    SIM_OS_SLAN_RCV_FUN     funcPtr

);
/*******************************************************************************
*   simOsSlanTransmit
*
* DESCRIPTION:
*       Transmit a message from slan id.
*
* INPUTS:
*       slanId         -  slan id.
*       msgCode         - message code.
*       len             - message length.
*       msgPtr          - pointer to the message
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
extern unsigned int simOsSlanTransmit (
    SIM_OS_SLAN_ID              slanId,
    SIM_OS_SLAN_MSG_TYPE_ENT    msgType,
    GT_U32                      len,
    char                        *msgPtr
);

/**
* @internal simOsSlanUnbind function
* @endinternal
*
* @brief   Unbinds a client from slan.
*
* @param[in] slanId                   - slan id.
*/
extern void simOsSlanUnbind (
    SIM_OS_SLAN_ID  slanId
);

/**
* @internal simOsSlanClose function
* @endinternal
*
* @brief   Closes SLAN lib.
*/
extern void simOsSlanClose (
    void
);

/**
* @internal simOsSlanInit function
* @endinternal
*
* @brief   Calls Linux related slan-like initialization.
*/
extern void simOsSlanInit (
    void
);

/**
* @internal simOsSlanStart function
* @endinternal
*
* @brief   Calls Linux related slan-like start mechanism.
*/
extern void simOsSlanStart (
    void
);


/**
* @internal simOsChangeLinkStatus function
* @endinternal
*
* @brief   Change the state of link for the SLAN .
*
* @param[in] slanId                   - slan id.
* @param[in] linkState                - 1 for up , 0 for down.
*/
extern void simOsChangeLinkStatus
(
    SIM_OS_SLAN_ID  slanId ,
    GT_BOOL      linkState
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __simOsSlanh */



