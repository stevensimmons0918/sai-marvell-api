/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvCpssDxChIpfixManager.c
*
* @brief Private CPSS APIs implementation for Ipfix Manager.
*
* @version   1
*********************************************************************************
**/
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>

/* set bits in input stream */
static GT_VOID prvCpssDxChIpfixManagerBitsSet
(
    IN    GT_U32   offset,
    IN    GT_U32   numBits,
    IN    GT_U32   val,
    INOUT GT_U8    *data
)
{
    GT_U32 ii, byte, bitShift;
    GT_U8 mask = 0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        mask = ~(1 << bitShift);
        data[byte] |= (data[byte] & mask) | (((val >> ii) & 1) << bitShift);
    }
}

/**
 * @internal prvCpssDxChIpfixManagerIpcMessageSend function
 * @endinternal
 *
 * @brief send ipc mesage to service cpu
 *
 * @param[in] devNum   - device number
 * @param[in] ipcMsg  - IPC messae
 *
 * @retVal    GT_OK     - on success
 */
GT_STATUS prvCpssDxChIpfixManagerIpcMessageSend
(
    IN GT_U8                                   devNum,
    IN PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT *ipcMsg
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     ipcData[PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_SIZE_CNS] = {0};
    GT_UINTPTR                                  fwChannel;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT  opcode;
    GT_U32                                      startOffset = 0;
    GT_U32                                      i;
    GT_U32                                      ipcMsgSize = 0;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipfixMgrDbPtr);

    fwChannel = ipfixMgrDbPtr->ipc.fwChannel;

    opcode = ipcMsg->opcodeGet.opcode;
    /* opcode */
    ipcData[0] = (GT_U8)opcode;

    switch (opcode)
    {
    /* Entry Add */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_ADD_E:
        /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
        prvCpssDxChIpfixManagerBitsSet(16, 16, (GT_U32)ipcMsg->entryAdd.flowId, ipcData);

        /* First TS Word1[14:0] (15 bits) */
        startOffset = 32 + 0;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 15,
                                       (GT_U32)ipcMsg->entryAdd.firstTs, ipcData);

        /* First TS Valid Word1[15] (1 bit) */
        startOffset = 32 + 15;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 1,
                                       (GT_U32)ipcMsg->entryAdd.firstTsValid, ipcData);

        /* IPC message size is two words */
        ipcMsgSize = 2 * 4; /* in bytes */

        break;

    /* Entry Delete */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_E:
        /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
        prvCpssDxChIpfixManagerBitsSet(16, 16, (GT_U32)ipcMsg->entryAdd.flowId, ipcData);

        /* IPC message size is one word */
        ipcMsgSize = 1 * 4; /* in bytes */

        break;

    /* Config Set */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_SET_E:
        /* ipfix enable Word0[8] (1 bit)  */
        prvCpssDxChIpfixManagerBitsSet(8, 1,
                                       (GT_U32)ipcMsg->configSet.ipfixEnable, ipcData);

        /* policer stage Word0[10:9] (2 bits) */
        prvCpssDxChIpfixManagerBitsSet(9, 2,
                                       (GT_U32)ipcMsg->configSet.policerStage, ipcData);

        /* agingOffload Word0[11] (1-bit) */
        prvCpssDxChIpfixManagerBitsSet(11, 1,
                                       (GT_U32)ipcMsg->configSet.agingOffload, ipcData);

        /* monitoringOffload Word0[12] (1-bit) */
        prvCpssDxChIpfixManagerBitsSet(12, 1,
                                       (GT_U32)ipcMsg->configSet.monitoringOffload, ipcData);

        /* deltaMode Word0[13] (1-bit) */
        prvCpssDxChIpfixManagerBitsSet(13, 1,
                                       (GT_U32)ipcMsg->configSet.deltaMode, ipcData);

        /* ipfix entries per flow Word0[16] (1-bit) */
        prvCpssDxChIpfixManagerBitsSet(16, 1,
                                       (GT_U32)ipcMsg->configSet.ipfixEntriesPerFlow, ipcData);

        /* idleTimeout Word1[11:0] (12 bits) */
        startOffset = 32 + 0;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 12,
                                       (GT_U32)ipcMsg->configSet.idleTimeout, ipcData);

        /* activeTimeout Word1[23:12] (12 bits) */
        startOffset = 32 + 12;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 12,
                                       (GT_U32)ipcMsg->configSet.activeTimeout, ipcData);

        /* Data packet MTU Word2[13:0] (14 bits) */
        startOffset = 2 * 32 + 0;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 14,
                                       (GT_U32)ipcMsg->configSet.dataPktMtu, ipcData);

        /* Local Queue Number Word2[23:16] (8 bits) */
        startOffset = 2 * 32 + 16;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 8,
                                       (GT_U32)ipcMsg->configSet.localQueueNum, ipcData);

        /* MG number Word2[31:24] (8 bits) */
        startOffset = 2 * 32 + 24;
        prvCpssDxChIpfixManagerBitsSet(startOffset, 8,
                                       (GT_U32)ipcMsg->configSet.mgNum, ipcData);

        /* TX DSA Tag Word<6-3> (4 * 32 bits)*/
        for (i=0; i<16; i++)
        {
            ipcData[12+i] = ipcMsg->configSet.txDsaTag[i];
        }

        /* 4 x maxIpfixIndex Word<8-7> (4 * 16 bits) */
        startOffset = 7*32;
        for (i = 0; i < CPSS_DXCH_MAX_PORT_GROUPS_CNS; i+=2)
        {
            /* maxIpfixIndex[i] */
            prvCpssDxChIpfixManagerBitsSet(startOffset, 16,
                                           (GT_U32)ipcMsg->configSet.maxIpfixIndex[i], ipcData);
            startOffset += 16;
        }

        /* IPC message size is 10 words */
        ipcMsgSize = 10 * 4; /* in bytes */

        break;

    /* Data Get */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_E:
        /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
        prvCpssDxChIpfixManagerBitsSet(16, 16, (GT_U32)ipcMsg->entryAdd.flowId, ipcData);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */

        break;

    /* Data Get all*/
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_ALL_E: /* Fall Through */
    /* config get */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_GET_E: /* Fall Through */
    /* Entry Delete All*/
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E:
        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode");
    }

    (void) ipcMsgSize;

#ifndef ASIC_SIMULATION
    /* Post the AttributesGet IPC message to FW */
    rc = prvCpssGenericSrvCpuIpcMessageSend(fwChannel, 1, ipcData,
                                            PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_SIZE_CNS);

#else
    (void)fwChannel;
#endif

    return rc;
}

/* get bits from input stream */
static GT_U32 prvCpssDxChIpfixManagerBitsGet
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBits
)
{
    GT_U32 ii, byte, bitShift, outData=0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        outData  |= ((data[byte] >> bitShift) & 1) << ii;
    }
    return outData;
}

/**
 * @internal prvCpssDxChIpfixManagerIpcMessageParse function
 * @endinternal
 *
 * @brief Receive ipc mesage from service cpu
 *
 * @param[in] ipcDataPtr   - (pointer to) IPC message stream read from FW.
 * @param[out] ipcMsg      - parsed IPC messae
 * @param[out] ipcMsgSizePtr - (pointer to) size of ipcData that is parsed.
 *
 * @retVal    GT_OK     - on success
 */
static GT_STATUS prvCpssDxChIpfixManagerIpcMessageParse
(
    IN GT_U8                                    *ipcDataPtr,
    OUT CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *ipcMsgPtr,
    OUT GT_U32                                  *ipcMsgSizePtr
)
{
    GT_STATUS                                   rc = GT_OK;
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT  opcode;
    GT_U32                                      startOffset = 0;
    GT_U32                                      i;

    cpssOsMemSet(ipcMsgPtr, 0, sizeof(CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT));

    CPSS_NULL_PTR_CHECK_MAC(ipcDataPtr);

    opcode = ipcDataPtr[0];
    /* opcode */
    ipcMsgPtr->opcodeGet.opcode = opcode;
    startOffset += 8;

    switch (opcode)
    {
    /* Entry Invalidate */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E:
        /* end reason - Idle/active timeout. Word0[8] (1-bit) */
        ipcMsgPtr->entryInvalidate.endReason =
            (GT_BOOL)prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 8, 1);

        /* Flow ID Word0[31:16] (16 bits) flowId[14:13] - tileNum, flowId[12:0] flowId */
        ipcMsgPtr->entryInvalidate.flowId =
            (GT_U32)prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 16, 16);

        /* first time stamp Word1[19:0] (20-bits) */
        startOffset = 1*32 + 0;
        ipcMsgPtr->entryInvalidate.firstTs =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 20);

        /* last time stamp Word1[31:20] and Word2[7:0] (20-bits) */
        startOffset = 1*32 + 20;
        ipcMsgPtr->entryInvalidate.lastTs =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 20);

        /* Packet Count Word2[31:8] and Word3[15:0] (40-bits) */
        startOffset = 2*32 + 8;
        ipcMsgPtr->entryInvalidate.packetCount[1] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 8);
        ipcMsgPtr->entryInvalidate.packetCount[0] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset + 8, 32);

        /* Drop Count Word3[31:16] and Word4[23:0] (40-bits) */
        startOffset = 3*32 + 16;
        ipcMsgPtr->entryInvalidate.dropCount[1] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 8);
        ipcMsgPtr->entryInvalidate.dropCount[0] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset + 8, 32);

        /* Byte Count Word4[31:24], Word5[32:0] and Word6[5:0] (46-bits) */
        startOffset = 4*32 + 24;
        ipcMsgPtr->entryInvalidate.byteCount[1] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 14);
        ipcMsgPtr->entryInvalidate.byteCount[0] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset + 14, 32);

        /* IPC message size 7 bytes */
        *ipcMsgSizePtr = 7 * 4; /* in bytes */

        break;

    /* Export Completion */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_EXPORT_COMPLETION_E:

        /* number of active entries Word0[31:16] (16 bits) */
        ipcMsgPtr->exportCompletion.numActive =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 16, 16);

        /* last active number. Word1[15:0] (16 bits) */
        startOffset = 1*32 + 0;
        ipcMsgPtr->exportCompletion.lastActiveNum =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 16);

        /* last packet number. Word1[31:16] (16 bits) */
        startOffset = 1*32 + 16;
        ipcMsgPtr->exportCompletion.lastPktNum =
            (GT_U8) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 16);

        /* TOD in 4ms units for last sampled packet. Word2[31:0] and Word3[23:0] (56 bits)*/
        startOffset = 2*32 + 0;
        ipcMsgPtr->exportCompletion.lastTod[1] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 24);
        ipcMsgPtr->exportCompletion.lastTod[0] =
            prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset + 24, 32);

        /* IPC message size 4 words */
        *ipcMsgSizePtr = 4 * 4; /* in bytes */

        break;

    /* config return */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_RETURN_E:
        /* ipfix enable Word0[8] (1 bit)  */
        ipcMsgPtr->configReturn.ipfixEnable =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 8, 1);

        /* policer stage Word0[10:9] (2 bits) */
        ipcMsgPtr->configReturn.policerStage =
            (GT_U8) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 9, 2);

        /* agingOffload Word0[11] (1-bit) */
        ipcMsgPtr->configReturn.agingOffload =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 11, 1);

        /* monitoringOffload Word0[12] (1-bit) */
        ipcMsgPtr->configReturn.monitoringOffload =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 12, 1);

        /* deltaMode Word0[13] (1-bit) */
        ipcMsgPtr->configReturn.deltaMode =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 13, 1);

        /* ipfix entries per flow Word0[16] (1-bit) */
        ipcMsgPtr->configReturn.ipfixEntriesPerFlow =
            (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 16, 1);

        /* idleTimeout Word1[11:0] (12 bits) */
        startOffset = 1*32 + 0;
        ipcMsgPtr->configReturn.idleTimeout =
            (GT_U32) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 12);

        /* activeTimeout Word1[23:12] (12 bits) */
        startOffset = 1*32 + 12;
        ipcMsgPtr->configReturn.activeTimeout =
            (GT_U32) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 12);

        /* Data packet MTU Word2[13:0] (14 bits) */
        startOffset = 2*32 + 0;
        ipcMsgPtr->configReturn.dataPktMtu =
            (GT_U32) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 14);

        /* Local Queue Number Word2[23:16] (8 bits) */
        startOffset = 2*32 + 16;
        ipcMsgPtr->configReturn.localQueueNum =
            (GT_U32) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 8);

        /* MG number Word2[31:24] (8 bits) */
        startOffset = 2*32 + 24;
        ipcMsgPtr->configReturn.mgNum =
            (GT_U32) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 8);

        /* TX DSA Tag Word<6-3> (4 * 32 bits)*/
        for (i=0; i<16; i++)
        {
            ipcMsgPtr->configReturn.txDsaTag[i] = ipcDataPtr[12+i];
        }

        /* 4 x maxIpfixIndex Word<8-7> (4 * 16 bits) */
        startOffset = 7*32;
        for (i = 0; i < CPSS_DXCH_MAX_PORT_GROUPS_CNS; i+=2)
        {
            ipcMsgPtr->configReturn.maxIpfixIndex[i]
                = (GT_U16) prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, startOffset, 16);
            startOffset += 16;
        }

        /* ipc message size is 10 words */
        *ipcMsgSizePtr = 10 * 4; /* in bytes */

        break;

    /* config error */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_E:
        /* return Code Word0[15:8] (8 bits)*/
        ipcMsgPtr->configError.errBmp = (GT_U32)  prvCpssDxChIpfixManagerBitsGet(ipcDataPtr, 8, 8);

        /* ipc message size is 1 word */
        *ipcMsgSizePtr = 1 * 4; /* in bytes */

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode");
    }

    return rc;
}

/**
 * @internal prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll function
 * @endinternal
 *
 * @brief  Fetch and handle all the pending IPC message (up to 1K max)
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum                 - device number
 * @param[in[  maxMsgToFetch             - max number of messages to fetch
 * @param[out] numOfMsgFetchedPtr        - (pointer to) number of fetched messages.
 *                                         Range: 0 to 1K.
 *
 * @retval GT_OK       - on success
 */
GT_STATUS prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll
(
    IN  GT_U8     devNum,
    IN  GT_U32     maxMsgToFetch,
    OUT GT_U32    *numOfMsgFetchedPtr
)
{
    GT_STATUS  rc = GT_OK;
    GT_U8      ipcData[PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_DATA_SIZE] = {0};
    GT_U8      *ipcMsgStartPtr;
    GT_UINTPTR                                  fwChannel;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT     ipcMsg;
    GT_U32                                      i;
    GT_U32                                      ipcMsgSize = 0;
    GT_U32                                      ipcMsgTotalSize   = 0;
    GT_U32                                      ipcMsgTotalSize2  = 0;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipfixMgrDbPtr);

    if (maxMsgToFetch > PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                      "maxMsgToFetch cannot exceed 1K");
    }

    fwChannel = ipfixMgrDbPtr->ipc.fwChannel;

    *numOfMsgFetchedPtr = 0;

    /* Read all IPC memory and store it in temporary buffer */
    ipcMsgTotalSize = 0;
    ipcMsgStartPtr  = &ipcData[0];
    for (i = 0; i < maxMsgToFetch; i++)
    {
#ifndef ASIC_SIMULATION
        rc = prvCpssGenericSrvCpuIpcMessageRecv(fwChannel, 1, ipcMsgStartPtr, &ipcMsgSize);
#else
        (void)fwChannel;
        return GT_OK;
#endif
        if (rc == GT_NO_MORE)
        {
             rc = GT_OK;
             break;
        }
        else if (rc != GT_OK)
        {
            return rc;
        }

        ipcMsgTotalSize += ipcMsgSize;
        if (ipcMsgTotalSize >= PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_DATA_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                      "Total IPC messages buffer exceeds max size");
        }

        ipcMsgStartPtr += ipcMsgSize;
    }

    /* Parse each IPC message and issue events */
    ipcMsgStartPtr   = &ipcData[0];
    ipcMsgTotalSize2 = 0;
    while (1) {
        /* If the IPC memory is empty, complete the API by returning 0 to the App.
           Otherwise, fetch the next pending IPC message */
        rc = prvCpssDxChIpfixManagerIpcMessageParse(ipcMsgStartPtr, &ipcMsg, &ipcMsgSize);
        if (rc == GT_BAD_VALUE)
        {
            return GT_OK;
        }
        else if (rc != GT_OK)
        {
            return rc;
        }

        (*numOfMsgFetchedPtr)++;

        if (ipfixMgrDbPtr->eventNotifyFunc)
        {
            /* Trigger and event */
            ipfixMgrDbPtr->eventNotifyFunc(&ipcMsg);
        }

        if (*numOfMsgFetchedPtr >= maxMsgToFetch)
        {
            break;
        }

        ipcMsgTotalSize2 += ipcMsgSize;
        if ((ipcMsgTotalSize2 >= ipcMsgTotalSize) ||
             (ipcMsgTotalSize2 >= PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_DATA_SIZE))
        {
            break;
        }

        ipcMsgStartPtr += ipcMsgSize;
        if ((ipcMsgStartPtr - &ipcData[0]) >=  PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_DATA_SIZE)
        {
            break;
        }
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChIpfixManagerFreeFlowIdGet function
 *
 * @brief Get Free Flow ID
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId -  port group id
 * @param[out] flowIdPtr   -  (pointer to) free flowId
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 * @retval GT_FULL      - flowId database is full
 */
GT_STATUS prvCpssDxChIpfixManagerFreeFlowIdGet
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    OUT GT_U32         *flowIdPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC  *flowIdDbPtr = NULL;
    GT_U32                                      flowId = 0;
    GT_U32                                      i = 0;
    GT_U32                                      j = 0;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(flowIdPtr);

    if (ipfixMgrDbPtr->ipfixEnable != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                                      "IPFIX Manager is not enabled");
    }

    flowIdDbPtr = &ipfixMgrDbPtr->flowIdDb[portGroupId];

    flowId = 0;
    for (i = 0; i < PRV_CPSS_DXCH_IPFIX_MANAGER_FREE_FLOW_ID_POOL_DB_ARR_SIZE_CNS; i++)
    {
        if (flowIdDbPtr->activeFlowIdPool[i] == 0xFFFFFFFF)
        {
            flowId += 32;
            continue;
        }
        for (j = 0; j < 32; j++)
        {
            if (((flowIdDbPtr->activeFlowIdPool[i] >> j) & 0x1) == 0)
            {
                flowId += j;
                break;
            }
        }
    }
    *flowIdPtr = flowId;
    flowIdDbPtr->activeFlowIdPool[flowId/32] |= (1 << (flowId%32));

    return rc;
}

/**
 * @internal prvCpssDxChIpfixManagerIsFlowIdActive function
 *
 * @brief   Check if Flow ID is active.
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId     -  port group id
 * @param[in]  flowId      -  flowId
 * @param[in]  isFlowActivePtr - flowId status
 *
 * @retval GT_OK        - on success
 */
GT_STATUS prvCpssDxChIpfixManagerIsFlowIdActive
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    IN  GT_U32         flowId,
    OUT GT_BOOL        *isFlowActivePtr
)
{
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC  *flowIdDbPtr = NULL;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFlowActivePtr);

    if ((flowId >= ipfixMgrDbPtr->portGroupConfig[(portGroupId/2)*2].maxIpfixIndex) ||
        (flowId >= PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    flowIdDbPtr = &ipfixMgrDbPtr->flowIdDb[portGroupId];

    if(((flowIdDbPtr->activeFlowIdPool[flowId/32] >> (flowId % 32)) & 0x1) == 1)
    {
        *isFlowActivePtr = GT_TRUE;
    }
    else
    {
        *isFlowActivePtr = GT_FALSE;
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChIpfixManagerFlowIdRelease function
 *
 * @brief Release a Flow ID and mark as free
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId     -  port group id
 * @param[out] flowId      -  flow id to be marked as free
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 */
GT_STATUS prvCpssDxChIpfixManagerFlowIdRelease
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    IN  GT_U32         flowId
)
{
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC  *flowIdDbPtr = NULL;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);

    if ((flowId >= ipfixMgrDbPtr->portGroupConfig[(portGroupId/2)*2].maxIpfixIndex) ||
        (flowId >= PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    flowIdDbPtr = &ipfixMgrDbPtr->flowIdDb[portGroupId];

    if(((flowIdDbPtr->activeFlowIdPool[flowId/32] >> (flowId % 32)) & 0x1) == 1)
    {
        flowIdDbPtr->activeFlowIdPool[flowId/32] &= (~(1 << (flowId % 32)));
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChIpfixManagerActiveFlowIdDump function
 *
 * @brief Dump all active flowIds
 *
 * @param[in]  devNum      -  device number
 *
 * @retval GT_OK        - on success
 */
GT_STATUS prvCpssDxChIpfixManagerActiveFlowIdDump
(
    IN  GT_U8          devNum
)
{
    GT_U32         portGroupId;
    GT_U32         flowId;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;
    PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC  *flowIdDbPtr = NULL;
    GT_U32         flowsCount;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);

    for (portGroupId=0; portGroupId<CPSS_DXCH_MAX_PORT_GROUPS_CNS; portGroupId++)
    {
        flowsCount = 0;
        cpssOsPrintf("PortGroup: %d\n", portGroupId);
        for (flowId=0; flowId<PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS; flowId++)
        {
            flowIdDbPtr = &ipfixMgrDbPtr->flowIdDb[portGroupId];

            if(((flowIdDbPtr->activeFlowIdPool[flowId/32] >> (flowId % 32)) & 0x1) == 1)
            {
                cpssOsPrintf("%d, ", flowId);
                flowsCount++;
                if ((flowsCount % 20) == 0)
                {
                    cpssOsPrintf("\n");
                }
            }
        }
        cpssOsPrintf("\n");
    }

    return GT_OK;
}
