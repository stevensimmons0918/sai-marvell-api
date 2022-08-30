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
* @file prvTgfEventDeviceGenerate.c
*
* @brief Includes code for test that checks event generation from cpssAPI
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <event/prvTgfEventDeviceGenerate.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsBobcat2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsBobK.h>
#include <port/prvTgfPortFWS.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <common/tgfBridgeGen.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static GT_BOOL prvTgfGenSkipEventCheck
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    currentEvent,
    IN CPSS_UNI_EV_CAUSE_ENT    skipEventArray[],
    IN GT_U32                   skipEventArraySize
);
#include <extUtils/common/cpssEnablerUtils.h>

/* Macros that fills evExtDataArray with
 * linear sequence starting with 0*/
#define PRV_TGF_FILL_EXT_DATA_ARRAY(size) \
        for (i = 0; i < size; i++) \
            evExtDataArray[i] = i; \
        *evExtDataSize = size;

/* Macros skips events which are contained in array */
#define PRV_TGF_SKIP_EVENTS(dev, event, array, arraySize)                                   \
        if (prvTgfGenSkipEventCheck(dev, event, array, arraySize) == GT_TRUE)   continue;

extern GT_BOOL tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal prvTgfPrintFailedEvents function
* @endinternal
*
* @brief   Print array of failed events.
*
* @param[in] eventArray               - array of PRV_PRINT_EVENT_CTX
*                                      structures for failed events
* @param[in] size                     -  of eventArray
*                                      eventNames - array of string names of unified events
*                                       None
*/
GT_VOID prvTgfPrintFailedEvents
(
    PRV_PRINT_EVENT_CTX* eventArray, /* Array of failed events */
    GT_U32 size,                     /* Size of array of failed events*/
    char** eventNames,               /* Array of string names of
                                        unified events */
    GT_BOOL overflow                 /* Flag that indicates overflowing
                                        of array of failed events */
)
{
    GT_U32 i;
    char *eventStr; /*string value of unified event*/
    PRV_UTF_LOG0_MAC("\n\n");
    if (size > 0)
    {
        PRV_UTF_LOG0_MAC("******************** Failed Events ********************\n");
        PRV_UTF_LOG0_MAC("\n");
        for (i = 0; i < size; i++)
        {
            eventStr = eventNames[(GT_U32)eventArray[i].event];
            PRV_UTF_LOG4_MAC("Event Name: %s; Returned Code: %d; Counter Value: %d; evData: %d\n",
                    eventStr, eventArray[i].rc, eventArray[i].counter, eventArray[i].evExtData);
        }
        if (overflow == GT_TRUE)
        {
            /* Print dots if array of failed events has been overflowed*/
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("\n");
        }
    }
    else
        PRV_UTF_LOG0_MAC("All events have been generated successfully\n");
}

static GT_VOID prvTgfEventSip6LmuExtendedDataFill
(
    IN  GT_U8   devNum,
    OUT GT_U32* evExtDataArray,
    OUT GT_U32* evExtDataSize
)
{
    GT_U32 ii = 0;
    GT_U32 lmuUnit;
    GT_U32 tile;
    GT_U32 raven;
    GT_U32 index = 0;

    for (lmuUnit = 0; lmuUnit < 2; lmuUnit++)
    {
        for(tile = 0; tile < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tile++)
        {
            for(raven = 0; raven < FALCON_RAVENS_PER_TILE; raven++)
            {
                for (index = 0; index < 512; index++)
                {
                    if ((index % 128) == 0)
                    {
                        if (tile % 2 == 0)
                        {
                            evExtDataArray[ii++] = (((lmuUnit | raven << 1) | tile << 3) << 16) + index;
                        }
                        else
                        {
                            evExtDataArray[ii++] = (((lmuUnit | (3 - raven) << 1) | tile << 3) << 16) + index;
                        }
                    }
                }
            }
        }
    }

    *evExtDataSize = ii;
}

/**
* @internal prvTgfEventInterruptsCountGet function
* @endinternal
*
* @brief   This function is used to get the interrupts count for critical event.
* @param[in] devNum                   - device number
* @param[in] event                    - a unified event
* @param[in] uniEvName                - unified event name
*
* @param[out] expectedCounter          - (pointer to) expected counter for unified event
*                                       None
*/
static GT_VOID prvTgfEventInterruptsCountGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT event,               /* unified event */
    IN  GT_CHAR*              uniEvName,           /* unified event name */
    OUT GT_U32*               expectedCounter      /* expected counter for unified event */
)
{
    GT_U32                       nodesPoolSize; /* size of nodes pool */
    GT_U32                       count;         /* critical interrupts count */
    GT_U32                       eventItr;      /* event iterator */
    GT_U32                       portGroupId;   /* port group Id */
    PRV_CPSS_DRV_EV_REQ_NODE_STC node;          /*interrupt node*/

    count = 0;
    /*Get Size of Nodes Pool*/
    nodesPoolSize=PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numOfIntBits;

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        /*Find unified event by number or additional data and write bit of cause register*/
        for (eventItr = 0; eventItr < nodesPoolSize; eventItr++)
        {
            node=PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool[eventItr];
            if (node.uniEvCause != (GT_U32)event)
            {
                continue;
            }
            count++;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId);
    *expectedCounter = count;
    PRV_UTF_LOG2_MAC("prvTgfEventInterruptsCountGet - Event %s interrupts count [%d] \n", uniEvName, count);
}

/**
* @internal prvTgfBobcat2InitEvExtDataArray function
* @endinternal
*
* @brief   This function is used to fill evExtData
*         array for unified event.
* @param[in] devNum                   - device number
* @param[in] event                    - a unified event
* @param[in] uniEvName                - unified event name
*
* @param[out] evExtDataArray           - array of evExtData values
* @param[out] evExtDataSize            - size of evExtDataArray
* @param[out] expectedCounter          - (pointer to) expected counter for unified event
*                                       None
*/
static GT_VOID prvTgfBobcat2InitEvExtDataArray
(
    IN  GT_U8                 devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT event,               /* unified event */
    IN  GT_CHAR*              uniEvName,           /* unified event name */
    OUT GT_U32*               evExtDataArray,      /* array of evExtData values */
    OUT GT_U32*               evExtDataSize,       /* size of evExtDataArray */
    OUT GT_U32*               expectedCounter      /* expected counter for unified event */
)
{
    GT_U32 i = 0;
    GT_U32 numOfTxQPorts;
    GT_U32 numOfTxQDqUnits;
    GT_U32 numOfProcessingPipes;
    GT_U32 isPerTile;
    GT_U32 isPerMg,mgNumOfUnits;
    GT_U32 index;
    GT_U32 numPerPipe;
    GT_U32 isPerMac,numMacPorts;
    GT_U32 interruptsCount;
    GT_U32 maxDp;
    GT_U32 numOfTsu;
    GT_U32 numOfChannels;

    maxDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1)
    {
        numOfProcessingPipes = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;
    }
    else
    {
        numOfProcessingPipes = 1;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
    {
        numOfTxQDqUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq;
        numOfTxQPorts = numOfTxQDqUnits *
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
    }
    else
    {
        numOfTxQPorts = 72;
        numOfTxQDqUnits = 1;
    }

    *evExtDataSize = 1;

    switch (event)
    {
        case CPSS_PP_GPP_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(8);
            break;
        case CPSS_PP_TQ_MISC_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(5);
            break;
        case CPSS_PP_EB_NA_FIFO_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(8);
            break;
        case CPSS_PP_MAC_SFLOW_E:
            if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                PRV_TGF_FILL_EXT_DATA_ARRAY(128);
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PRV_TGF_FILL_EXT_DATA_ARRAY(512);
            }
            else {
                PRV_TGF_FILL_EXT_DATA_ARRAY(256);
            }
            break;

        case CPSS_PP_SCT_RATE_LIMITER_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(256);
            break;
        case CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        case CPSS_PP_PCL_LOOKUP_DATA_ERROR_E:
        case CPSS_PP_PCL_LOOKUP_FIFO_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(3);
            break;
        case CPSS_PP_PCL_ACTION_TRIGGERED_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(3);
            break;
        case CPSS_PP_EGRESS_SFLOW_E:
        case CPSS_PP_TQ_PORT_DESC_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(numOfTxQPorts);
            break;

        case CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E:
        case CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E:
        case CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E:
        case CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E:
        case CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E:
        case CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E:
        case CPSS_PP_TTI_ACCESS_DATA_ERROR_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(2);
            break;

        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            if (IS_BOBK_DEV_MAC(devNum))
            {
                index = 0;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TTI_SUM_CRITICAL_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TTI_SUM_CRITICAL_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_HA_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_HA_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MLL_SUM_FILE_ECC_1_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MLL_SUM_FILE_ECC_2_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E;
#ifndef ASIC_SIMULATION
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_0_GENENAL_SUM_ECC_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_0_GENENAL_SUM_ECC_DOUBLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_1_GENENAL_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_1_GENENAL_SUM_ECC_DOUBLE_ERROR_E;
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_0_GEBERAL1_SUM_ECC_0_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_0_GEBERAL1_SUM_ECC_1_SINGLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_1_GEBERAL1_SUM_ECC_0_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_1_GEBERAL1_SUM_ECC_1_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_0_GEN_SUM_ECC_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_0_GEN_SUM_ECC_DOUBLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_1_GEN_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_1_GEN_SUM_ECC_DOUBLE_ERROR_E;
#endif
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_IPLR0_DATA_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_IPLR1_SUM_DATA_ERROR_E;

                *evExtDataSize = index;
            }
            else
            {
                evExtDataArray[0] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_ONE_ERROR_E;
                evExtDataArray[1] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_TWO_ERROR_E;
                evExtDataArray[2] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_ONE_ERROR_E;
                evExtDataArray[3] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_TWO_ERROR_E;
                evExtDataArray[4] = (GT_U32)PRV_CPSS_BOBCAT2_TTI_SUM_CRITICAL_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[5] = (GT_U32)PRV_CPSS_BOBCAT2_TTI_SUM_CRITICAL_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[6] = (GT_U32)PRV_CPSS_BOBCAT2_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E;
                evExtDataArray[7] = (GT_U32)PRV_CPSS_BOBCAT2_IPLR0_DATA_ERROR_E;
                evExtDataArray[8] = (GT_U32)PRV_CPSS_BOBCAT2_HA_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[9] = (GT_U32)PRV_CPSS_BOBCAT2_HA_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[10] = (GT_U32)PRV_CPSS_BOBCAT2_MLL_SUM__FILE_ECC_1_ERROR_E;
                evExtDataArray[11] = (GT_U32)PRV_CPSS_BOBCAT2_MLL_SUM__FILE_ECC_2_ERROR_E;
                evExtDataArray[12] = (GT_32)PRV_CPSS_BOBCAT2_IPLR1_SUM_DATA_ERROR_E;
                evExtDataArray[13] = (GT_U32)PRV_CPSS_BOBCAT2_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[14] = (GT_U32)PRV_CPSS_BOBCAT2_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[15] = (GT_U32)PRV_CPSS_BOBCAT2_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[16] = (GT_U32)PRV_CPSS_BOBCAT2_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[17] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP1_COUNTERS_PARITY_ERR_E;
                evExtDataArray[18] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP2_COUNTERS_PARITY_ERR_E;
                evExtDataArray[19] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP3_COUNTERS_PARITY_ERR_E;
                evExtDataArray[20] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP4_COUNTERS_PARITY_ERR_E;
                evExtDataArray[21] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP5_COUNTERS_PARITY_ERR_E;
                evExtDataArray[22] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP6_COUNTERS_PARITY_ERR_E;
                evExtDataArray[23] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP7_COUNTERS_PARITY_ERR_E;
                evExtDataArray[24] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[25] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP1_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[26] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP2_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[27] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP3_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[28] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP4_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[29] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP5_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[30] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP6_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[31] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP7_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[32] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
                evExtDataArray[33] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[34] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[35] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[36] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[37] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[38] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[39] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[40] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[41] = (GT_U32)PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[42] = (GT_U32)PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[43] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E;
                evExtDataArray[44] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E;
                evExtDataArray[45] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E;
                evExtDataArray[46] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E;
                evExtDataArray[47] = (GT_U32)PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[48] = (GT_U32)PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
                evExtDataArray[49] = (GT_U32)PRV_CPSS_BOBCAT2_ETH_TXFIFO_GEN_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[50] = (GT_U32)PRV_CPSS_BOBCAT2_ETH_TXFIFO_GEN_SUM_ECC_DOUBLE_ERROR_E;
                *evExtDataSize = 51;
            }
            break;

        case CPSS_PP_TQ_TXQ2_FLUSH_PORT_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    *evExtDataSize = numOfTxQPorts;
                    PRV_TGF_FILL_EXT_DATA_ARRAY(*evExtDataSize);
                    break;
                default:
                    break;
            }
            break;

        case CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E:
        case CPSS_PP_PTP_TAI_GENERATION_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(2);
                    break;
                case CPSS_PP_FAMILY_DXCH_AC5X_E:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(23);
                    break;
                case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(34);
                    break;
                case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(22);
                    break;
                default:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(1);
                    break;
            }
            break;

        case CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E:
            prvTgfEventSip6LmuExtendedDataFill(devNum, evExtDataArray, evExtDataSize);
            break;
        case CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(numOfProcessingPipes*4*3);
            break;

        default:
            evExtDataArray[0] = CPSS_PARAM_NOT_USED_CNS;
            break;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && GT_FALSE == prvUtfIsGmCompilation())
    {
        /* per MG */
        mgNumOfUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.mg.sip6MgNumOfUnits;
    }
    else
    {
        mgNumOfUnits = 1;
    }

    isPerMg   = 0;
    isPerTile = 1;
    *expectedCounter = 1;
    isPerMac  = 0;
    switch (event)
    {
        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* NOTE: the interrupt is from the SERDES */
                /* it is not supported by HW nor by WM    */
                *expectedCounter = 0;
            }
            break;

        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* !!! NOTE: supported ONLY by the WM simulation !!!  */
                /* we use it to check the tree of CPSS and simulation */

                isPerMac  = 1;
                isPerTile = 0;

                *expectedCounter = 2; /* this event connected to 2 interrupts :
                    MAC : 'LINK_STATUS_CHANGE_E'
                    PCS : 'LPCS_LINK_STATUS_CHANGE_E'
                */
            }
            break;
        case CPSS_PP_PORT_LANE_FEC_ERROR_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* !!! NOTE: supported ONLY by the WM simulation !!!  */
                /* we use it to check the tree of CPSS and simulation */

                isPerMac  = 1;
                isPerTile = 0;

                *expectedCounter = 4; /* this event connected to 2 (CE/NCE) * 2 lanes interrupts:
                    for CE and NCE errors
                */
            }
            break;

        case CPSS_PP_PORT_RX_FIFO_OVERRUN_E:
        case CPSS_PP_PORT_TX_FIFO_UNDERRUN_E:
        case CPSS_PP_PORT_TX_FIFO_OVERRUN_E:
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:  /* in sip6 : bound to LINK_OK_CHANGE_E */
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* !!! NOTE: supported ONLY by the WM simulation !!!  */
                /* we use it to check the tree of CPSS and simulation */
                isPerMac  = 1;
                isPerTile = 0;
            }
            break;

        case CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E:
        case CPSS_PP_MISC_GENXS_READ_DMA_DONE_E:
        case CPSS_PP_EB_AUQ_FULL_E:
        case CPSS_PP_EB_AUQ_PENDING_E:
        case CPSS_PP_EB_AUQ_OVER_E:
        case CPSS_PP_EB_AUQ_ALMOST_FULL_E:
        case CPSS_PP_EB_FUQ_FULL_E:
        case CPSS_PP_EB_FUQ_PENDING_E:
        case CPSS_PP_MISC_TWSI_TIME_OUT_E:
        case CPSS_PP_MISC_TWSI_STATUS_E:
        case CPSS_PP_MISC_ILLEGAL_ADDR_E:
            if(mgNumOfUnits > 1)
            {
                /* this event comes from 4 MGs */
                isPerMg   = 1;
                isPerTile = 0;
            }
            break;

        case CPSS_PP_TQ_SNIFF_DESC_DROP_E:
           isPerTile = 0;/*per DQ*/
           *expectedCounter = numOfTxQDqUnits;
           break;
        case CPSS_PP_PCL_LOOKUP_DATA_ERROR_E:
            isPerTile = 0;/*per pipe*/
            *expectedCounter = 4 * numOfProcessingPipes;
            break;

        case CPSS_PP_PHA_E:
            isPerTile = 0;/*per pipe*/

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
               GT_FALSE == prvUtfIsGmCompilation())
            {
                numPerPipe = /* from PHA */
                             /*   PHA_INTERNAL_ERROR_BAD_ADDRESS_ERROR_E,            */
                             /*   PHA_INTERNAL_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,   */
                             /*   PHA_INTERNAL_ERROR_HEADER_SIZE_VIOLATION_E,        */
                             /*   PHA_INTERNAL_ERROR_PPA_CLOCK_DOWN_VIOLATION_E,     */
                             4 +

                             /* from PPA */
                             0 +

                             /* from PPG */
                             /*   _ERROR_BAD_ADDRESS_ERROR_E                          */
                             /*   _ERROR_TABLE_ACCESS_OVERLAP_ERROR_E                 */
                             2 * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg +

                             /* from PPN */
                             /*  _HOST_UNMAPPED_ACCESS_E                              */
                             /*  _CORE_UNMAPPED_ACCESS_E                              */
                             /*  _NEAR_EDGE_IMEM_ACCESS_E                             */
                             /*  _DOORBELL_INTERRUPT_E                                */
                             4 *(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn *
                                 PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg);

            }
            else
            {
                numPerPipe = 0;
            }

            *expectedCounter = numPerPipe * numOfProcessingPipes;
            break;

        case CPSS_PP_PCL_LOOKUP_FIFO_FULL_E:
        case CPSS_PP_PCL_ACTION_TRIGGERED_E:
        case CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E:
        case CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E:
        case CPSS_PP_EB_SECURITY_BREACH_UPDATE_E:
        case CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E:
        case CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E:
        case CPSS_PP_SCT_RATE_LIMITER_E:
        case CPSS_PP_MAC_SFLOW_E:
        case CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E:
        case CPSS_PP_POLICER_DATA_ERR_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:

        case CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E:
        case CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E:
        case CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E:
        case CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E:
        case CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E:
        case CPSS_PP_TTI_ACCESS_DATA_ERROR_E:
            isPerTile = 0;/*per pipe*/
            *expectedCounter  = numOfProcessingPipes;
            break;

        case CPSS_PP_PORT_PTP_MIB_FRAGMENT_E:
            isPerMac = 1;
            break;

        case CPSS_PP_GTS_GLOBAL_FIFO_FULL_E:
        case CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E:
            isPerTile = 0;/*per pipe*/
            isPerMac = 1;
            *expectedCounter = 4 * numOfProcessingPipes;
            break;
        case CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E:
        case CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E:
            isPerTile = 0;
            break;
        case CPSS_PP_TQ_PORT_MICRO_BURST_E:
            isPerTile = 0;/*per pipe*/
            *expectedCounter = maxDp;
            break;
        case CPSS_PP_CRITICAL_HW_ERROR_E:
            interruptsCount = 0;
            prvTgfEventInterruptsCountGet(prvTgfDevNum, event, uniEvName, &interruptsCount);
            *expectedCounter = interruptsCount;
            break;
        case CPSS_PP_PORT_AN_HCD_FOUND_E:
        case CPSS_PP_PORT_AN_RESTART_E:
        case CPSS_PP_PORT_AN_PARALLEL_DETECT_E:
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E )
            {
                *expectedCounter = 12;
            }
            else
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E )
            {
                *expectedCounter = 20;
            }
            else
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E )
            {
                *expectedCounter = 6;/* on each of the first 6 USX units */
            }
            else
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )
            {
#ifdef ASIC_SIMULATION
                /* AC5P simulation does not supports these interrupts*/
                *expectedCounter = 0;
#else
                *expectedCounter = 34;
#endif
            }
            else
            {
                *expectedCounter = 0;
            }
            break;
        default:
            if(GT_TRUE == prvCpssDrvEventIsCpuSdmaPortPerQueue(devNum,event))
            {
                isPerTile = 0;/* event per queue */
            }
            break;
    }

    if(isPerTile && PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        *expectedCounter *= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    }

    if(isPerMg && (mgNumOfUnits >= 2))
    {
        *expectedCounter *= mgNumOfUnits;
    }

    if(isPerMac)
    {
        numMacPorts = 0;
        /* sum the number of MAC ports */
        for(i = 0 ; i < CPSS_MAX_PORTS_BMP_NUM_CNS ; i++)
        {
            if(PRV_CPSS_PP_MAC(devNum)->existingPorts.ports[i])
            {
                numMacPorts += prvCpssPpConfigBitmapNumBitsGet(PRV_CPSS_PP_MAC(devNum)->existingPorts.ports[i]);
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            switch (PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
                    numMacPorts = 80;
                    break;
                default:
                /* the cpss and simulation not supports yet the 'cpu netwrok ports' */
                numMacPorts = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            }
        }
        else
        /* AC3X generates equal to Aldrin number of PTP/MAC events */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
        {
            numMacPorts = 33;
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            numMacPorts = 80;
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            numMacPorts = 52;
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            numMacPorts = 20;
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            numMacPorts = 48;/* although the device hold 55 MACs the CPSS
                interrupts tree limited to ports 0..47
                till we have explicit Cider for it.
                */
        }
#ifndef ASIC_SIMULATION
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            /* Disconnected port(0,4,8,12,16,20) - 6 (3 MACs each)
             * numOfActiveMacs = numOfMacs - (6 * 3).
             * FWS tests changes one 100G to be 4 10G and this enables 4 MACs */
            numMacPorts -= ((prvUtf100GPortReconfiguredGet() == GT_FALSE) ? 18 : 15);
        }
#else
        else
        if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
        {
            /* CPU ethernet port is not powered UP in simulation */
            numMacPorts -= 1;
        }
#endif

        if (event == CPSS_PP_GTS_GLOBAL_FIFO_FULL_E || event == CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E)
        {
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                *expectedCounter += (numMacPorts * 2);
            }
            else
            {
                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
                {
                    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                            numOfTsu = 1;
                            numOfChannels = 64;
                            break;
                        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                            numOfTsu = 3;
                            numOfChannels = 32;
                            break;
                        case CPSS_PP_FAMILY_DXCH_AC5P_E:
                            numOfTsu = 4;
                            numOfChannels = 32;
                            break;
                        default: /* CPSS_PP_FAMILY_DXCH_AC5X_E: */
                            numOfTsu = 1;
                            numOfChannels = 64;
                    }
                    *expectedCounter += (numOfTsu * numOfChannels);
                }
            }
        }
        else
        {
            *expectedCounter *= numMacPorts;
        }

        switch (event)
        {
            case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
            case CPSS_PP_PORT_TX_FIFO_UNDERRUN_E:
            case CPSS_PP_PORT_TX_FIFO_OVERRUN_E:
            case CPSS_PP_PORT_RX_FIFO_OVERRUN_E:
                if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    /* add segmented ports interrupts and CPU ports */
                    *expectedCounter += PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts +
                                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts;
                }
                break;
            default:
                break;
        }
    }
}

/**
* @internal prvTgfCheckEventCounters function
* @endinternal
*
* @brief   This function checks event counter with
*         the expected value
* @param[in] eventCounter             - event counter
* @param[in] evExpectedCounter        - expected event counter
*
* @retval GT_TRUE                  - the event counter matches the expected value
* @retval GT_FALSE                 - the event counter doesn't match the expected value
*/
static GT_BOOL prvTgfCheckEventCounters
(
    GT_U32 eventCounter,            /* current value of event counter */
    GT_U32 evExpectedCounter         /* expected value of event counter */
)
{
    if (eventCounter == evExpectedCounter)
        return GT_TRUE;
    else
        return GT_FALSE;
}

/* flag to allow the 'MAC' portNumFrom that was not mapped to 'physical port' ,
   but still we want to get indication about it
*/
extern void drvEventExtDataConvert_allowNonMappedMac(IN GT_U32 allow);

/**
* @internal prvTgfGenEvent function
* @endinternal
*
* @brief   The body of prvTgfEventDeviceGenerate test
*/
GT_VOID prvTgfGenEvent(GT_VOID)
{
    GT_STATUS rc, rc1; /*CPSS returned codes */
    GT_U32 counter; /* counter for generated events */
    CPSS_UNI_EV_CAUSE_ENT event;
    GT_U32 eventItr; /* event iterator */
    PRV_PRINT_EVENT_CTX failedEvents[PRV_TGF_FAILED_EVENTS_ARRAY_SIZE];

    /*Array of failed events. Used for debugging*/

    GT_BOOL overflow = GT_FALSE; /* Flag that indicates overflowing
                                  of array of failed events */
    GT_U32 failedEventsCnt = 0; /* counter of failed events */

    /* Array of disabled by default events */
    CPSS_UNI_EV_CAUSE_ENT unsupportedEvents[] = PRV_TGF_UNSUPPORTED_EVENTS;
    CPSS_UNI_EV_CAUSE_ENT unsupportedFalconEvents[] = PRV_TGF_SIP6_UNSUPPORTED_EVENTS;

#ifdef ASIC_SIMULATION
    CPSS_UNI_EV_CAUSE_ENT wm_emulator_unsupportedEvents[] = {CPSS_PP_PORT_LINK_STATUS_CHANGED_E};
    GT_U32  wm_emulator_unsupportedEvents_size =  sizeof(wm_emulator_unsupportedEvents)/sizeof(wm_emulator_unsupportedEvents[0]);
#endif /*ASIC_SIMULATION*/
    /* Array of unsupported events */
    GT_U32  disabledEventsSize;      /* Size of disabledEvents array */
    GT_U32  unsupportedEventsSize;   /* Size of unsupportedEvents array */
    GT_U32 evExtDataItr;            /* Iterator for evExtDataArray */
    static GT_U32 evExtDataArray[576];     /* array of evExtData values */
    GT_U32 evExtDataSize;
    GT_U32 evExpectedCounter;
    GT_U32  iterator,lastValue;
    #ifdef ASIC_SIMULATION
    GT_U32 numTimesNoChange;/* number of times that counter not changed */
    #endif /*ASIC_SIMULATION*/
    GT_BOOL portMng;
    GT_U32 forceUnmask;
    CPSS_UNI_EV_CAUSE_ENT disabledEvents[] = PRV_TGF_DISABLED_BY_DEFAULT_EVENTS;
                                 /* Array of disabled events */

    char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
    /* Array that contains string names of unified events */

    disabledEventsSize=sizeof(disabledEvents)/sizeof(disabledEvents[0]);
    unsupportedEventsSize= PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) ?
                           sizeof(unsupportedFalconEvents)/sizeof(unsupportedFalconEvents[0]) :
                           sizeof(unsupportedEvents)/sizeof(unsupportedEvents[0]);

    /* call the CPSS to enable those interrupts in the HW of the device */
    rc = prvWrAppEventsToTestsHandlerBind(disabledEvents, disabledEventsSize,
                                         CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvWrAppEventsToTestsHandlerBind");
    }

    /* enable the support for MACs that are not mapped */
    drvEventExtDataConvert_allowNonMappedMac(1);

    /* Do the generation of all supported events */
    for (eventItr = (GT_U32)CPSS_PP_UNI_EV_MIN_E;
            eventItr <= (GT_U32)CPSS_PP_UNI_EV_MAX_E;
            eventItr++)
    {
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            /* BC2 B0 does not support GPP events */
            if (eventItr == (GT_U32)CPSS_PP_GPP_E)
                continue;
        }

        if ((eventItr == (GT_U32)CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E) ||
            (eventItr == (GT_U32)CPSS_PP_PTP_TAI_GENERATION_E))
        {
#ifndef ASIC_SIMULATION
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                /* TAI interrupts supported starting from BobK */
                continue;
            }
#else
            /* Simulation of TAI interrupts is not supported */
            continue;
#endif
        }

#ifdef ASIC_SIMULATION

        if ((eventItr == (GT_U32)CPSS_PP_TQ_PORT_MICRO_BURST_E)||
            (eventItr == (GT_U32)CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E)||
            (eventItr == (GT_U32)CPSS_PP_CRITICAL_HW_ERROR_E))
        {
             /* Simulation of Micro burst,headroom and critical interrupts is not supported */
            continue;
        }
#endif

        event = (CPSS_UNI_EV_CAUSE_ENT)eventItr;

        rc = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : cpssDxChPortManagerEnableGet FAILED: rc==%d", rc);

        if (event == CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E)
        {
            if (portMng == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("prvTgfGenEvent - CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E event restricted in port manager");
                continue;
            }
        }

        PM_TBD
        /*Does not work in PM mode*/
        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if (portMng == GT_TRUE)
            {
                switch(event)
                {
                    case CPSS_PP_GTS_GLOBAL_FIFO_FULL_E:
                    case CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E:
                    case CPSS_PP_PORT_PTP_MIB_FRAGMENT_E:
                     continue;
                     break;
                    default:
                        break;
                }

            }
        }
        /* Skip unsupported events */
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            PRV_TGF_SKIP_EVENTS(prvTgfDevNum, event, unsupportedFalconEvents, unsupportedEventsSize);
        }
        else
        {
            PRV_TGF_SKIP_EVENTS(prvTgfDevNum, event, unsupportedEvents, unsupportedEventsSize);
        }

#ifdef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            /* the appdemo for WM on emulator disabled those events , so it will not generate interrupts */
            /* see code in falcon_EventHandlerInit(...) */
            PRV_TGF_SKIP_EVENTS(prvTgfDevNum, event, wm_emulator_unsupportedEvents, wm_emulator_unsupportedEvents_size);
        }
#endif /*ASIC_SIMULATION*/
        /* get array of extData for current event */
        prvTgfBobcat2InitEvExtDataArray(prvTgfDevNum, event, uniEvName[eventItr], evExtDataArray,
                                        &evExtDataSize, &evExpectedCounter);

        forceUnmask = 0;
#ifdef ASIC_SIMULATION
        if(CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E == event)
        {
            /* enable the interrupt , because was not enabled during initialization */
            forceUnmask = 1;
        }
#endif /*ASIC_SIMULATION*/
        if(CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E == event)
        {
            /* enable the interrupt , because was not enabled during initialization */
            forceUnmask = 1;
        }

        if((CPSS_PP_PORT_LINK_STATUS_CHANGED_E == event) && PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            /* enable the interrupt , because may be disabled during other tests like PCL WFS */
            forceUnmask = 1;
        }

        if(forceUnmask)
        {
            /* enable the interrupt , because was not enabled during initialization */
            (void)cpssEventDeviceMaskSet(prvTgfDevNum, event, CPSS_EVENT_UNMASK_E);
        }

        for (evExtDataItr = 0; evExtDataItr < evExtDataSize; evExtDataItr++)
        {

            /* clean events counter */
            rc = utfGenEventCounterGet(prvTgfDevNum,event,GT_TRUE,NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet");

            /* generate event */
            rc = cpssEventDeviceGenerate(prvTgfDevNum,event,evExtDataArray[evExtDataItr]);
            rc1 = rc;
            /* Exclude failing test on GT_NOT_SUPPORTED and GT_NOT_FOUND codes */
            if (rc !=  GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssEventDeviceGenerate");

            if(cpssDeviceRunCheck_onEmulator())
            {
                cpssOsTimerWkAfter(50);
            }
            else
            {
                cpssOsTimerWkAfter(15);
            }

            counter = 0;
            iterator = 0;
#ifdef ASIC_SIMULATION
            numTimesNoChange = 0;
#endif /*ASIC_SIMULATION*/
            do{
                lastValue = counter;
                /* get events counter */
                rc=utfGenEventCounterGet(prvTgfDevNum,event,GT_FALSE,&counter);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet");

                if(rc1 != GT_OK)
                {
                    /* no sleep needed */
                    break;
                }

                if(counter < evExpectedCounter)
                {
                    cpssOsTimerWkAfter(20);
                }
                else
                {
                    break;
                }

                #ifdef ASIC_SIMULATION
                if(numTimesNoChange < 5 && (counter < evExpectedCounter)) /* allow big sleep up to 5 times */
                {
                    if(numTimesNoChange >= 1)
                    {
                        cpssOsPrintf("Event: %s, got: %d out of [%d] \n",uniEvName[eventItr], counter, evExpectedCounter);
                    }
                    numTimesNoChange ++;
                    cpssOsTimerWkAfter(1000);
                    continue;
                }

                /*numTimesNoChange = 0;*/
                #endif /*ASIC_SIMULATION*/
            }
            while(((counter == 0) || (lastValue < counter)) && (iterator++ < evExpectedCounter));/* counter keep on growing */


            /*cpssOsPrintf("Event: %s, counter: %d\n", uniEvName[eventItr], counter);*/

            if (prvTgfCheckEventCounters(counter, evExpectedCounter) == GT_FALSE &&
                    rc1 != GT_NOT_SUPPORTED &&
                    rc1 != GT_NOT_FOUND)
            {

                UTF_VERIFY_EQUAL5_STRING_MAC(
                    GT_TRUE,GT_FALSE, "\nFailed to generate event %s[%d] index[%d] ,counter [%d], evExpectedCounter [%d] \n",
                    uniEvName[eventItr], evExtDataArray[evExtDataItr] ,evExtDataItr, counter, evExpectedCounter);

                /* Filling in PRV_PRINT_EVENT_CTX structure*/
                if (failedEventsCnt < PRV_TGF_FAILED_EVENTS_ARRAY_SIZE)
                {
                    failedEvents[failedEventsCnt].event = eventItr;
                    failedEvents[failedEventsCnt].counter = counter;
                    failedEvents[failedEventsCnt].rc = rc1;
                    failedEvents[failedEventsCnt].evExtData = evExtDataArray[evExtDataItr];
                    failedEventsCnt++;
                }
                else
                    overflow = GT_TRUE;
            }
        }

        if(forceUnmask)
        {
            /* restore the mask on those we forced to unmask */
            (void)cpssEventDeviceMaskSet(prvTgfDevNum, event, CPSS_EVENT_MASK_E);
        }
    }

    /* restore (disable) the support for MACs that are not mapped */
    drvEventExtDataConvert_allowNonMappedMac(0);

    /* Mask events disabled by default */
    rc = prvWrAppEventsToTestsHandlerBind(disabledEvents, disabledEventsSize, CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvWrAppEventsToTestsHandlerBind");

    /* restore eventHandler task */
    rc = prvWrAppEventHandlerTaskRestore(disabledEvents,disabledEventsSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvWrAppEventHandlerTaskRestore");

    /* Print all failed events */
    prvTgfPrintFailedEvents(failedEvents, failedEventsCnt, uniEvName, overflow);
    return;
}

static GT_U32 prvTgPortEventReceptorLastEvExtData = 0xFFFFFFFF;
static CPSS_OS_SIG_SEM prvTgPortEventReceivedSem = CPSS_OS_SEMB_EMPTY_E;
static CPSS_OS_SIG_SEM prvTgPortEventReceivedAckSem = CPSS_OS_SEMB_FULL_E;

static GT_VOID prvTgPortEventsReceptor
(
        IN GT_U8    devNum,
        IN GT_U32   uniEv,
        IN GT_U32   evExtData
)
{
    if (CPSS_PP_PORT_LINK_STATUS_CHANGED_E == uniEv) {
        PRV_UTF_LOG2_MAC("SIGNAL STATE CHANGE for dev=%d port=%d\n", devNum, evExtData);
        cpssOsSigSemWait(prvTgPortEventReceivedAckSem, 10); /* wait but not a long time */
        prvTgPortEventReceptorLastEvExtData = evExtData;
        cpssOsSigSemSignal(prvTgPortEventReceivedSem); /* wake thread waiting for evt */
    } else {
        /* Do nothing for other events */
    }
}


static GT_STATUS prvTgfPhyToMacPortMappingWaitForEventOnPort(
    IN GT_PHYSICAL_PORT_NUM phyPort,
    IN GT_U32    forceLinkValue
)
{
    GT_STATUS st;

    while (1)
    {
        st = cpssOsSigSemWait(prvTgPortEventReceivedSem, 5000); /* wait 5 sec */
        UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st,"got no interrupt on port[%d] for link[%d] prvTgPortEventReceptorLastEvExtData[%d]",
            phyPort, forceLinkValue , prvTgPortEventReceptorLastEvExtData);
        if (GT_OK != st)
        {
            break;
        }
        if (phyPort != prvTgPortEventReceptorLastEvExtData) /* compare received value with expected value*/
        {
            cpssOsSigSemSignal(prvTgPortEventReceivedAckSem); /* Acknowledge reception - not expected value */
            continue; /* wait till correct port number arrives */
        } else
        {
            prvTgPortEventReceptorLastEvExtData = 0xFFFFFFFF; /* reset to known BAD value */
            cpssOsSigSemSignal(prvTgPortEventReceivedAckSem); /* Acknowledge reception - expected value */
            break;
        }
    }

    return st;
}

static GT_STATUS prvTgfPhyToMacPortMappingInit(GT_U8 devNum, int *portCnt, CPSS_DXCH_PORT_MAP_STC **portArray)
{
    GT_STATUS st = GT_FALSE;
    GT_U32    phyPortsMaxNum;
    GT_PHYSICAL_PORT_NUM        currentPort;
    GT_U32                      idx = 0;
    GT_BOOL                     isExtendedCascadePort;
    CPSS_DXCH_PORT_MAP_STC      *portMapArray = NULL;
    CPSS_PORT_INTERFACE_MODE_ENT phyPortIfConfig;
    CPSS_PORT_SPEED_ENT         phyPortSpeed;
    CPSS_DXCH_DETAILED_PORT_MAP_STC detailedMap;

    st = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, &phyPortsMaxNum);
    PRV_UTF_LOG1_MAC("total valid ports %d\n",phyPortsMaxNum);
    portMapArray = (CPSS_DXCH_PORT_MAP_STC  *)cpssOsMalloc(phyPortsMaxNum * sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (NULL == portMapArray)
    {
            PRV_UTF_LOG0_DEBUG_MAC("no mem\n");
            st = GT_FAIL;
            goto exit_freemem;
    }

    currentPort = 0;
    while (currentPort < phyPortsMaxNum)
    {
        st = cpssDxChPortPhysicalPortDetailedMapGet(devNum, currentPort, &detailedMap);
        if((GT_OK != st) || (detailedMap.valid!= GT_TRUE))
        {
            currentPort++;
            continue;
        }
        isExtendedCascadePort = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[currentPort].portMap.isExtendedCascadePort;
        if(isExtendedCascadePort != GT_FALSE)
        {
            /* Skip extended cascade ports */
            currentPort++;
            continue;
        }
        st = cpssDxChPortPhysicalPortMapGet(devNum, currentPort, 1, &portMapArray[idx]);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }

        if (GT_TRUE == portMapArray[idx].tmEnable  && (GT_32)portMapArray[idx].tmPortInd < -1)
        {
            portMapArray[idx].tmPortInd =  (GT_U32) -1; /* Get often return junk here set to -1 if its < -1 */
        }

        if (CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E !=  portMapArray[idx].mappingType)
        {
            currentPort++;
            continue;
        }

        st = cpssDxChPortInterfaceModeGet(devNum, currentPort, &phyPortIfConfig);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }

        st = cpssDxChPortSpeedGet(devNum , currentPort, &phyPortSpeed);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }
        if((CPSS_PORT_INTERFACE_MODE_NA_E == phyPortIfConfig) ||
                (CPSS_PORT_SPEED_NA_E == phyPortSpeed)) {
            currentPort++;
            continue; /* SKIP port with not initialized speed and/or mode */
        }

        PRV_UTF_LOG1_MAC("disable PASS for port=%d\n",currentPort);
        if (tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, currentPort) == GT_FALSE)
        {
            st = cpssDxChPortForceLinkPassEnableSet(devNum, currentPort, GT_FALSE);
            if(GT_OK != st)
            {
                goto exit_freemem;
            }
            PRV_UTF_LOG1_MAC("disable DOWN for port=%d\n",currentPort);
            st = cpssDxChPortForceLinkDownEnableSet(devNum, currentPort, GT_FALSE );
            if(GT_OK != st)
            {
                goto exit_freemem;
            }
        }
        else
        {
            st = tgfTrafficGeneratorPortForceLinkWa(devNum, currentPort, GT_FALSE);
            if (st != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : tgfTrafficGeneratorPortForceLinkWa - port[%d]\n",
                                 currentPort);
                goto exit_freemem;
            }
        }

        currentPort++;
        idx++;
    }

    /* provide time for events handlers after Force Link change manipulations.*/
    cpssOsTimerWkAfter(1000);

    st = cpssOsSigSemBinCreate("utfPortMapEvSem", CPSS_OS_SEMB_EMPTY_E, &prvTgPortEventReceivedSem);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if (GT_OK != st)
    {
        goto exit_freemem;
    }

    st = cpssOsSigSemBinCreate("utfPortMapEvAckSem", CPSS_OS_SEMB_FULL_E, &prvTgPortEventReceivedAckSem);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if (GT_OK != st)
    {
        (void)cpssOsSigSemDelete(prvTgPortEventReceivedSem);
        goto exit_freemem;
    }

    *portArray = portMapArray;
    *portCnt = idx;

    notifyEventArrivedFunc = prvTgPortEventsReceptor;   /* set cbk  */

    return st;   /* Normal return */

exit_freemem:
    if (portMapArray)
    {
        cpssOsFree(portMapArray);
    }

    return st;
}

static GT_STATUS prvTgfPhyToMacPortMappingProcess(GT_U8 devNum, int portCnt, CPSS_DXCH_PORT_MAP_STC *portArray)
{
    GT_STATUS                   st = GT_FAIL;
    GT_PHYSICAL_PORT_NUM        currentPort;
    int                         i;
    GT_BOOL                     islinkisUp;
    GT_U32                      forceLinkValue;
#ifndef ASIC_SIMULATION
    GT_BOOL                     portMgr = prvUtfIsPortManagerUsed();
#endif

    for (i= 0; i< portCnt; i++)
    {
        currentPort = portArray[i].physicalPortNumber;

        PRV_UTF_LOG0_MAC("----------------------------------------\n");
        st = cpssDxChPortLinkStatusGet(devNum, currentPort, &islinkisUp );
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if (GT_OK != st)
        {
            PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
            goto unset_ev_cbk;
        }

        for(forceLinkValue = 1 ; /* no criteria */ ; /* no modify*/ )
        {
            if (tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, currentPort) == GT_FALSE)
            {
                if (GT_FALSE == islinkisUp)
                {
                    PRV_UTF_LOG1_MAC("link is DOWN  FORCE port=%d to UP\n", currentPort);
                    st = cpssDxChPortForceLinkPassEnableSet(devNum, currentPort, forceLinkValue ? GT_TRUE : GT_FALSE );
                    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                        goto unset_ev_cbk;
                    }
                }
                else
                {
                    PRV_UTF_LOG1_MAC("link is UP    FORCE port=%d to DOWN\n", currentPort);
                    st = cpssDxChPortForceLinkDownEnableSet(devNum, currentPort, forceLinkValue ? GT_TRUE : GT_FALSE );
                    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                        goto unset_ev_cbk;
                    }
                }
            }
            else
            {
    #ifndef ASIC_SIMULATION
                if (portMgr)
                {
                    /* need to call cpssDxChSamplePortManagerLoopbackSet */
                    st = cpssDxChSamplePortManagerLoopbackSet(
                            devNum,currentPort,CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E, GT_FALSE,
                            (forceLinkValue ? CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E :
                                              CPSS_PORT_SERDES_LOOPBACK_DISABLE_E),
                            GT_FALSE);
                    if(st != GT_OK)
                    {
                        PRV_UTF_LOG1_MAC("FAIL : cpssDxChSamplePortManagerLoopbackSet - port[%d]\n",
                                         currentPort);
                        goto unset_ev_cbk;
                    }
                }
                else
                {
                    /* "force link up/down WA" does not generates link change interrupt on HW.
                       Use SERDES loopback to get interrupt. */
                    st = cpssDxChPortSerdesLoopbackModeSet(devNum, currentPort, 0xFFFFFFFF, forceLinkValue ? CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E : CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E);
                    if (st != GT_OK)
                    {
                        PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSerdesLoopbackModeSet - port[%d]\n",
                                         currentPort);
                        goto unset_ev_cbk;
                    }
                }

                if (forceLinkValue)
                {
                    /* need to execute SERDES RX Training to get stable link */
                    st = cpssDxChPortSerdesAutoTune(devNum, currentPort,CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E);
                    if (st != GT_OK)
                    {
                        PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSerdesAutoTune - port[%d]\n",
                                         currentPort);
                        /* ignore failure - link may be OK */
                        st = GT_OK;
                    }
                }
    #else
                st = tgfTrafficGeneratorPortForceLinkWa(devNum, currentPort, forceLinkValue ? GT_TRUE : GT_FALSE);
                if (st != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("FAIL : tgfTrafficGeneratorPortForceLinkWa - port[%d]\n",
                                     currentPort);
                    goto unset_ev_cbk;
                }
    #endif
            }
            /*PRV_UTF_LOG2_MAC("wait evt1 LINE %d %s\n", __LINE__ , __FILE__);*/
            st = prvTgfPhyToMacPortMappingWaitForEventOnPort(currentPort,forceLinkValue);
            if (GT_OK != st)
            {
                UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
                PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                goto unset_ev_cbk;
            }

            if(forceLinkValue == 1)/* we forced link to up if was down or forced link to down if was up */
            {
                /* state to forced link opposite from last time */
                forceLinkValue = 0;

#ifdef ASIC_SIMULATION
                cpssOsTimerWkAfter(50);/* allow the event handler to finish restoring the mask register */
#endif /*ASIC_SIMULATION*/
            }
            else /* forceLinkValue == 0 */
            {
                break;
            }
        }/* loop on forceLinkValue */
    }

unset_ev_cbk:

    return st;

}

static GT_VOID prvTgfPhyToMacPortMappingCleanup(CPSS_DXCH_PORT_MAP_STC *portArray)
{
    GT_STATUS st;

    notifyEventArrivedFunc = NULL; /* remove cbk after use */

    st = cpssOsSigSemDelete(prvTgPortEventReceivedSem);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    st = cpssOsSigSemDelete(prvTgPortEventReceivedAckSem);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    if (portArray)
    {
        cpssOsFree(portArray);
    }
    return;
}

GT_VOID prvTgfEventPortLinkStatusUnbindBind(GT_VOID)
{
    GT_STATUS rc;
    CPSS_UNI_EV_CAUSE_ENT evHndlrCauseAllArr[1] = {CPSS_PP_PORT_LINK_STATUS_CHANGED_E};

    PRV_CPSS_DRV_EVENT_HNDL_STC  *hnd;
    GT_U32      eventCount;
    GT_U32      i;

    /* get CPSS_PP_PORT_LINK_STATUS_CHANGED_E event handler */
    hnd = prvCpssDrvEvReqQUserHndlGet(CPSS_PP_PORT_LINK_STATUS_CHANGED_E);
    if (hnd == NULL)
    {
        /* should not happen */
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
        return;
    }

    /* wait event list is clear */
    i = 2000;
    eventCount = prvCpssDrvEvReqQBitmapGet((GT_UINTPTR)hnd, NULL, 0);
    while ((eventCount > 0) &&
            (--i > 0))
    {
        cpssOsTimerWkAfter(1);
        eventCount = prvCpssDrvEvReqQBitmapGet((GT_UINTPTR)hnd, NULL, 0);
    }
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, eventCount);

    /* unbind CPSS_PP_PORT_LINK_STATUS_CHANGED_E event */
    rc = cpssEventUnBind(evHndlrCauseAllArr, 1);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* clear event list  */
    hnd->evListPtr = NULL;

    /* bind CPSS_PP_PORT_LINK_STATUS_CHANGED_E event to the handler */
    rc = prvCpssDrvEvReqQUserHndlSet(CPSS_PP_PORT_LINK_STATUS_CHANGED_E, hnd);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

GT_VOID prvTgfPhyToMacPortMapping(GT_VOID)
{
    GT_STATUS                   st, st1;
    GT_U8                       devNum   = prvTgfDevNum;
    CPSS_DXCH_PORT_MAP_STC      *portMapArray = NULL;
    int                         portMapArrayCnt = 0;

#ifndef ASIC_SIMULATION
    PM_TBD
    /* test link change method conflicts with PM logic */
    if(GT_TRUE == prvUtfIsPortManagerUsed())
    {
        SKIP_TEST_MAC;
    }
#endif

    st = prvTgfPhyToMacPortMappingInit(devNum, &portMapArrayCnt, &portMapArray );
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    PRV_UTF_LOG2_MAC("cnt=%d ptr=%p\n",portMapArrayCnt, portMapArray);

    /* Event handler executes SERDES training procedures on Link UP.
       Need to disable it to avoid failure of link event. */
    prvWrAppDbEntryAdd("serdesTraining", 0);

    /* make sure that event is opened */
    st1 = cpssEventDeviceMaskSet(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, CPSS_EVENT_UNMASK_E);
    cpssOsTimerWkAfter(1000);

    st = prvTgfPhyToMacPortMappingProcess(devNum, portMapArrayCnt, portMapArray );

    prvTgfPhyToMacPortMappingCleanup(portMapArray);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st1,
        "cpssEventDeviceMaskWithEvExtDataSet");
}

/* Function checks current event to skip in array of skip events */
static GT_BOOL prvTgfGenSkipEventCheck
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    currentEvent,
    IN CPSS_UNI_EV_CAUSE_ENT    skipEventArray[],
    IN GT_U32                   skipEventArraySize
)
{
    GT_U32 ii;

    for (ii = 0; ii < skipEventArraySize; ii++)
    {
        if (currentEvent == skipEventArray[ii])
        {
            return GT_TRUE;
        }
    }
    /* Check additional events for SIP6 devices */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if ((currentEvent >= CPSS_PP_RX_BUFFER_QUEUE8_E  && currentEvent <= CPSS_PP_RX_ERR_QUEUE31_E) ||
            (currentEvent >= CPSS_PP_RX_BUFFER_QUEUE32_E && currentEvent <= CPSS_PP_RX_ERR_QUEUE127_E))
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/*****************************************************************/
/* L2 part of packet FWS */
static TGF_PACKET_L2_STC prvTgfPacketFWSL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataFWSArr[] = {
    0x12, 0x34, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
    0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
    0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadDataFWSArr),                       /* dataLength */
    prvTgfPayloadDataFWSArr                                /* dataPtr */
};

/* PARTS of packet FWS */
static TGF_PACKET_PART_STC prvTgfPacketFWSPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketFWSL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_FWS_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataFWSArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_FWS_CRC_LEN_CNS  PRV_TGF_PACKET_FWS_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET FWS to send */
static TGF_PACKET_STC prvTgfPacketFWSInfo = {
    PRV_TGF_PACKET_FWS_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketFWSPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketFWSPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* maximum coalescing period in nsec */
#define PRV_TGF_MAX_COALESCING_PERIOD_CNS 5242560

static GT_U32 periodConfig;

/* function to configure interrupt coalescing feature. */
GT_VOID prvTgfInterruptCoalescingConfig
(
    GT_BOOL config,
    GT_BOOL linkChange
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   enable;
    GT_U32    period;
    GT_BOOL   linkChangeOverride;
    static GT_BOOL   enableRestore = GT_FALSE;
    static GT_U32    periodRestore = 0;
    static GT_BOOL   linkChangeOverrideRestore = GT_TRUE;

    if (config == GT_TRUE)
    {
        /* store the default coalescing values */
        rc = cpssDxChHwInterruptCoalescingGet(prvTgfDevNum, &enableRestore, &periodRestore, &linkChangeOverrideRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChHwInterruptCoalescingSet: enable %d period linkChangeOverride %d",
                                     enableRestore, periodRestore, linkChangeOverrideRestore);
        enable = GT_TRUE;
        period = PRV_TGF_MAX_COALESCING_PERIOD_CNS;
        periodConfig = period;
        linkChangeOverride = linkChange;
        /* configure coalescing with maximum period */
        rc = cpssDxChHwInterruptCoalescingSet(prvTgfDevNum, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChHwInterruptCoalescingSet: enable %d period linkChangeOverride %d",
                                     enable, period, linkChangeOverride);
    }
    else
    {
        /* restore the default coalescing values */
        rc = cpssDxChHwInterruptCoalescingSet(prvTgfDevNum, enableRestore, periodRestore, linkChangeOverrideRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChHwInterruptCoalescingSet: enable %d period linkChangeOverride %d",
                                     enableRestore, periodRestore, linkChangeOverrideRestore);
    }
}

/**
* @internal prvTgfInterruptCoalescingWireSpeedTrafficGenerate function
* @endinternal
*
* @brief Run full-wire speed traffic test with and without coalescing behavior and
         observe the rate of occurence of event CPSS_PP_MAC_NA_NOT_LEARNED_E.
*/
GT_VOID prvTgfInterruptCoalescingWireSpeedTrafficGenerate
(
    GT_BOOL coalescingEnable
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_BRIDGE_INGRESS_CNTR_STC        ingressCntr;
    GT_U32    packetSize = PRV_TGF_PACKET_FWS_CRC_LEN_CNS; /* packet size */
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];       /* rate of packets per port */
    GT_U32    counter;               /* counter for generated events */
    static GT_U32    counterDefault; /* counter for generated events for default behavior */
    static GT_U32    counterCoalescing; /* counter for generated events with coalescing enabled */
    GT_U32    counterInterruptIntervalCount = 0; /* counter derived based on totalInterruptIntervalCount */
    GT_U32    totalInterruptIntervalCount; /* total interval count occurence in 1s based on coalescing period */

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    /* AUTODOC: get Bridge ingress counters: to reset it */
    rc = prvTgfBrgCntBridgeIngressCntrsGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E, &ingressCntr);

    /* AUTODOC: configure and generate FWS: */
    /* AUTODOC:   force link UP and configure maximal speed on all ports */
    /* AUTODOC:   create FDB and VLAN loop for each speed group */
    /* AUTODOC:   enable tail drop mode */
    /* AUTODOC:   configure WireSpeed mode */
    /* AUTODOC: for each speed group and port send packet with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:11 */
    rc = prvTgfFWSLoopConfigTrafficGenerate(PRV_TGF_VLANID_CNS, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", PRV_TGF_VLANID_CNS);

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* close not needed FDB interrupts to avoid interrupt storming because NA relearning */
    rc = cpssEventDeviceMaskSet(prvTgfDevNum, CPSS_PP_MAC_NA_NOT_LEARNED_E, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet");

    /* clean events counter */
    rc = utfGenEventCounterGet(prvTgfDevNum,CPSS_PP_MAC_NA_NOT_LEARNED_E,GT_TRUE,&counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %d\n", CPSS_PP_MAC_NA_NOT_LEARNED_E);

    PRV_UTF_LOG1_MAC("event CPSS_PP_MAC_NA_NOT_LEARNED_E count %d\n", counter);

    /* sleep for 1s for interrupts treatment to finish before rate measurement. */
    cpssOsTimerWkAfter(1000);

    /* get events counter */
    rc=utfGenEventCounterGet(prvTgfDevNum,CPSS_PP_MAC_NA_NOT_LEARNED_E,GT_FALSE,&counter);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %d count %d\n", CPSS_PP_MAC_NA_NOT_LEARNED_E, counter);
    cpssOsPrintf("event CPSS_PP_MAC_NA_NOT_LEARNED_E count %d\n", counter);

    if(coalescingEnable == GT_TRUE)
    {
        counterCoalescing = counter;
        /* we want to extrapolate the number of events which would have occured
         * per sec assuming the coalescing period configured interval of ~5ms.
         * so in 1s wait time we can have upto (1000/5) = 200 coalescing
         * periodic intervals and so the events can be computed as events
         * received during coalescing * 200 to match it to be less than without
         * coalescing.
         */
        totalInterruptIntervalCount = ((10^3) / (periodConfig / 10^6)/* in ms */);
        counterInterruptIntervalCount = totalInterruptIntervalCount * counterCoalescing;
    }
    else
    {
        counterDefault = counter;
    }
    /* compare the count of event CPSS_PP_MAC_NA_NOT_LEARNED_E. The event count for default behavior where
     * events are processed immediately must be greater than with coalescing where events are not triggered
     * till the configured coalescing period.
     */
    if(counterCoalescing != 0 &&
       counterDefault != 0 &&
       counterDefault < counterCoalescing &&
       counterInterruptIntervalCount < counterDefault)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FAIL, GT_OK, "CPSS_PP_MAC_NA_NOT_LEARNED_E event count with coalescing %d greater than during default case %d\n",
                                     counterCoalescing, counterDefault);
    }

    utfPrintKeepAlive();

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

}

/* Function to restore FWS */
GT_VOID prvTgfInterruptCoalescingWireSpeedConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: reset FWS test configuration: */
    /* AUTODOC:   disable forcing on all ports */
    /* AUTODOC:   restore default VLAN for all ports */
    /* AUTODOC:   clean VLAN table */
    /* AUTODOC:   restore FC mode */
    prvTgfFWSRestore();
}

#define PRV_TGF_PORT_NUM 4
#define PRV_TGF_COALESCING_MODE_EVENT_DIFF_NUM 2000000 /* 2msec must difference between with and without coalescing based events test */

GT_VOID prvTgfInterruptCoalescingForLinkChangeEvent
(
    GT_BOOL coalescingEnable
)
{
    GT_STATUS                   st, st1;
    GT_U8                       devNum   = 0;
    CPSS_DXCH_PORT_MAP_STC      *portMapArray = NULL;
    int                         portMapArrayCnt = 0;
    GT_U32                      counter;           /* counter for generated events */
    GT_U32       secondsStart, secondsEnd, nanoSecondsStart, nanoSecondsEnd, seconds, nanoSec = 0; /* time of init */
    /* time of default behavior with link change event processed immediately */
    static GT_U32 secDefault,nanoSecDefault;
    /* time of link change event after the configured coalescing period expires */
    static GT_U32 seclinkChangeOverride,nanoSeclinkChangeOverride;

    /* clean events counter */
    st = utfGenEventCounterGet(prvTgfDevNum,CPSS_PP_PORT_LINK_STATUS_CHANGED_E,GT_TRUE,&counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "utfGenEventCounterGet uniEv %d\n", CPSS_PP_PORT_LINK_STATUS_CHANGED_E);

    PRV_UTF_LOG1_MAC("event CPSS_PP_PORT_LINK_STATUS_CHANGED_E count %d\n", counter);

    st = prvTgfPhyToMacPortMappingInit(devNum, &portMapArrayCnt, &portMapArray );
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    PRV_UTF_LOG2_MAC("cnt=%d ptr=%p\n",portMapArrayCnt, portMapArray);

    /* Event handler executes SERDES training procedures on Link UP.
       Need to disable it to avoid failure of link event. */
    prvWrAppDbEntryAdd("serdesTraining", 0);

    /* make sure that event is opened */
    st1 = cpssEventDeviceMaskSet(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st1);

    cpssOsTimerWkAfter(1000);

    /* time stamp before the link state change event trigger */
    st = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssOsTimeRT\n");

    /* trigger and receive all the link state change events for all the valid physical ports */
    st = prvTgfPhyToMacPortMappingProcess(devNum, portMapArrayCnt, portMapArray);

    /* time stamp after receiving all the link state change events */
    st1 = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st1, "cpssOsTimeRT\n");

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    /* get events counter */
    st1=utfGenEventCounterGet(prvTgfDevNum,CPSS_PP_PORT_LINK_STATUS_CHANGED_E,GT_FALSE,&counter);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1, "utfGenEventCounterGet uniEv %d count %d\n", CPSS_PP_PORT_LINK_STATUS_CHANGED_E, counter);

    PRV_UTF_LOG1_MAC("event CPSS_PP_PORT_LINK_STATUS_CHANGED_E count %d\n", counter);

    prvTgfPhyToMacPortMappingCleanup(portMapArray);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    if(coalescingEnable == GT_TRUE)
    {
        seclinkChangeOverride = seconds;
        nanoSeclinkChangeOverride = nanoSec;
    }
    else
    {
        secDefault = seconds;
        nanoSecDefault = nanoSec;
    }

    PRV_UTF_LOG2_MAC("link state events time taken: %d sec., %d nanosec.\n", seconds, nanoSec);

    /* compare the time of the link state change event. The time taken for default behavior where
     * interrupt is proceesed immediately should be less than with the coalescing period configfured.
     */
    if(seclinkChangeOverride == secDefault &&
       nanoSecDefault != 0 &&
       nanoSeclinkChangeOverride != 0 &&
       /* link state events based test (with and without coalescing) must differ by 2msec. */
       nanoSeclinkChangeOverride - nanoSecDefault < PRV_TGF_COALESCING_MODE_EVENT_DIFF_NUM)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FAIL, GT_OK, "link state change event default time %d greater than with coalescing time period %d\n",
                                     nanoSecDefault, nanoSeclinkChangeOverride);
    }
}

