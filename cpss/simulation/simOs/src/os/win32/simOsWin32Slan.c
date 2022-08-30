/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smain.c
*
* DESCRIPTION:
*       SLAN API functions
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*******************************************************************************/

#include <os/simTypesBind.h>
#include <common/SLAN/SAGENT/EXP/SAGENT.H>


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
    SIM_OS_SLAN_RCV_FUN              funcPtr
)
{
    void *retStruct;
    char slanCpuUniqueName[SAGNTG_max_name_CNS + 1];/*8+1*/

    if(0 == strcmp(slanNamePtr,"slanCpu") ||
       0 == strcmp(slanNamePtr,"slancpu"))/*it seems we get here with 'Lower case' */
    {
        /*we give this special treatment for the CPU port only (and not to the B2B ports) ,
          because the B2B connections can be replaced by 'Internal connection' mechanism
          but the CPU port that sends packet to the CPU has limitation in MST projects:
          in MST each thread that cals MST_OS functions must be registered (SLAN are registered --> SHOST,SLAN Libs)
          but the 'Asic simualtion' threads are not. */

        /*use 'unique' SLAN to allow running of several processes in parallel*/
        sprintf(slanCpuUniqueName,"%8.8x",(GT_U32)GetCurrentProcessId());

        slanNamePtr = slanCpuUniqueName;
    }


    retStruct = SAGNTG_bind( slanNamePtr, clientNamePtr, 8*1024, 0,
                             usrInfoPtr, funcPtr );

    return retStruct;

}
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
)
{
    unsigned int retVal;
    GT_U32 msgCode = 0;

    switch (msgType) {
        case SIM_OS_SLAN_MSG_CODE_FRAME_CNS :
            msgCode = SAGNTG_user_first_msg_CNS;
        break;
        default: break;
    }

    retVal = (SAGNTG_transmit(slanId, NULL,
                              msgCode, len, msgPtr));

    return retVal;

}

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
)
{
#if __WORDSIZE == 64
    slanId = slanId;/* calling SAGNTG_unbind(...) cause dead lock on win64 bits */
#else
    SAGNTG_unbind(slanId);
#endif
}

/**
* @internal simOsSlanInit function
* @endinternal
*
* @brief   Calls Linux related slan-like initialization.
*/
extern void simOsSlanInit (
    void
)
{
    return;
}

/**
* @internal simOsSlanClose function
* @endinternal
*
* @brief   Closes SLAN lib.
*/
extern void simOsSlanClose (
    void
)
{
    return;
}

/**
* @internal simOsSlanStart function
* @endinternal
*
* @brief   Calls Linux related slan-like start mechanism.
*/
extern void simOsSlanStart (
    void
)
{
    return;
}


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
)
{
    SAGNTG_ChangeLinkStatus (slanId,(BOOLEAN )linkState);
}


