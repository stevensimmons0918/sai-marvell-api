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
* @file prvCpssDxChPortDpDebug.c
*
* @brief CPSS implementation for Rx/TX Debug dumps for Falcon.
*
* @version  1
********************************************************************************
*/


#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* file contains many prints, all of them use this macro */
/* that can easy be redefined                            */
#define PR cpssOsPrintf
#define RX_DMA_DEBUG_REG_ADDR_OFF(_unit, _path) \
        (GT_U32)((char*)&(((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC*)0)->sip6_rxDMA[_unit].debug._path) \
        - (char*)0)
#define RX_DMA_UNIT_REG_ADDR_OFF(_unit) \
        (GT_U32)((char*)&(((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC*)0)->sip6_rxDMA[_unit]) - (char*)0)

static GT_STATUS regRead
(
    IN  const  char* regName,
    IN  GT_U32 txIndex,  /* only for printed messages */
    IN  GT_U8  devNum,
    IN  GT_U32 regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_STATUS rc;
    if (regAddr & 3) /* bad register address - not initialized */
    {
        PR(
            "%s txIndex %d register address not initialized\n",
            regName, txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        PR("regRead failed, regAddr %08X, rc %d\n", regAddr, rc);
    }
    return rc;
}

static GT_STATUS regReadPerChannel
(
    IN  const  char* regName,
    IN  GT_U32 txIndex,    /* only for printed messages */
    IN  GT_U32 txChannel,  /* only for printed messages */
    IN  GT_U8  devNum,
    IN  GT_U32 regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_STATUS rc;
    if (regAddr & 3) /* bad register address - not initialized */
    {
        PR(
            "%s txChannel %d register address not initialized\n",
            regName, txIndex, txChannel);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        PR("regRead failed, regAddr %08X, rc %d\n", regAddr, rc);
    }
    return rc;
}

static GT_STATUS regWriteMask
(
    IN  const  char* regName,
    IN  GT_U32 txIndex,  /* only for printed messages */
    IN  GT_U8  devNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 mask,
    IN  GT_U32 value
)
{
    GT_STATUS rc;
    if (regAddr & 3) /* bad register address - not initialized */
    {
        PR(
            "%s txIndex %d register address not initialized\n",
            regName, txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, mask, value);
    if (rc != GT_OK)
    {
        PR("regWriteMask failed, regAddr %08X, rc %d\n", regAddr, rc);
    }
    return rc;
}

static GT_STATUS regTileUnitRead
(
    IN  const  char* regName,
    IN  GT_U32 tileIndex,
    IN  GT_U32 unitIndex, /* only for printed messages */
    IN  GT_U8  devNum,
    IN  GT_U32 regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  tileBase = (devPtr->multiPipe.tileOffset * tileIndex);
    GT_U32  exactRegAddr = (tileBase + regAddr);

    if (regAddr & 3) /* bad register address - not initialized */
    {
        PR(
            "%s tile %d unitIndex %d register address not initialized\n",
            regName, tileIndex, unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, exactRegAddr, dataPtr);
    if (rc != GT_OK)
    {
        PR("regTileUnitRead failed, portGroupId 0 regAddr %08X, rc %d\n", regAddr, rc);
    }
    return rc;
}

static GT_STATUS regTileUnitWriteMask
(
    IN  const  char* regName,
    IN  GT_U32 tileIndex,
    IN  GT_U32 unitIndex,  /* only for printed messages */
    IN  GT_U8  devNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 mask,
    IN  GT_U32 value
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  tileBase = (devPtr->multiPipe.tileOffset * tileIndex);
    GT_U32  exactRegAddr = (tileBase + regAddr);

    if (regAddr & 3) /* bad register address - not initialized */
    {
        PR(
            "%s tile %d unitIndex %d register address not initialized\n",
            regName, tileIndex, unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, exactRegAddr, mask, value);
    if (rc != GT_OK)
    {
        PR("regTileUnitWriteMask failed, portGroupId 0 regAddr %08X, rc %d\n", regAddr, rc);
    }
    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugChannelStatusPrint function
* @endinternal
*
* @brief   Print TxDma Channel status.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
* @param[in] txChannel                - TX channel index inside TX DMA unit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaChannelStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_U32               txChannel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              data0;
    GT_U32                              data1;
    GT_U32                              bitShift;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txChannel >= PRV_CPSS_MAX_DMA_NUM_CNS)
    {
        PR("txChannel %d out of range\n", txChannel);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.sdqCredit[txChannel];
    rc = regReadPerChannel("sdqCredit", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        data0 = (data & 0xFFFFF);
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            data1 = ((data >> 22) & 0x3FF);
        }
        else
        {
            data1 = ((data >> 20) & 0x3F);
        }
        PR("SDQ Credits 0x%X SDQ Bytes 0x%X\n", data1, data0);
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.txfDescriptorCredits[txChannel];
    rc = regReadPerChannel("txfDescriptorCredits", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Max Used Desc Credits 0x%X Used Desc Credits 0x%X\n",
            ((data >> 16) & 0x1FF), (data & 0x1FF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.txfCellCredits[txChannel];
    rc = regReadPerChannel("txfCellCredits", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Max Used Cell Credits 0x%X Used Cell Credits 0x%X\n",
            ((data >> 16) & 0x3FF), (data & 0x3FF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.txfWordCredits[txChannel];
    rc = regReadPerChannel("txfWordCredits", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Max Used Word Credits 0x%X Used Word Credits 0x%X\n",
            ((data >> 16) & 0x7FF), (data & 0x7FF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.pbCredits[txChannel];
    rc = regReadPerChannel("pbCredits", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "PB Descriptor Credits 0x%X PB Cell Credits 0x%X\n",
            ((data >> 16) & 0x3F), (data & 0x1FF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.txdInternalState[txChannel];
    rc = regReadPerChannel("txdInternalState", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "txd_channel_is_busy %d txd_desc_fifo_holds_data %d\n",
            ((data >> 1) & 1), (data & 1));
    }

    bitShift = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        bitShift = 1;
    }
    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.channelStatus.txdFaultEvents[txChannel];
    rc = regReadPerChannel("txdFaultEvents", txIndex, txChannel, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("no_descriptors %d ", ((data >> (bitShift + 14)) & 1));
        PR("no_inter_packet_cell_tokens %d ", ((data >> (bitShift + 13)) & 1));
        PR("no_inter_packet_tokens %d ", ((data >> (bitShift + 12)) & 1));
        PR("\n");
        PR("no_pb_desc_credits %d ", ((data >> (bitShift + 11)) & 1));
        PR("no_inner_packet_tokens %d ", ((data >> (bitShift + 10)) & 1));
        PR("no_txf_resources_for_request %d ", ((data >> (bitShift + 9)) & 1));
        PR("\n");
        PR("no_txf_resources_for_packet %d ", ((data >> (bitShift + 8)) & 1));
        PR("no_pb_cell_credits %d ", ((data >> (bitShift + 7)) & 1));
        PR("no_empty_tags %d ", ((data >> (bitShift + 6)) & 1));
        PR("\n");
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("sdq_bytes_underflow %d ", ((data >> 6) & 1));
        }
        PR("sdq_credits_underflow %d ", ((data >> 5) & 1));
        PR("txf_word_credits_underflow %d ", ((data >> 4) & 1));
        PR("txf_cell_credits_underflow %d ", ((data >> 3) & 1));
        PR("\n");
        PR("txf_desc_credits_underflow %d ", ((data >> 2) & 1));
        PR("pb_cell_credit_overflow %d ", ((data >> 1) & 1));
        PR("pb_desc_credit_overflow %d ", (data & 1));
        PR("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaEventCountersPrint function
* @endinternal
*
* @brief   Print TxDma Event counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaEventCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.descriptorRequests;
    rc = regRead("descriptorRequests", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Desc Request Counter 0x%X\n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.pdsDescriptors;
    rc = regRead("pdsDescriptors", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "PDS Real Descriptors Counter 0x%X PDS Total Descriptors Counter 0x%X\n",
            ((data >> 16) & 0xFFFF), (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.txfDescriptors;
    rc = regRead("txfDescriptors", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TXF Descriptors Counter 0x%X \n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.cellRequests;
    rc = regRead("cellRequests", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Cell Request Counter 0x%X \n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.txfWordResponses;
    rc = regRead("txfWordResponses", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TXF Word Response Counter 0x%X \n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.tdsDescriptorCredits;
    rc = regRead("tdsDescriptorCredits", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("TDS Descriptors Credits 0x%X \n", (data & 0x3F));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.tdsCellCredits;
    rc = regRead("tdsCellCredits", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TDS Cell Credits 0x%X \n",
            (data & 0x1FF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.tdsWordResponses;
    rc = regRead("tdsWordResponses", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TDS Word Response Counter 0x%X \n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.tdsDescriptorRequests;
    rc = regRead("tdsDescriptorRequests", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TDS Descriptor Request Counter 0x%X \n",
            (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.eventCnt.tdsCellRequests;
    rc = regRead("tdsCellRequests", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "TDS Cell Request Counter 0x%X \n",
            (data & 0xFFFF));
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaRateLimitEventCountersPrint function
* @endinternal
*
* @brief   Print TxDma Rate Limit Event counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaRateLimitEventCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noEmptyTags;
    rc = regRead("noEmptyTags", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No Empty Tags 0x%X\n", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noPbCellCredits;
    rc = regRead("noPbCellCredits", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "No PB Cell Credits 0x%X\n",
            (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noPbDescriptorCredits;
    rc = regRead("noPbDescriptorCredits", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No PB Cell Credits 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noTxfResourcesForPacket;
    rc = regRead("noTxfResourcesForPacket", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No TXF Resources For Packet 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noTxfResourcesForRequest;
    rc = regRead("noTxfResourcesForRequest", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No TXF resources For Request 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noInnerPacketTokens;
    rc = regRead("noInnerPacketTokens", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No Inner Packet Tokens 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noInterPacketTokens;
    rc = regRead("noInterPacketTokens", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No Inter Packet Tokens 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noInterCellTokens;
    rc = regRead("noInterCellTokens", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No Inter packet Cell Tokens 0x%X\n", (data & 0xFF));
    }

    regAddr =
        regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.rateLimitEventCnt.noDescriptors;
    rc = regRead("noDescriptors", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("No Descriptors 0x%X\n", (data & 0xFF));
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaStatusPrint function
* @endinternal
*
* @brief   Print TxDma Status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.txdIsIdle;
    rc = regRead("txdIsIdle", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("TXD is Idle %d\n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].txdStatus.globalStatus.maxPbCellLatency;
    rc = regRead("maxPbCellLatency", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Max PB Cell latency 0x%X\n", (data & 0x3FF));
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaCountersEnableSet function
* @endinternal
*
* @brief   Enable/disable TxDma debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
* @param[in] enable                   - GT_TRUE - enable debug counters, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaCountersEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].debug.enableDebugCounters;

    mask  = 1;
    value = BOOL2BIT_MAC(enable);
    return regWriteMask("enableDebugCounters", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaCountersChannelSet function
* @endinternal
*
* @brief   Set channel to be counted by TxDma debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
* @param[in] countAllChannels         - GT_TRUE - count all channels, GT_FALSE - count given channel.
* @param[in] channelToCount           - channel to count (relevant when countAllChannels is GT_FALSE).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaCountersChannelSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_BOOL              countAllChannels,
    IN  GT_U32               channelToCount
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].debug.txdDebugConfig;

    /* channel to count for Phoenix - 6 bit */
    mask  = 0x3F1;
    value = (BOOL2BIT_MAC(countAllChannels) | ((channelToCount & 0x3F) << 4));
    return regWriteMask("txdDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaPbLatencyCellIndexSet function
* @endinternal
*
* @brief   Set index of PB cell in packet for measuring latency.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
* @param[in] cellIndex                - index of PB cell in packet for measuring latency.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaPbLatencyCellIndexSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_U32               cellIndex
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].debug.txdDebugConfig;

    mask  = 0xFC00;
    value = ((cellIndex & 0x3F) << 10);
    return regWriteMask("txdDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxDmaCountersStopCyclesNumSet function
* @endinternal
*
* @brief   Set amount of cycles up to stop TxDma debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txDma unit index.
* @param[in] stopEnable               - GT_TRUE  - stop counting after stopCyclesNum cycles,
*                                       GT_FALSE - continue counting up to new configuration.
* @param[in] stopCyclesNum            - anount of cycles to count.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxDmaCountersStopCyclesNumSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_BOOL              stopEnable,
    IN  GT_U32               stopCyclesNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txDMA[txIndex].debug.txdDebugConfig;

    mask  = 0xFFFF0002;
    value = ((BOOL2BIT_MAC(stopEnable) << 1) | ((stopCyclesNum & 0xFFFF) << 16));
    return regWriteMask("txdDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoDebugPrint function
* @endinternal
*
* @brief   Print TxFifo debug registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoDebugPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.debugStatus.dpTxfGeneralStatus;
    rc = regRead("dpTxfGeneralStatus", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("txf initialization process finish %d ", ((data >> 4) & 1));
        PR("descriptor in fifo %d ", ((data >> 3) & 1));
        PR("\n");
        PR("tag in fifo %d ", ((data >> 2) & 1));
        PR("all fifo are empty %d ", ((data >> 1) & 1));
        PR("txf in idle %d\n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.debugStatus.dpTxfDescriptorFifoOverflowInfo;
    rc = regRead("dpTxfDescriptorFifoOverflowInfo", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("descriptors fifo ovr flw ch id  0x%X\n", (data & 0x3F));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.debugStatus.dpTxfDebugBus;
    rc = regRead("dpTxfDebugBus", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug_bus 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.debugStatus.dpTxfTagInfoFromTab;
    rc = regRead("dpTxfTagInfoFromTab", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("ass_tag_cnt 0x%X ", ((data >> 16) & 0xFFF));
        PR("associate_tag 0x%X ", ((data >> 4) & 0x3FF));
        PR("dbg_tag_words_validity 0x%X\n", (data & 0xF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.descriptorReadByReadCtrl;
    rc = regRead("descriptorReadByReadCtrl", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug count descriptor 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.cellsSplitFromDescriptors;
    rc = regRead("cellsSplitFromDescriptors", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug count cells 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.wordsGenerateInPrefetchFifo;
    rc = regRead("wordsGenerateInPrefetchFifo", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug count words 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.sopReadCtrlDeriveToWb;
    rc = regRead("sopReadCtrlDeriveToWb", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug count SOP 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.cyclesFromFirstDescriptor;
    rc = regRead("cyclesFromFirstDescriptor", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug counter time from first descriptor 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.alignerInputPackets;
    rc = regRead("alignerInputPackets", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Aligner input packets 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.alignerOutputPackets;
    rc = regRead("alignerOutputPackets", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("debug count aligner out packets 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxCounters.alignerOutputByteCount;
    rc = regRead("alignerOutputByteCount", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbg_cnt_align_out_byte 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxWordBufeer.cmn8wr4rdBufferStatus;
    rc = regRead("cmn8wr4rdBufferStatus", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("buff2_latched_max_gap_value 0x%X ", ((data >> 8) & 0x3F));
        PR("buff1_latched_max_gap_value 0x%X\n", (data & 0x3F));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxReadControl.creditCounter;
    rc = regRead("creditCounter", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("credit counter 0x%08X\n", (data & 0x7FF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxReadControl.minCreditCounter;
    rc = regRead("minCreditCounter", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("minimum credit counter 0x%08X\n", (data & 0x7FF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxReadControl.maxCreditCounter;
    rc = regRead("maxCreditCounter", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("maximum credit counter 0x%08X\n", (data & 0x7FF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxReadControl.fifoPointers;
    rc = regRead("fifoPointers", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("valid tag FIFO pop 0x%X ", ((data >> 23) & 0x1F));
        PR("tag FIFO pop 0x%X ", ((data >> 18) & 0x1F));
        PR("\n");
        PR("tag FIFO rd 0x%X ", ((data >> 13) & 0x1F));
        PR("tag FIFO wr 0x%X ", ((data >> 8) & 0x1F));
        PR("\n");
        PR("tag-byte-cnt FIFO pop 0x%X ", ((data >> 4) & 0xF));
        PR("tag-byte-cnt FIFO wr 0x%X\n", (data & 0xF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxReadControl.wordFifoPointers;
    rc = regRead("wordFifoPointers", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Reference valid tag FIFO pop 0x%X ", ((data >> 22) & 0x1F));
        PR("\n");
        PR("valid tag FIFO wr 0x%X ", ((data >> 17) & 0x1F));
        PR("Reference tag FIFO wr 0x%X ", ((data >> 12) & 0x1F));
        PR("\n");
        PR("word addr FIFO pop 0x%X ", ((data >> 6) & 0x3F));
        PR("word addr FIFO wr 0x%X\n", (data & 0x3F));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxAligner.alignerSdmaStatus;
    rc = regRead("alignerSdmaStatus", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SDMA credit counter 0x%X ", ((data >> 4) & 0x3));
        PR("SDMA FIFO level 0x%X\n", (data & 0xF));
    }

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.tfxAligner.alignerResidueCtrl;
    rc = regRead("alignerResidueCtrl", txIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Aligner current residue debug 0x%08X\n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoCountersEnableSet function
* @endinternal
*
* @brief   Enable/disable TxFifo debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
* @param[in] enable                   - GT_TRUE - enable debug counters, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoCountersEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfEnableDebugCounters;

    mask  = 1;
    value = BOOL2BIT_MAC(enable);
    return regWriteMask("dpTxfEnableDebugCounters", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoCountersChannelSet function
* @endinternal
*
* @brief   Set channel to be counted by TxFifo debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
* @param[in] countAllChannels         - GT_TRUE - count all channels, GT_FALSE - count given channel.
* @param[in] channelToCount           - channel to count (relevant when countAllChannels is GT_FALSE).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoCountersChannelSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_BOOL              countAllChannels,
    IN  GT_U32               channelToCount
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfDebugConfig;

    mask  = 0x13F;
    value = ((BOOL2BIT_MAC(countAllChannels) << 8) | (channelToCount & 0x3F));
    return regWriteMask("dpTxfDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoCountersStopConditionSet function
* @endinternal
*
* @brief   Set TxFifo debug counters stop condition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
* @param[in] counterSelect            - Select one of the counters in use as the one
*                                       will trigger the end of counting for all counters
*                                       0: No counter is selected (counters free running)
*                                       1: descriptors counter
*                                       2: Cells counter *16
*                                       3: Words counter *64
*                                       4: SOP counter
*                                       5: Time pass from first descriptor
*                                       6: Aligner input packets
*                                       7: Aligner output packets
* @param[in] stopThreshold            - Stop Threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoCountersStopConditionSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_U32               counterSelect,
    IN  GT_U32               stopThreshold
)
{
    GT_STATUS                           rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfDebugConfig;
    mask  = 0x000F0000;
    value = ((counterSelect & 0xF) << 16);
    rc = regWriteMask("dpTxfDebugConfig", txIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfDebugThresholdsConfig;
    mask  = 0xFFFFFFFF;
    value = stopThreshold;
    rc = regWriteMask("dpTxfDebugThresholdsConfig", txIndex, devNum, regAddr, mask, value);
    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoAlignerCountersModeSet function
* @endinternal
*
* @brief   Set TxFifo debug Align counters mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
* @param[in] alignerCountersMode      - Select one of the aligner counters that count the packets
*                                       in its input to drive the "Aligner input packets counter"
*                                       0: Aligner all input packets
*                                       1: Aligner input non-PCH packets
*                                       2: Aligner input packets with word remove due to CRC removal
*                                       3: Aligner input words with error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoAlignerCountersModeSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_U32               alignerCountersMode
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfDebugConfig;

    mask  = 0x3000;
    value = ((alignerCountersMode & 0x3) << 12);
    return regWriteMask("dpTxfDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugTxFifoDebugTagSet function
* @endinternal
*
* @brief   Set TxFifo debug Tag for status from tables.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] txIndex                  - txFifo unit index.
* @param[in] debugTag                 - debug Tag for status from tables.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugTxFifoDebugTagSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               txIndex,
    IN  GT_U32               debugTag
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (txIndex >= MAX_DP_CNS)
    {
        PR("txIndex %d out of range\n", txIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_txFIFO[txIndex].debug.dpTxfDebugConfig;

    mask  = 0x3FF00000;
    value = ((debugTag & 0x3FF) << 20);
    return regWriteMask("dpTxfDebugConfig", txIndex, devNum, regAddr, mask, value);
}

/* RX */

/**
* @internal prvCpssDxChPortDpDebugRxDmaStatusPrint function
* @endinternal
*
* @brief   Print RxDma debug status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] rxIndex                  - rxDma unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugRxDmaStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               rxIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rxIndex >= MAX_DP_CNS)
    {
        PR("rxIndex %d out of range\n", rxIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].interrupts.rxDMAInterrupt0Cause;
    rc = regRead("rxDMAInterrupt0Cause", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Rx DMA Interrupt0 Cause  0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].interrupts.rxDMAInterrupt1Cause;
    rc = regRead("rxDMAInterrupt1Causee", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Rx DMA Interrupt1 Cause  0x%08X\n", data);
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].interrupts.rxDMAInterrupt2Cause;
        rc = regRead("rxDMAInterrupt1Causee", rxIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Rx DMA Interrupt2 Cause  0x%08X\n", data);
        }
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.debug_status.dp_rx_general_status;
    rc = regRead("dp_rx_general_status", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dp rx idle %d\n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.debug_status.dp_rx_debug_bus;
    rc = regRead("dp_rx_debug_bus", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("DP RX Ingress debug bus (dp clock domain) 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_ingress_fifo_status;
    rc = regRead("rx_ingress_fifo_status", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_ingress_fifo_status *****\n");
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("pch_eop_timer_full %d ", ((data >> 15) & 1));
            PR("pch_eop_timer_empty %d ", ((data >> 14) & 1));
        }
        PR("dp_rx_accum_address_tracker_full %d ", ((data >> 13) & 1));
        PR("dp_rx_accum_address_tracker_empty %d ", ((data >> 12) & 1));
        PR("\n");
        PR("rd_tail_data_fifo_full %d ", ((data >> 11) & 1));
        PR("rd_tail_data_fifo_empty %d ", ((data >> 10) & 1));
        PR("rd_head_data_fifo_full %d ", ((data >> 9) & 1));
        PR("\n");
        PR("rd_head_data_fifo_empty %d ", ((data >> 8) & 1));
        PR("rd_attr_fifo_full %d ", ((data >> 7) & 1));
        PR("rd_attr_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("tail_req_fifo_full %d ", ((data >> 5) & 1));
        PR("tail_req_fifo_empty %d ", ((data >> 4) & 1));
        PR("head_req_fifo_full %d ", ((data >> 3) & 1));
        PR("\n");
        PR("head_req_fifo_empty %d ", ((data >> 2) & 1));
        PR("entry_mng_fifo_full %d ", ((data >> 1) & 1));
        PR("entry_mng_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_ing_async_fifo_status;
    rc = regRead("rx_ing_async_fifo_status", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_ing_async_fifo_status *****\n");
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("sdma_port0_sync_fifo_full %d ", ((data >> 19) & 1));
            PR("sdma_port0_sync_fifo_empty %d ", ((data >> 18) & 1));
            PR("\n");
        }
        else
        {
            PR("tail_bus_pb_sync_fifo_full %d ", ((data >> 21) & 1));
            PR("tail_bus_pb_sync_fifo_empty %d ", ((data >> 20) & 1));
            PR("\n");
            PR("tail_bc_update_sync_fifo_full %d ", ((data >> 19) & 1));
            PR("tail_bc_update_sync_fifo_empty %d ", ((data >> 18) & 1));
            PR("\n");
        }
        PR("head_bc_update_sync_fifo_full %d ", ((data >> 17) & 1));
        PR("head_bc_update_sync_fifo_empty %d ", ((data >> 16) & 1));
        PR("desc_bc_msg_sync_fifo_full %d ", ((data >> 15) & 1));
        PR("\n");
        PR("desc_bc_msg_sync_fifo_empty %d ", ((data >> 14) & 1));
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("tail_bus_pb_fifo_full %d ", ((data >> 13) & 1));
            PR("tail_bus_pb_sync_fifo_empty %d ", ((data >> 12) & 1));
        }
        else
        {
            PR("set_tail_id_sync_fifo_full %d ", ((data >> 13) & 1));
            PR("set_tail_id_sync_fifo_empty %d ", ((data >> 12) & 1));
        }
        PR("\n");
        PR("ch_id_sync_fifo_full %d ", ((data >> 11) & 1));
        PR("ch_id_sync_fifo_empty %d ", ((data >> 10) & 1));
        PR("tail_drop_sync_fifo_full %d ", ((data >> 9) & 1));
        PR("\n");
        PR("tail_drop_sync_fifo_empty %d ", ((data >> 8) & 1));
        PR("tail_counter_sync_fifo_full %d ", ((data >> 7) & 1));
        PR("tail_counter_sync_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("rx_id_sync_fifo_full %d ", ((data >> 5) & 1));
        PR("rx_id_sync_fifo_empty %d ", ((data >> 4) & 1));
        PR("pb_global_fill_level_sync_fifo_full %d ", ((data >> 3) & 1));
        PR("\n");
        PR("pb_global_fill_level_sync_fifo_empty %d ", ((data >> 2) & 1));
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("tail_bc_update_sync_fifo_full %d ", ((data >> 1) & 1));
            PR("tail_bc_update_sync_empty %d ", (data & 1));
        }
        else
        {
            PR("sdma_port_sync_fifo_full %d ", ((data >> 1) & 1));
            PR("sdma_port_sync_fifo_empty %d ", (data & 1));
        }
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_context_id_ctrl_fifos_status0;
    rc = regRead("rx_egr_context_id_ctrl_fifos_status0", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_egr_context_id_ctrl_fifos_status0 *****\n");
        PR("free_rx_local_ids_fifo_full %d ", ((data >> 31) & 1));
        PR("free_rx_local_ids_fifo_empty %d ", ((data >> 30) & 1));
        PR("head_packet_count_data_fifo_full %d ", ((data >> 29) & 1));
        PR("\n");
        PR("head_packet_count_data_fifo_empty %d ", ((data >> 28) & 1));
        PR("enqueue_ct2sf_head_pb_id_data_fifo_full %d ", ((data >> 27) & 1));
        PR("\n");
        PR("enqueue_ct2sf_head_pb_id_data_fifo_empty %d ", ((data >> 26) & 1));
        PR("enqueue_ct2sf_descriptor_data_fifo_full %d ", ((data >> 25) & 1));
        PR("\n");
        PR("enqueue_ct2sf_descriptor_data_fifo_empty %d ", ((data >> 24) & 1));
        PR("enqueue_uc_head_pb_id_data_fifo_full %d ", ((data >> 23) & 1));
        PR("\n");
        PR("enqueue_uc_head_pb_id_data_fifo_empty %d ", ((data >> 22) & 1));
        PR("enqueue_uc_head_pb_id_fifo_full %d ", ((data >> 21) & 1));
        PR("\n");
        PR("enqueue_uc_head_pb_id_fifo_empty %d ", ((data >> 20) & 1));
        PR("enqueue_type_fifo_full %d ", ((data >> 19) & 1));
        PR("\n");
        PR("enqueue_type_fifo_empty %d ", ((data >> 18) & 1));
        PR("final_tail_bc_data_fifo_full %d ", ((data >> 17) & 1));
        PR("\n");
        PR("final_tail_bc_data_fifo_empty %d ", ((data >> 16) & 1));
        PR("final_tail_bc_req_fifo_full %d ", ((data >> 15) & 1));
        PR("\n");
        PR("final_tail_bc_req_fifo_empty %d ", ((data >> 14) & 1));
        PR("ct2sf_descriptor_fifo_full %d ", ((data >> 13) & 1));
        PR("ct2sf_descriptor_fifo_empty %d ", ((data >> 12) & 1));
        PR("\n");
        PR("enqueue_requests_descriptor_fifo_full %d ", ((data >> 11) & 1));
        PR("enqueue_requests_descriptor_fifo_empty %d ", ((data >> 10) & 1));
        PR("ct2sf_rx_id_fifo_full %d ", ((data >> 9) & 1));
        PR("\n");
        PR("ct2sf_rx_id_fifo_empty %d ", ((data >> 8) & 1));
        PR("ct2sf_ll_rd_access_channel_id_fifo_full %d ", ((data >> 7) & 1));
        PR("ct2sf_ll_rd_access_channel_id_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("channel_id_data_fifo_1_full %d ", ((data >> 5) & 1));
        PR("channel_id_data_fifo_1_empty %d ", ((data >> 4) & 1));
        PR("channel_id_data_fifo_0_full %d ", ((data >> 3) & 1));
        PR("\n");
        PR("channel_id_data_fifo_0_empty %d ", ((data >> 2) & 1));
        PR("enqueue_uc_requests_rx_id_fifo_full %d ", ((data >> 1) & 1));
        PR("enqueue_uc_requests_rx_id_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_context_id_ctrl_fifos_status1;
    rc = regRead("rx_egr_context_id_ctrl_fifos_status1", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_egr_context_id_ctrl_fifos_status1 *****\n");
        PR("enqueue_ct2sf_head_pb_id_link_ll_full %d ", ((data >> 25) & 1));
        PR("\n");
        PR("enqueue_ct2sf_head_pb_id_link_ll_empty %d ", ((data >> 24) & 1));
        PR("enqueue_ct2sf_descriptor_ll_full %d ", ((data >> 23) & 1));
        PR("\n");
        PR("enqueue_ct2sf_descriptor_ll_empty %d ", ((data >> 22) & 1));
        PR("enqueue_ct2sf_requests_ll_full %d ", ((data >> 21) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_ll_empty %d ", ((data >> 20) & 1));
        PR("tail_packet_count_data_fifo_full %d ", ((data >> 19) & 1));
        PR("\n");
        PR("tail_packet_count_data_fifo_empty %d ", ((data >> 18) & 1));
        PR("reply_tail_count_data_fifo_full %d ", ((data >> 17) & 1));
        PR("\n");
        PR("reply_tail_count_data_fifo_empty %d ", ((data >> 16) & 1));
        PR("reply_tail_count_req_fifo_full %d ", ((data >> 15) & 1));
        PR("\n");
        PR("reply_tail_count_req_fifo_empty %d ", ((data >> 14) & 1));
        PR("reply_tail_pb_id_data_fifo_full %d ", ((data >> 13) & 1));
        PR("reply_tail_pb_id_data_fifo_empty %d ", ((data >> 12) & 1));
        PR("\n");
        PR("reply_tail_pb_id_req_fifo_full %d ", ((data >> 11) & 1));
        PR("reply_tail_pb_id_req_fifo_empty %d ", ((data >> 10) & 1));
        PR("reply_tail_bc_data_fifo_full %d ", ((data >> 9) & 1));
        PR("\n");
        PR("reply_tail_bc_data_fifo_empty %d ", ((data >> 8) & 1));
        PR("reply_tail_bc_req_fifo_full %d ", ((data >> 7) & 1));
        PR("reply_tail_bc_req_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("enqueue_uc_descriptor_data_fifo_full %d ", ((data >> 5) & 1));
        PR("enqueue_uc_descriptor_data_fifo_empty %d ", ((data >> 4) & 1));
        PR("\n");
        PR("enqueue_uc_descriptor_fifo_full %d ", ((data >> 3) & 1));
        PR("enqueue_uc_descriptor_fifo_empty %d ", ((data >> 2) & 1));
        PR("\n");
        PR("enqueue_dummy_descriptor_fifo_full %d ", ((data >> 1) & 1));
        PR("enqueue_dummy_descriptor_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_context_id_ctrl_fifos_status2;
    rc = regRead("rx_egr_context_id_ctrl_fifos_status2", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_egr_context_id_ctrl_fifos_status2 *****\n");
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 15 %d ", ((data >> 31) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 15 %d ", ((data >> 30) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 14 %d ", ((data >> 29) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 14 %d ", ((data >> 28) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 13 %d ", ((data >> 27) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 13 %d ", ((data >> 26) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 12 %d ", ((data >> 25) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 12 %d ", ((data >> 24) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 11 %d ", ((data >> 23) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 11 %d ", ((data >> 22) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 10 %d ", ((data >> 21) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 10 %d ", ((data >> 20) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 9 %d ", ((data >> 19) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 9 %d ", ((data >> 18) & 1));
            PR("\n");
        }
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 8 %d ", ((data >> 17) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 8 %d ", ((data >> 16) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 7 %d ", ((data >> 15) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 7 %d ", ((data >> 14) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 6 %d ", ((data >> 13) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 6 %d ", ((data >> 12) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 5 %d ", ((data >> 11) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 5 %d ", ((data >> 10) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 4 %d ", ((data >> 9) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 4 %d ", ((data >> 8) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 3 %d ", ((data >> 7) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 3 %d ", ((data >> 6) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 2 %d ", ((data >> 5) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 2 %d ", ((data >> 4) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 1 %d ", ((data >> 3) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 1 %d ", ((data >> 2) & 1));
        PR("\n");
        PR("enqueue_ct2sf_requests_data_fifo_full Channel 0 %d ", ((data >> 1) & 1));
        PR("enqueue_ct2sf_requests_data_fifo_empty Channel 0 %d ", (data & 1));
        PR("\n");
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_context_id_ctrl_fifos_status3;
        rc = regRead("rx_egr_context_id_ctrl_fifos_status3", rxIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("\n***** rx_egr_context_id_ctrl_fifos_status3 *****\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 31 %d ", ((data >> 31) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 31 %d ", ((data >> 30) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 30 %d ", ((data >> 29) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 30 %d ", ((data >> 28) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 29 %d ", ((data >> 27) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 29 %d ", ((data >> 26) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 28 %d ", ((data >> 25) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 28 %d ", ((data >> 24) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 27 %d ", ((data >> 23) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 27 %d ", ((data >> 22) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 26 %d ", ((data >> 21) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 26 %d ", ((data >> 20) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 25 %d ", ((data >> 19) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 25 %d ", ((data >> 18) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 24 %d ", ((data >> 17) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 24 %d ", ((data >> 16) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 23 %d ", ((data >> 15) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 23 %d ", ((data >> 14) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 22 %d ", ((data >> 13) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 22 %d ", ((data >> 12) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 21 %d ", ((data >> 11) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 21 %d ", ((data >> 10) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 20 %d ", ((data >> 9) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 20 %d ", ((data >> 8) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 19 %d ", ((data >> 7) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 19 %d ", ((data >> 6) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 18 %d ", ((data >> 5) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 18 %d ", ((data >> 4) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 17 %d ", ((data >> 3) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 17 %d ", ((data >> 2) & 1));
            PR("\n");
            PR("enqueue_ct2sf_requests_data_fifo_full Channel 16 %d ", ((data >> 1) & 1));
            PR("enqueue_ct2sf_requests_data_fifo_empty Channel 16 %d ", (data & 1));
            PR("\n");
        }
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_pb_manager_fifos_status0;
    rc = regRead("rx_egr_pb_manager_fifos_status0", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_egr_pb_manager_fifos_status *****\n");
        PR("slice3_outstanding_rx_id_fifo_full %d ", ((data >> 25) & 1));
        PR("slice3_outstanding_rx_id_fifo_empty  %d ", ((data >> 24) & 1));
        PR("\n");
        PR("slice2_outstanding_rx_id_fifo_full %d ", ((data >> 23) & 1));
        PR("slice2_outstanding_rx_id_fifo_empty  %d ", ((data >> 22) & 1));
        PR("\n");
        PR("slice1_outstanding_rx_id_fifo_full %d ", ((data >> 21) & 1));
        PR("slice1_outstanding_rx_id_fifo_empty  %d ", ((data >> 20) & 1));
        PR("\n");
        PR("slice0_outstanding_rx_id_fifo_full %d ", ((data >> 19) & 1));
        PR("slice0_outstanding_rx_id_fifo_empty  %d ", ((data >> 18) & 1));
        PR("\n");
        PR("pds_data_slice_order_fifo_full %d ", ((data >> 17) & 1));
        PR("pds_data_slice_order_fifo_empty %d ", ((data >> 16) & 1));
        PR("\n");
        PR("ct2sf_headers_slice_order_fifo_full %d ", ((data >> 15) & 1));
        PR("ct2sf_headers_slice_order_fifo_empty %d ", ((data >> 14) & 1));
        PR("\n");
        PR("uc_headers_slice_order_fifo_full %d ", ((data >> 13) & 1));
        PR("uc_headers_slice_order_fifo_empty %d ", ((data >> 12) & 1));
        PR("\n");
        PR("pb_tail_id_attr_fifo_full %d ", ((data >> 11) & 1));
        PR("pb_tail_id_attr_fifo_empty %d ", ((data >> 10) & 1));
        PR("\n");
        PR("pb_tail_id_data_fifo_full %d ", ((data >> 9) & 1));
        PR("pb_tail_id_data_fifo_empty %d ", ((data >> 8) & 1));
        PR("\n");
        PR("pb_tail_id_req_fifo_full %d ", ((data >> 7) & 1));
        PR("pb_tail_id_req_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("final_packet_head_fifo_full %d ", ((data >> 5) & 1));
        PR("final_packet_head_fifo_empty %d ", ((data >> 4) & 1));
        PR("\n");
        PR("packet_head_select_fifo_full %d ", ((data >> 3) & 1));
        PR("packet_head_select_fifo_empty %d ", ((data >> 2) & 1));
        PR("\n");
        PR("packet_tail_fifo_full %d ", ((data >> 1) & 1));
        PR("packet_tail_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_pb_manager_fifos_status1;
    rc = regRead("rx_egr_pb_manager_fifos_status1", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("slice3_pb_id_fifo_full %d ", ((data >> 9) & 1));
        PR("slice3_pb_id_fifo_empty %d ", ((data >> 8) & 1));
        PR("\n");
        PR("slice2_pb_id_fifo_full %d ", ((data >> 7) & 1));
        PR("slice2_pb_id_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("slice1_pb_id_fifo_full %d ", ((data >> 5) & 1));
        PR("slice1_pb_id_fifo_empty %d ", ((data >> 4) & 1));
        PR("\n");
        PR("slice0_pb_id_fifo_full %d ", ((data >> 3) & 1));
        PR("slice0_pb_id_fifo_empty %d ", ((data >> 2) & 1));
        PR("\n");
        PR("tails_slice_order_fifo_full %d ", ((data >> 1) & 1));
        PR("tails_slice_order_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_async_fifos_status0;
    rc = regRead("rx_egr_async_fifos_status0", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("\n***** rx_egr_async_fifos_status *****\n");
        PR("receiver_desc_dc_fifo_full %d ", ((data >> 15) & 1));
        PR("receiver_desc_dc_fifo_empty %d ", ((data >> 14) & 1));
        PR("\n");
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            PR("receiver_cd_inc_dc_fifo_full %d ", ((data >> 13) & 1));
            PR("receiver_cd_inc_dc_fifo_empty %d ", ((data >> 12) & 1));
        }
        else
        {
            PR("ch_freeze_rx_id_1_dc_fifo_full %d ", ((data >> 13) & 1));
            PR("ch_freeze_rx_id_1_dc_fifo_empty %d ", ((data >> 12) & 1));
        }
        PR("\n");
        PR("ch_freeze_rx_id_0_dc_fifo_full %d ", ((data >> 11) & 1));
        PR("ch_freeze_rx_id_0_dc_fifo_empty %d ", ((data >> 10) & 1));
        PR("\n");
        PR("clr_tail_pb_id_req_dc_fifo_full %d ", ((data >> 9) & 1));
        PR("clr_tail_pb_id_req_dc_fifo_empty %d ", ((data >> 8) & 1));
        PR("\n");
        PR("local_ids_dc_fifo_full %d ", ((data >> 7) & 1));
        PR("local_ids_dc_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("packet_count_if_dc_fifo_full %d ", ((data >> 5) & 1));
        PR("packet_count_if_dc_fifo_empty %d ", ((data >> 4) & 1));
        PR("\n");
        PR("packet_count_credit_count_if_dc_fifo_full %d ", ((data >> 3) & 1));
        PR("packet_count_credit_count_if_dc_fifo_empty %d ", ((data >> 2) & 1));
        PR("\n");
        PR("enqueue_fifo_full_dc_fifo_full %d ", ((data >> 1) & 1));
        PR("enqueue_fifo_full_dc_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.fifos_status.rx_egr_async_fifos_status1;
    rc = regRead("rx_egr_async_fifos_status1", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("slice3_pb_id_update_dc_fifo_full %d ", ((data >> 23) & 1));
        PR("slice3_pb_id_update_dc_fifo_empty %d ", ((data >> 22) & 1));
        PR("\n");
        PR("slice2_pb_id_update_dc_fifo_full %d ", ((data >> 21) & 1));
        PR("slice2_pb_id_update_dc_fifo_empty %d ", ((data >> 20) & 1));
        PR("\n");
        PR("slice1_pb_id_update_dc_fifo_full %d ", ((data >> 19) & 1));
        PR("slice1_pb_id_update_dc_fifo_empty %d ", ((data >> 18) & 1));
        PR("\n");
        PR("slice0_pb_id_update_dc_fifo_full %d ", ((data >> 17) & 1));
        PR("slice0_pb_id_update_dc_fifo_empty %d ", ((data >> 16) & 1));
        PR("\n");
        PR("context_id_head_dc_fifo_full %d ", ((data >> 15) & 1));
        PR("context_id_head_dc_fifo_empty %d ", ((data >> 14) & 1));
        PR("\n");
        PR("context_id_tail_dc_fifo_full %d ", ((data >> 13) & 1));
        PR("context_id_tail_dc_fifo_empty %d ", ((data >> 12) & 1));
        PR("\n");
        PR("tails_slice_dc_fifo_full %d ", ((data >> 11) & 1));
        PR("tails_slice_dc_fifo_empty %d ", ((data >> 10) & 1));
        PR("\n");
        PR("ct2sf_slice_dc_fifo_full %d ", ((data >> 9) & 1));
        PR("ct2sf_slice_dc_fifo_empty %d ", ((data >> 8) & 1));
        PR("\n");
        PR("uc_slice_dc_fifo_full %d ", ((data >> 7) & 1));
        PR("uc_slice_dc_fifo_empty %d ", ((data >> 6) & 1));
        PR("\n");
        PR("pds_reply_dc_fifo_full %d ", ((data >> 5) & 1));
        PR("pds_reply_dc_fifo_empty %d ", ((data >> 4) & 1));
        PR("\n");
        PR("pds_2_rx_ctrl_samp_dc_fifo_full %d ", ((data >> 3) & 1));
        PR("pds_2_rx_ctrl_samp_dc_fifo_empty %d ", ((data >> 2) & 1));
        PR("\n");
        PR("pds_2_rx_data_samp_dc_fifo_full %d ", ((data >> 1) & 1));
        PR("pds_2_rx_data_samp_dc_fifo_empty %d ", (data & 1));
        PR("\n");
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortDpDebugRxDmaCountersPrint function
* @endinternal
*
* @brief   Print RxDma debug counters registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] rxIndex                  - rxDma unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugRxDmaCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               rxIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              cntNumOf;
    GT_U32                              cntIdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rxIndex >= MAX_DP_CNS)
    {
        PR("rxIndex %d out of range\n", rxIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.ingr_stress_err_cnt.rx_ingr_stress_cnt;
    rc = regRead("rx_ingr_stress_cnt", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("rx_ingr_stress_cnt 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.ingr_stress_err_cnt.rx_ingr_stress_cnt_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("rx_ingr_stress_cnt_high", rxIndex, devNum, regAddr, &data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_accum_mem_cnt.accum_count;
    rc = regRead("", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("accum_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_accum_mem_cnt.accum_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("accum_count_high", rxIndex, devNum, regAddr, &data);
    }

    cntNumOf = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 5 : 1;
    PR("rx_ingress_drop_count ");
    for (cntIdx = 0; (cntIdx < cntNumOf); cntIdx++)
    {
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                rxIngressDropCounter[cntIdx].rx_ingress_drop_count;
        rc = regRead("rx_ingress_drop_count", rxIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }
        /* read high part to unlock counter */
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                rxIngressDropCounter[cntIdx].rx_ingress_drop_count_high;
        if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regRead("rx_ingress_drop_count_high", rxIndex, devNum, regAddr, &data);
        }
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_cp_cnt.rx_to_cp_count;
    rc = regRead("rx_to_cp_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("rx_to_cp_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_cp_cnt.rx_to_cp_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("rx_to_cp_count_high", rxIndex, devNum, regAddr, &data);
    }

    cntNumOf = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 2 : 1;
    PR("cp_to_rx_count ");
    for (cntIdx = 0; (cntIdx < cntNumOf); cntIdx++)
    {
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                cp_to_rx_cnt[cntIdx].cp_to_rx_count;
        rc = regRead("cp_to_rx_count", rxIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }

        /* read high part to unlock counter */
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                cp_to_rx_cnt[cntIdx].cp_to_rx_count_high;
        if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regRead("rx_to_cp_count_high", rxIndex, devNum, regAddr, &data);
        }
    }
    PR("\n");

    cntNumOf = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 2 : 1;
    PR("rx_egress_drop_count ");
    for (cntIdx = 0; (cntIdx < cntNumOf); cntIdx++)
    {
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                rxEgressDropCounter[cntIdx].rx_egress_drop_count;
        rc = regRead("rx_egress_drop_count", rxIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }

        /* read high part to unlock counter */
        regAddr =
            regsAddrPtr->sip6_rxDMA[rxIndex].debug.
                rxEgressDropCounter[cntIdx].rx_egress_drop_count_high;
        if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regRead("rx_egress_drop_count_high", rxIndex, devNum, regAddr, &data);
        }
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_pb_cnt.rx_to_pb_count;
    rc = regRead("rx_to_pb_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("rx_to_pb_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_pb_cnt.rx_to_pb_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("rx_to_pb_count_high", rxIndex, devNum, regAddr, &data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.pb_to_rx_cnt.pb_to_rx_count;
    rc = regRead("pb_to_rx_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("pb_to_rx_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.pb_to_rx_cnt.pb_to_rx_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("pb_to_rx_count_high", rxIndex, devNum, regAddr, &data);
    }

    /* counter has no high part - unlock not needed */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.pb_to_rx_cnt.pb_to_rx_packet_credit_count;
    rc = regRead("pb_to_rx_packet_credit_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("pb_to_rx_packet_credit_count 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_pdx_cnt.rx_to_pdx_count;
    rc = regRead("rx_to_pdx_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("rx_to_pdx_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.rx_to_pdx_cnt.rx_to_pdx_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("rx_to_pdx_count_high", rxIndex, devNum, regAddr, &data);
    }

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.pdx_to_rx_cnt.pdx_to_rx_count;
    rc = regRead("pdx_to_rx_count", rxIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("pdx_to_rx_count 0x%08X \n", data);
    }

    /* read high part to unlock counter */
    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.pdx_to_rx_cnt.pdx_to_rx_count_high;
    if (regAddr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regRead("pdx_to_rx_count_high", rxIndex, devNum, regAddr, &data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugRxDmaGlobalConfig function
* @endinternal
*
* @brief   Configure RxDma debug registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] rxIndex                  - rxDma unit index.
* @param[in] countigEnable            - GT_TRUE - couning done, GT_FALSE - no counting.
* @param[in] stopCountigEnable        - GT_TRUE - couning stopped afrer stopCyclesNum cycles,
*                                       GT_FALSE - no counting.
* @param[in] stopCyclesNum            - see stopCountigEnable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugRxDmaGlobalConfig
(
    IN  GT_U8                devNum,
    IN  GT_U32               rxIndex,
    IN  GT_BOOL              countigEnable,
    IN  GT_BOOL              stopCountigEnable,
    IN  GT_U32               stopCyclesNum
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rxIndex >= MAX_DP_CNS)
    {
        PR("rxIndex %d out of range\n", rxIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.dp_rx_enable_debug_counters;
    mask  = 1;
    value = BOOL2BIT_MAC(countigEnable);
    rc = regWriteMask("dp_rx_enable_debug_counters", rxIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.dp_rx_debug_configurations;
        mask  = (1 << 1);
        value = (BOOL2BIT_MAC(countigEnable) << 1);
        rc = regWriteMask("dp_rx_enable_debug_counters", rxIndex, devNum, regAddr, mask, value);
        if (rc != GT_OK)
        {
            return rc;
        }
        mask   = 0xFFFFFFFF;
        value  = (stopCountigEnable == GT_FALSE) ? 0xFFFFFFFF : stopCyclesNum;
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.dp_rx_debug_cycles_to_count;
        rc = regWriteMask("dp_rx_debug_cycles_to_count", rxIndex, devNum, regAddr, mask, value);
        if (rc != GT_OK)
        {
            return rc;
        }
        value  = (stopCountigEnable == GT_FALSE) ? 0xFFFFFFFF : 0;
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.dp_rx_debug_cycles_to_count_high;
        rc = regWriteMask("dp_rx_debug_cycles_to_count_high", rxIndex, devNum, regAddr, mask, value);
    }
    else
    {
        regAddr = regsAddrPtr->sip6_rxDMA[rxIndex].debug.dp_rx_debug_configurations;
        mask  = 0xFFFFFFFF;
        value = ((BOOL2BIT_MAC(stopCountigEnable) << 31) | (stopCyclesNum & 0x7FFFFFFF));
        rc = regWriteMask("dp_rx_debug_configurations", rxIndex, devNum, regAddr, mask, value);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugRxDmaCounterConfig function
* @endinternal
*
* @brief   Configure RxDma debug registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] rxIndex                  - rxDma unit index.
* @param[in] counterName              - counter Name.
* @param[in] typeMask                 - mask of counted event types.
* @param[in] typeRef                  - reference of counted event types.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugRxDmaCounterConfig
(
    IN  GT_U8                devNum,
    IN  GT_U32               rxIndex,
    IN  const char*          counterName,
    IN  GT_U32               typeMask,
    IN  GT_U32               typeRef
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              maskRegAddr;
    GT_U32                              refRegAddr;
    GT_U32                              unitOffest;
    GT_U32                              i;

    static struct
    {
        const char*     counterName;
        GT_U32          maskUnit0RegAddrOffset;
        GT_U32          refUnit0RegAddrOffset;
        const char*     helpString;
    } countersDataArr[] =
    {

        {"ingress_stress",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, ingr_stress_err_cnt.rx_ingr_stress_cnt_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, ingr_stress_err_cnt.rx_ingr_stress_cnt_type_ref),
            "bits: 12-7 channel_id, 6 port-err, 5 bc-trim, 4 accum-mem-trim, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"accum",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_accum_mem_cnt.accum_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_accum_mem_cnt.accum_count_type_ref),
            "bits: 11-6 channel_id, 5 word-type-manipulated, 4 consistency-validity, "
            "\n\t3 padding, 2 IDs, 1 accum, 0 os "
        },
        {"ingress_drop",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[0].rx_ingress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[0].rx_ingress_drop_count_type_ref),
            "bits: 13-8 channel_id, 7-6 priority, 5 dispatch-fail, 4 PB-fill-level, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"ingress_drop1",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[1].rx_ingress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[1].rx_ingress_drop_count_type_ref),
            "bits: 13-8 channel_id, 7-6 priority, 5 dispatch-fail, 4 PB-fill-level, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"ingress_drop2",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[2].rx_ingress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[2].rx_ingress_drop_count_type_ref),
            "bits: 13-8 channel_id, 7-6 priority, 5 dispatch-fail, 4 PB-fill-level, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"ingress_drop3",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[3].rx_ingress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[3].rx_ingress_drop_count_type_ref),
            "bits: 13-8 channel_id, 7-6 priority, 5 dispatch-fail, 4 PB-fill-level, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"ingress_drop4",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[4].rx_ingress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxIngressDropCounter[4].rx_ingress_drop_count_type_ref),
            "bits: 13-8 channel_id, 7-6 priority, 5 dispatch-fail, 4 PB-fill-level, "
            "\n\t3 dec-ct-trim, 2 dec-ct-pad, 1 pbm-bp, 0 disp-freeze "
        },
        {"rx_to_cp",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_cp_cnt.rx_to_cp_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_cp_cnt.rx_to_cp_count_type_ref),
            "bits: 10-5 channel_id, 4 header-transaction, "
            "\n\t3 latency-sensitive, 2-1 Pre Ingress Priority packet type, 0 cut through "
        },
        {"cp_to_rx",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, cp_to_rx_cnt[0].cp_to_rx_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, cp_to_rx_cnt[0].cp_to_rx_count_type_ref),
            "bits: Hawk: see cp_to_rx1; Falcon: 10-5 channel_id, 4 dummy, "
            "\n\t3 trunkated header, 2 ct_to_sf, 1 cut through, 0 multicast "
        },
        {"cp_to_rx1",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, cp_to_rx_cnt[1].cp_to_rx_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, cp_to_rx_cnt[1].cp_to_rx_count_type_ref),
            "bits: 13-8 channel_id, 7 descriptor valid, 6 conext done, 5 High Priority, 4 dummy, "
            "\n\t3 trunkated header, 2 ct_to_sf, 1 cut through, 0 multicast "
        },
        {"egress_drop",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxEgressDropCounter[0].rx_egress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxEgressDropCounter[0].rx_egress_drop_count_type_ref),
            "bits:Hawk - see egress_drop1,"
            "\n\t,Falcon 10-5 channel_id"
            "\n\tCommon 4 head count full, t3 message full, 2 ct_to_sf full, 1 sf_ct_full, 0 tail_id"
        },
        {"egress_drop1",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxEgressDropCounter[1].rx_egress_drop_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rxEgressDropCounter[1].rx_egress_drop_count_type_ref),
            "Hawk 15-10 channel_id, 9 high priority,"
            "\n\t8 descriptor FIFO ful, 7 PB manager headers FIFO full,"
            "\n\t4 head count full, t3 message full, 2 ct_to_sf full, 1 sf_ct_full, 0 tail_id"
        },
        {"rx_to_pb",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_pb_cnt.rx_to_pb_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_pb_cnt.rx_to_pb_count_type_ref),
            "bits: 12-7 channel_id, 6 MC header, 5 PDX descriptors, 4 PDX tail, "
            "\n\t3 PB header, 2-0 PB cell type  "
        },
        {"pb_to_rx",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, pb_to_rx_cnt.pb_to_rx_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, pb_to_rx_cnt.pb_to_rx_count_type_ref),
            "bits: 2 FIF0 state, 1 tail id, 0 header id  "
        },
        {"rx_to_pdx",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_pdx_cnt.rx_to_pdx_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, rx_to_pdx_cnt.rx_to_pdx_count_type_ref),
            "bits: 4 Dummy Descriptor, 3 Truncated Header type, "
            "\n\t2 CT to SF, 1 Cut Through, 0 Milticast "
        },
        {"pdx_to_rx",
            RX_DMA_DEBUG_REG_ADDR_OFF(0, pdx_to_rx_cnt.pdx_to_rx_count_type_mask),
            RX_DMA_DEBUG_REG_ADDR_OFF(0, pdx_to_rx_cnt.pdx_to_rx_count_type_ref),
            "bits: Hawk 3 Descriptor reply reject"
            "\n\tCommon 2 Dummy Descriptor, 1 Truncated Header reply, 0 Milticast "
        }
    };
    static GT_U32 countersDataArrSize = (sizeof(countersDataArr) / sizeof(countersDataArr[0]));

    if (cpssOsStrCmp(counterName, "help") == 0)
    {
        PR("prvCpssDxChPortDpDebugRxDmaCounterConfig devNum, rxIndex, counterName, typeMask, typeRef\n");
        PR("Supported counters:\n");
        for (i = 0; (i < countersDataArrSize); i++)
        {
            PR("%s \n\t%s \n", countersDataArr[i].counterName, countersDataArr[i].helpString);
        }
        return GT_OK;
    }

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rxIndex >= MAX_DP_CNS)
    {
        PR("rxIndex %d out of range\n", rxIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < countersDataArrSize); i++)
    {
        if (cpssOsStrCmp(counterName, countersDataArr[i].counterName) == 0) break;
    }

    if (i >= countersDataArrSize)
    {
        PR("print help calling prvCpssDxChPortDpDebugRxDmaCounterConfig 0,0,\"help\",0,0\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    unitOffest = RX_DMA_UNIT_REG_ADDR_OFF(rxIndex) - RX_DMA_UNIT_REG_ADDR_OFF(0);
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    maskRegAddr = *(GT_U32*)
        ((char*)regsAddrPtr + unitOffest + countersDataArr[i].maskUnit0RegAddrOffset);
    refRegAddr = *(GT_U32*)
        ((char*)regsAddrPtr + unitOffest + countersDataArr[i].refUnit0RegAddrOffset);

    PR("maskReg 0x%08X refReg 0x%08X\n", maskRegAddr, refRegAddr);

    rc = regWriteMask("mask_reg", rxIndex, devNum, maskRegAddr, 0xFFFFFFFF, typeMask);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = regWriteMask("ref_reg", rxIndex, devNum, refRegAddr, 0xFFFFFFFF, typeRef);
    return rc;
}

GT_STATUS prvCpssDxChPortDpDebugRxTxHelp(void)
{
    PR("prvCpssDxChPortDpDebugTxDmaChannelStatusPrint dev, tx_index, tx_channel\n");
    PR("prvCpssDxChPortDpDebugTxDmaEventCountersPrint dev, tx_index\n");
    PR("prvCpssDxChPortDpDebugTxDmaRateLimitEventCountersPrint dev, tx_index\n");
    PR("prvCpssDxChPortDpDebugTxDmaStatusPrint dev, tx_index\n");
    PR("prvCpssDxChPortDpDebugTxDmaCountersEnableSet dev, tx_index, enable\n");
    PR("prvCpssDxChPortDpDebugTxDmaPbLatencyCellIndexSet dev, tx_index, cell_index\n");
    PR("prvCpssDxChPortDpDebugTxDmaCountersStopCyclesNumSet dev, tx_index, cell_index, stop_cycles_num\n");
    PR("\n");
    PR("prvCpssDxChPortDpDebugTxFifoDebugPrint dev, tx_index\n");
    PR("prvCpssDxChPortDpDebugTxFifoCountersEnableSet dev, tx_index, enable\n");
    PR("prvCpssDxChPortDpDebugTxFifoCountersChannelSet dev, tx_index, count_all_channels, channel_to_count\n");
    PR("prvCpssDxChPortDpDebugTxFifoCountersStopConditionSet dev, tx_index, counter_select, stop_threshold\n");
    PR("counter_select (trigger the end of counting) values:\n");
    PR("0: No counter is selected (counters free running) \n");
    PR("1: descriptors counter                            \n");
    PR("2: Cells counter *16                              \n");
    PR("3: Words counter *64                              \n");
    PR("4: SOP counter                                    \n");
    PR("5: Time pass from first descriptor                \n");
    PR("6: Aligner input packets                          \n");
    PR("7: Aligner output packets                         \n");
    PR("prvCpssDxChPortDpDebugTxFifoAlignerCountersModeSet dev, tx_index, aligner_counters_mode\n");
    PR("aligner_counters_mode values:\n");
    PR("0: Aligner all input packets                                \n");
    PR("1: Aligner input non-PCH packets                            \n");
    PR("2: Aligner input packets with word remove due to CRC removal\n");
    PR("3: Aligner input words with error                           \n");
    PR("prvCpssDxChPortDpDebugTxFifoDebugTagSet dev, tx_index, debug_tag\n");
    PR("\n");
    PR("prvCpssDxChPortDpDebugRxDmaStatusPrint dev, rx_index\n");
    PR("prvCpssDxChPortDpDebugRxDmaCountersPrint dev, rx_index\n");
    PR("prvCpssDxChPortDpDebugRxDmaGlobalConfig dev, rx_index, countigEnable, stopCountigEnable, stopCyclesNum\n");
    PR("prvCpssDxChPortDpDebugRxDmaCounterConfig devNum, rxIndex, counterName, typeMask, typeRef\n");
    PR("for help prvCpssDxChPortDpDebugRxDmaCounterConfig 0, 0, \"help\", 0, 0\n");
    return GT_OK;
}

/* Packet Buffer */


/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketWriteStatusPrint function
* @endinternal
*
* @brief   Print PB Gpc Packet Write debug status and counters registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketWrite unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketWriteStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketWrite[unitIndex].interruptCause;
    rc = regTileUnitRead("interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupt cause 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketWrite[unitIndex].interruptMask;
    rc = regTileUnitRead("interrupt mask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupt mask 0x%08X \n", data);
    }

    PR("Statistics Counters:\n");
    for (i = 0; (i < 24); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketWrite[unitIndex].statisticsCounters[i];
        rc = regTileUnitRead("Statistics Counters", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
            if ((i % 8) == 7) PR("\n");
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketWriteDbgClearRegMaskWrite function
* @endinternal
*
* @brief   Print PB Gpc Packet Write debug clear register.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketWrite unit index.
* @param[in] mask                     - Write mask.
* @param[in] value                    - Write value.
*                                       Bits 19-7  Clear internal state per channel
*                                       Bits 6-3  Clear internal credit counters
*                                       6: Write port 2
*                                       5: Write port 1
*                                       4: Write port 0
*                                       3: Free list
*                                       Bits 2-0  Clear internal FIFO buffers
*                                       2: Next-pointer
*                                       1: Cell pointer
*                                       0: Free list
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketWriteDbgClearRegMaskWrite
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               mask,
    IN  GT_U32               value
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketWrite[unitIndex].debugClear;
    return regTileUnitWriteMask("debug clear", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadInterruptsPrint function
* @endinternal
*
* @brief   Print PB Gpc Packet Read interrupt registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadInterruptsPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS                           rc = GT_OK;
    GT_STATUS                           rc1 = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              mask;
    GT_U32                              i;
    GT_U32                              numOfChannels;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfChannels = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 28 : 10;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    PR("interrupt causes of channels:\n");
    for (i = 0; (i < numOfChannels); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.channelCause[i];
        rc = regTileUnitRead("channel interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }
    }
    PR("\n");
    PR("interrupt masks of channels:\n");
    for (i = 0; (i < numOfChannels); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.channelMask[i];
        rc = regTileUnitRead("channel interrupt mask", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetCountCause;
    rc = regTileUnitRead("packet count interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetCountMask;
    rc1 = regTileUnitRead("packet count interrupt mask", tileIndex, unitIndex, devNum, regAddr, &mask);
    if ((rc == GT_OK) && (rc1 == GT_OK))
    {
        PR("packet count interrupt cause 0x%08X mask 0x%08X \n", data, mask);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.miscCause;
    rc = regTileUnitRead("miscelanous interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.miscMask;
    rc1 = regTileUnitRead("miscelanous interrupt mask", tileIndex, unitIndex, devNum, regAddr, &mask);
    if ((rc == GT_OK) && (rc1 == GT_OK))
    {
        PR("miscelanous interrupt cause 0x%08X mask 0x%08X \n", data, mask);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetCountCreditCause;
    rc = regTileUnitRead("packet count credit interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetCountCreditMask;
    rc1 = regTileUnitRead("packet count credit interrupt mask", tileIndex, unitIndex, devNum, regAddr, &mask);
    if ((rc == GT_OK) && (rc1 == GT_OK))
    {
        PR("packet count credit interrupt cause 0x%08X mask 0x%08X \n", data, mask);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetReadCreditCause;
    rc = regTileUnitRead("packet read credit interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.packetReadCreditMask;
    rc1 = regTileUnitRead("packet read credit interrupt mask", tileIndex, unitIndex, devNum, regAddr, &mask);
    if ((rc == GT_OK) && (rc1 == GT_OK))
    {
        PR("packet read credit interrupt cause 0x%08X mask 0x%08X \n", data, mask);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.summaryCause;
    rc = regTileUnitRead("summary interrupt cause", tileIndex, unitIndex, devNum, regAddr, &data);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].interrupts.summaryMask;
    rc1 = regTileUnitRead("summary interrupt mask", tileIndex, unitIndex, devNum, regAddr, &mask);
    if ((rc == GT_OK) && (rc1 == GT_OK))
    {
        PR("summary interrupt cause 0x%08X mask 0x%08X \n", data, mask);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadStatusPrint function
* @endinternal
*
* @brief   Print PB Gpc Packet Read debug status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              i;
    GT_U32                              numOfChannels;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    numOfChannels = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 28 : 10;

    PR("fifoStatus0Channel channels 0-9:\n");
    for (i = 0; (i < numOfChannels); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.fifoStatus0Channel[i];
        rc = regTileUnitRead("fifoStatus0Channel", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }
    }
    PR("\n");

    PR("fifoStatus1Channel channels 0-9:\n");
    for (i = 0; (i < numOfChannels); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.fifoStatus1Channel[i];
        rc = regTileUnitRead("fifoStatus1Channel", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
        }
    }
    PR("\n");

    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.fillStatus0;
        rc = regTileUnitRead("fifoStatus0", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Reorder Maximum Fill: ");
            PR("Level2 %d ", ((data >> 24) & 0xFF));
            PR("Level1 %d ", ((data >> 16) & 0xFF));
            PR("Level0 %d ", ((data >> 8) & 0xFF));
            PR("Head-tail %d ", (data & 0xFF));
            PR("\n");
        }

        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.fillStatus1;
        rc = regTileUnitRead("fifoStatus1", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Reorder Cell Maximum Fill %d \n", (data & 0xFF));
        }

        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.pktCntFillStatus;
        rc = regTileUnitRead("pktCntFillStatus", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Packet Count Reorder Maximum Fill: ");
            PR("Cell %d ", ((data >> 24) & 0xFF));
            PR("Level2 %d ", ((data >> 16) & 0xFF));
            PR("Level1 %d ", ((data >> 8) & 0xFF));
            PR("Level0 %d ", (data & 0xFF));
            PR("\n");
        }
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.cellCntChannelMask;
    rc = regTileUnitRead("cellCntChannelMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Cell Counters Channel Mask 0x%X \n", (data & 0x3FF));
    }

    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.descriptorCellCnt;
        rc = regTileUnitRead("descriptorCellCnt", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Descriptor Cell Counter 0x%X \n", data);
        }

        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.requestCellCnt;
        rc = regTileUnitRead("requestCellCnt", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("Request Cell Counter 0x%X \n", data);
        }
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.requestCellCnt;
        rc = regTileUnitRead("reorderFreeListFillLevel0", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("reorderFreeListFillLevel0 0x%X \n", data);
        }
        rc = regTileUnitRead("reorderFreeListFillLevel1", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("reorderFreeListFillLevel1 0x%X \n", data);
        }
        rc = regTileUnitRead("pcReorderFreeListFillLevel", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("pcReorderFreeListFillLevel 0x%X \n", data);
        }
        rc = regTileUnitRead("cellCntChannelMask", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("cellCntChannelMask 0x%X \n", data);
        }
        rc = regTileUnitRead("descriptorCellCnt", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("descriptorCellCnt 0x%X \n", data);
        }
        rc = regTileUnitRead("requestCellCnt", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("requestCellCnt 0x%X \n", data);
        }
        rc = regTileUnitRead("mergeFifoFillLevel", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("mergeFifoFillLevel 0x%X \n", data);
        }
        rc = regTileUnitRead("pcMergeFifoFillLevel", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("pcMergeFifoFillLevel 0x%X \n", data);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadCellCntChannelMaskSet function
* @endinternal
*
* @brief   Set PB Gpc Packet Read debug counters channel mask.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
* @param[in] cellCntChannelMask       - cell Cnt Channel Mask.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadCellCntChannelMaskSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               cellCntChannelMask
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.cellCntChannelMask;

    mask  = 0x3FF;
    value = (cellCntChannelMask & 0x3FF);

    return regTileUnitWriteMask("cellCntChannelMask", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersChannelSet function
* @endinternal
*
* @brief   Set PB Gpc Packet Read channel to be counted by debug channel credit counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
* @param[in] channel                  - channel counted by debug channel credit counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersChannelSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel

)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.creditCntControl;

    mask  = 0x3F;
    value = (channel & 0x3F);

    return regTileUnitWriteMask("creditCntControl", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersPrint function
* @endinternal
*
* @brief   Print PB Gpc Packet Read debug credit counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.creditCntControl;
    rc = regTileUnitRead("creditCntControl", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Credit Counter Control Channel 0x%08X ", (data & 0x3F));
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.reorderChannelCreditCnt0;
    rc = regTileUnitRead("reorderChannelCreditCnt0", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Reorder Channel Credit Counters: ");
        PR("Level2 %d ", ((data >> 24) & 0xFF));
        PR("Level1 %d ", ((data >> 16) & 0xFF));
        PR("Level0 %d ", ((data >> 8) & 0xFF));
        PR("Head-tail %d ", (data & 0xFF));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.reorderChannelCreditCnt1;
    rc = regTileUnitRead("reorderChannelCreditCnt1", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Reorder Channel Credit Counter Cell %d \n", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.reorderCommandCreditCnt0;
    rc = regTileUnitRead("reorderCommandCreditCnt0", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Reorder Command Credit Counters: ");
        PR("Level2 %d ", ((data >> 24) & 0xFF));
        PR("Level1 %d ", ((data >> 16) & 0xFF));
        PR("Level0 %d ", ((data >> 8) & 0xFF));
        PR("Head-tail %d ", (data & 0xFF));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.reorderCommandCreditCnt1;
    rc = regTileUnitRead("reorderCommandCreditCnt1", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Reorder Command Credit Counter Cell %d \n", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.commandQueueCreditCnt0;
    rc = regTileUnitRead("commandQueueCreditCnt0", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Command Queue Credit Counters: ");
        PR("Level2 %d ", ((data >> 24) & 0xFF));
        PR("Level1 %d ", ((data >> 16) & 0xFF));
        PR("Level0 %d ", ((data >> 8) & 0xFF));
        PR("Head-tail %d ", (data & 0xFF));
        PR("\n");
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.commandQueueCreditCnt1;
    rc = regTileUnitRead("commandQueueCreditCnt1", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Command Queue Credit Counter Cell %d \n", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.txRequestCnt;
    rc = regTileUnitRead("txRequestCnt", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Credit counter for TX request credits %d \n", (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.cellReadCreditCnt;
    rc = regTileUnitRead("cellReadCreditCnt", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Credit counter for Cell Read cell credits %d \n", (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.readNpmRequestMswCreditCnt;
    rc = regTileUnitRead("readNpmRequestMswCreditCnt", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Credit counter for cell merge queue credits %d \n", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.credits.countNpmRequestCreditCnt;
    rc = regTileUnitRead("countNpmRequestCreditCnt", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Packet Count NPM Request Credit Counter: ");
        PR("Level2 %d ", ((data >> 20) & 0x1F));
        PR("Level1 %d ", ((data >> 15) & 0x1F));
        PR("Level0 %d ", ((data >> 10) & 0x1F));
        PR("Cell1 %d ", ((data >> 5) & 0x1F));
        PR("Cell0 %d ", (data & 0x1F));
        PR("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadLatencyMeasureEnableSet function
* @endinternal
*
* @brief   Set PB Gpc Packet Read Latency measure mechanism enable.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
* @param[in] priority                 - supported priorities 0 and 1.
* @param[in] enable                   - GT_TRUE enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadLatencyMeasureEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               priority,
    IN  GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsEnable;

    mask  = (1 << priority);
    value = (BOOL2BIT_MAC(enable) << priority);

    return regTileUnitWriteMask("latency.lsEnable", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadLatencyConfigSet function
* @endinternal
*
* @brief   Configure PB Gpc Packet Read Latency measure mechanism and reset counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
* @param[in] capture                  - to capture 0 - false and 1 - true.
* @param[in] priority                 - supported priorities 0 and 1.
* @param[in] minLatencyThreshold      - minimal Latency Threshold.
* @param[in] maxLatencyThreshold      - maximal Latency Threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadLatencyConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               capture,
    IN  GT_U32               priority,
    IN  GT_U32               minLatencyThreshold,
    IN  GT_U32               maxLatencyThreshold
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsMinTheshold[priority];
    mask  = 0xFFFFFFFF;
    value = minLatencyThreshold;
    rc = regTileUnitWriteMask("latency.lsMinTheshold", tileIndex, unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsMaxTheshold[priority];
    mask  = 0xFFFFFFFF;
    value = maxLatencyThreshold;
    rc = regTileUnitWriteMask("latency.lsMaxTheshold", tileIndex, unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsCapture;
    mask  = (1 << priority);
    value = ((capture & 1) << priority);

    return regTileUnitWriteMask("latency.lsCapture", tileIndex, unitIndex, devNum, regAddr, mask, value);
}


/**
* @internal prvCpssDxChPortDpDebugPbGpcPacketReadLatencyCountersPrint function
* @endinternal
*
* @brief   Print PB Gpc Packet Read Debug Latency counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead (tx to memory) ties 0 and 1.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcPacketReadLatencyCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               priority
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsTotalCount[priority];
    rc = regTileUnitRead("latency.lsTotalCount", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The total number of cell latency measurements %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsThesholdCount[priority];
    rc = regTileUnitRead("latency.lsThesholdCount", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The number of cell latency measurements that is within the thresholds %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsMinLatency[priority];
    rc = regTileUnitRead("latency.lsMinLatency", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The minimum latency measured since last capture %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].debug.latency.lsMaxLatency[priority];
    rc = regTileUnitRead("latency.lsMaxLatency", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The maximum latency measured since last capture %d \n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcCellReadStatusPrint function
* @endinternal
*
* @brief   Print PB Gpc Cell Read debug status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcPacketRead unit index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcCellReadStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].interruptCause;
    rc = regTileUnitRead("interruptCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Interrupt Cause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].interruptMask;
    rc = regTileUnitRead("interruptMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Interrupt Mask 0x%08X ", data);
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].fifoStatus;
    rc = regTileUnitRead("fifoStatus", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Free List Cell Reference FIFO: ");
        PR("full %d ", ((data >> 11) & 1));
        PR("not-empty %d ", ((data >> 10) & 1));
        PR("empty %d ", ((data >> 9) & 1));
        PR("\n");
        PR("SMB Packet Count Request FIFO: ");
        PR("full  %d ", ((data >> 8) & 1));
        PR("not-empty %d ", ((data >> 7) & 1));
        PR("empty  %d ", ((data >> 6) & 1));
        PR("\n");
        PR("SMB Cell Read Request FIFO: ");
        PR("interface0 - full %d ", ((data >> 4) & 1));
        PR("not-empty %d ", ((data >> 2) & 1));
        PR("empty %d ", (data & 1));
        PR("interface1 - full %d ", ((data >> 5) & 1));
        PR("not-empty %d ", ((data >> 3) & 1));
        PR("empty %d ", ((data >> 1) & 1));
        PR("\n");
    }
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].fillStatus;
    rc = regTileUnitRead("fillStatus", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Cell Reference Free List Maximum Fill %d \n", (data & 0xFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].miscStatus;
    rc = regTileUnitRead("miscStatus", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Debug Multicast Collapse %d \n", (data & 1));
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcCellReadLatencyMeasureEnableSet function
* @endinternal
*
* @brief   Set PB Gpc Cell Read Latency measure mechanism enable.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcCellRead unit index.
* @param[in] priority                 - supported priorities 0 and 1.
* @param[in] enable                   - GT_TRUE enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcCellReadLatencyMeasureEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               priority,
    IN  GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsEnable;

    mask  = (1 << priority);
    value = (BOOL2BIT_MAC(enable) << priority);

    return regTileUnitWriteMask("latency.lsEnable", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbGpcCellReadLatencyConfigSet function
* @endinternal
*
* @brief   Configure PB Gpc Cell Read Latency measure mechanism and reset counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcCellRead unit index.
* @param[in] capture                  - to capture 0 - false and 1 - true.
* @param[in] priority                 - supported priorities 0 and 1.
* @param[in] minLatencyThreshold      - minimal Latency Threshold.
* @param[in] maxLatencyThreshold      - maximal Latency Threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcCellReadLatencyConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               capture,
    IN  GT_U32               priority,
    IN  GT_U32               minLatencyThreshold,
    IN  GT_U32               maxLatencyThreshold
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsMinTheshold[priority];
    mask  = 0xFFFFFFFF;
    value = minLatencyThreshold;
    rc = regTileUnitWriteMask("latency.lsMinTheshold", tileIndex, unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsMaxTheshold[priority];
    mask  = 0xFFFFFFFF;
    value = maxLatencyThreshold;
    rc = regTileUnitWriteMask("latency.lsMaxTheshold", tileIndex, unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsCapture;
    mask  = (1 << priority);
    value = ((capture & 1) << priority);

    return regTileUnitWriteMask("latency.lsCapture", tileIndex, unitIndex, devNum, regAddr, mask, value);
}


/**
* @internal prvCpssDxChPortDpDebugPbGpcCellReadLatencyCountersPrint function
* @endinternal
*
* @brief   Print PB Gpc Cell Read Debug Latency counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] unitIndex                - PbGpcCellRead unit index.
* @param[in] priority                 - supported priorities 0 and 1.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbGpcCellReadLatencyCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               priority
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_DP_PER_TILE_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 2)
    {
        PR("priority %d out of range\n", priority);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsTotalCount[priority];
    rc = regTileUnitRead("latency.lsTotalCount", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The total number of cell latency measurements %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsThesholdCount[priority];
    rc = regTileUnitRead("latency.lsThesholdCount", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The number of cell latency measurements that is within the thresholds %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsMinLatency[priority];
    rc = regTileUnitRead("latency.lsMinLatency", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The minimum latency measured since last capture %d \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.gpcCellRead[unitIndex].latency.lsMaxLatency[priority];
    rc = regTileUnitRead("latency.lsMaxLatency", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("The maximum latency measured since last capture %d \n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbWriteArbiertStatusPrint function
* @endinternal
*
* @brief   Print PB Write Arbiter status.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] waInstance               - write arbiter instance.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbWriteArbiterStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               waInstance
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              numOfRegs;
    GT_U32                              i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (waInstance >= 4)
    {
        PR("waInstance %d out of range\n", waInstance);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].cfgEnable;
    rc = regTileUnitRead("cfgEnable", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgEnable(CFG_EN_EN) %d \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].cfgReady;
    rc = regTileUnitRead("cfgReady", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgReady(CFG_CA_THRESHOLD) %d \n", (data & 0x3F));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].dbgLpmFailLow;
    rc = regTileUnitRead("dbgLpmFailLow", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgLpmFailLow %d ", data);
    }

    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].dbgLpmFailHigh;
        rc = regTileUnitRead("dbgLpmFailHigh", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("dbgLpmFailHigh %d ", data);
        }
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].dbgLpmFailCounter;
    rc = regTileUnitRead("dbgLpmFailCounter", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgLpmFailCounter %d ", data);
    }

    numOfRegs = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 8 : 1;
    PR("dbgLpmRepeatCounter ");
    for (i = 0; (i < numOfRegs); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].dbgLpmRepeatCounter[i];
        rc = regTileUnitRead("dbgLpmRepeatCounter", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("%d ", data);
        }
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].interruptMiscCause;
    rc = regTileUnitRead("", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMiscCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbWriteArbiter[waInstance].interruptMiscMask;
    rc = regTileUnitRead("", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMiscMask 0x%08X \n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbCounterRegistersPrint function
* @endinternal
*
* @brief   Print PB Counter registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCounterRegistersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.enablePbc;
    rc = regTileUnitRead("enablePbc", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("enablePbc %d ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.enableMasterMode;
    rc = regTileUnitRead("enableMasterMode", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("enableMasterMode %d ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.enableInterTileTransact;
    rc = regTileUnitRead("enableInterTileTransact", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("enableInterTileTransact %d ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.cfgTile;
    rc = regTileUnitRead("cfgTile", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgTile %d \n", (data & 7));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbCells;
    rc = regTileUnitRead("resizeSmbCells", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeSmbCells Max %d ", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbMcCells;
    rc = regTileUnitRead("resizeSmbMcCells", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeSmbMcCells %d ", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbSumCells;
    rc = regTileUnitRead("resizeSmbSumCells", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeSmbSumCells %d \n", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmWords;
    rc = regTileUnitRead("resizeNpmWords", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeNpmWords Max %d ", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmMcWords;
    rc = regTileUnitRead("resizeNpmMcWords", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeNpmMcWords %d ", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmSumWords;
    rc = regTileUnitRead("resizeNpmSumWords", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("resizeNpmSumWords %d \n", (data & 0xFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.cfgFillThreshold;
    rc = regTileUnitRead("cfgFillThreshold", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgFillThreshold %d \n", (data & 0x3FF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMax;
    rc = regTileUnitRead("dbgSmbLocalMax", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbLocalMax %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMin;
    rc = regTileUnitRead("dbgSmbLocalMin", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbLocalMin %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalSum;
    rc = regTileUnitRead("dbgSmbLocalSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbLocalSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMcSum;
    rc = regTileUnitRead("dbgSmbLocalMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbLocalMcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMax;
    rc = regTileUnitRead("dbgNpmLocalMax", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmLocalMax %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMin;
    rc = regTileUnitRead("dbgNpmLocalMin", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmLocalMin %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalSum;
    rc = regTileUnitRead("dbgNpmLocalSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmLocalSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMcSum;
    rc = regTileUnitRead("dbgNpmLocalMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmLocalMcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMax;
    rc = regTileUnitRead("dbgSmbMax", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbMax %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMin;
    rc = regTileUnitRead("dbgSmbMin", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbMin %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbSum;
    rc = regTileUnitRead("dbgSmbSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMcSum;
    rc = regTileUnitRead("dbgSmbMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbUcSum;
    rc = regTileUnitRead("dbgSmbUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMax;
    rc = regTileUnitRead("dbgNpmMax", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmMax %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMin;
    rc = regTileUnitRead("dbgNpmMin", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmMin %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmSum;
    rc = regTileUnitRead("dbgNpmSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMcSum;
    rc = regTileUnitRead("dbgNpmMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmUcSum;
    rc = regTileUnitRead("dbgNpmUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbDiff;
    rc = regTileUnitRead("dbgSmbDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbDiff %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmDiff;
    rc = regTileUnitRead("dbgNpmDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmDiff %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxSum;
    rc = regTileUnitRead("dbgSmbOvertimeMaxSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMaxSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxMcSum;
    rc = regTileUnitRead("dbgSmbOvertimeMaxMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMaxMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxUcSum;
    rc = regTileUnitRead("dbgSmbOvertimeMaxUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMaxUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinSum;
    rc = regTileUnitRead("dbgSmbOvertimeMinSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMinSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinMcSum;
    rc = regTileUnitRead("dbgSmbOvertimeMinMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMinMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinUcSum;
    rc = regTileUnitRead("dbgSmbOvertimeMinUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMinUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxDiff;
    rc = regTileUnitRead("dbgSmbOvertimeMaxDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMaxDiff %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinDiff;
    rc = regTileUnitRead("dbgSmbOvertimeMinDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgSmbOvertimeMinDiff %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxSum;
    rc = regTileUnitRead("dbgNpmOvertimeMaxSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMaxSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxMcSum;
    rc = regTileUnitRead("dbgNpmOvertimeMaxMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMaxMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxUcSum;
    rc = regTileUnitRead("dbgNpmOvertimeMaxUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMaxUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinSum;
    rc = regTileUnitRead("dbgNpmOvertimeMinSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMinSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinMcSum;
    rc = regTileUnitRead("dbgNpmOvertimeMinMcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMinMcSum %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinUcSum;
    rc = regTileUnitRead("dbgNpmOvertimeMinUcSum", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMinUcSum %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxDiff;
    rc = regTileUnitRead("dbgNpmOvertimeMaxDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMaxDiff %d ", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinDiff;
    rc = regTileUnitRead("dbgNpmOvertimeMinDiff", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("dbgNpmOvertimeMinDiff %d \n", (data & 0xFFFFFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.interruptCause;
    rc = regTileUnitRead("interruptCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.interruptMask;
    rc = regTileUnitRead("interruptMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMask 0x%08X \n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterRegistersPrint function
* @endinternal
*
* @brief   Print PB Center registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterRegistersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.npmRandomizerEnable;
        rc = regTileUnitRead("npmRandomizerEnable", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("npmRandomizer Credit Generation enable  input3 %d input2 %d input1 %d input0 %d \n",
                ((data >> 7) & 1), ((data >> 6) & 1), ((data >> 5) & 1), ((data >> 4) & 1));
            PR("npmRandomizer enable output3 %d output2 %d output1 %d output0 %d \n",
                ((data >> 3) & 1), ((data >> 2) & 1), ((data >> 1) & 1), (data & 1));
        }

        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.tileId;
        rc = regTileUnitRead("tileId", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("tileId %d ", (data & 3));
        }

        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.tileMode;
        rc = regTileUnitRead("tileMode", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("tileMode %d \n", (data & 3));
        }
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswCollapceCfg;
    rc = regTileUnitRead("mswCollapceCfg", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("mswCollapceCfg enable SMB refCnt %d enable not-SMB refCnt %d ",
           ((data >> 1)& 1), (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.pbOutOfReset;
    rc = regTileUnitRead("pbOutOfReset", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("pbOutOfReset %d \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug0;
    rc = regTileUnitRead("mswDebug0", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("mswDebug0 MSW NPM CPRR Dequeue Disable 0x%05X MSW NPM Write Dequeue Disable 0x%02X ",
           ((data >> 8) & 0xFFFFF), (data & 0xFF));
    }

    if (! PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug1;
        rc = regTileUnitRead("mswDebug1", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("mswDebug1 MSW NPM CPPR level Dequeue Disable 0x%06X \n", (data & 0xFFFFFF));
        }
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.pbCenterCause;
    rc = regTileUnitRead("interrupts.pbCenterCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.pbCenterCause 0x%08X ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.pbCenterMask;
    rc = regTileUnitRead("interrupts.pbCenterMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.pbCenterMask 0x%08X \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.npmSumCause;
    rc = regTileUnitRead("interrupts.npmSumCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.npmSumCause 0x%08X ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.npmSumMask;
    rc = regTileUnitRead("interrupts.npmSumMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.npmSumMask 0x%08X \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.gpcSumCause;
    rc = regTileUnitRead("interrupts.gpcSumCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.gpcSumCause 0x%08X ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.gpcSumMask;
    rc = regTileUnitRead("interrupts.gpcSumMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.gpcSumMask 0x%08X \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.smbSumCause;
    rc = regTileUnitRead("interrupts.smbSumCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.smbSumCause 0x%08X ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.smbSumMask;
    rc = regTileUnitRead("interrupts.smbSumMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.smbSumMask 0x%08X \n", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.pbSumCause;
    rc = regTileUnitRead("interrupts.pbSumCause", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.pbSumCause 0x%08X ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.interrupts.pbSumMask;
    rc = regTileUnitRead("interrupts.pbSumMask", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interrupts.pbSumMask 0x%08X ", (data & 1));
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterMswCollapceCfgSet function
* @endinternal
*
* @brief   Set PB MswCollapceCfg Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileIndex                - tile index.
* @param[in] mswSmbRefCntEnable       - msw Smb RefCnt Enable.
* @param[in] mswNonSmbRefCntEnable    - msw NonSmb RefCnt Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterMswCollapceCfgSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_BOOL              mswSmbRefCntEnable,
    IN  GT_BOOL              mswNonSmbRefCntEnable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswCollapceCfg;
    data = ((BOOL2BIT_MAC(mswSmbRefCntEnable) << 1) | BOOL2BIT_MAC(mswNonSmbRefCntEnable));
    return regTileUnitWriteMask(
        "mswCollapceCfg", tileIndex, 0/*unitIndex*/, devNum, regAddr, 3/*mask*/, data);
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterFalconMswDebugCfgSet function
* @endinternal
*
* @brief   Set PB MswDebug Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                        - device number
* @param[in] tileIndex                     - tile index.
* @param[in] npmCprrDequeueDisableBmp,     - npmCprrDequeueDisableBmp.
* @param[in] npmWriteDequeueDisableBmp,    - npmWriteDequeueDisableBmp.
* @param[in] npmCprrDequeueLevelDisableBmp - npmCprrDequeueLevelDisableBmp.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterFalconMswDebugCfgSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               npmCprrDequeueDisableBmp,
    IN  GT_U32               npmWriteDequeueDisableBmp,
    IN  GT_U32               npmCprrDequeueLevelDisableBmp
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d SIP6_10, use prvCpssDxChPortDpDebugPbCenterHawkMswDequeCfgSet\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug0;
    value = (((npmCprrDequeueDisableBmp & 0xFFFFF) << 8) | (npmWriteDequeueDisableBmp & 0xFF));
    mask  = 0xFFFFFFF;
    rc = regTileUnitWriteMask(
        "mswDebug0", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug1;
    value = (npmCprrDequeueLevelDisableBmp & 0xFFFFFF);
    mask  = 0xFFFFFFF;
    rc = regTileUnitWriteMask(
        "mswDebug1", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);

    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterHawkMswDequeCfgSet function
* @endinternal
*
* @brief   Set PB MswDeque Configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                        - device number
* @param[in] smbCprDequeueDisableBmp       - smbCprDequeueDisableBmp.
* @param[in] smbCrrDequeueDisableBmp       - smbCrrDequeueDisableBmp.
* @param[in] npmWriteDequeueDisableBmp     - npmWriteDequeueDisableBmp.
* @param[in] npmPcprDequeueDisableBmp      - npmPcprDequeueDisableBmp.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterHawkMswDequeCfgSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               smbCprDequeueDisableBmp,
    IN  GT_U32               smbCrrDequeueDisableBmp,
    IN  GT_U32               npmWriteDequeueDisableBmp,
    IN  GT_U32               npmPcprDequeueDisableBmp
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug0;
    value = (((smbCprDequeueDisableBmp & 0xFFF) << 4) | (smbCrrDequeueDisableBmp & 0xF));
    mask  = 0xFFFF;
    rc = regTileUnitWriteMask(
        "mswDebug0", 0/*tileIndex*/, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    if (! PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.mswDebug1;
        value = ((npmPcprDequeueDisableBmp & 0xFFFFFF) | ((npmWriteDequeueDisableBmp & 0xF) << 24));
        mask  = 0xFFFFFFF;
        rc = regTileUnitWriteMask(
            "mswDebug1", 0/*tileIndex*/, 0/*unitIndex*/, devNum, regAddr, mask, value);
        {
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterDebugRingStart function
* @endinternal
*
* @brief   Configure and start PB debug ring.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] ringSelect            - ringSelect.
* @param[in] matchValue            - matchValue.
* @param[in] matchMask             - matchMask.
* @param[in] stopCyclesNum         - stopCyclesNum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterDebugRingStart
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               ringSelect,
    IN  GT_U32               matchValue,
    IN  GT_U32               matchMask,
    IN  GT_U32               stopCyclesNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* clear match count register */
    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchCount;
    value = 0;
    mask  = 0xFFFFFFFF;
    rc = regTileUnitWriteMask(
        "debugRing.drMatchCount", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* select bitmap */
    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drSelect;
    value = (ringSelect & 0xFFFFFF);
    mask  = 0xFFFFFF;
    rc = regTileUnitWriteMask(
        "debugRing.drSelect", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchValue;
    value = matchValue;
    mask  = 0xFFFFFFFF;
    rc = regTileUnitWriteMask(
        "debugRing.drMatchValue", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchMask;
    value = matchMask;
    mask  = 0xFFFFFFFF;
    rc = regTileUnitWriteMask(
        "debugRing.drMatchMask", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* configuration: stop cycles number and trigger bit */
    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drTraceCfg;
    value = (((stopCyclesNum & 0x3F) << 1) | 1);
    mask  = 0x7F;
    rc = regTileUnitWriteMask(
        "debugRing.drTraceCfg", tileIndex, 0/*unitIndex*/, devNum, regAddr, mask, value);
    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugPbCenterDebugRingTracePrint function
* @endinternal
*
* @brief   Print PB debug ring trace.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbCenterDebugRingTracePrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchCount;
    rc = regTileUnitRead(
        "debugRing.drMatchCount", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("drMatchCount %d ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchMin;
    rc = regTileUnitRead(
        "debugRing.drMatchMin", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("drMatchMin %d ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drMatchMax;
    rc = regTileUnitRead(
        "debugRing.drMatchMax", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("drMatchMax %d ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drTraceLastWritePtr;
    rc = regTileUnitRead(
        "debugRing.drTraceLastWritePtr", tileIndex, 0, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("drTraceLastWritePtr %d \n", data);
    }

    PR("Debug Ring Trace: \n", data);
    for (i = 0; (i < 16); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.pbCenter.debugRing.drTraceArray[i];
        rc = regTileUnitRead(
            "debugRing.drTraceArray", tileIndex, 0, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("0x%08X ", data);
            if ((i % 8) == 7)
            {
                PR("\n");
            }
        }
    }
    PR("\n");

    return GT_OK;
}

/* SMB_MC */

/**
* @internal prvCpssDxChPortDpDebugPbSmbRamPrint function
* @endinternal
*
* @brief  dump RAM memory.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] baseCellIndex         - cell address of the entry being read.
* @param[in] numOfLines            - number of cells to print
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbSmbRamPrint
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileIndex,
    IN  GT_U32      unitIndex,
    IN  GT_U32      baseCellIndex,
    IN  GT_U32      numOfLines
)
{
    GT_STATUS   rc;          /* return code */
    GT_U32      regAddr;     /* register adress */
    GT_U32      value;       /* register's value */
    GT_U32      mask;        /* mask */
    GT_U32      cellToPrint; /* cell index to print */
    GT_U32      ii,jj;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 6)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (numOfLines < 1)
    {
        PR("number of lines to read must be bigger than 1\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((baseCellIndex + numOfLines - 1)  > 0xfff)
    {
        PR("cells indexes [0x%X-0x%X] out of range \n", baseCellIndex , (baseCellIndex + numOfLines - 1) );
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    PR("+------+----------------------------------------------------------------------------------------+\n");
    PR("|index |data[0]    data[1]    data[2]    data[3]    data[4]    data[5]    data[6]    data[7]    |\n");
    PR("+------+----------------------------------------------------------------------------------------+\n");
    for (jj = 0 ; jj < numOfLines ; jj++)
    {
        cellToPrint = baseCellIndex + jj;
        /* start reading from RAM */
        regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].dbgPayloadCtrl;
        mask = 0x3FFFF;
        rc = regTileUnitWriteMask("dbgPayloadCtrl", tileIndex, unitIndex, devNum, regAddr, mask, cellToPrint);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsTimerWkAfter(500);

        /* verify reading action is done */
        rc = regTileUnitRead("dbgPayloadCtrl", tileIndex, unitIndex, devNum, regAddr, &value);
        if (rc!= GT_OK && !(value & (1<<17)))
        {
            PR("reading Failed\n");
            return rc;
        }

        PR("|0x%03X |",jj);
        /* print RAM's blocks */
        for (ii = 0 ; ii < 8 ; ii++)
        {
            regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].dbgPayloadData[ii];
            rc = regTileUnitRead("dbgPayloadData[]", tileIndex, unitIndex, devNum, regAddr, &value);
            if (rc == GT_OK)
            {
                PR("0x%08X ", value);
            }
            else
            {
                PR("Error     ");
            }
        }
        PR("|\n");
        PR("+------+----------------------------------------------------------------------------------------+\n");
    }



    return rc;
}

/**
* @internal prvCpssDxChPortDpDebugPbSmbStatusPrint function
* @endinternal
*
* @brief   Print SMB_MC status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbSmbStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 6)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].cfgEnable;
    rc = regTileUnitRead(
        "cfgEnable", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgEnable %d ", (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].cfgList;
    rc = regTileUnitRead(
        "cfgList", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgList select delay %d enable delay %d ", ((data >> 1) & 1), (data & 1));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].cfgMsws;
    rc = regTileUnitRead(
        "cfgMsws", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgMsws 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMiscCause;
    rc = regTileUnitRead(
        "interruptMiscCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMiscCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMiscMask;
    rc = regTileUnitRead(
        "interruptMiscMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMiscMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptAgeCause;
    rc = regTileUnitRead(
        "interruptAgeCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptAgeCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptAgeMask;
    rc = regTileUnitRead(
        "interruptAgeMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptAgeMask 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptFifoCause;
    rc = regTileUnitRead(
        "interruptFifoCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptFifoCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptFifoMask;
    rc = regTileUnitRead(
        "interruptFifoMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptFifoMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptRwbCause;
    rc = regTileUnitRead(
        "interruptRwbCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptRwbCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptRwbMask;
    rc = regTileUnitRead(
        "interruptRwbMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptRwbMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMemoriesCause;
    rc = regTileUnitRead(
        "interruptMemoriesCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMemoriesCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMemoriesMask;
    rc = regTileUnitRead(
        "interruptMemoriesMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMemoriesMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMemories2Cause;
    rc = regTileUnitRead(
        "interruptMemories2Cause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMemories2Cause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptMemories2Mask;
    rc = regTileUnitRead(
        "interruptMemories2Mask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptMemories2Mask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptSummaryCause;
    rc = regTileUnitRead(
        "interruptSummaryCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptSummaryCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].interruptSummaryMask;
    rc = regTileUnitRead(
        "interruptSummaryMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("interruptSummaryMask 0x%08X \n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPbSmbDelayedInsertionSet function
* @endinternal
*
* @brief   Set SMB_MC delay insertion configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] selectDelay           - selectDelay
* @param[in] enableDelay           - enableDelay
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbSmbDelayedInsertionSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_BOOL              selectDelay,
    IN  GT_BOOL              enableDelay
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              value;
    GT_U32                              mask;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 6)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].cfgList;
    mask  = 0x3;
    value = ((BOOL2BIT_MAC(selectDelay) << 1) | BOOL2BIT_MAC(enableDelay));
    return regTileUnitWriteMask(
        "cfgList", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbSmbStatusPrint function
* @endinternal
*
* @brief   Print SMB_MC status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] disableMswDequeueingBmp - bitmap of disable dequeueing MSW clients.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbSmbDisableMswDequeueingSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_U32               disableMswDequeueingBmp
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              value;
    GT_U32                              mask;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 6)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.smbMc[unitIndex].cfgMsws;
    mask  = 0xFFFFFFFF;
    value = disableMswDequeueingBmp;
    return regTileUnitWriteMask(
        "cfgMsws", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/* NPM_MC */

/**
* @internal prvCpssDxChPortDpDebugPbNpmDelayedInsertionSet function
* @endinternal
*
* @brief   Set NPM_MC delay insertion configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] selectDelay           - selectDelay
* @param[in] enableDelay           - enableDelay
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbNpmDelayedInsertionSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex,
    IN  GT_BOOL              selectDelay,
    IN  GT_BOOL              enableDelay
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              value;
    GT_U32                              mask;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 3)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].cfgList;
    mask  = 0x3;
    value = ((BOOL2BIT_MAC(selectDelay) << 1) | BOOL2BIT_MAC(enableDelay));
    return regTileUnitWriteMask(
        "cfgList", tileIndex, unitIndex, devNum, regAddr, mask, value);
}

/**
* @internal prvCpssDxChPortDpDebugPbNpmStatusPrint function
* @endinternal
*
* @brief   Print NPM_MC status registers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileIndex             - tile index.
* @param[in] unitIndex             - unit index inside the tile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPbNpmStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileIndex,
    IN  GT_U32               unitIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ( tileIndex && tileIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles)
    {
        PR("tileIndex %d out of range\n", tileIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= 3)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].cfgEnable;
    rc = regTileUnitRead(
        "cfgEnable", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgEnable 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].cfgUnit;
    rc = regTileUnitRead(
        "cfgUnit", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgUnit reinsert timeout %d ", (data & 0xFF));
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].cfgRefs;
    rc = regTileUnitRead(
        "cfgRefs", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgRefs aging timeuot 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].cfgList;
    rc = regTileUnitRead(
        "cfgList", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("cfgList select delay %d enable delay %d \n", ((data >> 1) & 1), (data & 1));
    }

    PR("insList: ");
    for (i = 0; (i < 4); i++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].insList[i];
        rc = regTileUnitRead(
            "insList", tileIndex, unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR("%d ", (data & 0xFFF));
        }
    }
    PR("\n");

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.summaryCause;
    rc = regTileUnitRead(
        "interrupts.summaryCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("summaryCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.summaryMask;
    rc = regTileUnitRead(
        "interrupts.summaryMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("summaryMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.genericCause;
    rc = regTileUnitRead(
        "interrupts.genericCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("genericCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.genericMask;
    rc = regTileUnitRead(
        "interrupts.genericMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("genericMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitErrorCause;
    rc = regTileUnitRead(
        "interrupts.unitErrorCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitErrorCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitErrorMask;
    rc = regTileUnitRead(
        "interrupts.unitErrorMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitErrorMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.reinsertCause;
    rc = regTileUnitRead(
        "interrupts.reinsertCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("reinsertCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.reinsertMask;
    rc = regTileUnitRead(
        "interrupts.reinsertMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("reinsertMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.reinsertFailCause;
    rc = regTileUnitRead(
        "interrupts.reinsertFailCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("reinsertFailCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.reinsertFailMask;
    rc = regTileUnitRead(
        "interrupts.reinsertFailMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("reinsertFailMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitRefsCause;
    rc = regTileUnitRead(
        "interrupts.unitRefsCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitRefsCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitRefsMask;
    rc = regTileUnitRead(
        "interrupts.unitRefsMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitRefsMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitDataCause;
    rc = regTileUnitRead(
        "interrupts.unitDataCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitDataCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.unitDataMask;
    rc = regTileUnitRead(
        "interrupts.unitDataMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitDataMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.refsErrorCause;
    rc = regTileUnitRead(
        "interrupts.refsErrorCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("refsErrorCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.refsErrorMask;
    rc = regTileUnitRead(
        "interrupts.refsErrorMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("refsErrorMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.agingErrorCause;
    rc = regTileUnitRead(
        "interrupts.agingErrorCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("agingErrorCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.agingErrorMask;
    rc = regTileUnitRead(
        "interrupts.agingErrorMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("agingErrorMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.refsFifoCause;
    rc = regTileUnitRead(
        "interrupts.refsFifoCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("refsFifoCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.refsFifoMask;
    rc = regTileUnitRead(
        "interrupts.refsFifoMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("refsFifoMask 0x%08X \n", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.listErrorCause;
    rc = regTileUnitRead(
        "interrupts.listErrorCause", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("listErrorCause 0x%08X ", data);
    }

    regAddr = regsAddrPtr->sip6_packetBuffer.npmMc[unitIndex].interrupts.listErrorMask;
    rc = regTileUnitRead(
        "interrupts.listErrorMask", tileIndex, unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("listErrorMask 0x%08X \n", data);
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortDpDebugPbHelp(void)
{
    PR("prvCpssDxChPortDpDebugPbGpcPacketWriteStatusPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketWriteDbgClearRegMaskWrite dev, tile, unit, mask, value\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadInterruptsPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadStatusPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadCellCntChannelMaskSet dev, tile, unit, channelMask\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersChannelSet dev, tile, unit, channel\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadCreditCountersPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadLatencyMeasureEnableSet dev, tile, unit, priority, enable\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadLatencyConfigSet dev, tile, unit,\n"
       "       capture, priority, minLatencyThreshold, minLatencyThreshold\n");
    PR("prvCpssDxChPortDpDebugPbGpcPacketReadLatencyCountersPrint dev, tile, unit, priority\n");
    PR("prvCpssDxChPortDpDebugPbGpcCellReadStatusPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbGpcCellReadLatencyMeasureEnableSet dev, tile, unit, priority, enable\n");
    PR("prvCpssDxChPortDpDebugPbGpcCellReadLatencyConfigSet dev, tile, unit,\n"
       "       capture, priority, minLatencyThreshold, minLatencyThreshold\n");
    PR("prvCpssDxChPortDpDebugPbGpcCellReadLatencyCountersPrint dev, tile, unit, priority\n");
    PR("prvCpssDxChPortDpDebugPbWriteArbiterStatusPrint dev, tile, waInstance\n");
    PR("prvCpssDxChPortDpDebugPbCounterRegistersPrint dev, tile\n");
    PR("prvCpssDxChPortDpDebugPbCenterRegistersPrint dev, tile\n");
    PR("prvCpssDxChPortDpDebugPbCenterMswCollapceCfgSet dev, tile,\n"
       "       mswSmbRefCntEnable, mswNonSmbRefCntEnable\n");
    PR("prvCpssDxChPortDpDebugPbCenterFalconMswDebugCfgSet dev, tile,\n"
       "       npmCprrDequeueDisableBmp, npmWriteDequeueDisableBmp, npmCprrDequeueLevelDisableBmp\n");
    PR("prvCpssDxChPortDpDebugPbCenterHawkMswDequeCfgSet dev, tile,\n"
       "       smbCprDequeueDisableBmp, smbCrrDequeueDisableBmp,\n"
       "       npmWriteDequeueDisableBmp, npmPcprDequeueDisableBmp\n");
    PR("prvCpssDxChPortDpDebugPbCenterDebugRingStart dev, tile,\n"
       "       ringSelect, matchValue, matchMask, stopCyclesNum\n");
    PR("prvCpssDxChPortDpDebugPbCenterDebugRingTracePrint dev, tile\n");
    PR("prvCpssDxChPortDpDebugPbSmbStatusPrint dev, tile, unit\n");
    PR("prvCpssDxChPortDpDebugPbSmbDelayedInsertionSet dev, tile, unit, selectDelay, enableDelay\n");
    PR("prvCpssDxChPortDpDebugPbSmbDisableMswDequeueingSet dev, tile, unit, disableMswDequeueingBmp\n");
    PR("prvCpssDxChPortDpDebugPbNpmDelayedInsertionSet dev, tile, unit, selectDelay, enableDelay\n");
    PR("prvCpssDxChPortDpDebugPbNpmStatusPrint dev, tile, unit\n");

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaSffPrint function
* @endinternal
*
* @brief   Print PCA SFF registers.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaSffGlobalPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               printFifoRings
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (printFifoRings)
    {
        /* print channel configuration, channelControlEnable, channelControlConfig registers*/
        prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump(devNum, unitIndex);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    rc = regRead("globalConfig", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "Credit Sync Theshold %d FW Command %d Token Timeout interval %d\n",
            (data & 0x7), ((data >> 4) & 0x3), ((data >> 8) & 0xFF));
        PR(
            "Statistics: Channel select %d SOP not EOP %d Collection enable %d\n",
            ((data >>16) & 0xFF), ((data >> 23) & 0x1), ((data >> 24) & 0x1));
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].spareConfig;
    rc = regRead("spareConfig", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("spareConfig 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalInterruptCause;
    rc = regRead("globalInterruptCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("globalInterrupt Cause 0x%08X ", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalInterruptMask;
    rc = regRead("globalInterruptMask", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Mask 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].unitInterruptSummaryCause;
    rc = regRead("unitInterruptSummaryCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("unitInterruptSummary Cause 0x%08X ", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].unitInterruptSummaryMask;
    rc = regRead("unitInterruptSummaryMask", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Mask 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_0_15_InterruptSummaryCause;
    rc = regRead("channel_0_15_InterruptSummaryCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("channel_0_15_InterruptSummary Cause 0x%08X ", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_0_15_InterruptSummaryMask;
    rc = regRead("channel_0_15_InterruptSummaryMask", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Mask 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_16_31_InterruptSummaryCause;
    rc = regRead("channel_16_31_InterruptSummaryCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("channel_16_31_InterruptSummary Cause 0x%08X ", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_16_31_InterruptSummaryMask;
    rc = regRead("channel_16_31_InterruptSummaryMask", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Mask 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalStatus;
    rc = regRead("globalStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("globalStatus 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdAndGlobalPacketInStatus;
    rc = regRead("selChIdAndGlobalPacketInStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdAndGlobalPacketInStatus 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistSopOrEop;
    rc = regRead("selChIdStatistSopOrEop", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistSopOrEop 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExp24;
    rc = regRead("selChIdStatistExp24", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistExp24 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExp32;
    rc = regRead("selChIdStatistExp32", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistExp32 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExpOn;
    rc = regRead("selChIdStatistExpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistExpOn 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExpGo;
    rc = regRead("selChIdStatistExpGo", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistExpGo 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistThdToken0Halt;
    rc = regRead("selChIdStatistThdToken0Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistThdToken0Halt 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistThdToken1Halt;
    rc = regRead("selChIdStatistThdToken1Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistThdToken1Halt 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistNoToken0Halt;
    rc = regRead("selChIdStatistNoToken0Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistNoToken0Halt 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistNoToken1Halt;
    rc = regRead("selChIdStatistNoToken1Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistNoToken1Halt 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPauseOn;
    rc = regRead("selChIdStatistPauseOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistPauseOn 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistChannelUp;
    rc = regRead("selChIdStatistChannelUp", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistChannelUp 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPtpOn;
    rc = regRead("selChIdStatistPtpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistPtpOn 0x%08X\n",data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPrecisePtpOn;
    rc = regRead("selChIdStatistPrecisePtpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("selChIdStatistPrecisePtpOn 0x%08X\n",data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaSffPrint function
* @endinternal
*
* @brief   Print PCA SFF registers.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaSffChannelPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelInterruptCause[channel];
    rc = regRead("channelInterruptCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("channel %d Interrupt Cause 0x%08X ", channel, data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelInterruptMask[channel];
    rc = regRead("channelInterruptMask", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("Mask 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelTokenOccupStatist[channel];
    rc = regRead("channelTokenOccupStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("channel %d TokenOccupStatist 0x%08X ", channel, data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelOccupStatist[channel];
    rc = regRead("channelOccupStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("OccupStatist 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelLLAttribStatus[channel];
    rc = regRead("channelLLAttribStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("channel %d LLAttribStatus 0x%08X ", channel, data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelEventStatus[channel];
    rc = regRead("channelEventStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("EventStatus 0x%08X\n", data);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaSffGlobalCfgSet function
* @endinternal
*
* @brief   Set PCA SFF Global Miscelanous Configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] validParamsBitmap     - valid Params Bitmap - each bit related to one parameter below.
* @param[in] creditSyncThreshold   - credit sync threshold.
* @param[in] fwCommand             - FW Command.
* @param[in] tokenTimeoutInterval  - token timeout interval.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaSffGlobalMiscCfgSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               validParamsBitmap,
    IN  GT_U32               creditSyncThreshold,            /*bit0*/
    IN  GT_U32               fwCommand,                      /*bit1*/
    IN  GT_U32               tokenTimeoutInterval            /*bit2*/
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    rc = regRead("globalConfig", unitIndex, devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (validParamsBitmap & 1)
    {
        /* bits 2:0 Credit Sync Threshold */
        if (creditSyncThreshold > 7)
        {
            PR("creditSyncThreshold %d out of range\n", creditSyncThreshold);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        data &= (~ 0x7);
        data |= creditSyncThreshold;
    }
    if (validParamsBitmap & 2)
    {
        /* bits 5:4 FW command */
        if (fwCommand > 3)
        {
            PR("fwCommand %d out of range\n", fwCommand);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        data &= (~ (0x3 << 4));
        data |= (fwCommand << 4);
    }
    if (validParamsBitmap & 4)
    {
        /* bits 15:8 Token Timeout Interval */
        if (tokenTimeoutInterval > 255)
        {
            PR("tokenTimeoutInterval %d out of range\n", tokenTimeoutInterval);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        data &= (~ (0xFF << 8));
        data |= (tokenTimeoutInterval << 8);
    }

    return regWriteMask("globalConfig", unitIndex, devNum, regAddr, 0xFFFFFFFF, data);
}

/**
* @internal prvCpssDxChPortDpDebugPcaSffGlobalStatisticsCfgSet function
* @endinternal
*
* @brief   Set PCA SFF Global Statistics Configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                      - device number
* @param[in] unitIndex                   - unit index inside the tile.
* @param[in] statisticsChannelSelect     - Channel to Select.
* @param[in] statisticsSopNotEop         - 1 - SOP counted, 0 - EOP counted .
* @param[in] statisticsCollectionEnable  - statistics collection enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaSffGlobalStatisticsCfgSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               statisticsChannelSelect,
    IN  GT_U32               statisticsSopNotEop,
    IN  GT_U32               statisticsCollectionEnable
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    rc = regRead("globalConfig", unitIndex, devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* bits 21:16 Statistics Channel ID Select */
    if (statisticsChannelSelect > 63)
    {
        PR("statisticsChannelSelect %d out of range\n", statisticsChannelSelect);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    data &= (~ (0x3F << 16));
    data |= (statisticsChannelSelect << 16);

    /* bits 23:23 Statistics for SOP not EOP */
    if (statisticsSopNotEop > 1)
    {
        PR("statisticsSopNotEop %d out of range\n", statisticsSopNotEop);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    data &= (~ (1 << 23));
    data |= (statisticsSopNotEop << 23);

    /* bits 24:24 Statistics Collection Enable */
    if (statisticsCollectionEnable > 1)
    {
        PR("statisticsCollectionEnable %d out of range\n", statisticsCollectionEnable);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    data &= (~ (1 << 24));
    data |= (statisticsCollectionEnable << 24);

    return regWriteMask("globalConfig", unitIndex, devNum, regAddr, 0xFFFFFFFF, data);
}

/**
* @internal prvCpssDxChPortDpDebugPcaSffChannelStatisticsEnableSet function
* @endinternal
*
* @brief   Set PCA SFF Channel Statistics Enable.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index.
* @param[in] channelIndex          - channel index.
* @param[in] statisticsEnable      - statistics enable (not 0) or disable (0).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaSffChannelStatisticsEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channelIndex,
    IN  GT_U32               statisticsEnable
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              channelAmount;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) &&
       !PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) )
    {
        /*Harrier*/
        channelAmount  = 26;
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /*Phoenix*/
        channelAmount  = 54;
    }
    else
    {
        /*Hawk*/
        channelAmount  = 26;
    }
    if (channelIndex >= channelAmount)
    {
        PR("channelIndex %d out of range\n", channelIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channelIndex];
    rc = regRead("channelControlEnable", unitIndex, devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ (1 << 7));
    if (statisticsEnable)
    {
        data |= (1 << 7);
    }
    return regWriteMask("channelControlEnable", unitIndex, devNum, regAddr, 0xFFFFFFFF, data);
}

/**
* @internal prvCpssDxChPortDpDebugPcaMacSecExtCountersPrint function
* @endinternal
*
* @brief   Print PCA MAC_SEC_EXT debug counters of all instances of given datapath.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] dataPath              - dataPath index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaMacSecExtCountersPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               dataPath
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data[8];
    GT_U32                              type; /*0 - 163, 1 - 164*/
    GT_U32                              direction; /*0 - egress, 1 - ingress*/
    GT_U32                              stage; /*0 - preMacSec, 1 - postMacSec*/
    GT_U32                              column;
    const char* typeStr[]               = {"163", "164"};
    const char* directionStr[]          = {"egress", "ingress"};
    const char* stageStr[]              = {"PRE_MACSEC", "POST_MACSEC"};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (dataPath >= MAX_PCA_CNS)
    {
        PR("dataPath %d out of range\n", dataPath);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("Column %d - direction %s type %s stage %s ", column,
                   directionStr[direction], typeStr[type], stageStr[stage]);
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].sdbDbgMonitorControl;
                regRead("sdbDbgMonitorControl", dataPath, devNum, regAddr, &(data[0]));
                if (data[0] & 2)
                {
                    PR("Channel %d ", ((data[0] >> 2) & 0x7F));
                }
                else
                {
                    PR("Global ");
                }
                PR("Statistics Enable %d \n", (data[0] & 1));
            }
        }
    }

    PR("column \t\t\t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", column);
            }
        }
    }
    PR("\n");

    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].packetLength;
                column = (direction * 2 * 2) + (type * 2) + stage;
                regRead("packetLength", dataPath, devNum, regAddr, &(data[column]));
            }
        }
    }
    PR("monitored packetLength min \t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", (data[column] & 0xFFFF));
            }
        }
    }
    PR("\n");
    PR("monitored packetLength max \t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", ((data[column] >> 16) & 0xFFFF));
            }
        }
    }
    PR("\n");

    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].sopEopError;
                regRead("sopEopError", dataPath, devNum, regAddr, &(data[column]));
            }
        }
    }
    PR("sopEopError Error2 SOP \t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", (data[column] & 1));
            }
        }
    }
    PR("\n");
    PR("sopEopError Error2 EOP \t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", (data[column] & 2));
            }
        }
    }
    PR("\n");
    PR("sopEopError valid outside packet \t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                column = (direction * 2 * 2) + (type * 2) + stage;
                PR("\t %d ", (data[column] & 4));
            }
        }
    }
    PR("\n");

    PR("maxInnerPacketGap \t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].maxInnerPacketGap;
                regRead("maxInnerPacketGap", dataPath, devNum, regAddr, &(data[0]));
                PR("\t %d ", data[0]);
            }
        }
    }
    PR("\n");

    PR("maxPacketGap \t\t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].maxPacketGap;
                regRead("maxPacketGap", dataPath, devNum, regAddr, &(data[0]));
                PR("\t %d ", data[0]);
            }
        }
    }
    PR("\n");

    PR("minPacketGap \t\t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].minPacketGap;
                regRead("minPacketGap", dataPath, devNum, regAddr, &(data[0]));
                PR("\t %d ", data[0]);
            }
        }
    }
    PR("\n");

    PR("numOfPackets \t\t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].numOfPackets;
                regRead("numOfPackets", dataPath, devNum, regAddr, &(data[0]));
                PR("\t %d ", data[0]);
            }
        }
    }
    PR("\n");

    PR("numOfErrorPackets \t\t\t ");
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].numOfErrorPackets;
                regRead("numOfErrorPackets", dataPath, devNum, regAddr, &(data[0]));
                PR("\t %d ", data[0]);
            }
        }
    }
    PR("\n");

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaMacSecExtCountersEnableSet function
* @endinternal
*
* @brief   Enable PCA MAC_SEC_EXT debug counters for all unit instances of given datapath.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] dataPath              - dataPath index.
* @param[in] enable                - Enable the SDB debug monitor status registers (APPLICABLE VALUES: 0 or 1)
* @param[in] perChannel            - Decides whether the SDB debug monitor works globally or per CHID (APPLICABLE VALUES: 0 or 1)
* @param[in] channel               - Channel ID to monitor and debug
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaMacSecExtCountersEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               dataPath,
    IN  GT_U32               enable,
    IN  GT_U32               perChannel,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              type; /*0 - 163, 1 - 164*/
    GT_U32                              direction; /*0 - egress, 1 - ingress*/
    GT_U32                              stage; /*0 - preMacSec, 1 - postMacSec*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (dataPath >= MAX_PCA_CNS)
    {
        PR("dataPath %d out of range\n", dataPath);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = (enable ? 1 : 0) | ((perChannel ? 1 : 0) << 1) | ((channel & 0xFF) << 2);
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    for (direction = 0; (direction < 2); direction++)
    {
        for (type = 0; (type < 2); type++)
        {
            for (stage = 0; (stage < 2); stage++)
            {
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[dataPath][type][direction].
                    sdbDbgMonitor[stage].sdbDbgMonitorControl;
                rc = regWriteMask(
                    "SDB Debug Monitor Control", dataPath, devNum, regAddr, 0x3FF, data);
                if (rc == GT_OK)
                {
                    PR("SDB Debug Monitor Control %d \n", data);
                }
            }
        }
    }
    return GT_OK;
}


typedef struct
{
    GT_U32      regOffset;
    GT_CHAR*    regName;
    GT_U32      numRegs;
    GT_U32      offsetFormula;
}REG_INFO;

#define REG_INFO_MAC(_regOffset,_regName,_numRegs,_offsetFormula) {_regOffset,_regName,_numRegs,_offsetFormula}
#define LAST_REG_INFO_MAC   {0,NULL,0,0}

typedef struct
{
    PRV_CPSS_DXCH_UNIT_ENT      unitId;
    GT_CHAR*                    unitName;
    GT_U32                      startDma;/* DMA# of the first channel in the unit */
}UNIT_INFO;

#define UNIT_INFO_MAC(_unitId,_startDma) {_unitId,#_unitId,_startDma}
#define LAST_UNIT_INFO_MAC     {0,NULL,0}


/**
* @internal prvCpssDxChPortDpDebugMifCountersPrint function
* @endinternal
*
* @brief   Print the MIF mib counters when are not ZERO.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugMifCountersPrint
(
    IN  GT_U8                devNum
)
{
    UNIT_INFO phoenixMifUnits[] = {
         UNIT_INFO_MAC(PRV_CPSS_DXCH_UNIT_MIF_USX_0_E , 0)
        ,UNIT_INFO_MAC(PRV_CPSS_DXCH_UNIT_MIF_USX_1_E ,16)
        ,UNIT_INFO_MAC(PRV_CPSS_DXCH_UNIT_MIF_USX_2_E ,32)
        ,UNIT_INFO_MAC(PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E ,48)
        ,UNIT_INFO_MAC(PRV_CPSS_DXCH_UNIT_MIF_400G_0_E,50)

         /* must be last */
        ,LAST_UNIT_INFO_MAC
     };
    UNIT_INFO *unitInfoPtr;

    REG_INFO reg_info[] = {
         REG_INFO_MAC(0x00001300, "Mif Type 8 Tx Good Packets Count<<%n>>"       ,25,0x4)
        ,REG_INFO_MAC(0x00001380, "Mif Type 8 Tx Bad Packets Count<<%n>>"        ,25,0x4)
        ,REG_INFO_MAC(0x00001400, "Mif Type 8 Tx Discarded Packets Count<<%n>>"  ,25,0x4)
        ,REG_INFO_MAC(0x00001480, "Mif Type 8 Tx Link Fail Count<<%n>>"          ,25,0x4)

        ,REG_INFO_MAC(0x00001500, "Mif Type 32 Tx Good Packets Count<<%n>>"      ,16,0x4)
        ,REG_INFO_MAC(0x00001540, "Mif Type 32 Tx Bad Packets Count<<%n>>"       ,16,0x4)
        ,REG_INFO_MAC(0x00001580, "Mif Type 32 Tx Discarded Packets Count<<%n>>" ,16,0x4)
        ,REG_INFO_MAC(0x000015C0, "Mif Type 32 Tx Link Fail Count<<%n>>"         ,16,0x4)

        ,REG_INFO_MAC(0x00001600, "Mif Type 128 Tx Good Packets Count<<%n>>"     ,2,0x4)
        ,REG_INFO_MAC(0x00001610, "Mif Type 128 Tx Bad Packets Count<<%n>>"      ,2,0x4)
        ,REG_INFO_MAC(0x00001620, "Mif Type 128 Tx Discarded Packets Count<<%n>>",2,0x4)
        ,REG_INFO_MAC(0x00001630, "Mif Type 128 Tx Link Fail Count<<%n>>"        ,2,0x4)

        ,REG_INFO_MAC(0x00000B00, "Mif Type 8 Rx Good Packets Count<<%n>>"       ,25,0x4)
        ,REG_INFO_MAC(0x00000B80, "Mif Type 8 Rx Bad Packets Counter<<%n>>"      ,25,0x4)
        ,REG_INFO_MAC(0x00000C00, "Mif Type 8 Rx Discarded Packets Count<<%n>>"  ,25,0x4)

        ,REG_INFO_MAC(0x00000D00, "Mif Type 32 Rx Good Packets Count<<%n>>"      ,16,0x4)
        ,REG_INFO_MAC(0x00000D40, "Mif Type 32 Rx Bad Packets Count<<%n>>"       ,16,0x4)
        ,REG_INFO_MAC(0x00000D80, "Mif Type 32 Rx Discarded Packets Count<<%n>>" ,16,0x4)

        ,REG_INFO_MAC(0x00000E00, "Mif Type 128 Rx Good Packets Count<<%n>>"     ,2,0x4)
        ,REG_INFO_MAC(0x00000E10, "Mif Type 128 Rx Bad Packets Count<<%n>>"      ,2,0x4)
        ,REG_INFO_MAC(0x00000E20, "Mif Type 128 Rx Discarded Packets Count<<%n>>",2,0x4)

        /* must be last */
        ,LAST_REG_INFO_MAC
    };
    REG_INFO *regInfoPtr;
    GT_U32  baseAddr,regAddr,regValue;
    GT_U32  allCountersZero = 1;
    GT_U32  ii;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                                 | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            unitInfoPtr = &phoenixMifUnits[0];
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "function not implemented (yet) for AC5P(Hawk) family ");
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "function not implemented for this device family");
    }

    for(/*no init*/ ; unitInfoPtr->unitName ; unitInfoPtr++)
    {
        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitInfoPtr->unitId, NULL);

        regInfoPtr  = &reg_info[0];

        for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
        {
            for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
            {
                regAddr = baseAddr + regInfoPtr->regOffset +
                    ii * regInfoPtr->offsetFormula;

                rc = prvCpssDrvHwPpPortGroupReadRegister(
                    devNum, 0/*portGroupId*/, regAddr, &regValue);
                if (rc != GT_OK)
                {
                    PR("regRead failed, regAddr %08X, rc %d\n", regAddr, rc);
                }

                if(regValue != 0)
                {
                    /* print only registers with value != 0 !!!! */
                    PR("[0x%8.8x][%s] with value [0x%8.8x] (global DMA[%d])\n",
                        regAddr,regInfoPtr->regName,regValue,unitInfoPtr->startDma + ii);

                    allCountersZero = 0;
                }
            }
        }
    }

    if(allCountersZero)
    {
        PR("NOTE: All the MIF counters are ZERO !!! \n");
    }

    return GT_OK;
}

/* PCA TX General Channel utils - BRG, SFF, */

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelInterruptEnable function
* @endinternal
*
* @brief   Print PCA SFF registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
* @param[in] enable                - 0 - disable, other - enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelInterruptSummaryEnable
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel,
    IN  GT_U32               enable
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;
    GT_U32                              macSecType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert not-zero enable to 1 */
    enable = enable ? 1 : 0;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF*/
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_0_15_InterruptSummaryMask;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_16_31_InterruptSummaryMask;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_32_47_InterruptSummaryMask;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_48_63_InterruptSummaryMask;
            break;
    }
    mask  = (1 << (channel % 16));
    value = (enable << (channel % 16));
    rc = regWriteMask("SFF channel...InterruptSummaryMask", unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    /*BRG TX*/
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_0_15_InterruptSummaryCause;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_16_31_InterruptSummaryCause;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_32_47_InterruptSummaryCause;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_48_63_InterruptSummaryCause;
            break;
    }
    mask  = (1 << (channel % 16));
    value = (enable << (channel % 16));
    rc = regWriteMask("BRG TX channel...InterruptSummaryMask", unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    /*MACSEC_EXT*/
    for (macSecType = 0; (macSecType < 2); macSecType++)
    {
        switch (channel / 16)
        {
            case 0:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts0Mask;
                break;
            case 1:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts1Mask;
                break;
            case 2:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts2Mask;
                break;
            default: /*3*/
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts3Mask;
                break;
        }
        mask  = (1 << (channel % 16));
        value = (enable << (channel % 16));
        rc = regWriteMask(
            "MAC_SEC_EXT TX channel...InterruptSummaryMask", unitIndex, devNum, regAddr, mask, value);
        if (rc != GT_OK) return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelInterruptSummaryPrint function
* @endinternal
*
* @brief   Print PCA SFF, BRG TX, MACSEC_EXT TX interrupt summary registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelInterruptSummaryPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;
    GT_U32                              macSecType;
    GT_U32                              macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF*/
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_0_15_InterruptSummaryMask;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_16_31_InterruptSummaryMask;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_32_47_InterruptSummaryMask;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_48_63_InterruptSummaryMask;
            break;
    }
    rc = regRead("SFF channel_..._InterruptSummaryMask", unitIndex, devNum, regAddr, &data);
    if ((rc == GT_OK) && (data & (1 << (channel % 16))))
    {
        PR("SFF channel_..._InterruptSummaryMask set to 1\n");
    }
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_0_15_InterruptSummaryCause;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_16_31_InterruptSummaryCause;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_32_47_InterruptSummaryCause;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_SFF[unitIndex].channel_48_63_InterruptSummaryCause;
            break;
    }
    rc = regRead("SFF channel_..._InterruptSummaryCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF channel_..._InterruptSummaryCause \t\t %d\n",
            (1 & (data >> (channel % 16))));
    }

    /*BRG TX*/
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_0_15_InterruptSummaryMask;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_16_31_InterruptSummaryMask;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_32_47_InterruptSummaryMask;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_48_63_InterruptSummaryMask;
            break;
    }
    rc = regRead("BRG TX channel_..._InterruptSummaryMask", unitIndex, devNum, regAddr, &data);
    if ((rc == GT_OK) && (data & (1 << (channel % 16))))
    {
        PR("BRG TX channel_..._InterruptSummaryMask set to 1\n");
    }
    switch (channel / 16)
    {
        case 0:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_0_15_InterruptSummaryCause;
            break;
        case 1:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_16_31_InterruptSummaryCause;
            break;
        case 2:
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_32_47_InterruptSummaryCause;
            break;
        default: /*3*/
            regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel_48_63_InterruptSummaryCause;
            break;
    }
    rc = regRead("BRG TX channel_..._InterruptSummaryCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "BRG TX channel_..._InterruptSummaryCause \t %d\n",
            (1 & (data >> (channel % 16))));
    }

    /*MACSEC_EXT*/
    for (macSecType = 0; (macSecType < 2); macSecType++)
    {
        macSecUnitId = (macSecType == 0) ? 163 : 164;
        switch (channel / 16)
        {
            case 0:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts0Mask;
                break;
            case 1:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts1Mask;
                break;
            case 2:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts2Mask;
                break;
            default: /*3*/
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts3Mask;
                break;
        }
        rc = regRead(
            "MACSEC_EXT TX msChannelInterrupts...Mask", unitIndex, devNum, regAddr, &data);
        if ((rc == GT_OK) && (data & (1 << (channel % 16))))
        {
            PR("MACSEC_EXT %d TX msChannelInterrupts...Mask set to 1\n", macSecUnitId);
        }
        switch (channel / 16)
        {
            case 0:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts0Cause;
                break;
            case 1:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts1Cause;
                break;
            case 2:
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts2Cause;
                break;
            default: /*3*/
                regAddr = regsAddrPtr->PCA_MACSEC_EXT[unitIndex][macSecType][1].msChannelInterrupts3Cause;
                break;
        }
        rc = regRead(
            "MACSEC_EXT TX msChannelInterrupts...Cause", unitIndex, devNum, regAddr, &data);
        if (rc == GT_OK)
        {
            PR(
                "MACSEC_EXT %d TX msChannelInterrupts...Cause \t %d\n",
                macSecUnitId, (1 & (data >> (channel % 16))));
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelInterruptCausesPrint function
* @endinternal
*
* @brief   Print PCA SFF, BRG TX  registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelInterruptCausesPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF*/
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelInterruptMask[channel];
    rc = regRead("SFF channelInterruptMask", unitIndex, devNum, regAddr, &data);
    if ((rc == GT_OK) && (data != 0))
    {
        PR("SFF Interrupt Mask \t 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelInterruptCause[channel];
    rc = regRead("SFF channelInterruptCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF Interrupt Cause \t 0x%08X\n", data);
        if (data != 0)
        {
            PR("\t Found not-zero-bits:\n");
            if (data & 1)
            {
                PR("\t\t Bit0 - summary\n");
            }
            if (data & (1 << 1))
            {
                PR("\t\t Bit1 - Occup Counter OverFlow\n");
            }
            if (data & (1 << 2))
            {
                PR("\t\t Bit2 - Token0 Credit Counter OverFlow\n");
            }
            if (data & (1 << 3))
            {
                PR("\t\t Bit3 - Token1 Credit Counter OverFlow\n");
            }
            if (data & (1 << 4))
            {
                PR("\t\t Bit4 - Packet integrity\n");
            }
            if (data & (1 << 5))
            {
                PR("\t\t Bit5 - Token Timeout\n");
            }
            if (data & (1 << 6))
            {
                PR("\t\t Bit6 - Wrong Token\n");
            }
            if (data & (1 << 7))
            {
                PR("\t\t Bit7 - Wrong <<valid>>\n");
            }
            if (data & (1 << 8))
            {
                PR("\t\t Bit8 - Token1 Correction adder OverFlow\n");
            }
            if (data & (1 << 9))
            {
                PR("\t\t Bit9 - Token0 Correction adder OverFlow\n");
            }
            if (data & (1 << 10))
            {
                PR("\t\t Bit10 - Token1 Accumulation OverFlow\n");
            }
            if (data & (1 << 11))
            {
                PR("\t\t Bit11 - Token0 Accumulation OverFlow\n");
            }
        }
    }

    /*BRG_TX*/
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannelInterruptMask[channel];
    rc = regRead("BRG TX channelInterruptMask", unitIndex, devNum, regAddr, &data);
    if ((rc == GT_OK) && (data != 0))
    {
        PR("BRG TX Interrupt Mask \t 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannelInterruptCause[channel];
    rc = regRead("BRG TX txChannelInterruptCause", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG TX Interrupt Cause \t 0x%08X\n", data);
        if (data != 0)
        {
            PR("\t Found not-zero-bits:\n");
            if (data & 1)
            {
                PR("\t\t Bit0 - summary\n");
            }
            if (data & (1 << 1))
            {
                PR("\t\t Bit1 - Tx Slice Packet On Disabled Channel\n");
            }
            if (data & (1 << 2))
            {
                PR("\t\t Bit2 - Tx SDB Credit On Disabled Channel\n");
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelStatisticsEnable function
* @endinternal
*
* @brief   Write PCA SFF, BRG TX  registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
* @param[in] sopNotEop             - 0 - EOP, other - SOP.
* @param[in] enable                - 0 - disable, other - enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelStatisticsEnable
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel,
    IN  GT_U32               sopNotEop,
    IN  GT_U32               enable
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              mask;
    GT_U32                              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert not-zero enable to 1 */
    enable    = enable ? 1 : 0;
    sopNotEop = sopNotEop ? 1 : 0;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF global*/
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    mask = (1 << 24 | (1 << 23) | (0x3F << 16));
    value = (enable << 24 | (sopNotEop << 23) | (channel << 16));
    if (enable)
    {
        /*reset before enable*/
        rc = regWriteMask(
            "SFF globalConfig reset", unitIndex, devNum, regAddr, mask, 0/*value*/);
        if (rc != GT_OK) return rc;
    }
    rc = regWriteMask(
        "SFF globalConfig", unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    /*SFF per channel*/
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
    mask = (1 << 7);
    value = (enable << 7);
    if (enable)
    {
        /*reset before enable*/
        rc = regWriteMask(
            "SFF channelControlEnable reset", unitIndex, devNum, regAddr, mask, 0/*value*/);
        if (rc != GT_OK) return rc;
    }
    rc = regWriteMask(
        "SFF channelControlEnable", unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    /*BRG TX*/
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCounterCfgChannel;
    mask = 0xFF;
    value = channel;
    rc = regWriteMask(
        "SFF globalConfig", unitIndex, devNum, regAddr, mask, value);
    if (rc != GT_OK) return rc;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelStatisticsPrint function
* @endinternal
*
* @brief   Print PCA SFF, BRG TX  registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelStatisticsPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF per channel counters */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelOccupStatist[channel];
    rc = regRead("SFF channelOccupStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF channelMinOccupancy %d channelMaxOccupancy %d\n",
            ((data >> 8) & 0x3F), (data & 0x3F));
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelTokenOccupStatist[channel];
    rc = regRead("SFF channelTokenOccupStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF tokenOccupancy token1Min %d token1Max %d token0Min %d token0Max %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF), ((data >> 8) & 0xFF), (data & 0xFF));
    }

    /*SFF global counters */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdAndGlobalPacketInStatus;
    rc = regRead("SFF selChIdAndGlobalPacketInStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF GlobalPacketInStatus %d ChannelPacketInStatus %d\n",
            ((data >> 8) & 0xFF), (data & 0xFF));
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistSopOrEop;
    rc = regRead("SFF selChIdStatistSopOrEop", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistSopOrEop \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExp24;
    rc = regRead("SFF selChIdStatistExp24", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistExp24 \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExp32;
    rc = regRead("SFF selChIdStatistExp32", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistExp32 \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExpOn;
    rc = regRead("SFF selChIdStatistExpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistExpOn \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistExpGo;
    rc = regRead("SFF selChIdStatistExpGo", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistExpGo \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistThdToken0Halt;
    rc = regRead("SFF selChIdStatistThdToken0Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistThdToken0Halt \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistThdToken1Halt;
    rc = regRead("SFF selChIdStatistThdToken1Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistThdToken1Halt \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistNoToken0Halt;
    rc = regRead("SFF selChIdStatistNoToken0Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistNoToken0Halt \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistNoToken1Halt;
    rc = regRead("SFF selChIdStatistNoToken1Halt", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistNoToken1Halt \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPauseOn;
    rc = regRead("SFF selChIdStatistPauseOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistPauseOn \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistChannelUp;
    rc = regRead("SFF selChIdStatistChannelUp", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistChannelUp \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPtpOn;
    rc = regRead("SFF selChIdStatistPtpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistPtpOn \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdStatistPrecisePtpOn;
    rc = regRead("SFF selChIdStatistPrecisePtpOn", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdStatistPrecisePtpOn \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdTokenCorrection10BStatist;
    rc = regRead("SFF selChIdTokenCorrection10BStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdTokenCorrection10BStatist \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdTokenCorrection24BStatist;
    rc = regRead("SFF selChIdTokenCorrection24BStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdTokenCorrection24BStatist \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].selChIdTokenCorrection32BStatist;
    rc = regRead("SFF selChIdTokenCorrection32BStatist", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("SFF selChIdTokenCorrection32BStatist \t 0x%08X\n", data);
    }

    /* BRG_TX */
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannelStatistics[channel];
    rc = regRead("BRG_TX txChannelStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txChannelStatistics \t\t 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannelCreditsStatistics[channel];
    rc = regRead("BRG_TX txChannelCreditsStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txChannelCreditsStatistics \t 0x%08X\n", data);
    }
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSdbSopCounter;
    rc = regRead("BRG_TX txSdbSopCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSdbSopCounter \t\t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSdbEopCounter;
    rc = regRead("BRG_TX txSdbEopCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSdbEopCounter \t\t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSliceSopCounter;
    rc = regRead("BRG_TX txSliceSopCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSliceSopCounter \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSliceEopCounter;
    rc = regRead("BRG_TX txSliceEopCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSliceEopCounter \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txShortPacketsCounter;
    rc = regRead("BRG_TX txShortPacketsCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txShortPacketsCounter \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txPch_1_ByteCounter;
    rc = regRead("BRG_TX txPch_1_ByteCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txPch_1_ByteCounter \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txPch_8_BytesCounter;
    rc = regRead("BRG_TX txPch_8_BytesCounter", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txPch_8_BytesCounter \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCounterCfgChannel;
    rc = regRead("BRG_TX txCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCounterCfgChannel \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSdbSopCounterCfgChannel;
    rc = regRead("BRG_TX txSdbSopCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSdbSopCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSdbEopCounterCfgChannel;
    rc = regRead("BRG_TX txSdbEopCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSdbEopCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSliceSopCounterCfgChannel;
    rc = regRead("BRG_TX txSliceSopCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSliceSopCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txSliceEopCounterCfgChannel;
    rc = regRead("BRG_TX txSliceEopCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txSliceEopCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txShortPacketsCounterCfgChannel;
    rc = regRead("BRG_TX txShortPacketsCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txShortPacketsCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txPch_1_ByteCounterCfgChannel;
    rc = regRead("BRG_TX txPch_1_ByteCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txPch_1_ByteCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txPch_8_BytesCounterCfgChannel;
    rc = regRead("BRG_TX txPch_8_BytesCounterCfgChannel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txPch_8_BytesCounterCfgChannel \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCdcFifoStatus;
    rc = regRead("BRG_TX txCdcFifoStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCdcFifoStatus \t\t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCdcFifoStatistics;
    rc = regRead("BRG_TX txCdcFifoStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCdcFifoStatistics \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCreditCdcFifoStatus;
    rc = regRead("BRG_TX txCreditCdcFifoStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCreditCdcFifoStatus \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCreditCdcFifoStatistics;
    rc = regRead("BRG_TX txCreditCdcFifoStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCreditCdcFifoStatistics \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCreditRotatedFifoStatus;
    rc = regRead("BRG_TX txCreditRotatedFifoStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCreditRotatedFifoStatus \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txCreditRotatedFifoStatistics;
    rc = regRead("BRG_TX txCreditRotatedFifoStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txCreditRotatedFifoStatistics \t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txAccomulatorStatus;
    rc = regRead("BRG_TX txAccomulatorStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txAccomulatorStatus \t\t 0x%08X\n", data);
    }

    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txAccomulatorStatistics;
    rc = regRead("BRG_TX txAccomulatorStatistics", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR("BRG_TX txAccomulatorStatistics \t\t 0x%08X\n", data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelCfgPrint function
* @endinternal
*
* @brief   Print PCA SFF, BRG TX  registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelCfgPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*SFF per channel GFG */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
    rc = regRead("SFF channelControlEnable", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF Channel Enable CfgTokenMax %d Token1RateThd %d Token0RateThd %d StatEn %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF), ((data >> 8) & 0xFF),
            ((data >> 7) & 1));
        PR(
            "------ FwCmdGo %d Token1Up %d Token0Up %d Stop %d Enable %d\n",
            ((data >> 6) & 1), ((data >> 3) & 1), ((data >> 2) & 1),
            ((data >> 1) & 1), (data & 1));
    }


    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
    rc = regRead("SFF channelControlConfig", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF Channel CONFIG rdPtr %d wrPtr %d txFifoWidth %d MaxOccup %d WaitSlots %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF), ((data >> 14) & 3),
            ((data >> 8) & 0x3F), (data & 0xFF));
    }

    /* BRG TX */
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel[channel];
    rc = regRead("BRG TX txChanel", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "BRG TX channel %02d tx_credits_allocated %d drain %d tx_pch_en %d tx_en %d\n",
            channel, ((data >> 16) & 0xFF),
            ((data >> 2) & 1), ((data >> 1) & 1), (data & 1));

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpDebugPcaTxChannelStatusPrint function
* @endinternal
*
* @brief   Print PCA SFF, BRG TX  registers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] unitIndex             - unit index inside the tile.
* @param[in] channel               - channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_FAIL                  - on error
*
*
*/
GT_STATUS prvCpssDxChPortDpDebugPcaTxChannelStatusPrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               unitIndex,
    IN  GT_U32               channel
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32                              regAddr;
    GT_U32                              data;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PR("device %d does not support SIP6_10\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (unitIndex >= MAX_PCA_CNS)
    {
        PR("unitIndex %d out of range\n", unitIndex);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* SFF per channel Status */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelLLAttribStatus[channel];
    rc = regRead("SFF channelLLAttribStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF Channel LL Occup Counter %d LL Wr Pointer %d LL Rd Pointer %d\n",
            ((data >> 16) & 0x3F), ((data >> 8) & 0xFF), (data & 0xFF));
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelEventStatus[channel];
    rc = regRead("SFF channelEventStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF Channel Token1 Occup Counter %d Token0 Occup Counter %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF));
        if (data & 0x1FFF)
        {
            PR("Found Event bits:\n");
            if (data & (1 << 12))
            {
                PR("\t Bit 12 - Pipe Empty\n");
            }
            if (data & (1 << 11))
            {
                PR("\t Bit 11 - Pause\n");
            }
            if (data & (1 << 10))
            {
                PR("\t Bit 10 - Read packet is in progress\n");
            }
            if (data & (1 << 9))
            {
                PR("\t Bit 9 - Occupancy counter is not empty\n");
            }
            if (data & (1 << 8))
            {
                PR("\t Bit 8 - Occupancy counter is not full\n");
            }
            if (data & (1 << 7))
            {
                PR("\t Bit 7 - token-1 credit counter is not empty\n");
            }
            if (data & (1 << 6))
            {
                PR("\t Bit 6 - token-1 credit counter is not full\n");
            }
            if (data & (1 << 5))
            {
                PR("\t Bit 5 - token-0 credit counter is not empty\n");
            }
            if (data & (1 << 4))
            {
                PR("\t Bit 4 - token-0 credit counter is not full\n");
            }
            if (data & (1 << 3))
            {
                PR("\t Bit 3 - reserved\n");
            }
            if (data & (1 << 2))
            {
                PR("\t Bit 2 - token-up is in progress\n");
            }
            if (data & (1 << 1))
            {
                PR("\t Bit 1 - expansion-go is in progress\n");
            }
            if (data & 1)
            {
                PR("\t Bit 0 - expansion-on is in progress\n");
            }
        }
    }
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelTokenCorrectStatus[channel];
    rc = regRead("SFF channelTokenCorrectStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "SFF Channel Correction Position %d Token-1 Accumulator %d Token-0 Accumulator %d\n",
            ((data >> 16) & 0x7), ((data >> 13) & 0x3F), (data & 0x3F));
    }

    /* BRG TX */
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannelStatus[channel];
    rc = regRead("SFF txChannelStatus", unitIndex, devNum, regAddr, &data);
    if (rc == GT_OK)
    {
        PR(
            "BRG TX Channel Accomulator fill level %d SDB Credits Counter %d Credit state(IDLE,INIT,REGULAR,RECLAIM) %d\n",
            ((data >> 10) & 0x3FFF), ((data >> 2) & 0xFF), (data & 0x3));
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortDpDebugPcaTxHelp(GT_VOID)
{
    PR("prvCpssDxChPortDpDebugPcaTxChannelInterruptSummaryEnable devNum, unutIndex, channel, enable\n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelInterruptSummaryPrint devNum, unutIndex, channel \n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelInterruptCausesPrint devNum, unutIndex, channel \n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelStatisticsEnable devNum, unutIndex, channel, sopNotEop, enable \n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelStatisticsPrint devNum, unutIndex, channel \n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelCfgPrint devNum, unutIndex, channel \n");
    PR("prvCpssDxChPortDpDebugPcaTxChannelStatusPrint devNum, unutIndex, channel \n");
    return GT_OK;
}
