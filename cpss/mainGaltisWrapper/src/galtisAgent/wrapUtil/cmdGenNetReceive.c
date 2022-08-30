/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cmdDxChNetReceive.c
*
* DESCRIPTION:
*       galtis agent interface for packet capture using networkIf -- DXCH
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*******************************************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>

#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#endif

/******************************** Locals *************************************/

/**
* @internal rxInfoFree function
* @endinternal
*
* @brief   function to free the specific rx info format.
*
* @param[in] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH / EXMX /DXSAL ...
*                                       None
*/
static void rxInfoFree
(
    IN void*  specificDeviceFormatPtr
)
{
    if(specificDeviceFormatPtr)
    {
        cmdOsFree(specificDeviceFormatPtr);
    }
}

/**
* @internal rxInfoCopy function
* @endinternal
*
* @brief   function to COPY the specific rx info format.
*
* @param[in] srcSpecificDeviceFormatPtr - (pointer to) the SOURCE specific device Rx
*                                      info format.
*                                      format of DXCH / EXMX /DXSAL ...
*
* @param[out] dstSpecificDeviceFormatPtr - (pointer to) the DESTINATION specific device Rx
*                                      info format.
*                                      format of DXCH / EXMX /DXSAL ...
*                                       None
*/
static void rxInfoCopy
(
    IN  const void*  srcSpecificDeviceFormatPtr,
    OUT void*        dstSpecificDeviceFormatPtr
)
{
    GT_U32 size = 0;


#if defined CHX_FAMILY
    size = sizeof(CPSS_DXCH_NET_RX_PARAMS_STC);
#endif

    if(srcSpecificDeviceFormatPtr == NULL || dstSpecificDeviceFormatPtr == NULL)
    {
        return;
    }

    cmdOsMemCpy(dstSpecificDeviceFormatPtr,
             srcSpecificDeviceFormatPtr,
             size);

    return;
}

/*******************************************************************************
* cmdDxChRxPktReceive
*
* DESCRIPTION:
*       This function returns packets from PP destined to the CPU port.
*        -- SDMA relate.
*
* APPLICABLE DEVICES: ALL PCI/SDMA DXCH Devices
*
* INPUTS:
*       devNum      - Device number.
*       queueIdx    - The queue from which this packet was received.
*       numOfBuffPtr- Num of buffs in packetBuffs.
*       packetBuffs  - The received packet buffers list.
*       buffLen      - List of buffer lengths for packetBuffs.
*       rxParamsPtr  - (Pointer to)information parameters of received packets
*
* RETURNS:
*
*
* COMMENTS:
*
*
*******************************************************************************/
#if defined CHX_FAMILY
extern GT_STATUS prvCpssGenNetIfRx2Tx
(
    IN GT_U8  *buffList[],
    IN GT_U32 buffLenList[],
    IN GT_U32 numOfBufs
);
#else /*CHX_FAMILY*/
static GT_STATUS prvCpssGenNetIfRx2Tx
(
    IN GT_U8  *buffList[],
    IN GT_U32 buffLenList[],
    IN GT_U32 numOfBufs
)
{
    GT_UNUSED_PARAM(buffList);
    GT_UNUSED_PARAM(buffLenList);
    GT_UNUSED_PARAM(numOfBufs);

    return GT_NOT_IMPLEMENTED;
}
#endif /*CHX_FAMILY*/

GT_STATUS cmdGenRxPktReceive
(
    IN GT_U8      devNum,
    IN GT_U8      queueIdx,
    IN GT_U32     numOfBuff,
    IN GT_U8     *packetBuffs[],
    IN GT_U32     buffLen[],
    IN void      *rxParamsPtr
)
{
    GT_STATUS rc;
    void  *tmpRxParamPtr;
    GT_U32 size = 0;

#if defined CHX_FAMILY
    if(size < sizeof(CPSS_DXCH_NET_RX_PARAMS_STC))
       size = sizeof(CPSS_DXCH_NET_RX_PARAMS_STC);
#endif

    /* If Rx to Tx disabled the function prvCpssGenNetIfRx2Tx returns
    without sending the packet. */
    prvCpssGenNetIfRx2Tx(packetBuffs,buffLen,numOfBuff);


    if(GT_FALSE == cmdCpssRxPktIsCaptureReady())
    {
        return GT_OK;
    }

    tmpRxParamPtr = cmdOsMalloc(size);
    if(tmpRxParamPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cmdOsMemCpy(tmpRxParamPtr, rxParamsPtr, size);

    rc = cmdCpssRxPktReceive(devNum,queueIdx,tmpRxParamPtr,
                         &rxInfoFree,&rxInfoCopy,
                         numOfBuff,packetBuffs,buffLen);
    if(rc != GT_OK)
    {
        cmdOsFree(tmpRxParamPtr);
    }

    return rc;
}

#if defined CHX_FAMILY
GT_STATUS cmdDxChRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               *packetBuffs[],
    IN GT_U32                               buffLen[],
    IN CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
);
#endif
#if defined PX_FAMILY
GT_STATUS cmdPxRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               *packetBuffs[],
    IN GT_U32                               buffLen[]
);
#endif
static GT_STATUS prvCmdRxPktReceive
(
  IN  GT_UINTPTR                cookie,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  IN  GT_U8                     devNum,
  IN  GT_U8                     queueIdx,
  IN  GT_U32                    numOfBuff,
  IN  GT_U8                     *packetBuffs[],
  IN  GT_U32                    buffLen[],
  IN  GT_VOID                   *rxParamsPtr
)
{
    GT_UNUSED_PARAM(cookie);
    GT_UNUSED_PARAM(evType);

#if defined CHX_FAMILY
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        cmdDxChRxPktReceive(devNum, queueIdx, numOfBuff,
                packetBuffs, buffLen, (CPSS_DXCH_NET_RX_PARAMS_STC*)rxParamsPtr);
    }
#endif
#if defined PX_FAMILY
    GT_UNUSED_PARAM(rxParamsPtr);
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        cmdPxRxPktReceive(devNum, queueIdx, numOfBuff,
                packetBuffs, buffLen);
    }
#endif
    return GT_OK;
}

/**
* @internal cmdLibInitRxPktReveive function
* @endinternal
*
* @brief   Initialize RxPkt functions
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitRxPktReveive
(
    GT_VOID
)
{
    /* to be sure library initialized */
    rxEventHandlerLibInit();
    rxEventHandlerAddCallback(
            RX_EV_HANDLER_DEVNUM_ALL,
            RX_EV_HANDLER_DEVNUM_ALL,
            RX_EV_HANDLER_TYPE_ANY_E,
            prvCmdRxPktReceive,
            0);
    return GT_OK;
}


