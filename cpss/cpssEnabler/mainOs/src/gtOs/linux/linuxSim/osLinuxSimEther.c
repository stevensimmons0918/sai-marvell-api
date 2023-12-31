/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osLinuxSimEther.c
*
* @brief Operating System wrapper for Linux
*
* @version   1.1.2.2
********************************************************************************
*/
#include <gtOs/gtGenTypes.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <asicSimulation/SKernel/skernel.h>

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxSimEtherSrc

/**
* @internal osEtherInit function
* @endinternal
*
* @brief   This function inits the internal data structure of the ethernet port
*/
GT_STATUS osEtherInit
(
    void
)
{
    return GT_OK;
}
/**
* @internal osInputCallback function
* @endinternal
*
* @brief   This function is the internal Rx callback
*
* @param[in] pIf                      - interface packet was received on
* @param[in] buffer_PTR               - received packet
* @param[in] bufferLen                - length of received packet
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_VOID osInputCallback
(
    IN GT_VOID		*pIf,
    IN GT_U8 *      buffer_PTR,
    IN GT_32        bufferLen
)
{
    
    /* Call the user callback routine */
    if (!PRV_SHARED_DB.userRxCallback)
    {
        return;
    }
    PRV_SHARED_DB.userRxCallback(buffer_PTR, bufferLen);
}

/**
* @internal osEtherInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callbak function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS osEtherInputHookAdd
(
    IN GT_Rx_FUNCP userRxFunc
)
{

    skernelNicRxBind((SKERNEL_NIC_RX_CB_FUN)osInputCallback);
	
    PRV_SHARED_DB.userRxCallback = userRxFunc;

    return GT_OK;
}

/**
* @internal osEtherOutput function
* @endinternal
*
* @brief   This function transmits an Ethenet packet to the Packet processor
*
* @param[in] header_PTR[]             - pointer the the 14 bytes packet header.
* @param[in] paylaod_PTR[]            - pointer to the reset of the packet.
* @param[in] payloadLen               - length ot the payload.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS osEtherOutput
(
    IN GT_U8_PTR header_PTR[],
    IN GT_U32    paylaod_PTR[],
    IN GT_U32    payloadLen
)
{
    GT_STATUS status;

    status = skernelNicOutput(header_PTR, paylaod_PTR, payloadLen);

    return GT_OK == status ? GT_OK : GT_FAIL;
}

/**
* @internal osEtherInputHookDelete function
* @endinternal
*
* @brief   This unbinds the user Rx callback
*/
GT_STATUS osEtherInputHookDelete
(
    void
)
{
    skernelNicRxUnBind((SKERNEL_NIC_RX_CB_FUN)osInputCallback);

    return GT_OK;
}



