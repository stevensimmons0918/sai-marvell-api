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
* @file cpssPxCnc.c
*
* @brief CPSS PX Centralized Counters (CNC) API.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cnc/private/prvCpssPxCncLog.h>
#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* DMA message is 4 words, can store 2 counters*/
#define PRV_CPSS_DMA_MESSAGE_SIZE 4

/**
* @internal prvCpssPxCncCounterSwToHw function
* @endinternal
*
* @brief   The function converts the SW counter to 64-bit HW format.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] format                   - CNC counter HW format
* @param[in] swCounterPtr             - (pointer to) CNC Counter in SW format
*
* @param[out] hwCounterArr[]           - (pointer to) CNC Counter in HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssPxCncCounterSwToHw
(
    IN   CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    IN   CPSS_PX_CNC_COUNTER_STC           *swCounterPtr,
    OUT  GT_U32                            hwCounterArr[]
)
{
    switch (format)
    {
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            hwCounterArr[0] =
                (swCounterPtr->packetCount.l[0] & 0x1FFFFFFF)
                | (swCounterPtr->byteCount.l[1] << 29);
            hwCounterArr[1] = swCounterPtr->byteCount.l[0];
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            hwCounterArr[0] =
                (swCounterPtr->packetCount.l[0] & 0x07FFFFFF)
                | (swCounterPtr->byteCount.l[1] << 27);
            hwCounterArr[1] = swCounterPtr->byteCount.l[0];
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            hwCounterArr[0] = swCounterPtr->packetCount.l[0];
            hwCounterArr[1] =
                (swCounterPtr->byteCount.l[0] & 0x07FFFFFF)
                | (swCounterPtr->packetCount.l[1] << 27);
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            hwCounterArr[0] = swCounterPtr->packetCount.l[0];
            hwCounterArr[1] = swCounterPtr->packetCount.l[1];
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            hwCounterArr[0] = swCounterPtr->byteCount.l[0];
            hwCounterArr[1] = swCounterPtr->byteCount.l[1];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxCncCounterHwToSw function
* @endinternal
*
* @brief   The function converts the 64-bit HW counter to SW format.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] format                   - CNC counter HW format
* @param[in] hwCounterArr[]           - (pointer to) CNC Counter in HW format
*
* @param[out] swCounterPtr             - (pointer to) CNC Counter in SW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssPxCncCounterHwToSw
(
    IN   CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    IN   GT_U32                            hwCounterArr[],
    OUT  CPSS_PX_CNC_COUNTER_STC           *swCounterPtr
)
{
    switch (format)
    {
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            swCounterPtr->packetCount.l[0] =
                (hwCounterArr[0] & 0x1FFFFFFF);
            swCounterPtr->packetCount.l[1] = 0;
            swCounterPtr->byteCount.l[0] = hwCounterArr[1];
            swCounterPtr->byteCount.l[1] =
                ((hwCounterArr[0] >> 29) & 0x07);
            break;

        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            swCounterPtr->packetCount.l[0] =
                (hwCounterArr[0] & 0x07FFFFFF);
            swCounterPtr->packetCount.l[1] = 0;
            swCounterPtr->byteCount.l[0] = hwCounterArr[1];
            swCounterPtr->byteCount.l[1] =
                ((hwCounterArr[0] >> 27) & 0x1F);
            break;

        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            swCounterPtr->packetCount.l[0] = hwCounterArr[0];
            swCounterPtr->packetCount.l[1] =
                ((hwCounterArr[1] >> 27) & 0x1F);
            swCounterPtr->byteCount.l[0] =
                (hwCounterArr[1] & 0x07FFFFFF);
            swCounterPtr->byteCount.l[1] = 0;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            swCounterPtr->packetCount.l[0] = hwCounterArr[0];
            swCounterPtr->packetCount.l[1] = hwCounterArr[1];
            swCounterPtr->byteCount.l  [0] = 0;
            swCounterPtr->byteCount.l  [1] = 0;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            swCounterPtr->packetCount.l[0] = 0;
            swCounterPtr->packetCount.l[1] = 0;
            swCounterPtr->byteCount.l  [0] = hwCounterArr[0];
            swCounterPtr->byteCount.l  [1] = hwCounterArr[1];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxRewindDma function
* @endinternal
*
* @brief   Rewind DMA queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS prvCpssPxRewindDma
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;       /* Return code */
    GT_UINTPTR phyAddr; /* Physical address */
    GT_UINTPTR cpyAddr; /* Virtual address */
    GT_U32 regAddr;     /* Register address */

    cpyAddr = PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block;
    rc = cpssOsVirt2Phy(cpyAddr, /*OUT*/&phyAddr);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQBaseAddr;
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, phyAddr);
    if(GT_OK != rc)
    {
        return rc;
    }
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx = 0;
    return GT_OK;
}

/**
* @internal internal_cpssPxCncUploadInit function
* @endinternal
*
* @brief   Initialize the CNC upload configuration, including DMA queue. Should be
*         used once before first cpssPxCncBlockUploadTrigger call.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
* @param[in] cncCfgPtr                - (pointer to) DMA queue configuration.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - invalid cncCfgPtr.
* @retval GT_OUT_OF_RANGE          - bad address
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssPxCncUploadInit
(
    IN GT_SW_DEV_NUM          devNum,
    IN CPSS_DMA_QUEUE_CFG_STC *cncCfgPtr
)
{
    GT_UINTPTR  cpyAddr; /* Host address of DMA block */
    GT_UINTPTR  phyAddr; /* Physical address of DMA block */
    GT_STATUS   rc;      /* Return code */
    GT_U32      regAddr; /* Register address */
    GT_U32      mesNumBytes; /* Number of bytes in message */
    GT_U32      alignmentNumBytes; /* Byte count to adjust alignment */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cncCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(cncCfgPtr->dmaDescBlock);

    /* Check if block size is valid */
    if(cncCfgPtr->dmaDescBlockSize == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mesNumBytes = PRV_CPSS_DMA_MESSAGE_SIZE * 4;

    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block = (GT_UINTPTR)cncCfgPtr->dmaDescBlock;

    /* Set the descBlock to point to an aligned start address. */
    alignmentNumBytes = 0;
    if((((GT_UINTPTR)cncCfgPtr->dmaDescBlock) % mesNumBytes) != 0)
    {
        alignmentNumBytes =  (mesNumBytes -
            (((GT_UINTPTR)cncCfgPtr->dmaDescBlock) % mesNumBytes));
    }
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block += alignmentNumBytes;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.blockSize =
        (cncCfgPtr->dmaDescBlockSize - alignmentNumBytes) / mesNumBytes;

    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount = 0;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx = 0;

    /* Zero all values */
    cpssOsMemSet((void *)PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block, 0,
        mesNumBytes * PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.blockSize);

    /* Covert virtual address to physical */
    cpyAddr = PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block;
    rc = cpssOsVirt2Phy(cpyAddr, /*OUT*/&phyAddr);
    if (rc != GT_OK)
    {
        return rc;
    }
    #if __WORDSIZE == 64
        if (0 != (phyAddr & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    /* Disable queue */
    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQControl;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 31, 1, 0);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Write block size */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 30,
        PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.blockSize);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Write base address */
    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQBaseAddr;
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, phyAddr);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Enable queue */
    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQControl;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 31, 1, 1);
    return rc;
}

/**
* @internal cpssPxCncUploadInit function
* @endinternal
*
* @brief   Initialize the CNC upload configuration, including DMA queue. Should be
*         used once before first cpssPxCncBlockUploadTrigger call.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
* @param[in] cncCfgPtr                - (pointer to) DMA queue configuration.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - invalid cncCfgPtr.
* @retval GT_OUT_OF_RANGE          - bad address
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssPxCncUploadInit
(
    IN GT_SW_DEV_NUM          devNum,
    IN CPSS_DMA_QUEUE_CFG_STC *cncCfgPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncUploadInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cncCfgPtr));

    rc = internal_cpssPxCncUploadInit(devNum, cncCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cncCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockClientEnableSet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
)
{
    GT_U32 regAddr; /* Register address */
    GT_U32 clientOffset; /* Bit offset for selected client */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if(blockNum >= PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.cnc.cncBlocks)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(client)
    {
        case CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E:
            clientOffset = 0;
            break;
        case CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E:
            clientOffset = 1;
            break;
        case CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E:
            clientOffset = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.clientEnable.
        CNCBlockConfigReg0[blockNum];
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, clientOffset,
        1, BOOL2BIT_MAC(updateEnable));
}

/**
* @internal cpssPxCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientEnableSet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockClientEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, updateEnable));

    rc = internal_cpssPxCncBlockClientEnableSet(devNum, blockNum, client, updateEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, updateEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client.
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockClientEnableGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    GT_STATUS rc; /* Return code */
    GT_U32 regAddr; /* Register address */
    GT_U32 value; /* Register value */
    GT_U32 clientOffset; /* Client bit offset */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(updateEnablePtr);

    if(blockNum >= PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.cnc.cncBlocks)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(client)
    {
        case CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E:
            clientOffset = 0;
            break;
        case CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E:
            clientOffset = 1;
            break;
        case CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E:
            clientOffset = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.clientEnable.
        CNCBlockConfigReg0[blockNum];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, clientOffset,
        1, &value);

    *updateEnablePtr = BIT2BOOL_MAC(value & 0x1);

    return rc;
}

/**
* @internal cpssPxCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client.
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientEnableGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockClientEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, updateEnablePtr));

    rc = internal_cpssPxCncBlockClientEnableGet(devNum, blockNum, client, updateEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, updateEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] indexRangesBmp           - the counter index ranges bitmap
*                                      There are 64 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockClientRangesSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          blockNum,
    IN  GT_U64          indexRangesBmp
)
{
    GT_U32 ii; /* Iterator */
    GT_U32 regAddr; /* Register address */
    GT_U32 rangeBmp; /* Half of index ranges bmp */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = GT_OK;

    for(ii = 0; ii < 2; ii++)
    {
        rangeBmp = indexRangesBmp.l[ii];

        switch(blockNum)
        {
            case 0:
                regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.
                            rangeCount.CNCBlockRangeCountEnable[ii][0];
                break;
            case 1:
                regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.
                            rangeCount.CNCBlockRangeCountEnable[ii][1];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr,
            rangeBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssPxCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] indexRangesBmp           - the counter index ranges bitmap
*                                      There are 64 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientRangesSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          blockNum,
    IN  GT_U64          indexRangesBmp
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockClientRangesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, indexRangesBmp));

    rc = internal_cpssPxCncBlockClientRangesSet(devNum, blockNum, indexRangesBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, indexRangesBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      Pipe has 10 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockClientRangesGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    GT_U32 ii; /* Iterator */
    GT_U32 regAddr; /* Register address */
    GT_U32 rangeBmp; /* Half of index ranges bmp */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(indexRangesBmpPtr);

    rc = GT_OK;

    for(ii = 0; ii < 2; ii++)
    {
        switch(blockNum)
        {
            case 0:
                regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.
                            rangeCount.CNCBlockRangeCountEnable[ii][blockNum];
                break;
            case 1:
                regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.
                            rangeCount.CNCBlockRangeCountEnable[ii][blockNum];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr,
            &rangeBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
        indexRangesBmpPtr->l[ii] = rangeBmp;
    }
    return rc;
}

/**
* @internal cpssPxCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      Pipe has 10 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientRangesGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockClientRangesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, indexRangesBmpPtr));

    rc = internal_cpssPxCncBlockClientRangesGet(devNum, blockNum, indexRangesBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, indexRangesBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] format                   - CNC counter format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterFormatSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format
)
{
    GT_U32 regAddr; /* Register address */
    GT_U32 blockBitOffset; /* Bit offset for selected block */
    GT_U32 mode; /* HW value for counter format mode */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(blockNum)
    {
        case 0:
            blockBitOffset = 0;
            break;
        case 1:
            blockBitOffset = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(format)
    {
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E:
            mode = 0;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E:
            mode = 1;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E:
            mode = 2;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E:
            mode = 3;
            break;
        case CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E:
            mode = 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.entryMode.
        CNCBlocksCounterEntryModeRegister;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        blockBitOffset, 3, mode);
}

/**
* @internal cpssPxCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] format                   - CNC counter format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterFormatSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterFormatSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, format));

    rc = internal_cpssPxCncCounterFormatSet(devNum, blockNum, format);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, format));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong counting mode read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
static GT_STATUS internal_cpssPxCncCounterFormatGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_PX_CNC_COUNTER_FORMAT_ENT    *formatPtr
)
{
    GT_STATUS rc; /* Return code */
    GT_U32 regAddr; /* Register address */
    GT_U32 blockBitOffset; /* Bit offset for selected block */
    GT_U32 mode; /* HW value for counter format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(formatPtr);

    switch(blockNum)
    {
        case 0:
            blockBitOffset = 0;
            break;
        case 1:
            blockBitOffset = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.entryMode.
        CNCBlocksCounterEntryModeRegister;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        blockBitOffset, 3, &mode);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(mode)
    {
        case 0:
            *formatPtr = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case 1:
            *formatPtr = CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case 2:
            *formatPtr = CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        case 3:
            *formatPtr = CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E;
            break;
        case 4:
            *formatPtr = CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssPxCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong counting mode read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS cpssPxCncCounterFormatGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_PX_CNC_COUNTER_FORMAT_ENT    *formatPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterFormatGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, formatPtr));

    rc = internal_cpssPxCncCounterFormatGet(devNum, blockNum, formatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, formatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterClearByReadEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32 regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCGlobalConfigReg;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        1, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterClearByReadEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxCncCounterClearByReadEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterClearByReadEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_U32 regAddr; /* Register address */
    GT_U32 value; /* Register value */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCGlobalConfigReg;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, &value);

     *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssPxCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterClearByReadEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxCncCounterClearByReadEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
* @param[in] counterPtr               - (pointer to) counter contents.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterClearByReadValueSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    IN  CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_STATUS rc; /* Return code */
    GT_U32    value[2]; /* HW value of counter */
    GT_U32    regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    rc = prvCpssPxCncCounterSwToHw(format, counterPtr, value);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.
        CNCClearByReadValueRegWord0;
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, value[0]);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.
        CNCClearByReadValueRegWord1;
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, value[1]);
    return rc;
}

/**
* @internal cpssPxCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
* @param[in] counterPtr               - (pointer to) counter contents.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadValueSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    IN  CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterClearByReadValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, format, counterPtr));

    rc = internal_cpssPxCncCounterClearByReadValueSet(devNum, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterClearByReadValueGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_STATUS rc; /* Return code */
    GT_U32    value[2]; /* HW value of counter */
    GT_U32    regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.
        CNCClearByReadValueRegWord0;
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &value[0]);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.
        CNCClearByReadValueRegWord1;
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &value[1]);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssPxCncCounterHwToSw(format, value, counterPtr);
    return rc;
}

/**
* @internal cpssPxCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadValueGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterClearByReadValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, format, counterPtr));

    rc = internal_cpssPxCncCounterClearByReadValueGet(devNum, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         In order to initialize storage needed for uploaded counters,
*         cpssPxCncUploadInit must be called first.
*         An application may check that CNC upload finished by
*         cpssPxCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockUploadTrigger
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      blockNum
)
{
    GT_U32    regAddr; /* Register address */
    GT_U32    regValue; /* Register value */
    GT_U32    triggerOffset; /* Bit offset which triggers block upload */
    GT_U32    inProcessBlocksBmp; /* Block bitmap with busy bits asserted */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(blockNum)
    {
        case 0:
            triggerOffset = 0;
            break;
        case 1:
            triggerOffset = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQControl;
    prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 31, 1, &regValue);
    if((regValue & 0x1) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxCncBlockUploadInProcessGet(devNum, &inProcessBlocksBmp);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(inProcessBlocksBmp & (1 << blockNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount +=
        PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries;

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCFastDumpTriggerReg;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        triggerOffset, 1, GT_TRUE);
}

/**
* @internal cpssPxCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         In order to initialize storage needed for uploaded counters,
*         cpssPxCncUploadInit must be called first.
*         An application may check that CNC upload finished by
*         cpssPxCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockUploadTrigger
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      blockNum
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockUploadTrigger);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum));

    rc = internal_cpssPxCncBlockUploadTrigger(devNum, blockNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncBlockUploadInProcessGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
    GT_U32 regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(inProcessBlocksBmpPtr);

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCFastDumpTriggerReg;

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 2,
        inProcessBlocksBmpPtr);
}

/**
* @internal cpssPxCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockUploadInProcessGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncBlockUploadInProcessGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, inProcessBlocksBmpPtr));

    rc = internal_cpssPxCncBlockUploadInProcessGet(devNum, inProcessBlocksBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, inProcessBlocksBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssPxCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (1K CNC counters)
*         to address upload queue. An application must get all transferred
*         counters. An application may sequentially upload several CNC blocks
*         before start to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of DMA queue
*         full. In this case the cpssPxCncUploadedBlockGet may return only part of
*         the CNC block with return GT_OK. An application must to call
*         cpssPxCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                       waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncUploadedBlockGet
(
    IN     GT_SW_DEV_NUM                     devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT    CPSS_PX_CNC_COUNTER_STC           *counterValuesPtr
)
{
    GT_U32 messageBytes; /* Size of message in bytes */
    GT_U32 regAddr; /* Register address */
    GT_U32 isFull; /* DMA message queue is full */
    GT_STATUS rc; /* Return code */
    GT_U32 inProcessBlocksBmp; /* Block bitmap upload of which in progress*/
    GT_U32 *memPtr; /* Pointer on DMA mapped host memory */
    GT_U32 ii; /* Iterator */
    GT_U32 firstCncWordIdx; /* Word offset in DMA memory */
    GT_U32 word[2]; /* HW value of counter */
    GT_U32 maxCounters; /* Counters number to read */
    CPSS_PX_CNC_COUNTER_STC counter /* SW counter */;
    PRV_CPSS_AU_DESC_STC *descPtr; /* Pointer to DMA message */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfCounterValuesPtr);
    CPSS_NULL_PTR_CHECK_MAC(counterValuesPtr);

    maxCounters = *numOfCounterValuesPtr;
    if(*numOfCounterValuesPtr == 0)
    {
        return GT_OK;
    }

    /* Get DMA message size in bytes */
    messageBytes = 4 * PRV_CPSS_DMA_MESSAGE_SIZE;

    /* Check if queue is full */
    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.fuQControl;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 30, 1, &isFull);
    if(GT_OK != rc)
    {
        return GT_OK;
    }

    if(isFull)
    {
        /* upload paused due to queue is full                  */
        /* queue contains ready portion of counters to retrieve */
    }
    else
    {
        /* If queue is not full check if block upload in progress */
        rc = cpssPxCncBlockUploadInProcessGet(devNum, &inProcessBlocksBmp);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(inProcessBlocksBmp)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, LOG_ERROR_NO_MSG);
        }
    }

    memPtr = (GT_U32 *)(
        PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block + messageBytes *
        PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx);

    /* Process uploaded CNC counters */
    for(ii = 0; ii < maxCounters; ii++)
    {
        /* Check if we read all counters */
        if(PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount == 0)
        {
            *numOfCounterValuesPtr = ii;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
        }
        firstCncWordIdx = (PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount & 1) * 2;

        /* Read counter HW value */
        word[0] = CPSS_32BIT_LE(memPtr[firstCncWordIdx]);
        firstCncWordIdx++;
        word[1] = CPSS_32BIT_LE(memPtr[firstCncWordIdx]);

        /* Convert counter */
        rc = prvCpssPxCncCounterHwToSw(format, word, &counter);
        if (rc != GT_OK)
        {
            return rc;
        }

        counterValuesPtr[ii].byteCount = counter.byteCount;
        counterValuesPtr[ii].packetCount = counter.packetCount;

        PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount--;

        /* Do not go to next DMA message before all words processed in this
           one */
        if (firstCncWordIdx < (PRV_CPSS_DMA_MESSAGE_SIZE - 1))
        {
            continue;
        }

        /* Clear processed value */
        descPtr = (PRV_CPSS_AU_DESC_STC*)memPtr;
        AU_DESC_RESET_MAC(descPtr);

        PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx =
            (PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx + 1) %
            PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.blockSize;
        memPtr = (GT_U32 *)(PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block
            + (messageBytes * PRV_CPSS_PX_PP_MAC(devNum)->
            cncDmaDesc.curIdx));

        if(PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx == 0)
        {
            /* If end of the block reached, reset DMA base address */
            rc = prvCpssPxRewindDma(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }

            *numOfCounterValuesPtr = (ii + 1);
        }
    }

    *numOfCounterValuesPtr = maxCounters;
    return GT_OK;
}

/**
* @internal cpssPxCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssPxCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (1K CNC counters)
*         to address upload queue. An application must get all transferred
*         counters. An application may sequentially upload several CNC blocks
*         before start to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of DMA queue
*         full. In this case the cpssPxCncUploadedBlockGet may return only part of
*         the CNC block with return GT_OK. An application must to call
*         cpssPxCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                       waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncUploadedBlockGet
(
    IN     GT_SW_DEV_NUM                     devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT    CPSS_PX_CNC_COUNTER_STC           *counterValuesPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncUploadedBlockGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfCounterValuesPtr, format, counterValuesPtr));

    rc = internal_cpssPxCncUploadedBlockGet(devNum, numOfCounterValuesPtr, format, counterValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfCounterValuesPtr, format, counterValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterWraparoundEnableSet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_BOOL          enable
)
{
    GT_U32    regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCGlobalConfigReg;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterWraparoundEnableSet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterWraparoundEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxCncCounterWraparoundEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterWraparoundEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_U32    regAddr; /* Register address */
    GT_STATUS rc; /* Return code */
    GT_U32    value; /* Register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).globalRegs.CNCGlobalConfigReg;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr,
        0, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssPxCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterWraparoundEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterWraparoundEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxCncCounterWraparoundEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes.
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes.
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
static GT_STATUS internal_cpssPxCncCounterWraparoundIndexesGet
(
    IN    GT_SW_DEV_NUM    devNum,
    IN    GT_U32           blockNum,
    INOUT GT_U32           *indexNumPtr,
    OUT   GT_U32           indexesArr[]
)
{
    GT_U32    regAddr; /* Register address */
    GT_U32    *regPtr; /* Base of selected counter block */
    GT_STATUS rc; /* Return code */
    GT_U32    value; /* Register value */
    GT_U32    ii; /* Iterator */
    GT_U32    indexCount; /* Index count */
    GT_BOOL   valid; /* Wrap around entry validity */
    GT_U32    index; /* Entry index */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(indexNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    rc = GT_OK;

    switch(blockNum)
    {
        case 0:
            regPtr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.wraparound.
                CNCBlockWraparoundStatusReg[blockNum];
            break;
        case 1:
            regPtr = PRV_PX_REG1_UNIT_CNC_MAC(devNum).perBlockRegs.wraparound.
                CNCBlockWraparoundStatusReg[blockNum];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(*indexNumPtr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    indexCount = 0;
    for(ii = 0; ii < 4; ii++)
    {
        regAddr = regPtr[ii];
        rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr,
            &value);
        if(GT_OK != rc)
        {
            return rc;
        }

        valid = BOOL2BIT_MAC(U32_GET_FIELD_MAC(value, 0, 1));
        index = U32_GET_FIELD_MAC(value, 1, 12);
        if(GT_TRUE == valid)
        {
            indexesArr[indexCount] = index;
            indexCount++;
        }

        if(indexCount >= *indexNumPtr)
        {
            break;
        }

        valid = BOOL2BIT_MAC(U32_GET_FIELD_MAC(value, 16, 1));
        index = U32_GET_FIELD_MAC(value, 17, 12);
        if(GT_TRUE == valid)
        {
            indexesArr[indexCount] = index;
            indexCount++;
        }

        if(indexCount >= *indexNumPtr)
        {
            break;
        }

    }
    *indexNumPtr = indexCount;
    return rc;
}

/**
* @internal cpssPxCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes.
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes.
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS cpssPxCncCounterWraparoundIndexesGet
(
    IN    GT_SW_DEV_NUM    devNum,
    IN    GT_U32           blockNum,
    INOUT GT_U32           *indexNumPtr,
    OUT   GT_U32           indexesArr[]
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterWraparoundIndexesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, indexNumPtr, indexesArr));

    rc = internal_cpssPxCncCounterWraparoundIndexesGet(devNum, blockNum, indexNumPtr, indexesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, indexNumPtr, indexesArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] index                    - counter  in the block
*                                      (APPLICABLE RANGES: 0..1023).
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncCounterGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_U32 value[2]; /* HW value for counter */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    if(index > 1023)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(blockNum)
    {
        case 0:
            rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_CNC_0_COUNTERS_E,
                    index, value);
            break;
        case 1:
            rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_CNC_1_COUNTERS_E,
                    index, value);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxCncCounterHwToSw(format, value, counterPtr);
    return rc;
}

/**
* @internal cpssPxCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] index                    - counter  in the block
*                                      (APPLICABLE RANGES: 0..1023).
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, index, format, counterPtr));

    rc = internal_cpssPxCncCounterGet(devNum, blockNum, index, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, index, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncEgressQueueClientModeSet
(
    IN  GT_SW_DEV_NUM                              devNum,
    IN  CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   mode
)
{
    GT_U32 regAddr; /* Register address */
    GT_U32 value; /* Register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(mode)
    {
        case CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
            value = 1;
            break;
        case CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
            value = 2;
            break;
        case CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E:
            value = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.
        CNCModes;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 2, value);
}

/**
* @internal cpssPxCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncEgressQueueClientModeSet
(
    IN  GT_SW_DEV_NUM                              devNum,
    IN  CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   mode
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssPxCncEgressQueueClientModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncEgressQueueClientModeGet
(
    IN  GT_SW_DEV_NUM                              devNum,
    OUT CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc; /* Return code */
    GT_U32      regAddr; /* Register address */
    GT_U32      value; /* Register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.
        CNCModes;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 2, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            *modePtr = CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;
            break;
        case 1:
            *modePtr = CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
            break;
        case 2:
            *modePtr = CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssPxCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncEgressQueueClientModeGet
(
    IN  GT_SW_DEV_NUM                              devNum,
    OUT CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *modePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncEgressQueueClientModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssPxCncEgressQueueClientModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
* @param[in] mode                     - byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncClientByteCountModeSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN CPSS_PX_CNC_CLIENT_ENT            client,
    IN CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   mode
)
{
    GT_U32 value; /* Register value */
    GT_U32 regAddr; /* Register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E != client)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(mode)
    {
        case CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E:
            value = 0;
            break;
        case CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.
        CNCModes;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 2, value);
}

/**
* @internal cpssPxCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
* @param[in] mode                     - byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncClientByteCountModeSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN CPSS_PX_CNC_CLIENT_ENT            client,
    IN CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   mode
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncClientByteCountModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, client, mode));

    rc = internal_cpssPxCncClientByteCountModeSet(devNum, client, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, client, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
*
* @param[out] modePtr                  - (pointer to) byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCncClientByteCountModeGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT            client,
    OUT CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   *modePtr
)
{
    GT_U32 value; /* Register value */
    GT_U32 regAddr; /* Register address */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E != client)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.
        CNCModes;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 2, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            *modePtr = CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case 1:
            *modePtr = CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssPxCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
*
* @param[out] modePtr                  - (pointer to) byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncClientByteCountModeGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT            client,
    OUT CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   *modePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCncClientByteCountModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, client, modePtr));

    rc = internal_cpssPxCncClientByteCountModeGet(devNum, client, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, client, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

