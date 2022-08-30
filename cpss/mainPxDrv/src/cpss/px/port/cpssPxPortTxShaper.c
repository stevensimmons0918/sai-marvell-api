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
* @file cpssPxPortTxShaper.c
*
* @brief CPSS implementation for configuring the Physical Port Tx Shaper
* This covers:
* - shaping on queues and the logical port
*
*
* @version   1
********************************************************************************
*/
/*#define CPSS_LOG_IN_MODULE_ENABLE*/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/private/prvCpssPxErrataMng.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>
/* #include <cpss/px/port/cpssPxPortPfc.h> */
#include <cpss/px/port/cpssPxPortBufMg.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define PRV_CPSS_PX_TXQREV1_TOKEN_BCKT_UPD_RATE_MIN_CNS        1
#define PRV_CPSS_PX_TXQREV1_TOKEN_BCKT_UPD_RATE_MAX_CNS        15
#define PRV_CPSS_PX_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MIN_CNS  1
#define PRV_CPSS_PX_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MAX_CNS  16
#define PRV_CPSS_PX_TXQREV1_TOKENS_REFILL_MAX_CNS              0xFFF

/* number of words in the shaper per port per TC entry */
#define TXQ_SHAPER_ENTRY_WORDS_NUM_CNS  16

/*
 * typedef: struct PORT_TX_SHAPER_TOKEN_BUCKET_STC
 *
 * Description:  structure to hold port's shaper info (per port/tc)
 *
 * Enumerations:
 *         tokenBucketEn - token Bucket Enable
 *         slowRateEn    - slow Rate Enable
 *         tbUpdateRatio - token bucket update ratio
 *         tokens - number of tokens
 *         maxBucketSize - max Bucket Size
 *         currentBucketSize - current Bucket Size
 *         tokenBucketAvbEnabled - controls enabling/disabling AVB feature
 *                                 GT_TRUE - AVB enabled
 *                                 GT_FALSE - AVB disabled
 *
 * Comments:
 *
 */
typedef struct
{
    GT_BOOL tokenBucketEn;
    GT_BOOL slowRateEn;
    GT_U32  tbUpdateRatio;
    GT_U32  tokens;
    GT_U32  maxBucketSize;
    GT_U32  currentBucketSize;
    GT_BOOL tokenBucketAvbEnabled;
}PORT_TX_SHAPER_TOKEN_BUCKET_STC;


/**
* @enum SHAPER_DISABLE_STAGE_ENT
 *
 * @brief enum for stage in the disabling shaper operation
*/
typedef enum{

    /** stage 1 -- until sleep , without sleep */
    SHAPER_DISABLE_STAGE_1_E,

    /** stage 2 -- from sleep , without sleep */
    SHAPER_DISABLE_STAGE_2_E

} SHAPER_DISABLE_STAGE_ENT;


/**
* @internal portTxShaperTokenBucketBuild function
* @endinternal
*
* @brief   build entry format for tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in,out] startHwArray             - pointer to start of HW format of the full entry
*
* @param[out] maskPtr                  - (pointer to) mask buffer for HW entry with
*                                      raised bits appropriating to entry fields assigned
*                                      inside this function
*                                       GT_OK
*/
static GT_STATUS portTxShaperTokenBucketBuild
(
    INOUT GT_U32  *startHwArray,
    OUT GT_U32    *maskPtr,
    IN    GT_BOOL usePerTc,
    IN    GT_U32  tc,
    IN    PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    GT_U32  startSubEntryOffset;/* offset till the sub entry  */
    GT_U32  offset;/* offset in side the sub entry */
    GT_U32  value;/* hwValue*/

    startSubEntryOffset = (usePerTc == GT_TRUE) ? (61 * tc) : 0;

    value = BOOL2BIT_MAC(subEntryPtr->tokenBucketEn);
    offset = 0;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        1, value);

    offset = 1;
    value = BOOL2BIT_MAC(subEntryPtr->slowRateEn);
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        1, value);

    offset = 2;
    value = subEntryPtr->tbUpdateRatio;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        4, value);

    offset = 6;
    value = subEntryPtr->tokens;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        18, value);

    offset = 24;
    value = subEntryPtr->maxBucketSize;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        12, value);

    offset = 36;
    value = subEntryPtr->currentBucketSize;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        24, value);

    value = BOOL2BIT_MAC(subEntryPtr->tokenBucketAvbEnabled);
    offset = 60;
    U32_SET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        1, value);

    raiseBitsInMemory(maskPtr, startSubEntryOffset, 61);
    return GT_OK;
}

/**
* @internal portTxShaperTokenBucketParse function
* @endinternal
*
* @brief   parse entry format from tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[out] subEntryPtr              - (pointer to) entry when usePerTc = GT_FALSE
*                                      (pointer to) sub entry when usePerTc = GT_TRUE
*                                       GT_OK
*/
static GT_STATUS portTxShaperTokenBucketParse
(
    IN  GT_U32  *startHwArray,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    OUT  PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    GT_U32  startSubEntryOffset;/* offset till the sub entry  */
    GT_U32  offset;/* offset in side the sub entry */
    GT_U32  value;/* hwValue*/

    startSubEntryOffset = (usePerTc == GT_TRUE) ? (61 * tc) : 0;

    offset = 0;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        1, value);
    subEntryPtr->tokenBucketEn = BIT2BOOL_MAC(value);

    offset = 1;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        1, value);
    subEntryPtr->slowRateEn = BIT2BOOL_MAC(value);

    offset = 2;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        4, value);
    subEntryPtr->tbUpdateRatio = value;

    offset = 6;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        18, value);
    subEntryPtr->tokens = value;

    offset = 24;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        12, value);
    subEntryPtr->maxBucketSize = value;

    offset = 36;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray, (offset + startSubEntryOffset),
        24, value);
    subEntryPtr->currentBucketSize = value;

    offset = 60;
    U32_GET_FIELD_IN_ENTRY_MAC(startHwArray,(offset + startSubEntryOffset),
        1, value);
    subEntryPtr->tokenBucketAvbEnabled = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal portTxShaperTokenBucketEntryWrite function
* @endinternal
*
* @brief   write entry to tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
* @param[in] subEntryPtr              - (pointer to) entry when usePerTc = GT_FALSE
*                                      (pointer to) sub entry when usePerTc = GT_TRUE
*                                       GT_OK
*/
static GT_STATUS portTxShaperTokenBucketEntryWrite
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    IN  PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    GT_U32  hwEntryArray[TXQ_SHAPER_ENTRY_WORDS_NUM_CNS];/* HW entry array */
    GT_U32  hwMaskArray[TXQ_SHAPER_ENTRY_WORDS_NUM_CNS];/* HW mask array */
    GT_STATUS rc; /* return code */
    CPSS_PX_TABLE_ENT tableType; /* table type */
    GT_U32      txqPortNum; /* TXQ port number */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);

    cpssOsMemSet(hwEntryArray, 0, sizeof(hwEntryArray));
    cpssOsMemSet(hwMaskArray,  0, sizeof(hwMaskArray));


    if(usePerTc == GT_FALSE)
    {
        tableType = CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E;
    }
    else
    {
        tableType =
            CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E;
    }

    /* build the entry */
    rc = portTxShaperTokenBucketBuild(hwEntryArray, hwMaskArray, usePerTc, tc,
        subEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* write the entry */
    rc = prvCpssPxWriteTableEntryMasked(devNum, tableType, txqPortNum,
        hwEntryArray, hwMaskArray);

    return rc;
}

/**
* @internal portTxShaperTokenBucketEntryRead function
* @endinternal
*
* @brief   read entry from tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
*
* @param[out] subEntryPtr              - (pointer to) entry when usePerTc = GT_FALSE
*                                      (pointer to) sub entry when usePerTc = GT_TRUE
*                                       GT_OK
*/
static GT_STATUS portTxShaperTokenBucketEntryRead
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    OUT PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    GT_U32  hwEntryArray[TXQ_SHAPER_ENTRY_WORDS_NUM_CNS];/* HW entry array */
    GT_STATUS rc; /* return code */
    CPSS_PX_TABLE_ENT tableType; /* table type */
    GT_U32      txqPortNum; /* TXQ port number */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);

    if(usePerTc == GT_FALSE)
    {
        tableType = CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E;
    }
    else
    {
        tableType =
            CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E;
    }

    /* read the entry */
    rc = prvCpssPxReadTableEntry(devNum, tableType, txqPortNum, hwEntryArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* parse the entry */
    rc = portTxShaperTokenBucketParse(hwEntryArray, usePerTc, tc, subEntryPtr);

    return rc;
}

/**
* @internal prvPxPortTxShaperTokenBucketEntryRead function
* @endinternal
*
* @brief   read entry from tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
*
* @param[out] subEntryPtr              - (pointer to) entry when usePerTc = GT_FALSE
*                                      (pointer to) sub entry when usePerTc = GT_TRUE
*                                       GT_OK
*/
GT_STATUS prvPxPortTxShaperTokenBucketEntryRead
(
    IN GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              usePerTc,
    IN  GT_U32               tc,
    OUT PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    return portTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc, tc,
        subEntryPtr);
}

/**
* @internal prvCpssPxPortTxShaperTokenBucketEntryWrite function
* @endinternal
*
* @brief   Write entry to tx shaper token bucket configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
* @param[in] slowRateEn               - slow rate enable value
* @param[in] tbUpdateRatio            - TB interval update ratio value
* @param[in] tokens                   - TB refill value
* @param[in] maxBucketSize            - max Bucket Size
* @param[in] updCurrentBucketSize     - GT_TRUE: update CurrentBucketSize
*                                      GT_FALSE: don't update CurrentBucketSize
* @param[in] currentBucketSize        - current Bucket Size. Relevant only when
* @param[in] updCurrentBucketSize     is GT_TRUE.
*                                       GT_OK
*/
GT_STATUS prvCpssPxPortTxShaperTokenBucketEntryWrite
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    IN  GT_BOOL slowRateEn,
    IN  GT_U32  tbUpdateRatio,
    IN  GT_U32  tokens,
    IN  GT_U32  maxBucketSize,
    IN  GT_BOOL updCurrentBucketSize,
    IN  GT_U32  currentBucketSize
)
{
    GT_STATUS rc; /*return code */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry; /* token bucket descriptor */

    /* read entry from HW */
    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc, tc,
        &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update the entry */
    subEntry.slowRateEn = slowRateEn;
    subEntry.tbUpdateRatio = tbUpdateRatio;
    subEntry.tokens = tokens;
    subEntry.maxBucketSize = maxBucketSize;

    if (updCurrentBucketSize == GT_TRUE)
    {
        subEntry.currentBucketSize = currentBucketSize;
    }

    /* write updated entry to HW */
    rc = portTxShaperTokenBucketEntryWrite(devNum, portNum, usePerTc, tc,
        &subEntry);
    return rc;
}


/**
* @internal portTxShaperTokenBucketEnable function
* @endinternal
*
* @brief   enable entry to tx shaper token bucket configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
* @param[in] enable                   - GT_TRUE,  shaping
*                                      GT_FALSE, disable shaping
*                                       GT_OK
*/
static GT_STATUS portTxShaperTokenBucketEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    IN  GT_BOOL enable,
    IN  GT_BOOL avbModeEnable
)
{
    GT_STATUS rc; /* return code */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry; /* token bucket descriptor */

    /* temporary token bucket descriptor */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC tmpSubEntry;

    /* read entry from HW */
    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc, tc,
        &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Don't update if value remains the same */
    if ((subEntry.tokenBucketEn == enable) &&
        (subEntry.tokenBucketAvbEnabled == avbModeEnable))
    {
        return GT_OK;
    }

    subEntry.tokenBucketAvbEnabled = avbModeEnable;

    if((enable == GT_TRUE) ||
       (PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_TOKEN_BUCKET_RATE_SHAPING_DISABLE_WA_E) == GT_FALSE))
    {
        /* update the entry */
        subEntry.tokenBucketEn = enable;

        /* write updated entry to HW */
        rc = portTxShaperTokenBucketEntryWrite(devNum, portNum, usePerTc,
            tc, &subEntry);
        return rc;
    }

    /*
        NOTE: next logic replace the  prvPxShaperDisable(...)
    */

    /* Disabling the Egress Rate Shaper under traffic may hang its relevant
       transmit queue. (FEr#47) */

    tmpSubEntry = subEntry;
    /* Set Number Tokens and Bucket Size to a maximal values
       and disable slow rate */
    tmpSubEntry.slowRateEn    = GT_FALSE;
    tmpSubEntry.tokens        = 0x3FFFF;
    tmpSubEntry.maxBucketSize = 0xFFF;
    tmpSubEntry.tbUpdateRatio = 0;

    /* write updated entry to HW */
    rc = portTxShaperTokenBucketEntryWrite(devNum, portNum, usePerTc, tc,
        &tmpSubEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Token bucket update to new maximal values will be configured by next
       pulse of token bucket update clock. It's need to be done delay to
       disable token bucket to guaranty maximal values update.
       The delay need to be for time more than maximal token bucket update
       period - 15360 core clock cycles. For Core Clock 144MHz delay should be
       more than 100 micro seconds. */
    cpssOsTimerWkAfter(1);

    /* disable shaping */
    tmpSubEntry.tokenBucketEn = GT_FALSE;
    tmpSubEntry.currentBucketSize = 0xFFFFFF;

    /* write updated entry to HW */
    rc = portTxShaperTokenBucketEntryWrite(devNum, portNum, usePerTc, tc,
        &tmpSubEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* restore old values */
    subEntry.tokenBucketEn = GT_FALSE;

    /* write updated entry to HW */
    return portTxShaperTokenBucketEntryWrite(devNum, portNum, usePerTc, tc,
        &subEntry);
}

/**
* @internal prvPxPortTxCalcShaperTokenBucketRate function
* @endinternal
*
* @brief   Get shaper parameters for token bucket per port / per port per queue:
*         - Token refill value.
*         - slow-rate mode.
*         - tokens rate for XG/Gs.
*         - slow rate ratio.
*         The Formula is:
*         <Core Clock Frequency in Kbps> <Token refill value in bits>
*         Rate in Kbps = -------------------------------------------------------------
*         <Refill period in clock cycles>
*         Refill period in clock cycles when slow rate is enabled =
*         <tokensRate> <tokensRateGran> <Slow Rate>
*         Refill period in clock cycles when slow rate is disabled =
*         <tokensRate> <tokensRateGran> 2 ^ < tbUpdateRatio >
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] shaperConfigPtr          - (pointer to) shaper configuration
* @param[in] shaperMode               - shaper mode
* @param[in] slowRateEn               - slow rate enable value
* @param[in] tbUpdateRatio            - TB interval update ratio value
* @param[in] tokenRefillValue         - TB refill value
*
* @param[out] tokenBucketRatePtr       - (pointer to) the actual Rate in Kbps or pps.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvPxPortTxCalcShaperTokenBucketRate
(
    IN    GT_SW_DEV_NUM           devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    IN    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *shaperConfigPtr,
    IN    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   shaperMode,
    IN    GT_BOOL                 slowRateEn,
    IN    GT_U32                  tbUpdateRatio,
    IN    GT_U32                  tokenRefillValue,
    OUT   GT_U32                 *tokenBucketRatePtr
)
{
    GT_U32 refillPeriod;     /* Refill period in clock cycles */
    GT_U32 coreClockInK;     /* clock rate in Khz */
    GT_U32 packetLengthBits; /* packet length used in packet based shaping */
                             /* in bits */
    GT_U32 tempRate;         /* temporary token bucket rate */
    GT_U32 remainder;        /* remainder from division */
    GT_U64 tempResult;       /* used for temp calculation with U64 */
    GT_U16 tempResult1;      /* used for temp calculation with U16 */
    GT_STATUS rc;
    GT_BOOL   isCpu;

    if (tbUpdateRatio > 10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = cpssPxPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
    if (rc != GT_OK)
    {
        return rc;
    }
    packetLengthBits = ((isCpu == GT_TRUE) ?
                    shaperConfigPtr->cpuPacketLength :
                    shaperConfigPtr->portsPacketLength) * 8;

    coreClockInK = PRV_CPSS_PP_MAC(devNum)->coreClock * 1000;

    switch(shaperConfigPtr->tokensRateGran)
    {
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
            refillPeriod = 64 + 1;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
            refillPeriod = 1024 + 1;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E:
            refillPeriod = 128 + 1;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E:
            refillPeriod = 256 + 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    refillPeriod *= shaperConfigPtr->tokensRate;
    if (slowRateEn)
    {
        refillPeriod *= shaperConfigPtr->slowRateRatio;
    }
    else
    {
        refillPeriod *= 1 << tbUpdateRatio;
    }

    if(refillPeriod == 0)
    {
        /* the values of shaperConfigPtr->tokensRate from HW indicate problem */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

   /* use 64-bit data to avoid 32-bit overflow caused errors */
    tempResult = prvCpssMathMul64(tokenRefillValue, coreClockInK * 8);
    tempResult = prvCpssMathDiv64By32(tempResult, refillPeriod, &remainder);
    *tokenBucketRatePtr = tempResult.l[0];

    /* round up the result */
    if (remainder)
    {
        *tokenBucketRatePtr += 1;
    }

    if (packetLengthBits != 0 && shaperMode == CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E)
    {
        tempRate = *tokenBucketRatePtr;
        if(packetLengthBits < BIT_17)
        {
            tempResult1 = (GT_U16)(packetLengthBits/2);
            tempResult = prvCpssMathMul64(tempRate, 500);
            tempResult = prvCpssMathDiv64By16(tempResult, tempResult1);
            *tokenBucketRatePtr = tempResult.l[0];
        }
        else
        {
            *tokenBucketRatePtr = tempRate * 1000 / packetLengthBits;
        }

        /* round up the result */
        if ((tempRate * 1000) % packetLengthBits)
        {
            *tokenBucketRatePtr += 1;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortTxReCalcShaperTokenBucketRate function
* @endinternal
*
* @brief   Calculate shaping rate parameters for given tokenBucketRate:
*         1. TB refill value
*         2. slow rate enable value
*         3. TB interval update ratio value
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] shaperConfigPtr          - (pointer to) shaper configuration.
* @param[in] shaperMode               - shaper mode
* @param[in] tokenBucketRate          - the shaping rate - actual Rate in Kbps.
* @param[in] burstSize                - burst size in bytes
*                                      (already adjusted for baseline)
*
* @param[out] slowRateEnPtr            - (pointer to) slow rate enable value
* @param[out] tbUpdateRatioPtr         - (pointer to) TB interval update ratio value
* @param[out] tokensPtr                - (pointer to) TB refill value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortTxReCalcShaperTokenBucketRate
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC     *shaperConfigPtr,
    IN  CPSS_PX_PORT_TX_SHAPER_MODE_ENT       shaperMode,
    IN  GT_U32                                tokenBucketRate,
    IN  GT_U32                                burstSize,
    OUT GT_BOOL                               *slowRateEnPtr,
    OUT GT_U32                                *tbUpdateRatioPtr,
    OUT GT_U32                                *tokensPtr
)
{
    GT_U32      rateDivisor;       /* rate divisor due to token refill period */
    GT_U16      coreClockInM;      /* clock rate in Mhz */
    GT_U32      packetLengthBits;/* packet length used in packet based shaping*/
    GT_U32      TBIntervalUpdateRatio; /* bucket update interval ratio */
    GT_U32      TBIntervalUpdateRatioSaved;  /* Saved value for Slow Rate calculation. */
    GT_U64      tempResult;        /* used for temp calculation with U64 */
    GT_U64      tempResult1;       /* used for temp calculation with U64 */
    GT_U32      temp;              /* temporary value */
    GT_U32      tokensGran;        /* HW granularity */
    GT_STATUS   rc;                /* return code */
    GT_BOOL     isCpu;             /* is CPU port*/
    GT_U32      maxTokenRefill;

    maxTokenRefill = PRV_CPSS_PX_TXQREV1_TOKENS_REFILL_MAX_CNS;

    /* After each adding tokens to the bucket                                        */
    /* the amount of tokens in the bucket immediately reduced to maximal bucket size */
    /* and the redundant tokens are lost.                                            */
    /* Amount of added tokens should not be greater then maximal bucket size,        */
    /* otherwise tokens will be lost and the planned rate will not be reached.       */
    /* burstSize == 0 should be ignored (used for debugging only)                    */
    if ((burstSize > 0) && (maxTokenRefill > burstSize))
    {
        maxTokenRefill = burstSize;
    }

    rc = cpssPxPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
    if (rc != GT_OK)
    {
        return rc;
    }

    tempResult.l[0] = tempResult.l[1] = 0;

    packetLengthBits = ((isCpu == GT_TRUE) ?
                    shaperConfigPtr->cpuPacketLength :
                    shaperConfigPtr->portsPacketLength) * 8;

    if (packetLengthBits != 0 &&
        shaperMode == CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E)
    {
        tempResult1 = prvCpssMathMul64(tokenBucketRate, packetLengthBits);
        tempResult1 = prvCpssMathDiv64By16(tempResult1, 1000);
        tokenBucketRate = tempResult1.l[0];
    }

    coreClockInM = (GT_U16)PRV_CPSS_PP_MAC(devNum)->coreClock;

    switch(shaperConfigPtr->tokensRateGran)
    {
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
            tokensGran = 64 + 1;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
            tokensGran = 1024 + 1;
            break;

        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E:
            tokensGran = 128 + 1;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E:
            tokensGran = 256 + 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rateDivisor = shaperConfigPtr->tokensRate * tokensGran;

    TBIntervalUpdateRatio=11;
    do
    {
        TBIntervalUpdateRatio--;
        temp = ( 1 << TBIntervalUpdateRatio) * rateDivisor;
        tempResult = prvCpssMathMul64(tokenBucketRate,temp);
        tempResult = prvCpssMathDiv64By16(tempResult,coreClockInM);
        tempResult = prvCpssMathDiv64By16(tempResult,1000 * 8);
    }
    while ((TBIntervalUpdateRatio != 0) && ((tempResult.l[1] > 0) ||
          (tempResult.l[0] > maxTokenRefill)));

    /* check for minimum */
    if ((tempResult.l[0] == 0) && (tempResult.l[1] == 0))
    {
        tempResult.l[0] = 1;
    }

    /* check for maximum */
    if ((tempResult.l[1] > 0) ||
        (tempResult.l[0] > maxTokenRefill))
    {
        tempResult.l[0] = maxTokenRefill;
        tempResult.l[1] = 0;

    }

    /* When token refill value is even:
     *  Halve both the token refill value and the update interval,
     *  it will not affect the traffic rate, and shaper burstiness will be improved.
     */
    TBIntervalUpdateRatioSaved = TBIntervalUpdateRatio;
    while ((TBIntervalUpdateRatio != 0) &&
            ((tempResult.l[0] % 2) == 0))
    {
        TBIntervalUpdateRatio--;
        tempResult.l[0] /= 2;
    }

    *slowRateEnPtr = GT_FALSE;
    *tokensPtr = tempResult.l[0];
    *tbUpdateRatioPtr = TBIntervalUpdateRatio;

    /* check if slowRate should be used */
    if ((1U << TBIntervalUpdateRatioSaved) < shaperConfigPtr->slowRateRatio)
    {
        temp = shaperConfigPtr->slowRateRatio * rateDivisor;
        tempResult = prvCpssMathMul64(tokenBucketRate,temp);
        tempResult = prvCpssMathDiv64By16(tempResult,coreClockInM);
        tempResult = prvCpssMathDiv64By16(tempResult,1000 * 8);

        if ((tempResult.l[1] == 0) &&
            (tempResult.l[0] < maxTokenRefill))
        {
            *slowRateEnPtr = GT_TRUE;
            *tokensPtr = tempResult.l[0];
            *tbUpdateRatioPtr = TBIntervalUpdateRatioSaved;
        }
    }
    return GT_OK;
}


/**
* @internal prvPxPortTxSetShaperTokenBucketParams function
* @endinternal
*
* @brief   Set shaper parameters for token bucket per port / per port per queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] shaperMode               - shaper mode
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tcQueue                  - traffic class queue on this Physical Port
* @param[in] oldShaperConfigPtr       - (pointer to) old shaper global configurations
* @param[in] newShaperConfigPtr       - (pointer to) new shaper global configurations
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvPxPortTxSetShaperTokenBucketParams
(
    IN GT_SW_DEV_NUM                       devNum,
    IN GT_PHYSICAL_PORT_NUM                portNum,
    IN CPSS_PX_PORT_TX_SHAPER_MODE_ENT     shaperMode,
    IN GT_BOOL                             usePerTc,
    IN GT_U32                              tcQueue,
    IN CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *oldShaperConfigPtr,
    IN CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *newShaperConfigPtr
)
{
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry; /* sub entry format */
    GT_STATUS   rc;                /* return code */
    GT_U32      tokenBucketRate;   /* TB rate in Kbps or pps */
    GT_BOOL     slowRateEn;        /* slow rate enable */
    GT_U32      tbUpdateRatio;     /* TB interval update ratio value */
    GT_U32      tokenRefillValue;  /* TB refill value */
    GT_U32      baseline;         /* baseline value */

    /* get global shaper baseline */
    rc = cpssPxPortTxShaperBaselineGet(devNum,&baseline);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc,
        tcQueue, &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* The function should do nothing in the case disabled bucket. */
    if(subEntry.tokenBucketEn == GT_FALSE)
    {
        return GT_OK;
    }

    /* calculate the rate */
    rc = prvPxPortTxCalcShaperTokenBucketRate(devNum, portNum,
        oldShaperConfigPtr, shaperMode, subEntry.slowRateEn,
        subEntry.tbUpdateRatio, subEntry.tokens, &tokenBucketRate);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate per TB configurations with new global shaper configurations */
    rc = prvCpssPxPortTxReCalcShaperTokenBucketRate(devNum,
                                                    portNum,
                                                    newShaperConfigPtr,
                                                    shaperMode,
                                                    tokenBucketRate,
                                                    (subEntry.maxBucketSize * _4K) -
                                                         baseline,
                                                    &slowRateEn,
                                                    &tbUpdateRatio,
                                                    &tokenRefillValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update the TB entry */
    rc = prvCpssPxPortTxShaperTokenBucketEntryWrite(devNum, portNum,
        usePerTc, tcQueue, slowRateEn, tbUpdateRatio, tokenRefillValue,
        subEntry.maxBucketSize, GT_FALSE, 0);
    return rc;
}




/**
* @internal internal_cpssPxPortTxShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperEnableSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     enable,
    IN  GT_BOOL                     avbModeEnable
)
{
    GT_U32      txqPortNum; /* TXQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);

    return portTxShaperTokenBucketEnable(devNum, portNum, GT_FALSE, 0,
        enable, avbModeEnable);
}

/**
* @internal cpssPxPortTxShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable,
    IN  GT_BOOL              avbModeEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, avbModeEnable));

    rc = internal_cpssPxPortTxShaperEnableSet(devNum, portNum, enable, avbModeEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, avbModeEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping status on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *enablePtr,
    OUT GT_BOOL              *avbModeEnablePtr
)
{
    GT_STATUS   rc;         /* return value */
    GT_U32      txqPortNum; /* TXQ port number */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(avbModeEnablePtr);

    /* read entry from HW */
    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, GT_FALSE, 0,
        &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = subEntry.tokenBucketEn;
    *avbModeEnablePtr = subEntry.tokenBucketAvbEnabled;

    return GT_OK;
}

/**
* @internal cpssPxPortTxShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping status on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *enablePtr,
    OUT GT_BOOL              *avbModeEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, avbModeEnablePtr));

    rc = internal_cpssPxPortTxShaperEnableGet(devNum, portNum, enablePtr, avbModeEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, avbModeEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxPortTxShaperQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
* @param[in] enable                   - GT_TRUE,  shaping on this traffic queue
*                                      GT_FALSE, disable shaping on this traffic queue
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperQueueEnableSet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 tcQueue,
    IN  GT_BOOL                enable,
    IN  GT_BOOL                avbModeEnable
)
{
    GT_U32      txqPortNum;  /* TXQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    return portTxShaperTokenBucketEnable(devNum, portNum, GT_TRUE, tcQueue,
        enable, avbModeEnable);
}

/**
* @internal cpssPxPortTxShaperQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
* @param[in] enable                   - GT_TRUE,  shaping on this traffic queue
*                                      GT_FALSE, disable shaping on this traffic queue
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable,
    IN  GT_BOOL                 avbModeEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperQueueEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enable, avbModeEnable));

    rc = internal_cpssPxPortTxShaperQueueEnableSet(devNum, portNum, tcQueue, enable, avbModeEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enable, avbModeEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable shaping status
*         of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperQueueEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                tcQueue,
    OUT GT_BOOL              *enablePtr,
    OUT GT_BOOL              *avbModeEnablePtr
)
{
    GT_STATUS   rc;         /* return value */
    GT_U32      txqPortNum;  /* TXQ port number */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry; /* entry to read */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(avbModeEnablePtr);

    /* read entry from HW */
    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, GT_TRUE,
        tcQueue, &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = subEntry.tokenBucketEn;
    *avbModeEnablePtr = subEntry.tokenBucketAvbEnabled;

    return GT_OK;
}

/**
* @internal cpssPxPortTxShaperQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable shaping status
*         of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
*
* @param[out] enablePtr                - (pointer to) token Bucket rate shaping status.
* @param[out] avbModeEnablePtr         - (pointer to) AVB mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tcQueue,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_BOOL                 *avbModeEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperQueueEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enablePtr, avbModeEnablePtr));

    rc = internal_cpssPxPortTxShaperQueueEnableGet(devNum, portNum, tcQueue, enablePtr, avbModeEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enablePtr,avbModeEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortShaperProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port /
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on the port
*                                      CPSS_PARAM_NOT_USED_CNS - to get per port Shaper Profile.
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device, tcQueue
*/
static GT_STATUS prvCpssPxPortShaperProfileSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               tcQueue,
    IN  GT_U32               burstSize,
    INOUT GT_U32             *maxRatePtr
)
{
    GT_STATUS   rc;                     /* return value */
    GT_U32      tokens;  /* number of tokens that are added to the bucket */
    GT_BOOL     usePerTc;/* do we set 'per port' or 'per port per tc'*/
    GT_U32      txqPortNum; /* TXQ port number */

    /* global shaper configurations */
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC shaperConfig;
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   shaperMode;   /* shaper mode on port */
    GT_U32 tbUpdateRatio;    /* TB interval update ratio value */
    GT_BOOL slowRateEnable;  /* slow rate enable value */
    GT_U32 baseline;         /* baseline value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    /* check burst size validity - 12 bits in HW */
    if(burstSize >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(tcQueue == CPSS_PARAM_NOT_USED_CNS)
    {
        usePerTc = GT_FALSE;
    }
    else
    {
        /* validate Traffic Classes queue */
        PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
        usePerTc = GT_TRUE;
    }

    /* get global shaper baseline */
    rc = cpssPxPortTxShaperBaselineGet(devNum,&baseline);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get global shaper configurations */
    rc = cpssPxPortTxShaperConfigurationGet(devNum, &shaperConfig);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get shaper mode on port */
    rc = cpssPxPortTxShaperModeGet(devNum,portNum, &shaperMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate per TB configurations */
    rc = prvCpssPxPortTxReCalcShaperTokenBucketRate(devNum,
                                                    portNum,
                                                    &shaperConfig,
                                                    shaperMode,
                                                    *maxRatePtr,
                                                    (burstSize*_4K) - baseline,
                                                    &slowRateEnable,
                                                    &tbUpdateRatio,
                                                    &tokens);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update TB entry */
    /* set the current Bucket Size to max Bucket Size */
    rc = prvCpssPxPortTxShaperTokenBucketEntryWrite(devNum,
        portNum, usePerTc, tcQueue, slowRateEnable, tbUpdateRatio, tokens,
        burstSize, GT_TRUE, burstSize * _4KB);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* calculate the actual Rate value */
    return prvPxPortTxCalcShaperTokenBucketRate(devNum, portNum,
        &shaperConfig, shaperMode, slowRateEnable, tbUpdateRatio, tokens,
        maxRatePtr);
}

/**
* @internal internal_cpssPxPortTxShaperProfileSet function
* @endinternal
*
* @brief   Set Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to port shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperProfileSet
(
    IN    GT_SW_DEV_NUM         devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U32                burstSize,
    INOUT GT_U32               *maxRatePtr
)
{
    return prvCpssPxPortShaperProfileSet(devNum, portNum,
        CPSS_PARAM_NOT_USED_CNS, burstSize, maxRatePtr);
}

/**
* @internal cpssPxPortTxShaperProfileSet function
* @endinternal
*
* @brief   Set Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to port shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperProfileSet
(
    IN    GT_SW_DEV_NUM         devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U32                burstSize,
    INOUT GT_U32               *maxRatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, burstSize, maxRatePtr));

    rc = internal_cpssPxPortTxShaperProfileSet(devNum, portNum, burstSize, maxRatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, burstSize, maxRatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_prvCpssPxPortShaperProfileGet function
* @endinternal
*
* @brief   Get Shaper Profile for Traffic Class Queue of specified port /
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on the port
*                                      CPSS_PARAM_NOT_USED_CNS - to get per port Shaper Profile.
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device, tcQueue
*/
static GT_STATUS internal_prvCpssPxPortShaperProfileGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32   tcQueue,
    OUT GT_U32   *burstSizePtr,
    OUT GT_U32   *maxRatePtr,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS   rc;            /* return value */

    /* sub entry format - relevant for TXQ_REV_1 */
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry;
    GT_BOOL     usePerTc;/* do we set 'per port' or 'per port per tc'*/

    /* shaper global configuration */
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC shaperConfig;
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   shaperMode;   /* shaper mode on port */
    GT_U32      txqPortNum; /* TXQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(burstSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(tcQueue == CPSS_PARAM_NOT_USED_CNS)
    {
        usePerTc = GT_FALSE;
    }
    else
    {
        /* validate Traffic Classes queue */
        PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
        usePerTc = GT_TRUE;
    }

    /* read the entry */
    rc = portTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc,
        tcQueue, &subEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = subEntry.tokenBucketEn;
    /* Token Bucket rate shaping is disabled. */
    if(*enablePtr == GT_FALSE)
    {
        return GT_OK;
    }
    /* Token Bucket rate shaping is enabled. */
    *burstSizePtr = (GT_U16)subEntry.maxBucketSize;

    /* get global shaper configurations */
    rc = cpssPxPortTxShaperConfigurationGet(devNum, &shaperConfig);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortTxShaperModeGet(devNum, portNum, &shaperMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvPxPortTxCalcShaperTokenBucketRate(devNum, portNum,
        &shaperConfig, shaperMode, subEntry.slowRateEn, subEntry.tbUpdateRatio,
        subEntry.tokens, maxRatePtr);
}

/**
* @internal cpssPxPortTxShaperProfileGet function
* @endinternal
*
* @brief   Get Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperProfileGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32  *burstSizePtr,
    OUT GT_U32  *maxRatePtr,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, burstSizePtr, maxRatePtr, enablePtr));

    rc = internal_prvCpssPxPortShaperProfileGet(devNum, portNum, CPSS_PARAM_NOT_USED_CNS, burstSizePtr, maxRatePtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, burstSizePtr, maxRatePtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode, packet or byte based for given port.
*         Shapers are configured by cpssPxPortTxShaperProfileSet or
*         cpssPxPortTxShaperQueueProfileSet
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] mode                     - shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_TX_SHAPER_MODE_ENT mode
)
{
    GT_U32  regAddr;        /* register address             */
    GT_U32  hwPortTbMode;   /* The Port Token bucket Rate shaper mode  */
    GT_U32  txqPortNum;     /* TXQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);

    switch (mode)
    {
        case CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E:
            hwPortTbMode = 0;
            break;
        case CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E:
            hwPortTbMode = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.
        portTokenBucketMode[txqPortNum];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, hwPortTbMode);
}

/**
* @internal cpssPxPortTxShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode, packet or byte based for given port.
*         Shapers are configured by cpssPxPortTxShaperProfileSet or
*         cpssPxPortTxShaperQueueProfileSet
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] mode                     - shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_TX_SHAPER_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssPxPortTxShaperModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode, packet or byte based for given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] modePtr                  - (pointer to) shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperModeGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_TX_SHAPER_MODE_ENT *modePtr
)
{
    GT_U32     regAddr;     /* register address             */
    GT_U32     value;       /* value to read from register */
    GT_STATUS  rc;          /* return code */
    GT_U32     txqPortNum;  /* TXQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum,
        txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.
        portTokenBucketMode[txqPortNum];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &value);

    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (value == 1) ? CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E :
                              CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;
    return GT_OK;
}

/**
* @internal cpssPxPortTxShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode, packet or byte based for given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] modePtr                  - (pointer to) shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperModeGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_TX_SHAPER_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssPxPortTxShaperModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperBaselineSet function
* @endinternal
*
* @brief   Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes(0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU.
*       2. If PFC response is enabled, the Baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortPfcEnableSet.
*       3. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. portsPacketLength
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. cpuPacketLength
*       see:
*       cpssPxPortTxShaperConfigurationSet.
*
*/
static GT_STATUS internal_cpssPxPortTxShaperBaselineSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          baseline
)
{
    GT_U32      regAddr;    /* register address             */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (baseline > 0xFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set Token Bucket Base Line */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.tokenBucketBaseLine;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, baseline);
}

/**
* @internal cpssPxPortTxShaperBaselineSet function
* @endinternal
*
* @brief   Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes(0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU.
*       2. If PFC response is enabled, the Baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortPfcEnableSet.
*       3. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. portsPacketLength
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. cpuPacketLength
*       see:
*       cpssPxPortTxShaperConfigurationSet.
*
*/
GT_STATUS cpssPxPortTxShaperBaselineSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           baseline
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperBaselineSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseline));

    rc = internal_cpssPxPortTxShaperBaselineSet(devNum, baseline);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseline));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperBaselineGet function
* @endinternal
*
* @brief   Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxShaperBaselineGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT GT_U32   *baselinePtr
)
{
    GT_U32      regAddr;    /* register address                */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(baselinePtr);

    /* Get Token Bucket Base Line */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.tokenBucketBaseLine;

    return prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, baselinePtr);
}

/**
* @internal cpssPxPortTxShaperBaselineGet function
* @endinternal
*
* @brief   Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperBaselineGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT GT_U32   *baselinePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperBaselineGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baselinePtr));

    rc = internal_cpssPxPortTxShaperBaselineGet(devNum, baselinePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baselinePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperConfigurationSet function
* @endinternal
*
* @brief   Set global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       configsPtr->portsPacketLength
*       configsPtr->cpuPacketLength
*       see:
*       cpssPxPortTxShaperBaselineSet.
*
*/
static GT_STATUS internal_cpssPxPortTxShaperConfigurationSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC   *configsPtr
)
{
    GT_U32      value;      /* value to write into register */
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address             */
    GT_U32      mask;       /* register mask */
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC oldConfigs; /* old global configurations */
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   shaperMode; /* shaper mode on port */
    GT_U32 port;
    GT_U32 tcQueue = 0;
    GT_BOOL isValid;
    GT_U32  txqPortNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(configsPtr);

    if((configsPtr->tokensRate <    PRV_CPSS_PX_TXQREV1_TOKEN_BCKT_UPD_RATE_MIN_CNS) ||
       (configsPtr->tokensRate >    PRV_CPSS_PX_TXQREV1_TOKEN_BCKT_UPD_RATE_MAX_CNS) ||
       (configsPtr->slowRateRatio < PRV_CPSS_PX_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MIN_CNS) ||
       (configsPtr->slowRateRatio > PRV_CPSS_PX_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MAX_CNS) ||
       (configsPtr->portsPacketLength > 0xFFFFFF) ||
       (configsPtr->cpuPacketLength > 0xFFFFFF))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* save the old configurations */
    rc = cpssPxPortTxShaperConfigurationGet(devNum,&oldConfigs);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set update rates */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.tokenBucketUpdateRate;

    value = ((configsPtr->slowRateRatio - 1)& 0xF) << 17 |
            (configsPtr->tokensRate & 0xF) << 25;

    switch (configsPtr->tokensRateGran)
    {
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
            value |= 0 << 30;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
            value |= 3 << 30;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E:
            value |= 1 << 30;
            break;
        case CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E:
            value |= 2 << 30;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = 0xDE1E0000;

    rc = prvCpssHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, mask, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set CPU Token Bucket MTU */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.CPUTokenBucketMTU;

    value = configsPtr->cpuPacketLength;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set Ports Token Bucket MTU */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.portsTokenBucketMTU;
    value = configsPtr->portsPacketLength;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Reconfigure all enabled Token Buckets per port and per port per queue. */
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    for (port = 0; port < PRV_CPSS_PX_PORTS_NUM_CNS; port++)
    {

        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, port, &isValid);
        if((rc != GT_OK) || (isValid != GT_TRUE))
        {
            continue;
        }

        rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(devNum, port,
            PRV_CPSS_PX_PORT_TYPE_TXQ_E, &txqPortNum);
        if ((rc == GT_BAD_PARAM) &&
            (txqPortNum == CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS))
        {
            continue;
        }


        /* get shaper mode on the port */
        rc = cpssPxPortTxShaperModeGet(devNum,port,&shaperMode);
        if (rc != GT_OK)
        {
            return rc;
        }


        /* per port */
        rc = prvPxPortTxSetShaperTokenBucketParams(devNum, port,
            shaperMode, GT_FALSE, tcQueue, &oldConfigs, configsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* per port per traffic class (tcQueue) */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc = prvPxPortTxSetShaperTokenBucketParams(devNum, port,
                shaperMode, GT_TRUE, tcQueue, &oldConfigs, configsPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortTxShaperConfigurationSet function
* @endinternal
*
* @brief   Set global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
* @param[in] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       configsPtr->portsPacketLength
*       configsPtr->cpuPacketLength
*       see:
*       cpssPxPortTxShaperBaselineSet.
*
*/
GT_STATUS cpssPxPortTxShaperConfigurationSet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperConfigurationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configsPtr));

    rc = internal_cpssPxPortTxShaperConfigurationSet(devNum, configsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxShaperConfigurationGet function
* @endinternal
*
* @brief   Get global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static GT_STATUS internal_cpssPxPortTxShaperConfigurationGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PORT_TX_SHAPER_CONFIG_STC   *configsPtr
)
{
    GT_U32      value;      /* value to write into register */
    GT_STATUS   rc;         /* return code                  */
    GT_U32      regAddr;    /* register address             */
    GT_U32      mask;       /* register mask                */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(configsPtr);
    /* Get update rates */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.tokenBucketUpdateRate;

    mask = 0xDE1E0000;

    rc = prvCpssDrvHwPpReadRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, mask, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configsPtr->slowRateRatio = ((value >> 17) & 0xF) + 1;
    configsPtr->tokensRate = (value >> 25) & 0xF;

    switch ((value >> 30))
    {
        case 0:
            configsPtr->tokensRateGran =
                CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
            break;
        case 3:
            configsPtr->tokensRateGran =
                CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
            break;
        case 1:
            configsPtr->tokensRateGran =
                CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E;
            break;
        case 2:
            configsPtr->tokensRateGran =
                CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }


    /* Get CPU Token Bucket MTU */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.CPUTokenBucketMTU;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    configsPtr->cpuPacketLength = value;

    /* Get Ports Token Bucket MTU */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.portsTokenBucketMTU;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    configsPtr->portsPacketLength = value;

    return GT_OK;
}

/**
* @internal cpssPxPortTxShaperConfigurationGet function
* @endinternal
*
* @brief   Get global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number.
*
* @param[out] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssPxPortTxShaperConfigurationGet
(
    IN GT_SW_DEV_NUM    devNum,
    OUT CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperConfigurationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configsPtr));

    rc = internal_cpssPxPortTxShaperConfigurationGet(devNum, configsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortTxShaperQueueProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port /
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on the port
*                                      CPSS_PARAM_NOT_USED_CNS - to get per port Shaper Profile.
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device, tcQueue
*/
static GT_STATUS internal_cpssPxPortTxShaperQueueProfileSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_U32                  burstSize,
    INOUT GT_U32                *maxRatePtr
)
{
    return prvCpssPxPortShaperProfileSet(CAST_SW_DEVNUM(devNum), portNum,
            tcQueue, burstSize, maxRatePtr);
}


/**
* @internal cpssPxPortTxShaperQueueProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Logical Port
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueProfileSet
(
    IN    GT_SW_DEV_NUM         devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U32                tcQueue,
    IN    GT_U32                burstSize,
    INOUT GT_U32               *maxRatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperQueueProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, burstSize, maxRatePtr));

    rc = internal_cpssPxPortTxShaperQueueProfileSet(devNum, portNum, tcQueue, burstSize, maxRatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, burstSize, maxRatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssPxPortTxShaperQueueProfileGet function
* @endinternal
*
* @brief   Get Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxShaperQueueProfileGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32    tcQueue,
    OUT GT_U32   *burstSizePtr,
    OUT GT_U32   *maxRatePtr,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxShaperQueueProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, burstSizePtr, maxRatePtr, enablePtr));

    rc = internal_prvCpssPxPortShaperProfileGet(devNum, portNum, tcQueue, burstSizePtr, maxRatePtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, burstSizePtr, maxRatePtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

