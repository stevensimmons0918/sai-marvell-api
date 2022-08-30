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
* @file cpssDxChCnc.c
*
* @brief CPSS DxCh Centralized Counters (CNC) API.
*
* @version   65
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCncLog.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCnc.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
 

/* address of register increments: */
/* wraparound indexes per client */
#define CNC_WRAPAROUND_STATUS_ADDR_INCREMENT_CNS  0x00000100
/* 2k-counter block */
#define CNC_COUNTER_BLOCK_ADDR_INCREMENT_CNS      0x00004000

/* check that CNC block upload operation is supported,      */
/* not supported if PCI is not available or that workaround */
/* for FDB AU messages problem is used.                     */
#define PRV_CNC_BLOCK_UPLOAD_SUPPORT_CHECK_MAC(_dev)                    \
    if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(_dev) == 0) ||                \
       (PRV_CPSS_DXCH_PP_MAC(_dev)->errata.                             \
                   info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.    \
                       enabled == GT_TRUE))                             \
    {                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); \
    }

/* The macro checks the _format parameter validity for Lion and        */
/* above devices. For other devices it assigns the _format to the      */
/* alone value supported for these devices.                            */
/* Such behavior of the macro makes the _format value passed by the    */
/* the caller not relevant for not Lion devices.                       */
#define PRV_CNC_COUNTER_FORMAT_CHECK_MAC(_devNum,_format)                             \
{                                                                                     \
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(_devNum))                                 \
    {                                                                                 \
        if (PRV_CPSS_SIP_6_CHECK_MAC(_devNum))                                        \
        {                                                                             \
            if (((GT_U32)_format) > CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E)            \
            {                                                                         \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);        \
            }                                                                         \
        }                                                                             \
        else if (PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum))                                \
        {                                                                             \
            if (((GT_U32)_format) > CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E)            \
            {                                                                         \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);        \
            }                                                                         \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            if (((GT_U32)_format) > CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E)            \
            {                                                                         \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);        \
            }                                                                         \
        }                                                                             \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        _format = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E;                              \
    }                                                                                 \
}

void  resetBitsInMemory(
    IN GT_U32                  *memPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
);
#define PRINT_p_MAC(x)    cpssOsPrintf("%s = [%p] \n",#x,x)
#define PRINT_d_MAC(x)    cpssOsPrintf("%s = [%d] \n",#x,x)
#define PRINT_b_MAC(x)    cpssOsPrintf("%s = [%s] \n",#x,x?"GT_TRUE":"GT_FALSE")

#ifdef ASIC_SIMULATION
    #define CNC_MINIMAL_SLEEP_TIME_CNS 50
#else /* HW */
    #define CNC_MINIMAL_SLEEP_TIME_CNS 1
#endif /* HW */


/**
* @internal prvCpssDxChCncCounterSwToHw function
* @endinternal
*
* @brief   The function converts the SW counter to 64-bit HW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNumdevNum
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] swCounterPtr             - (pointer to) CNC Counter in SW format
*
* @param[out] hwCounterArr[]           - (pointer to) CNC Counter in HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncCounterSwToHw
(
    IN   GT_U8 devNum,
    IN   CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN   CPSS_DXCH_CNC_COUNTER_STC         *swCounterPtr,
    OUT  GT_U32                            hwCounterArr[]
)
{
    switch (format)
    {
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            hwCounterArr[0] =
                (swCounterPtr->packetCount.l[0] & 0x1FFFFFFF)
                | (swCounterPtr->byteCount.l[1] << 29);
            hwCounterArr[1] = swCounterPtr->byteCount.l[0];
            break;

        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            hwCounterArr[0] =
                (swCounterPtr->packetCount.l[0] & 0x07FFFFFF)
                | (swCounterPtr->byteCount.l[1] << 27);
            hwCounterArr[1] = swCounterPtr->byteCount.l[0];
            break;

        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            hwCounterArr[0] = swCounterPtr->packetCount.l[0];
            hwCounterArr[1] =
                (swCounterPtr->byteCount.l[0] & 0x07FFFFFF)
                | (swCounterPtr->packetCount.l[1] << 27);
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                hwCounterArr[0] = swCounterPtr->packetCount.l[0];
                hwCounterArr[1] = swCounterPtr->packetCount.l[1];
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                hwCounterArr[0] = swCounterPtr->byteCount.l[0];
                hwCounterArr[1] = swCounterPtr->byteCount.l[1];
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E:
            /* Trigger counter: 44 bits  Max Value : 20 bits*/
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                hwCounterArr[0] = swCounterPtr->packetCount.l[0];
                hwCounterArr[1] = (swCounterPtr->maxValue.l[0] & 0xFFFFF )
                                  | (swCounterPtr->packetCount.l[1] << 20);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncCounterHwToSw function
* @endinternal
*
* @brief   The function converts the 64-bit HW counter to SW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNumdevNum
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] hwCounterArr[]           - (pointer to) CNC Counter in HW format
*
* @param[out] swCounterPtr             - (pointer to) CNC Counter in SW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncCounterHwToSw
(
    IN   GT_U8 devNum,
    IN   CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN   GT_U32                            hwCounterArr[],
    OUT  CPSS_DXCH_CNC_COUNTER_STC         *swCounterPtr
)
{
    switch (format)
    {
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            swCounterPtr->packetCount.l[0] =
                (hwCounterArr[0] & 0x1FFFFFFF);
            swCounterPtr->packetCount.l[1] = 0;
            swCounterPtr->byteCount.l[0] = hwCounterArr[1];
            swCounterPtr->byteCount.l[1] =
                ((hwCounterArr[0] >> 29) & 0x07);
            break;

        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            swCounterPtr->packetCount.l[0] =
                (hwCounterArr[0] & 0x07FFFFFF);
            swCounterPtr->packetCount.l[1] = 0;
            swCounterPtr->byteCount.l[0] = hwCounterArr[1];
            swCounterPtr->byteCount.l[1] =
                ((hwCounterArr[0] >> 27) & 0x1F);
            break;

        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            swCounterPtr->packetCount.l[0] = hwCounterArr[0];
            swCounterPtr->packetCount.l[1] =
                ((hwCounterArr[1] >> 27) & 0x1F);
            swCounterPtr->byteCount.l[0] =
                (hwCounterArr[1] & 0x07FFFFFF);
            swCounterPtr->byteCount.l[1] = 0;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                swCounterPtr->packetCount.l[0] = hwCounterArr[0];
                swCounterPtr->packetCount.l[1] = hwCounterArr[1];
                swCounterPtr->byteCount.l  [0] = 0;
                swCounterPtr->byteCount.l  [1] = 0;

            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                swCounterPtr->packetCount.l[0] = 0;
                swCounterPtr->packetCount.l[1] = 0;
                swCounterPtr->byteCount.l  [0] = hwCounterArr[0];
                swCounterPtr->byteCount.l  [1] = hwCounterArr[1];
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E:
            /* Trigger counter: 44 bits, Max Value: 20 bits */
            swCounterPtr->packetCount.l[0] = hwCounterArr[0];
            swCounterPtr->packetCount.l[1] =
                ((hwCounterArr[1] >> 20) & 0xFFF);
            swCounterPtr->maxValue.l[0] =
                (hwCounterArr[1] & 0xFFFFF);
            swCounterPtr->maxValue.l[1] = 0;
            swCounterPtr->byteCount.l[0] = 0;
            swCounterPtr->byteCount.l[1] = 0;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncPortGroupCounterValueAverageSet function
* @endinternal
*
* @brief   The function sets the average counter value.
*         The get function must sum the values per port groups
*         and the calculated result must be as given.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] regAddr                  - address of 64-bit memory to set the average value
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCncPortGroupCounterValueAverageSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            regAddr,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;               /* return code                */
    GT_U32    data[2];          /* registers value            */
    GT_U16    portGroupNum;     /* number of port groups      */
    GT_U16    portGroupIndex;   /* index of port group in BMP */
    GT_U32    portGroupId;      /* the port group Id - support multi port groups device */
    CPSS_DXCH_CNC_COUNTER_STC   counter; /* work counter      */
    GT_U16    pktCntrMod;       /* packetCouner % portGroupNum*/
    GT_U16    byteCntrMod;      /* byteCouner % portGroupNum  */
    GT_U64    w64;              /* work variable              */


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CNC_COUNTER_FORMAT_CHECK_MAC(devNum, format);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_CNC_0_E,&portGroupNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* calculate average counter value */
    pktCntrMod = prvCpssMathMod64By16(
        counterPtr->packetCount, portGroupNum);
    byteCntrMod = prvCpssMathMod64By16(
        counterPtr->byteCount, portGroupNum);
    counter.packetCount = prvCpssMathDiv64By16(
        counterPtr->packetCount, portGroupNum);
    counter.byteCount = prvCpssMathDiv64By16(
        counterPtr->byteCount, portGroupNum);

    /* add one to average */
    w64.l[1] = 0;
    w64.l[0] = 1;
    counter.packetCount = prvCpssMathAdd64(
        counter.packetCount, w64);
    counter.byteCount = prvCpssMathAdd64(
        counter.byteCount, w64);

    counter.maxValue.l[0] = counterPtr->maxValue.l[0];
    counter.maxValue.l[1] = counterPtr->maxValue.l[1];

    /* to avoid compiler warning */
    data[0] = 0;
    data[1] = 0;

    /* loop on all port groups to get the counter value from */
    portGroupIndex = 0;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        {
            if (portGroupIndex == pktCntrMod)
            {
                /* subtract one from average back */
                counter.packetCount = prvCpssMathSub64(
                    counter.packetCount, w64);
            }

            if (portGroupIndex == byteCntrMod)
            {
                /* subtract one from average back */
                counter.byteCount = prvCpssMathSub64(
                    counter.byteCount, w64);
            }

            if ((portGroupIndex == 0)
                || (portGroupIndex == pktCntrMod)
                || (portGroupIndex == byteCntrMod))
            {
                /* convert counter from SW to HW format       */
                /* this conversion done at the first loop and */
                /* any time when "counter" updated            */
                rc = prvCpssDxChCncCounterSwToHw(devNum,format, &counter, /*OUT*/data);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            rc = prvCpssHwPpPortGroupWriteRam(
                devNum, portGroupId, regAddr, 2/* words*/, data);
            if (rc != GT_OK)
            {
                return rc;
            }

            portGroupIndex ++;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDxChCncPortGroupCounterValueSummingGet function
* @endinternal
*
* @brief   The function gets the counter contents summing per port groups
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] regAddr                  - address of 64-bit memory to get the sum value
*                                      (APPLICABLE RANGES: xCat3, AC5 0..2047)
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) counter contents
*                                      The result counter contains the sum of counters
*                                      read from all specified port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCncPortGroupCounterValueSummingGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            regAddr,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;               /* return code       */
    GT_U32    data[2];          /* field value       */
    GT_U32    portGroupId;      /* port group Id     */
    CPSS_DXCH_CNC_COUNTER_STC  counter; /* work counter */

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CNC_COUNTER_FORMAT_CHECK_MAC(devNum, format);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    cpssOsMemSet(counterPtr, 0, sizeof(CPSS_DXCH_CNC_COUNTER_STC));

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupReadRam(
            devNum, portGroupId, regAddr, 2/*words*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChCncCounterHwToSw(devNum,format, data, /*OUT*/&counter);
        if (rc != GT_OK)
        {
            return rc;
        }

        counterPtr->packetCount =
            prvCpssMathAdd64(
                counterPtr->packetCount, counter.packetCount);

        counterPtr->byteCount =
            prvCpssMathAdd64(
                counterPtr->byteCount, counter.byteCount);
        if ( format == CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E)
        {
            counterPtr->maxValue =
                ((prvCpssMathCompare64(counterPtr->maxValue, counter.maxValue) > 0)
                     ? counterPtr->maxValue : counter.maxValue);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDxChCncMultiUnitsBlockUnitGet function
* @endinternal
*
* @brief   The function calculates unit and relative block by block number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*
* @param[out] unitNumPtr               - (pointer to) number of CNC unit.
* @param[out] blockInUnitPtr           - (pointer to) number of block in CNC unit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncMultiUnitsBlockUnitGet
(
    IN   GT_U8                     devNum,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *unitNumPtr,
    OUT  GT_U32                    *blockInUnitPtr
)
{
    if((blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }

    if (1 == PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits)/*Lion2/xCat3*/
    {
        /* no conversion , since single CNC unit exists */
        *unitNumPtr     = 0;
        *blockInUnitPtr = blockNum;
        return GT_OK;
    }

    *unitNumPtr     = blockNum / PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks;
    *blockInUnitPtr = blockNum % PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks;

    return GT_OK;
}

#define ERROR_ON_NOT_SUPPORTED_CLIENT(client)                             \
    case client:                                                          \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "%s - not supported", \
            #client)

/**
* @internal sip_6_10_BlockClientToBitMask function
* @endinternal
*
* @brief   sip_6_10: The function returns bit mask for register configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[out] bitMaskPtr               - (pointer to) bit mask of the block configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS sip_6_10_BlockClientToBitMask
(
    IN   GT_U8                     devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *bitMaskPtr
)
{
    devNum = devNum;

    if(blockNum < 32)/*0..31*/
    {
        switch (client)
        {
            case CPSS_DXCH_CNC_CLIENT_TTI_E:                                 /* fall through */
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:                      *bitMaskPtr = BIT_28 ; break;
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:                      *bitMaskPtr = BIT_29 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:                /* fall through */
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:     *bitMaskPtr = BIT_0  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:     *bitMaskPtr = BIT_1  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:     *bitMaskPtr = BIT_10 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:     *bitMaskPtr = BIT_11 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:              /* fall through */
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:     *bitMaskPtr = BIT_2  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:     *bitMaskPtr = BIT_3  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:     *bitMaskPtr = BIT_4  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:     *bitMaskPtr = BIT_5  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:                *bitMaskPtr = BIT_6  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:     *bitMaskPtr = BIT_6  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:     *bitMaskPtr = BIT_7  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:     *bitMaskPtr = BIT_8  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:     *bitMaskPtr = BIT_9  ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E: *bitMaskPtr = BIT_13 ; break;
            case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:                   *bitMaskPtr = BIT_14 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:                   *bitMaskPtr = BIT_15 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:              *bitMaskPtr = BIT_16 ; break;
            case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:               *bitMaskPtr = BIT_17 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:               *bitMaskPtr = BIT_18 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:              *bitMaskPtr = BIT_19 ; break;
            case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:                    *bitMaskPtr = BIT_20 ; break;
            case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:                        *bitMaskPtr = BIT_21 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:                    *bitMaskPtr = BIT_22 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:                          *bitMaskPtr = BIT_23 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:               *bitMaskPtr = BIT_23 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:               *bitMaskPtr = BIT_24 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:               *bitMaskPtr = BIT_25 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:               *bitMaskPtr = BIT_26 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:        *bitMaskPtr = BIT_27 ; break;
            case CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E:                          *bitMaskPtr = BIT_30 ; break;
            case CPSS_DXCH_CNC_CLIENT_PORT_STAT_E:                           *bitMaskPtr = BIT_31 ; break;
            case CPSS_DXCH_CNC_CLIENT_PHA_E:                                 *bitMaskPtr = BIT_12 ; break;
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E - blockNum[%d] not supported (supports only 32..63)",
                    blockNum);
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E - blockNum[%d] not supported (supports only 32..63)",
                    blockNum);
            default: CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(client);
        }
    }
    else  /*blockNum : 32..63 */
    {
        switch (client)
        {
            case CPSS_DXCH_CNC_CLIENT_TTI_E:                                 /* fall through */
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E - blockNum[%d] not supported (supports only 0..31)",
                    blockNum);
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E - blockNum[%d] not supported (supports only 0..31)",
                    blockNum);
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:                /* fall through */
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:     *bitMaskPtr = BIT_0   ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:     *bitMaskPtr = BIT_1  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:     *bitMaskPtr = BIT_10 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:     *bitMaskPtr = BIT_11 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:              /* fall through */
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:     *bitMaskPtr = BIT_2  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:     *bitMaskPtr = BIT_3  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:     *bitMaskPtr = BIT_4  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:     *bitMaskPtr = BIT_5  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:                *bitMaskPtr = BIT_6  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:     *bitMaskPtr = BIT_6  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:     *bitMaskPtr = BIT_7  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:     *bitMaskPtr = BIT_8  ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:     *bitMaskPtr = BIT_9  ; break;
            case CPSS_DXCH_CNC_CLIENT_PHA_E:                                 *bitMaskPtr = BIT_12 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E: *bitMaskPtr = BIT_13 ; break;
            case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:                   *bitMaskPtr = BIT_14 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:                   *bitMaskPtr = BIT_15 ; break;
            case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:              *bitMaskPtr = BIT_16 ; break;
            case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:               *bitMaskPtr = BIT_17 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:               *bitMaskPtr = BIT_18 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:              *bitMaskPtr = BIT_19 ; break;
            case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:                    *bitMaskPtr = BIT_20 ; break;
            case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:                        *bitMaskPtr = BIT_21 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:                    *bitMaskPtr = BIT_22 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:                          *bitMaskPtr = BIT_23 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:               *bitMaskPtr = BIT_23 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:               *bitMaskPtr = BIT_24 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:               *bitMaskPtr = BIT_25 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:               *bitMaskPtr = BIT_26 ; break;
            case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:        *bitMaskPtr = BIT_27 ; break;
            case CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E:                          *bitMaskPtr = BIT_30 ; break;
            case CPSS_DXCH_CNC_CLIENT_PORT_STAT_E:                           *bitMaskPtr = BIT_31 ; break;
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E:                      *bitMaskPtr = BIT_28 ; break;
            case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:                      *bitMaskPtr = BIT_29 ; break;
            default: CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(client);
        }
    }
    return GT_OK;
}

/**
* @internal sip_6_15_BlockClientToBitMask function
* @endinternal
*
* @brief   sip_6_15: The function returns bit mask for register configuration.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P.
*
* @param[in] client                   - CNC client
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[out] bitMaskPtr               - (pointer to) bit mask of the block configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS sip_6_15_BlockClientToBitMask
(
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *bitMaskPtr
)
{
    if(blockNum >= 16)/*0..31*/
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "blockNum[%d] not supported (supporting only 16 blocks)",
            blockNum);
    }

    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_TTI_E:                                 /* fall through */
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:                      *bitMaskPtr = BIT_0  ; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:                      *bitMaskPtr = BIT_1  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:                /* fall through */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:     *bitMaskPtr = BIT_9  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:     *bitMaskPtr = BIT_10 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:     *bitMaskPtr = BIT_11 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:     *bitMaskPtr = BIT_12 ; break;/* MUXED on bit 12 !!! */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:              /* fall through */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:     *bitMaskPtr = BIT_2  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:     *bitMaskPtr = BIT_3  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:     *bitMaskPtr = BIT_4  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:     *bitMaskPtr = BIT_5  ; break;/* MUXED on bit 5 !!! */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:                *bitMaskPtr = BIT_6  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:     *bitMaskPtr = BIT_6  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:     *bitMaskPtr = BIT_7  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:     *bitMaskPtr = BIT_8  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:     *bitMaskPtr = BIT_5  ; break;/* MUXED on bit 5 !!! */
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E: *bitMaskPtr = BIT_13 ; break;
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:                   *bitMaskPtr = BIT_14 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:                   *bitMaskPtr = BIT_15 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:              *bitMaskPtr = BIT_16 ; break;
        case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:               *bitMaskPtr = BIT_17 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:               *bitMaskPtr = BIT_18 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:              *bitMaskPtr = BIT_19 ; break;
        case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:                    *bitMaskPtr = BIT_20 ; break;
        case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:                        *bitMaskPtr = BIT_21 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:                    *bitMaskPtr = BIT_22 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:                          *bitMaskPtr = BIT_23 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:               *bitMaskPtr = BIT_23 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:               *bitMaskPtr = BIT_24 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:               *bitMaskPtr = BIT_25 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:               *bitMaskPtr = BIT_26 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:        *bitMaskPtr = BIT_27 ; break;
        case CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E:                          *bitMaskPtr = BIT_30 ; break;
        case CPSS_DXCH_CNC_CLIENT_PORT_STAT_E:                           *bitMaskPtr = BIT_31 ; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E:                      *bitMaskPtr = BIT_28 ; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:                      *bitMaskPtr = BIT_12 ; break;/* MUXED on bit 12 !!! */
        case CPSS_DXCH_CNC_CLIENT_PHA_E:                                 *bitMaskPtr = BIT_29 ; break;
        default: CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(client);
    }

    return GT_OK;
}
/**
* @internal sip_6_30_BlockClientToBitMask function
* @endinternal
*
* @brief   sip_6_30: The function returns bit mask for register configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] client                   - CNC client
* @param[out] clientIdPtr             - (pointer to) bit mask of the block configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS sip_6_30_BlockClientToBitMask
(
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT  GT_U32                    *clientIdPtr
)
{
    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_TTI_E:                                 /* fall through */
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:                      *clientIdPtr = 0  ; break;/*tti2cnc_action0_valid*/
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:                      *clientIdPtr = 1  ; break;/*tti2cnc_action1_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:                /* fall through */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:     *clientIdPtr = 2  ; break;/*pcl2cnc_update_plcy1_action0_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:     *clientIdPtr = 3  ; break;/*pcl2cnc_update_plcy1_action1_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:              /* fall through */
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:     *clientIdPtr = 4  ; break;/*pcl2cnc_update_plcy2_action0_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:     *clientIdPtr = 5  ; break;/*pcl2cnc_update_plcy2_action1_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E: *clientIdPtr = 6  ; break;/*eft2cnc_evlan_valid*/
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:                   *clientIdPtr = 7 ; break;/*pcl2cnc_update_vlan_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:                   *clientIdPtr = 8 ; break;/*pcl2cnc_update_eport_valid*/
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:              *clientIdPtr = 9 ; break;/*eq2cnc_ing_vlan_valid*/
        case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:               *clientIdPtr = 10 ; break;/*eq2cnc_drop_pass_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:               *clientIdPtr = 11 ; break;/*preq2cnc_evlan_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:              *clientIdPtr = 12 ; break;/*preq2cnc_cn_valid*/
        case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:                    *clientIdPtr = 13 ; break;/*ha2cnc_arp_valid*/
        case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:                        *clientIdPtr = 14 ; break;/*ha2cnc_ts_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:                    *clientIdPtr = 15 ; break;/*ha2cnc_eport_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:                          /* fall through */
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:               *clientIdPtr = 16 ; break;/*epcl2cnc_action0_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:               *clientIdPtr = 17 ; break;/*epcl2cnc_action1_valid*/
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:        *clientIdPtr = 18 ; break;/*erep2cnc_valid*/
        case CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E:                          *clientIdPtr = 30 ; break;/*preq2cnc_qstat_valid*/
        case CPSS_DXCH_CNC_CLIENT_PORT_STAT_E:                           *clientIdPtr = 31 ; break;/*preq2cnc_pstat_valid*/

        /* give explicit ERROR indication for non-supported clients
           for clients that support by other sip6 devices */
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_PHA_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E);/* considered same as CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E*/
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E);
        ERROR_ON_NOT_SUPPORTED_CLIENT(CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E);

        default: CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(client);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncEArchBlockClientToCfgAddr function
* @endinternal
*
* @brief   The function returns address and bit mask for register configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups. needed for sip6.30
* @param[in] client                   - CNC client
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] calledForWrite           - indication of calling as part of 'write' to HW or 'read' from HW .
*                                       needed for sip6.30
*                                       GT_TRUE  - 'write'
*                                       GT_FALSE - 'read'
*
* @param[out] regAddrPtr               - (pointer to) address of the block configuration register.
*                                        if NULL indication that bitMaskPtr will return 'client instance'
* @param[out] bitMaskPtr               - (pointer to) bit mask of the block configuration.
*                                    for sip 6.30 this is 'clientIdPtr' (value 0..31 of the client)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncEArchBlockClientToCfgAddr
(
    IN   GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    IN   GT_BOOL                   calledForWrite,
    OUT  GT_U32                    *regAddrPtr,
    OUT  GT_U32                    *bitMaskPtr
)
{
    GT_STATUS   rc;
    GT_U32      unitNum;     /* number of CNC unit          */
    GT_U32      blockInUnit; /* number of block in CNC unit */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      value;      /* a value from register */
    GT_U32      lowestFreeIndex = GT_NA;/* first free client index for sip6.30*/
    GT_U32      clientIndex;/* client index for sip6.30*/
    GT_U32      hwClient;   /* HW client for sip6.30 (convert from SW client)  */
    GT_U32      regAddr;    /* register address */

    if(((client == CPSS_DXCH_CNC_CLIENT_TTI_E) ||
            (client == CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E) ||
            (client == CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E)) &&
        (blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass)
    {
        switch (client)
        {
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                /* IPCL stage bypassed */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The device NOT support IPCL-2 (only IPCL-0 and IPCL-1)");
            default:
                /* OK */
                break;
        }
    }

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(regAddrPtr)
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
            clientEnable.CNCBlockConfigReg0[blockInUnit][0/*BWC*/];
    }

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        rc = sip_6_30_BlockClientToBitMask(client,&hwClient);
        if(rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
            devNum, portGroupsBmp, portGroupId);

        for(clientIndex = 0 ;clientIndex < 3; clientIndex++)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                clientEnable.CNCBlockConfigReg0[blockInUnit][clientIndex];
            rc = prvCpssHwPpPortGroupReadRegister(
                devNum, portGroupId, regAddr, &value);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(regAddrPtr)
            {
                *regAddrPtr = regAddr;
            }

            if(0 == (value & 0x1))
            {
                /* not valid client */
                if(lowestFreeIndex == GT_NA)
                {
                    lowestFreeIndex = clientIndex;
                }
                continue;
            }

            /* check if the existing client in this index  match the new client */
            if(((value >> 4) & 0x1F) == hwClient)
            {
                /* we have a match */
                /* so the caller will get the register address of current index */
                if(regAddrPtr == NULL)
                {
                    /* indication that we want 'clientIndex' instead of 'hwClient' */
                    *bitMaskPtr = clientIndex;
                }
                else
                {
                    *bitMaskPtr = hwClient;
                }

                return GT_OK;
            }
        }

        if(lowestFreeIndex == GT_NA)
        {
            if(calledForWrite == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "other 3 clients occupy block [%d] already !",blockNum);
            }
            else
            {
                /* let the caller to give 'block not enabled' for the client */
                lowestFreeIndex = 0;
            }
        }

        if(regAddrPtr == NULL)
        {
            /* indication that we want 'clientIndex' instead of 'hwClient' */
            *bitMaskPtr = lowestFreeIndex;
        }
        else
        {
            *bitMaskPtr = hwClient;

            *regAddrPtr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                clientEnable.CNCBlockConfigReg0[blockInUnit][lowestFreeIndex];
        }

        return GT_OK;
    }
    if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        return sip_6_15_BlockClientToBitMask(client,blockNum,bitMaskPtr);
    }
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return sip_6_10_BlockClientToBitMask(devNum,client,blockNum,bitMaskPtr);
    }

    /* for PCL and TTI mask contains all parallel sublookup clients */
    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_TTI_E:                             *bitMaskPtr = BIT_0  ; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:                  *bitMaskPtr = BIT_0  ; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:                  *bitMaskPtr = BIT_1  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:            *bitMaskPtr = BIT_2  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E: *bitMaskPtr = BIT_2  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E: *bitMaskPtr = BIT_3  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E: *bitMaskPtr = BIT_4  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E: *bitMaskPtr = BIT_5  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:          *bitMaskPtr = BIT_6  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E: *bitMaskPtr = BIT_6  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E: *bitMaskPtr = BIT_7  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E: *bitMaskPtr = BIT_8  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E: *bitMaskPtr = BIT_9  ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:            *bitMaskPtr = BIT_10 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E: *bitMaskPtr = BIT_10 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E: *bitMaskPtr = BIT_11 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E: *bitMaskPtr = BIT_12 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E: *bitMaskPtr = BIT_13 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E:
             if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
             }
             *bitMaskPtr = BIT_13 ;
             break;
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:               *bitMaskPtr = BIT_14 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:               *bitMaskPtr = BIT_15 ; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:          *bitMaskPtr = BIT_16 ; break;
        case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:           *bitMaskPtr = BIT_17 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:           *bitMaskPtr = BIT_18 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:          *bitMaskPtr = BIT_19 ; break;
        case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_20;
             break;
        case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_21 ;
             break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_22 ;
             break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:                      *bitMaskPtr = BIT_23 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:           *bitMaskPtr = BIT_23 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:           *bitMaskPtr = BIT_24 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:           *bitMaskPtr = BIT_25 ; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:           *bitMaskPtr = BIT_26 ; break;
        case CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E:
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_27 ;
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_27 ;
            break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_28 ;
            break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_29 ;
            break;
        case CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_30 ;
            break;
        case CPSS_DXCH_CNC_CLIENT_PORT_STAT_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *bitMaskPtr = BIT_31 ;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncBlockClientToCountMode function
* @endinternal
*
* @brief   The function returns address and start bit and number of bits for 'count mode' field.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] regAddrPtr               - (pointer to) address of the block count mode register.
* @param[out] startBitPtr              - (pointer to) start bit of the field.
* @param[out] numBitsPtr               - (pointer to) number of bits in the field.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncBlockClientToCountMode
(
    IN   GT_U8                     devNum,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *regAddrPtr,
    OUT  GT_U32                    *startBitPtr,
    OUT  GT_U32                    *numBitsPtr
)
{
    GT_U32    unitNum;       /* number of CNC unit          */
    GT_U32    blockInUnit;   /* number of block in CNC unit */
    GT_U32    regAddr;       /* register address  */
    GT_U32    offset;        /* field Offset      */
    GT_U32    index;         /* register index    */
    GT_U32    numOfBits = 2; /* number of bits in the field */
    GT_U32    sip5_20Supported = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* the GM currently not supports this new format */
        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            sip5_20Supported = 0;
        }
        else
        {
            /* fixed in SIP6 ! (otherwise cause JIRA : CPSS-8209 : GM : Falcon : CNC counter on PCL0 counts twice) */

            /* must access to registers 'CNCBlocksCounterEntryModeRegister'
                and not to those of CNCBlockConfigReg0

                because it will set bits 30,31 --> clients 30,31 exists in Falcon !!!
            */
        }
    }

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (!sip5_20Supported)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                clientEnable.CNCBlockConfigReg0[blockInUnit][0/*BWC*/];

            offset = 30;
        }
        else
        {
            numOfBits = 3; /* 3 bits each field */
            index  = blockInUnit / 8;/* 8 fields in register */
            offset = (blockInUnit % 8) * numOfBits;

            regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                entryMode.CNCBlocksCounterEntryModeRegister[index];
        }
    }
    else
    {
        /* Not eArch devices */

        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockClientCfgAddrBase
            + (blockInUnit * 0x100);

        offset = 12;
    }

    *regAddrPtr = regAddr;
    *startBitPtr = offset;
    *numBitsPtr = numOfBits;

    return GT_OK;
}

/**
* @internal prvCpssDxChCncBlockClientToCfgAddr function
* @endinternal
*
* @brief   The function converts the client to HW base address of configuration register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] cfgRegPtr                - (pointer to) address of the block configuration register.
* @param[out] bitOffsetPtr             - (pointer to) bit offset of the block configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChCncBlockClientToCfgAddr
(
    IN   GT_U8                     devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *cfgRegPtr,
    OUT  GT_U32                    *bitOffsetPtr
)
{
    GT_U32      index = 0;  /* client HW index */
    GT_U32      unitNum;     /* number of CNC unit          */
    GT_U32      blockInUnit; /* number of block in CNC unit */

    PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:      index = 0; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:   index = 1; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E: index = 2; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:   index = 3; break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E: index = 4; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:  index = 5; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E: index = 6; break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:             index = 7; break;
        case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:       index = 8; break;
        case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:           index = 9; break;
        case CPSS_DXCH_CNC_CLIENT_TTI_E:                    index = 10; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2   */
        /* per block registers base */
        *cfgRegPtr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                cncRegs[0].blockClientCfgAddrBase
           + (CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS * (blockNum));

        /* client enable bit */
        *bitOffsetPtr = index;
    }
    else
    {
        if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
            devNum, blockNum, &unitNum, &blockInUnit))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* xCat3 */
        /* block + client register */
        *cfgRegPtr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockClientCfgAddrBase
        + (CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS * blockInUnit)
        + (4 * (index / 2));


        /* bits 12:0 for even, bits 26:13 for odd */
        *bitOffsetPtr = (index & 1) ? 13 : 0;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncClientCountModeRegGet function
* @endinternal
*
* @brief   The function calculates register address, bit offset and L2-mode
*         pattern for CNC client count mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
*
* @param[out] regAddrPtr               - (pointer to) register address.
* @param[out] bitOffPtr                - (pointer to) bit offset in register.
* @param[out] fldLenPtr                - (pointer to) field length in register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncClientCountModeRegGet
(
    IN   GT_U8                     devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT  GT_U32                    *regAddrPtr,
    OUT  GT_U32                    *bitOffPtr,
    OUT  GT_U32                    *fldLenPtr
)
{
    if (fldLenPtr != NULL)
    {
        *fldLenPtr = 1; /* default length -1 ,
                           for client CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E :
                              in E-ARCh (BC2 and higher) 2 bits */
    }
    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
            }
            else
            {
                *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
            }
            *bitOffPtr = 3;
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
                {
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                }
                else
                {
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
                }
                *bitOffPtr = 0;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                *bitOffPtr = 1;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
                {
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                }
                else
                {
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
                }
                *bitOffPtr = 1;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                *bitOffPtr = 2;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
                {
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                }
                else
                {
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
                }
                *bitOffPtr = 2;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                        preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
                *bitOffPtr = 13;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        bufferMng.eqBlkCfgRegs.preEgrEngineGlobal;
                *bitOffPtr = 14;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
                *bitOffPtr = 18;
                if (fldLenPtr != NULL)
                {                   /* bit 18 at /PREQ/Global Config   */
                    *fldLenPtr = 1; /*   field <Vlan Byte Count Mode > */
                }
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
                *bitOffPtr = 2;
                if (fldLenPtr != NULL)
                {                   /* bits 2-3 at TXQ_IP_queue/Peripheral Access/CNC Modes/CNC Modes */
                    *fldLenPtr = 2; /*   field <CNC Egress VLAN Client Pac ket Length Mode> */
                }
            }
            else
            {
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
                {
                    *regAddrPtr =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.cncCountMode;
                }
                else
                {
                    *regAddrPtr =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
                            peripheralAccess.cncModes.modesRegister;
                }
                *bitOffPtr = 2;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
                *bitOffPtr = 15;
                if (fldLenPtr != NULL)
                {                   /* bit 15 at /PREQ/Global Config              */
                    *fldLenPtr = 1; /*   field <Queue Pass Drop Byte Count mode > */
                }
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
                *bitOffPtr = 0;
                if (fldLenPtr != NULL)
                {                   /* bits 2-3 at TXQ_IP_queue/Peripheral Access/CNC Modes/CNC Modes */
                    *fldLenPtr = 2; /*   field <CNC Egress Queue Client Packet Length Mode> */
                }
            }
            else
            {
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
                {
                    *regAddrPtr =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.cncCountMode;
                }
                else
                {
                    *regAddrPtr =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
                            peripheralAccess.cncModes.modesRegister;
                }
                *bitOffPtr = 0;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;
                *bitOffPtr = 10;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.epclGlobalConfig;
                *bitOffPtr = 25;
            }
            break;
    case CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
                *bitOffPtr = 0;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
                *bitOffPtr = 0;
            }
            break;
    case CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                     PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
                *bitOffPtr = 1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
                *bitOffPtr = 1;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E:
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E:
        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no case break */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_TTI_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
                *bitOffPtr = 4;
            }
            else if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
                *bitOffPtr = 4;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
                *bitOffPtr = 4;
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
                *bitOffPtr = 2;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
                *bitOffPtr = 5;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                    preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
                *bitOffPtr = 13;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *regAddrPtr =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.cncEvlanConfigs.vlanPassDropConfigs;
            *bitOffPtr = 2;
            if (fldLenPtr != NULL)
            {                   /* bit 2 at vlan pass drop Config  */
                *fldLenPtr = 1; /*   field <vlan Byte Count mode > */
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *regAddrPtr =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.erepGlobalConfigs;
            *bitOffPtr = 1;
            if (fldLenPtr != NULL)
            {                   /* bit 1 at EREPGlobal Config              */
                *fldLenPtr = 1; /*   field <Egress Byte Count mode > */
            }
            break;
        /* L3/L2 mode not relevant for CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E client */
        case CPSS_DXCH_CNC_CLIENT_PHA_E:
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not supports the PHA unit");
            }

            *regAddrPtr =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PHA.pha_regs.generalRegs.PHACtrl;
            *bitOffPtr = 3;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCncMuxModeInfoGet function
* @endinternal
*
* @brief   The function calculates CNC client mux mode informations.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P;
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
*
* @param[out] checkMuxedClientPtr       - (pointer to) do mux mode logic
* @param[out] muxedClientPtr            - (pointer to) muxed client
* @param[out] muxRegAddrPtr             - (pointer to) mux register addres
* @param[out] muxBitMaskPtr             - (pointer to) bit of the client in the mux register
* @param[out] muxRegBitValue            - (pointer to) CNC mux mode value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssDxChCncMuxModeInfoGet
(
    IN   GT_U8                      devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT   client,
    OUT  GT_BOOL                    *checkMuxedClientPtr,
    OUT  CPSS_DXCH_CNC_CLIENT_ENT   *muxedClientPtr,
    OUT  GT_U32                     *muxRegAddrPtr,
    OUT  GT_U32                     *muxBitMaskPtr,
    OUT  GT_U32                     *muxRegBitValuePtr
)
{
    /* Validate device */
    if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *checkMuxedClientPtr = GT_TRUE;
    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:
            *muxedClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E;
            *muxBitMaskPtr = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ? BIT_31 : BIT_4;
            *muxRegBitValuePtr = 0;
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
            *muxedClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E;
            *muxBitMaskPtr = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ? BIT_31 : BIT_4;
            *muxRegBitValuePtr = *muxBitMaskPtr;
            break;

        case CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E:
            *muxedClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E;
            *muxBitMaskPtr = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ? BIT_30 : BIT_5;
            *muxRegBitValuePtr = 0;
            break;
        case CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:
            *muxedClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E;
            *muxBitMaskPtr = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ? BIT_30 : BIT_5;
            *muxRegBitValuePtr = *muxBitMaskPtr;
            break;
        default:
            *checkMuxedClientPtr = GT_FALSE;
    }

    if(*checkMuxedClientPtr)
    {
        /* write into the MUX register */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            *muxRegAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;
        }
        else
        {
            *muxRegAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on fail due muxed clients
*
* @note In AC5X there are two pairs of clients which are muxed.
*       If one of the clients is enabled in a block, enabling the other will fail.
*       1) CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E
*       2) CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E
*/
static GT_STATUS internal_cpssDxChCncBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
)
{
    return cpssDxChCncPortGroupBlockClientEnableSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, client, updateEnable);
}

/**
* @internal cpssDxChCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on fail due muxed clients
*
* @note In AC5X there are two pairs of clients which are muxed.
*       If one of the clients is enabled in a block, enabling the other will fail.
*       1) CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E
*       2) CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E
*/
GT_STATUS cpssDxChCncBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockClientEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, updateEnable));

    rc = internal_cpssDxChCncBlockClientEnableSet(devNum, blockNum, client, updateEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, updateEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    return cpssDxChCncPortGroupBlockClientEnableGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, client, updateEnablePtr);
}

/**
* @internal cpssDxChCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockClientEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, updateEnablePtr));

    rc = internal_cpssDxChCncBlockClientEnableGet(devNum, blockNum, client, updateEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, updateEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
* @param[in] indexRangesBmpPtr        - Pointer to
*                                        counter index ranges bitmap
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      *indexRangesBmpPtr
)
{
    return cpssDxChCncPortGroupBlockClientRangesSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, client, indexRangesBmpPtr);
}

/**
* @internal cpssDxChCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
*  @param[in] indexRangesBmpPtr        - Pointer to
*                                        counter index ranges bitmap
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      *indexRangesBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockClientRangesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, indexRangesBmpPtr));

    rc = internal_cpssDxChCncBlockClientRangesSet(devNum, blockNum, client, indexRangesBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, indexRangesBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    return cpssDxChCncPortGroupBlockClientRangesGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, client, indexRangesBmpPtr);
}

/**
* @internal cpssDxChCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockClientRangesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, client, indexRangesBmpPtr));

    rc = internal_cpssDxChCncBlockClientRangesGet(devNum, blockNum, client, indexRangesBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, client, indexRangesBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortClientEnableSet function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   enable
)
{
    GT_U32 regAddr;          /* register address  */
    GT_U32 fieldOffset;      /* field Offset      */
    GT_U32 value;            /* field value       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).L2L3IngrVLANCountingEnable[OFFSET_TO_WORD_MAC(portNum)];
        fieldOffset = OFFSET_TO_BIT_MAC(portNum);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncl2l3IngressVlanCountEnable;
        fieldOffset = (localPort != CPSS_CPU_PORT_NUM_CNS) ? localPort : 31;
    }

    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* no break; */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E: break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                return prvCpssDxChPortGroupWriteTableEntryField(
                    devNum,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_VLAN_COUNTING_ENABLE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    enable);
            }
            /* no break; */
            GT_ATTR_FALLTHROUGH;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = (enable == GT_FALSE) ? 0 : 1;

    return prvCpssHwPpPortGroupSetRegField(
        devNum, portGroupId ,regAddr, fieldOffset, 1/*fieldLength*/, value);
}

/**
* @internal cpssDxChCncPortClientEnableSet function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortClientEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, client, enable));

    rc = internal_cpssDxChCncPortClientEnableSet(devNum, portNum, client, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, client, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortClientEnableGet function
* @endinternal
*
* @brief   The function gets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] client                   - CNC client
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *enablePtr
)
{
    GT_STATUS rc;               /* return code       */
    GT_U32    regAddr;          /* register address  */
    GT_U32    fieldOffset;      /* field Offset      */
    GT_U32    value;            /* field value       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).L2L3IngrVLANCountingEnable[OFFSET_TO_WORD_MAC(portNum)];
        fieldOffset = OFFSET_TO_BIT_MAC(portNum);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncl2l3IngressVlanCountEnable;
        fieldOffset = (localPort != CPSS_CPU_PORT_NUM_CNS) ? localPort : 31;
    }

    switch (client)
    {
        case CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E:
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* no break; */
        GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E: break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChPortGroupReadTableEntryField(
                devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
                portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_VLAN_COUNTING_ENABLE_E, /* field name */
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                &value);

            if (rc != GT_OK)
            {
                return rc;
            }

            *enablePtr = (value == 0) ? GT_FALSE : GT_TRUE;
            return GT_OK;
        }
        GT_ATTR_FALLTHROUGH;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupGetRegField(
        devNum, portGroupId , regAddr, fieldOffset, 1/*fieldLength*/, &value);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChCncPortClientEnableGet function
* @endinternal
*
* @brief   The function gets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] client                   - CNC client
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortClientEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, client, enablePtr));

    rc = internal_cpssDxChCncPortClientEnableGet(devNum, portNum, client, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, client, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet function
* @endinternal
*
* @brief   The function enables or disables counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_U32 data;             /* data              */
    GT_U32 regAddr;          /* register address  */
    GT_U32 fieldOffset;      /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
        fieldOffset = 14;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.preEgrEngineGlobal;
        fieldOffset = 15;
    }

    data = (enable == GT_FALSE) ? 0 : 1;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, fieldOffset, 1/*fieldLength*/, data);
}

/**
* @internal cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet function
* @endinternal
*
* @brief   The function enables or disables counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet function
* @endinternal
*
* @brief   The function gets status of counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;               /* return code       */
    GT_U32    data;             /* data              */
    GT_U32    regAddr;          /* register address  */
    GT_U32 fieldOffset;         /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
        fieldOffset = 14;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.preEgrEngineGlobal;
        fieldOffset = 15;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, fieldOffset, 1/*fieldLength*/, &data);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet function
* @endinternal
*
* @brief   The function gets status of counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncVlanClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncVlanClientIndexModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT indexMode
)
{
    GT_U32 data;             /* data               */
    GT_U32 regAddr;          /* register address   */
    GT_U32 offset;           /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (vlanClient)
    {
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                    preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
            offset = 17;
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
                offset = 19;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
                offset = 10;
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))  /* BC3  -- field started at bit 11 */
                {
                    offset = 11;
                }
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.cncEvlanConfigs.vlanPassDropConfigs;
            offset = 0;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        && ((vlanClient == CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E) ||
              (vlanClient == CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E)))
    {
        switch (indexMode)
        {
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E:
                data = 2;
                break;
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E:
                data = 0;
                break;
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E:
                data = 1;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* suspicious to be wrong for egress VLAN client, in Cider as for SIP6 */
        switch (indexMode)
        {
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E:
                data = 0;
                break;
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E:
                data = 1;
                break;
            case CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E:
                data = 2;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, offset, 2/*fieldLength*/, data);
}

/**
* @internal cpssDxChCncVlanClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncVlanClientIndexModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT indexMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncVlanClientIndexModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanClient, indexMode));

    rc = internal_cpssDxChCncVlanClientIndexModeSet(devNum, vlanClient, indexMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanClient, indexMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncVlanClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - enexpected HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncVlanClientIndexModeGet
(
    IN   GT_U8                             devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    OUT  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT *indexModePtr
)
{
    GT_STATUS rc;               /* return code        */
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */
    GT_U32    offset;           /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(indexModePtr);

    switch (vlanClient)
    {
        case CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                    preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
            offset = 17;
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
                offset = 19;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
                offset = 10;
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))  /* BC3  -- field started at bit 11 */
                {
                    offset = 11;
                }
            }
            break;
        case CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.cncEvlanConfigs.vlanPassDropConfigs;
            offset = 0;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, offset, 2/*fieldLength*/, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        && ((vlanClient == CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E) ||
            (vlanClient == CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E)))
    {
        switch (data)
        {
            case 0:
                *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E;
                break;
            case 1:
                *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E;
                break;
            case 2:
                 *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* suspicious to be wrong for egress VLAN client, in Cider as for SIP6 */
        switch (data)
        {
            case 0:
                 *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E;
                break;
            case 1:
                *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E;
                break;
            case 2:
                *indexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncVlanClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - enexpected HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncVlanClientIndexModeGet
(
    IN   GT_U8                             devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    OUT  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT *indexModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncVlanClientIndexModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanClient, indexModePtr));

    rc = internal_cpssDxChCncVlanClientIndexModeGet(devNum, vlanClient, indexModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanClient, indexModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPacketTypePassDropToCpuModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] toCpuMode                - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPacketTypePassDropToCpuModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT toCpuMode
)
{
    GT_U32 data;             /* data              */
    GT_U32 regAddr;          /* register address  */
    GT_U32    fieldOffset,fieldLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
        fieldOffset = 13;
        fieldLength =  2;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
        fieldOffset = 16;
        fieldLength =  1;
    }

    switch (toCpuMode)
    {
        case CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E:
            data = 0;
            break;
        case CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E:
            data = 1;
            break;
        case CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_STREAM_GATE_MODE_E:
            if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "toCpuMode [%d] is not supported for device [%d] \n",toCpuMode,devNum);
            }
            data = 2;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "toCpuMode [%d] is not supported for device [%d] \n",toCpuMode,devNum);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, fieldOffset, fieldLength, data);
}

/**
* @internal cpssDxChCncPacketTypePassDropToCpuModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] toCpuMode                - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPacketTypePassDropToCpuModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT toCpuMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPacketTypePassDropToCpuModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, toCpuMode));

    rc = internal_cpssDxChCncPacketTypePassDropToCpuModeSet(devNum, toCpuMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, toCpuMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPacketTypePassDropToCpuModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] toCpuModePtr             - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - when register read return data not supported
*/
static GT_STATUS internal_cpssDxChCncPacketTypePassDropToCpuModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT *toCpuModePtr
)
{
    GT_STATUS rc;            /* return code       */
    GT_U32    data;          /* data              */
    GT_U32    regAddr;       /* register address  */
    GT_U32    fieldOffset,fieldLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(toCpuModePtr);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
        fieldOffset = 13;
        fieldLength =  2;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.preEgrEngineGlobalConfig;
        fieldOffset = 16;
        fieldLength =  1;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, fieldOffset, fieldLength, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set output parameter toCpuModePtr */
    if (data == 0)
    {
        *toCpuModePtr = CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E;
    }
    else if (data == 1)
    {
        *toCpuModePtr = CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E;
    }
    else if (data == 2)
    {
        *toCpuModePtr = CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_STREAM_GATE_MODE_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                      "toCpuMode value [%d] read from HW register is not supported for device [%d]\n"
                                      ,data,devNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncPacketTypePassDropToCpuModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] toCpuModePtr             - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - when register read return data not supported
*/
GT_STATUS cpssDxChCncPacketTypePassDropToCpuModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT *toCpuModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPacketTypePassDropToCpuModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, toCpuModePtr));

    rc = internal_cpssDxChCncPacketTypePassDropToCpuModeGet(devNum, toCpuModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, toCpuModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncTmClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncTmClientIndexModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT indexMode
)
{
    GT_U32 data;             /* data              */
    GT_U32 regAddr;          /* register address  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_DROP_MAC(devNum).globalConfigs;

    switch (indexMode)
    {
        case CPSS_DXCH_CNC_TM_INDEX_MODE_0_E:
            data = 0;
            break;
        case CPSS_DXCH_CNC_TM_INDEX_MODE_1_E:
            data = 1;
            break;
        case CPSS_DXCH_CNC_TM_INDEX_MODE_2_E:
            data = 2;
            break;
        case CPSS_DXCH_CNC_TM_INDEX_MODE_3_E:
            data = 3;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0/*fieldOffset*/, 2/*fieldLength*/, data);
}

/**
* @internal cpssDxChCncTmClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncTmClientIndexModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT indexMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncTmClientIndexModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indexMode));

    rc = internal_cpssDxChCncTmClientIndexModeSet(devNum, indexMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, indexMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncTmClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncTmClientIndexModeGet
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT *indexModePtr
)
{
    GT_STATUS rc;            /* return code       */
    GT_U32    data;          /* data              */
    GT_U32    regAddr;       /* register address  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(indexModePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TM_DROP_MAC(devNum).globalConfigs;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0/*fieldOffset*/, 2/*fieldLength*/, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (data)
    {
        case 0:
            *indexModePtr = CPSS_DXCH_CNC_TM_INDEX_MODE_0_E;
            break;
        case 1:
            *indexModePtr = CPSS_DXCH_CNC_TM_INDEX_MODE_1_E;
            break;
        case 2:
            *indexModePtr = CPSS_DXCH_CNC_TM_INDEX_MODE_2_E;
            break;
        case 3:
            *indexModePtr = CPSS_DXCH_CNC_TM_INDEX_MODE_3_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncTmClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncTmClientIndexModeGet
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT *indexModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncTmClientIndexModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indexModePtr));

    rc = internal_cpssDxChCncTmClientIndexModeGet(devNum, indexModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, indexModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] countMode                - count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncClientByteCountModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT countMode
)
{
    GT_STATUS rc;               /* return code        */
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */
    GT_U32    bitOff;           /* offset of mode bit */
    GT_U32    fldLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    rc = prvCpssDxChCncClientCountModeRegGet(
        devNum, client, &regAddr, &bitOff,&fldLength);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (countMode)
    {
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E: data = 0; break;
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E: data = 1; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, bitOff, fldLength, data);
}

/**
* @internal cpssDxChCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] countMode                - count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncClientByteCountModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT countMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncClientByteCountModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, client, countMode));

    rc = internal_cpssDxChCncClientByteCountModeSet(devNum, client, countMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, client, countMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
*
* @param[out] countModePtr             - (pointer to) count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncClientByteCountModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT *countModePtr
)
{
    GT_STATUS rc;               /* return code        */
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */
    GT_U32    bitOff;           /* offset of mode bit */
    GT_U32    fldLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(countModePtr);

    rc = prvCpssDxChCncClientCountModeRegGet(
        devNum, client, /*OUT*/&regAddr, &bitOff,&fldLength);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, bitOff, fldLength, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *countModePtr = (data == 0)
        ? CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E
        : CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E;

    return GT_OK;
}

/**
* @internal cpssDxChCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
*
* @param[out] countModePtr             - (pointer to) count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncClientByteCountModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT *countModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncClientByteCountModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, client, countModePtr));

    rc = internal_cpssDxChCncClientByteCountModeGet(devNum, client, countModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, client, countModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncEgressVlanDropCountModeSet function
* @endinternal
*
* @brief   The function sets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncEgressVlanDropCountModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT mode
)
{
    GT_STATUS rc;               /* return code        */
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* don't need offset & length, just register address */
    rc = prvCpssDxChCncClientCountModeRegGet(
           devNum, CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E, /*OUT*/&regAddr, &data,(GT_U32*)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (mode)
    {
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E:
            data = 0;
            break;
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E:
            data = 1;
            break;
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E:
            data = 2;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 4/*fieldOffset*/, 2/*fieldLength*/, data);
}

/**
* @internal cpssDxChCncEgressVlanDropCountModeSet function
* @endinternal
*
* @brief   The function sets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressVlanDropCountModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncEgressVlanDropCountModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChCncEgressVlanDropCountModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncEgressVlanDropCountModeGet function
* @endinternal
*
* @brief   The function gets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on reserved value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncEgressVlanDropCountModeGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT *modePtr
)
{
    GT_STATUS rc;               /* return code        */
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* don't need offset & length, just register address */
    rc = prvCpssDxChCncClientCountModeRegGet(
           devNum, CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E, /*OUT*/&regAddr, &data,(GT_U32*)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 4/*fieldOffset*/, 2/*fieldLength*/, &data);

    switch (data)
    {
        case 0:
            *modePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E;
            break;
        case 1:
            *modePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E;
            break;
        case 2:
            *modePtr =CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncEgressVlanDropCountModeGet function
* @endinternal
*
* @brief   The function gets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on reserved value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressVlanDropCountModeGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncEgressVlanDropCountModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChCncEgressVlanDropCountModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterClearByReadEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */
    GT_U32    i;                /* loop index         */
    GT_STATUS rc;               /* return code        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    data = (enable == GT_FALSE) ? 0 : 1;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2; Caelum; Bobcat3 */
        for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, i).globalRegs.CNCGlobalConfigReg;
            rc = prvCpssHwPpSetRegField(
                devNum, regAddr, 1/*fieldOffset*/, 1/*fieldLength*/, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Not eArch devices */
    for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[i].globalCfg;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 1/*fieldOffset*/, 1/*fieldLength*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterClearByReadEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCncCounterClearByReadEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterClearByReadEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;               /* return code       */
    GT_U32    regAddr;          /* register address  */
    GT_U32    data;             /* field value       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, 0).globalRegs.CNCGlobalConfigReg;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[0].globalCfg;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 1/*fieldOffset*/, 1/*fieldLength*/, &data);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterClearByReadEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCncCounterClearByReadEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterClearByReadValueSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_U32    regAddr;          /* register address           */
    GT_U32    i;                /* loop index         */
    GT_STATUS rc;               /* return code        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2; Caelum; Bobcat3 */
        for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, i).globalRegs.CNCClearByReadValueRegWord0;
            rc = prvCpssDxChCncPortGroupCounterValueAverageSet(
                devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                regAddr, format, counterPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Not eArch devices */
    for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[i].clearByReadValueWord0;

        rc = prvCpssDxChCncPortGroupCounterValueAverageSet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            regAddr, format, counterPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadValueSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterClearByReadValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, format, counterPtr));

    rc = internal_cpssDxChCncCounterClearByReadValueSet(devNum, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterClearByReadValueGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_U32    regAddr;          /* register address  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);



    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, 0).globalRegs.CNCClearByReadValueRegWord0;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[0].clearByReadValueWord0;
    }

    return prvCpssDxChCncPortGroupCounterValueSummingGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        regAddr, format, counterPtr);
}

/**
* @internal cpssDxChCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadValueGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterClearByReadValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, format, counterPtr));

    rc = internal_cpssDxChCncCounterClearByReadValueGet(devNum, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterWraparoundEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_U32    data;             /* data               */
    GT_U32    regAddr;          /* register address   */
    GT_U32    i;                /* loop index         */
    GT_STATUS rc;               /* return code        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);



    data = (enable == GT_FALSE) ? 0 : 1;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2; Caelum; Bobcat3 */
        for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, i).globalRegs.CNCGlobalConfigReg;
            rc = prvCpssHwPpSetRegField(
                devNum, regAddr, 0/*fieldOffset*/, 1/*fieldLength*/, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Not eArch devices */
    for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[i].globalCfg;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 0/*fieldOffset*/, 1/*fieldLength*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterWraparoundEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterWraparoundEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCncCounterWraparoundEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterWraparoundEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;               /* return code       */
    GT_U32    regAddr;          /* register address  */
    GT_U32    data;             /* field value       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, 0).globalRegs.CNCGlobalConfigReg;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[0].globalCfg;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0/*fieldOffset*/, 1/*fieldLength*/, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterWraparoundEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterWraparoundEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCncCounterWraparoundEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around
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
static GT_STATUS internal_cpssDxChCncCounterWraparoundIndexesGet
(
    IN    GT_U8    devNum,
    IN    GT_U32   blockNum,
    INOUT GT_U32   *indexNumPtr,
    OUT   GT_U32   indexesArr[]
)
{
    return cpssDxChCncPortGroupCounterWraparoundIndexesGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, indexNumPtr,
        NULL /*portGroupIdArr*/, indexesArr);
}

/**
* @internal cpssDxChCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around
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
GT_STATUS cpssDxChCncCounterWraparoundIndexesGet
(
    IN    GT_U8    devNum,
    IN    GT_U32   blockNum,
    INOUT GT_U32   *indexNumPtr,
    OUT   GT_U32   indexesArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterWraparoundIndexesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, indexNumPtr, indexesArr));

    rc = internal_cpssDxChCncCounterWraparoundIndexesGet(devNum, blockNum, indexNumPtr, indexesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, indexNumPtr, indexesArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    return cpssDxChCncPortGroupCounterSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, index, format, counterPtr);
}

/**
* @internal cpssDxChCncCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, index, format, counterPtr));

    rc = internal_cpssDxChCncCounterSet(devNum, blockNum, index, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, index, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    return cpssDxChCncPortGroupCounterGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, index, format, counterPtr);
}

/**
* @internal cpssDxChCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, index, format, counterPtr));

    rc = internal_cpssDxChCncCounterGet(devNum, blockNum, index, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, index, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         Note: Bobcat3 has CNC unit per port group.
*         Triggering upload from both CNC ports groups in parallel is not supported.
*         Application must choose a single Port-Group at a time.
*         (need to use cpssDxChCncPortGroupBlockUploadTrigger(...))
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
static GT_STATUS internal_cpssDxChCncBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E);

    return cpssDxChCncPortGroupBlockUploadTrigger(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, blockNum);
}

/**
* @internal cpssDxChCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         Note: Bobcat3 has CNC unit per port group.
*         Triggering upload from both CNC ports groups in parallel is not supported.
*         Application must choose a single Port-Group at a time.
*         (need to use cpssDxChCncPortGroupBlockUploadTrigger(...))
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockUploadTrigger);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum));

    rc = internal_cpssDxChCncBlockUploadTrigger(devNum, blockNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
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
static GT_STATUS internal_cpssDxChCncBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *inProcessBlocksBmpPtr
)
{
    return cpssDxChCncPortGroupBlockUploadInProcessGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, inProcessBlocksBmpPtr);
}

/**
* @internal cpssDxChCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *inProcessBlocksBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncBlockUploadInProcessGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, inProcessBlocksBmpPtr));

    rc = internal_cpssDxChCncBlockUploadInProcessGet(devNum, inProcessBlocksBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, inProcessBlocksBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChSharedCncDescCtrlDbManagerDump function
* @endinternal
*
* @brief   debug function that prints the internal info that relates to CNC shared port
*         groups management.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSharedCncDescCtrlDbManagerDump
(
    IN  GT_U8                       devNum
)
{
    GT_U32                      portGroupId; /*the port group*/
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *current_cncPortGroupInfoPtr;/* current - CNC port group info */
    GT_PORT_GROUPS_BMP          mgUnit_portGroupsBmp = 0;/* bmp of shared mg units */
    GT_BOOL                     isNewMgUnit;
    GT_U32 mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    PRV_CPSS_AU_DESC_CTRL_STC   *fuqDescCtrlPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\n Start Dump: \n");

    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_FALSE)
    {
        cpssOsPrintf("The device not support this DB (use_sharedCncDescCtrl == GT_FALSE) \n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "use_sharedCncDescCtrl == GT_FALSE");
    }

    /* we need to check how many bits from start are zero already */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        cpssOsPrintf("PortGroup[%d]: start info \n",portGroupId);
        current_cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId);

        mgUnit_portGroupId = current_cncPortGroupInfoPtr->mgPortGroupId;

        PRINT_d_MAC(current_cncPortGroupInfoPtr->mgPortGroupId);
        PRINT_d_MAC(current_cncPortGroupInfoPtr->currCncDescIdxInTrigger);
        PRINT_d_MAC(current_cncPortGroupInfoPtr->savedCncDescEntriesBeforeRewind);

        cpssOsPrintf("current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr: (bitmap)");
        printBitmapArr(
            "current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr",
            current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr,
            current_cncPortGroupInfoPtr->numBitsInBmp,
            0xFFFFFFFF/* ignore the value */
        );

        PRINT_d_MAC(current_cncPortGroupInfoPtr->numBitsInBmp);
        cpssOsPrintf("current_cncPortGroupInfoPtr->otherSharedPortGroupsOnMyMgUnit: ");
        printBitmapArr(
            "current_cncPortGroupInfoPtr->otherSharedPortGroupsOnMyMgUnit (bitmap)",
            &current_cncPortGroupInfoPtr->otherSharedPortGroupsOnMyMgUnit,
            32,
            0xFFFFFFFF/* ignore the value */
        );

        isNewMgUnit = (mgUnit_portGroupsBmp & (1<<portGroupId)) ? GT_FALSE : GT_TRUE;

        mgUnit_portGroupsBmp |=
                        current_cncPortGroupInfoPtr->otherSharedPortGroupsOnMyMgUnit |
                        (1<<portGroupId);

        cpssOsPrintf("PortGroup : ended info \n");

        if(isNewMgUnit == GT_TRUE)
        {
            cpssOsPrintf("perMgUnit [%d] start info: \n",mgUnit_portGroupId);

            PRINT_d_MAC(current_cncPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads);
            PRINT_d_MAC(current_cncPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads);
            PRINT_b_MAC(current_cncPortGroupInfoPtr->perMgUnit.endOfFuqReached);
            PRINT_d_MAC(current_cncPortGroupInfoPtr->perMgUnit.cncPortGroupId_endOfFuqReached);

            fuqDescCtrlPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId]);

            PRINT_p_MAC(fuqDescCtrlPtr->blockAddr);
            PRINT_d_MAC(fuqDescCtrlPtr->blockSize);
            PRINT_d_MAC(fuqDescCtrlPtr->currDescIdx);
            PRINT_d_MAC(fuqDescCtrlPtr->unreadCncCounters);

            cpssOsPrintf("perMgUnit ended info: \n");
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    cpssOsPrintf("End Dump. \n");
    return GT_OK;
}


/**
* @internal sharedCncDescCtrlCollapseCheck function
* @endinternal
*
* @brief   Check if shared CNC port groups manager can collapse next:
*         isMyCncUploadTriggeredBmpArr[] (on all CNC shared port groups)
*         perMgUnit.triggeredNumOfCncUploads
*         perMgUnit.servedNumOfCncUploads
*         and :
*         fuqDescCtrlPtr->currDescIdx
*         The logic check that the lowest triggered blocked were fully read.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group
* @param[in] fuqDescCtrlPtr           - pointer to the FUQ descriptor
*
* @param[out] useFuqDescCtrlIndexPtr   - (pointer to) indication to use the fuqDescCtrlPtr->currDescIdx
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sharedCncDescCtrlCollapseCheck
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  PRV_CPSS_AU_DESC_CTRL_STC   *fuqDescCtrlPtr,
    OUT GT_BOOL                   *useFuqDescCtrlIndexPtr
)
{
    GT_PORT_GROUPS_BMP          portGroupsBmp; /* check bitmap of port groups */
    GT_U32 mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *mgPortGroupInfoPtr;/* MG port group info */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *cncPortGroupInfoPtr;/* CNC port group info */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *current_cncPortGroupInfoPtr;/* current - CNC port group info */
    GT_U32      temp_portGroupId;
    GT_U32      ii;
    GT_U32      currentFirstUsedBit , globalFirstUsedBit = 0xffffffff;
    GT_U32      offsetDescIdx;
    GT_U32      saved_currCncDescIdxInTrigger;

    *useFuqDescCtrlIndexPtr = GT_FALSE;

    cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId);

    mgUnit_portGroupId = cncPortGroupInfoPtr->mgPortGroupId;

    mgPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,mgUnit_portGroupId);

    /* save the value */
    saved_currCncDescIdxInTrigger = cncPortGroupInfoPtr->currCncDescIdxInTrigger;

    /* we are done reading from this block in the FUQ */
    cncPortGroupInfoPtr->currCncDescIdxInTrigger = 0;
    /* we can also reset the 'history' from before the rewind */
    cncPortGroupInfoPtr->savedCncDescEntriesBeforeRewind = 0;

    portGroupsBmp = cncPortGroupInfoPtr->otherSharedPortGroupsOnMyMgUnit;
    /* add 'my port group' */
    portGroupsBmp |= (1 << portGroupId);

    /* we need to check how many bits from start are zero already */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,temp_portGroupId)
    {
        current_cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,temp_portGroupId);

        /* we need to check how many bits from start are zero already */
        for(ii = 0; ii < current_cncPortGroupInfoPtr->numBitsInBmp; ii += 32)
        {
            if(0 == current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr[ii>>5])
            {
                continue;
            }
            currentFirstUsedBit = ii + prvCpssPortGroupsBitmapFirstActiveBitGet(current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr[ii>>5]);

            if(currentFirstUsedBit < globalFirstUsedBit)
            {
                /* we have new global lowest bit */
                globalFirstUsedBit = currentFirstUsedBit;
            }

            /* we are done with this port group */
            break;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,temp_portGroupId)

    if(globalFirstUsedBit == 0)
    {
        /* we can not collapse due to one of the port groups */
        return GT_OK;
    }

    if(globalFirstUsedBit == 0xffffffff)
    {
        /* all the port groups are without any waiting trigger */
        /* both DB counters should be 1 at this point*/
        if(mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads != 1 ||
           mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads != 1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "DB management ERROR: triggeredNumOfCncUploads[%d] and servedNumOfCncUploads[%d] should be value 1",
                mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads,
                mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads);
        }

        /* treat it as <globalFirstUsedBit> = 1 */
        globalFirstUsedBit = 1;
    }
    else
    if(globalFirstUsedBit > mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads ||
       globalFirstUsedBit > mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "DB management ERROR: globalFirstUsedBit[%d] > triggeredNumOfCncUploads[%d] or > servedNumOfCncUploads[%d]",
            globalFirstUsedBit,
            mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads,
            mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads);
    }

    /* collapse the DB management counters */
    mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads -= globalFirstUsedBit;
    mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads    -= globalFirstUsedBit;

    /* we need to update the FUQ 'Current descriptor' with the blocks that we 'jumped' */
    /* 4 CNC counters in FUQ message */
    offsetDescIdx = (globalFirstUsedBit - 1) * /* (-1) because for 'first block' using saved_currCncDescIdxInTrigger */
        (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries / 4);
    /* add the first block */
    offsetDescIdx += saved_currCncDescIdxInTrigger;

    fuqDescCtrlPtr->currDescIdx += offsetDescIdx;

    /* check for rewind */
    if(fuqDescCtrlPtr->currDescIdx > fuqDescCtrlPtr->blockSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "DB management ERROR: (currDescIdx)[%d] > blockSize[%d]",
            fuqDescCtrlPtr->currDescIdx,
            fuqDescCtrlPtr->blockSize);
    }

    /* the caller need to use the 'fuqDescCtrlPtr->currDescIdx' for index calculations */
    *useFuqDescCtrlIndexPtr = GT_TRUE;

    /* collapse the DB management bitmaps */
    /* we need to check how many bits from start are zero already */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,temp_portGroupId)
    {
        current_cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,temp_portGroupId);

        copyBitsInMemory(current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr,
            0, /*targetStartBit*/
            globalFirstUsedBit,/*sourceStartBit*/
            (current_cncPortGroupInfoPtr->numBitsInBmp - globalFirstUsedBit));/*numBits*/

        /* reset the bits at the end of the isMyCncUploadTriggeredBmpArr[] */
        resetBitsInMemory(current_cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr,
            current_cncPortGroupInfoPtr->numBitsInBmp - globalFirstUsedBit,/*startBit*/
            globalFirstUsedBit);/*numBits*/
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,temp_portGroupId)

    return GT_OK;
}

/**
* @internal sharedCncDescCtrlRewindCheck function
* @endinternal
*
* @brief   Check if shared CNC port groups FUQ needs to do 'rewind'.
*         by setting : fuqDescCtrlPtr->currDescIdx = 0
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group
* @param[in] fuqDescCtrlPtr           - (pointer to) the FUQ descriptor
*
* @param[out] needRewindPtr            - (pointer to) indication that the FUQ should be rewind
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sharedCncDescCtrlRewindCheck
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  PRV_CPSS_AU_DESC_CTRL_STC   *fuqDescCtrlPtr,
    IN  GT_BOOL                     *needRewindPtr
)
{
    GT_U32 mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *mgPortGroupInfoPtr;/* MG port group info */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *cncPortGroupInfoPtr;/* CNC port group info */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *endOfFuqReached_cncPortGroupInfoPtr;/* CNC port group info */
    GT_U32 cncPortGroupId_endOfFuqReached;/*the portGroupId that did 'endOfFuqReached = GT_TRUE'*/

    cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId);

    mgUnit_portGroupId = cncPortGroupInfoPtr->mgPortGroupId;

    mgPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,mgUnit_portGroupId);

    *needRewindPtr = GT_FALSE;

    if(mgPortGroupInfoPtr->perMgUnit.endOfFuqReached == GT_FALSE)
    {
        /* no port group reached end of FUQ */
        return GT_OK;
    }

    if((mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads -
        mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads) > 1)
    {
        /* we can rewind only when this is the last trigger to serve */
        /* or when we finished serve all (and last CNC index is also the 'end of FUQ') */
        return GT_OK;
    }

    cncPortGroupId_endOfFuqReached = mgPortGroupInfoPtr->perMgUnit.cncPortGroupId_endOfFuqReached;
    endOfFuqReached_cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,cncPortGroupId_endOfFuqReached);

    *needRewindPtr = GT_TRUE;

    /* we now can reset the currDescIdx */
    fuqDescCtrlPtr->currDescIdx = 0;

    /* reset the FUQ reached indication */
    mgPortGroupInfoPtr->perMgUnit.endOfFuqReached = GT_FALSE;

    /* save the number of indexes that we done reading , and reset the number for next time */
    endOfFuqReached_cncPortGroupInfoPtr->savedCncDescEntriesBeforeRewind =
        endOfFuqReached_cncPortGroupInfoPtr->currCncDescIdxInTrigger;
    endOfFuqReached_cncPortGroupInfoPtr->currCncDescIdxInTrigger = 0;

    return GT_OK;
}

/**
* @internal sharedCncDescCtrlUpdateAndAddrGet function
* @endinternal
*
* @brief   1. Get shared CNC port groups address in the FUQ.
*         2. update internal DB that manage the current place in the FUQ 'per port group'
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group
* @param[in] fuqDescCtrlPtr           - pointer to the FUQ descriptor
* @param[in] auMessageNumBytes        number of bytes in AU/FU message
*
* @param[out] auMemPtr_Ptr             - (pointer to) pointer to the current descriptor
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - DB management error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sharedCncDescCtrlUpdateAndAddrGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  PRV_CPSS_AU_DESC_CTRL_STC   *fuqDescCtrlPtr,
    IN  GT_U32                      auMessageNumBytes,
    OUT GT_U32_PTR                  *auMemPtr_Ptr,
    IN GT_BOOL                      getNext
)
{
    GT_STATUS   rc;
    GT_U32 * auMemPtr;/* pointer to the current descriptor */
    GT_U32 mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    GT_U32  globalDescIdx; /* global index into FUQ (in units of FUQ messages) */
    GT_U32  relativeCncIndex;/* relative CNC index into FUQ (in units of FUQ messages) from 'fuqDescCtrlPtr->currDescIdx' */
    GT_U32  savedCurrDescIdx = fuqDescCtrlPtr->currDescIdx;/* saved value from fuqDescCtrlPtr */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *mgPortGroupInfoPtr;/* MG port group info */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *cncPortGroupInfoPtr;/* CNC port group info */
    GT_U32  triggerId;/*trigger Id */
    GT_U32  ii;/* iterator*/
    GT_BOOL useFuqDescCtrlIndex = GT_FALSE;

    cncPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId);

    mgUnit_portGroupId = cncPortGroupInfoPtr->mgPortGroupId;

    mgPortGroupInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,mgUnit_portGroupId);

    if(mgPortGroupInfoPtr->perMgUnit.endOfFuqReached == GT_TRUE &&
       mgPortGroupInfoPtr->perMgUnit.cncPortGroupId_endOfFuqReached == portGroupId)
    {
        /* this port group reach the end of the FUQ and can not give more counters */

        return /* it's not error for log */ GT_NO_MORE;
    }

    triggerId = 0;/* just to avoid compilation warning */

    for(ii = 0; ii < cncPortGroupInfoPtr->numBitsInBmp; ii += 32)
    {
        if(0 == cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr[ii>>5])
        {
            continue;
        }

        triggerId = ii + prvCpssPortGroupsBitmapFirstActiveBitGet(cncPortGroupInfoPtr->isMyCncUploadTriggeredBmpArr[ii>>5]);

        break;
    }

    if(ii == cncPortGroupInfoPtr->numBitsInBmp)
    {
        return /* it's not error for log */ GT_NO_MORE;
    }

    if(ii > cncPortGroupInfoPtr->numBitsInBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB management ERROR: loop index[%d] over the range[%d]",
            ii,cncPortGroupInfoPtr->numBitsInBmp);
    }

    /* 2 CNC counters in line */
    /* 4 CNC counters in FUQ message */
    relativeCncIndex = triggerId * (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries / 4) +
                       cncPortGroupInfoPtr->currCncDescIdxInTrigger;

    if(getNext == GT_TRUE)
    {
        relativeCncIndex++;
        cncPortGroupInfoPtr->currCncDescIdxInTrigger++; /* update for next time */

        /* check if we gave the last index in the block */
        /* 4 CNC counters in FUQ message */
        if((cncPortGroupInfoPtr->currCncDescIdxInTrigger +
            cncPortGroupInfoPtr->savedCncDescEntriesBeforeRewind) ==
           (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries / 4))
        {
           /* update the MG port group that this block is done */
           mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads++;

           if(mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads >
              mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads)
           {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                   "DB management ERROR: servedNumOfCncUploads[%d] > triggeredNumOfCncUploads[%d]",
                   mgPortGroupInfoPtr->perMgUnit.servedNumOfCncUploads,
                   mgPortGroupInfoPtr->perMgUnit.triggeredNumOfCncUploads);
           }

           /* clear the triggerId in isMyCncUploadTriggeredBmpArr[] */
           PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_SET_MAC(devNum,portGroupId,
               triggerId, 0/*set to 0*/);


           /* after every 'reset' of bit in the isMyCncUploadTriggeredBmpArr[] ,
               we need to Check if can collapse info */
           rc = sharedCncDescCtrlCollapseCheck(devNum,portGroupId,fuqDescCtrlPtr,&useFuqDescCtrlIndex);
           if(rc != GT_OK)
           {
               return rc;
           }
        }
    }

    if(useFuqDescCtrlIndex == GT_FALSE)
    {
        globalDescIdx =
                savedCurrDescIdx + /* offset in FUQ since last time FUQ was empty from CNC counters */
                relativeCncIndex;/* each CNC counter is 8 bytes*/
    }
    else
    {
        /* was updated by sharedCncDescCtrlCollapseCheck(...) */
        globalDescIdx = fuqDescCtrlPtr->currDescIdx;
    }

    if(globalDescIdx > fuqDescCtrlPtr->blockSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB management ERROR: globalDescIdx[%d] > blockSize[%d]",
            globalDescIdx ,
            fuqDescCtrlPtr->blockSize);
    }

    if (globalDescIdx == fuqDescCtrlPtr->blockSize)
    {
        if (fuqDescCtrlPtr->currDescIdx == fuqDescCtrlPtr->blockSize)
        {
            /* this reset should not really impact 'good' behavior , logic should be ok without it.
               as it should come from sharedCncDescCtrlRewindCheck(...)
              But it is done anyway. */
            fuqDescCtrlPtr->currDescIdx = 0;
        }

        /* we are done with this 'port group' , as it reached 'end of FUQ'
           but we may still have other port groups that hold counters in the FUQ

           so we can't 'rewind' just yet !
        */
        if(GT_TRUE == mgPortGroupInfoPtr->perMgUnit.endOfFuqReached)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB management ERROR: endOfFuqReached already set");
        }
        /* indication of 'END of FUQ' */
        mgPortGroupInfoPtr->perMgUnit.endOfFuqReached = GT_TRUE;
        mgPortGroupInfoPtr->perMgUnit.cncPortGroupId_endOfFuqReached = portGroupId;

        /* this port group reach the end of the FUQ and can not give more counters */
        return /* it's not error for log */ GT_NO_MORE;
    }

    auMemPtr = (GT_U32 *)(fuqDescCtrlPtr->blockAddr + (auMessageNumBytes * globalDescIdx));

    *auMemPtr_Ptr = auMemPtr;

    return GT_OK;
}

/**
* @internal cncUploadedBlockGet_perPortGroup_perCncUnit function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupId              - the port group Id. to support multi-port-groups device.
* @param[in] initOutCounters          - GT_TRUE - override the memory contents
*                                      - GT_FALSE - add to memory contents
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
static GT_STATUS cncUploadedBlockGet_perPortGroup_perCncUnit
(
    IN     GT_U8                             devNum,
    IN     GT_U32                            portGroupId,
    IN     PRV_CPSS_AU_DESC_CTRL_STC        *descCtrlPtr,
    IN     PRV_CPSS_DXCH_UNIT_ENT            cncUnit,
    IN     GT_BOOL                           initOutCounters,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
)
{
    GT_STATUS                   rc;            /* return code                   */
    PRV_CPSS_AU_DESC_STC        *descPtr;    /* pointer to the current descriptor */
    GT_U32                     *auMemPtr;/* pointer to the SDMA memory , to start of current message */

    GT_U32                      ii;            /* iterator                      */
    GT_U32                      cncMaxCnt;     /* number of CNC messages        */
    GT_U32                      word[2];       /* work data                     */
    CPSS_DXCH_CNC_COUNTER_STC   cuppPortGroupCounter;/* current counter value for multi-port-groups support */
    GT_U32                      auMessageNumWords;  /* number of words in AU/FU message */
    GT_U32                      auMessageNumBytes;  /* number of bytes in AU/FU message */
    GT_U32                      firstCncWordIdx;    /* AU memory index of current CNC counter's first word */
    GT_PORT_GROUPS_BMP          portGroupsBmp; /* check bitmap of port groups */
    GT_PORT_GROUPS_BMP          fullPortGroupsBmp; /* full queue bitmap of port groups */
    GT_U32                      inProcessBlocksBmp[2]; /* bitmap of blocks being uploaded */
    GT_BOOL                     needRewind;/* indication that we need rewind of the FUQ */
    GT_U32                     *overTheTopAddrPtr;/* pointer to the 'just after' of the SDMA memory */
    GT_STATUS                   rc_nextDesc = GT_OK;/* return code for getting next descriptor index */


    /* check that the FUQ initialized */
    PRV_CPSS_FUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum);

    auMessageNumWords = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;
    auMessageNumBytes = 4 * auMessageNumWords;

    cncMaxCnt = *numOfCounterValuesPtr;

    if(cncMaxCnt == 0)
    {
        return GT_OK;
    }

    *numOfCounterValuesPtr = 0;

    portGroupsBmp =
        PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum)
        ? (GT_U32)(1 << portGroupId) : CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    /* must call the API with portGroupsBmp that is 'application' bitmap and not 'swapped' or manipulated BMP
       as it will not work ok for Falcon ... so need to use new internal function :
        prvCpssDxChBrgFdbPortGroupQueueFullGet(...) (instead of cpssDxChBrgFdbPortGroupQueueFullGet(...)) */
    rc = prvCpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,
        CPSS_DXCH_FDB_QUEUE_TYPE_FU_E,
        cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ?
            PRV_CPSS_DXCH_MG_CLIENT_FUQ_FOR_CNC01_E:
            PRV_CPSS_DXCH_MG_CLIENT_FUQ_FOR_CNC23_E,
        &fullPortGroupsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((fullPortGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS) ||
        (portGroupsBmp & fullPortGroupsBmp) != 0)
    {
        /* upload paused due to queue is full                  */
        /* queue contains ready portion of counters to retrieve */
    }
    else
    {
        inProcessBlocksBmp[0] = 0;
        inProcessBlocksBmp[1] = 0;
        /* queue is not full. Check if upload finished */
        rc = cpssDxChCncPortGroupBlockUploadInProcessGet(
            devNum, portGroupsBmp, inProcessBlocksBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((inProcessBlocksBmp[0] != 0 && cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E) ||
            (inProcessBlocksBmp[1] != 0 && cncUnit == PRV_CPSS_DXCH_UNIT_CNC_2_E))
        {
            #ifdef CPSS_LOG_ENABLE
            GT_U32  failedBmp = cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ? inProcessBlocksBmp[0] : inProcessBlocksBmp[1];
            GT_CHAR*  failedBmpStr = cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ? "31..0" : "63..32";
            /* block upload yet not finished and not paused */
            /* the queue contents yet not valid             */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY,
                "the device (portGroupId[%d]) still processing a block bitmap [blocks %s] [0x%8.8x]",
                portGroupId,
                failedBmpStr,
                failedBmp);
            #else /*!CPSS_LOG_ENABLE*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY,LOG_ERROR_NO_MSG);
            #endif/*!CPSS_LOG_ENABLE*/
        }
    }

    overTheTopAddrPtr = (GT_U32 *)(descCtrlPtr->blockAddr + (auMessageNumBytes * descCtrlPtr->blockSize));

    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
    {
        /* get first <auMemPtr> and update the 'DB management' of shared port groups CNC */
        rc = sharedCncDescCtrlUpdateAndAddrGet(devNum, portGroupId , descCtrlPtr ,
            auMessageNumBytes ,&auMemPtr, GT_FALSE);

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* the pointer to start of 'next message to handle'  */
        auMemPtr = (GT_U32 *)(descCtrlPtr->blockAddr + (auMessageNumBytes * descCtrlPtr->currDescIdx));
    }

    /* read number of AU descriptors */
    for (ii = 0; (ii < cncMaxCnt); ii++)
    {
        if (descCtrlPtr->unreadCncCounters == 0)
        {
            *numOfCounterValuesPtr = ii;
            return /* it's not error for log */ GT_NO_MORE;
        }

        if(auMessageNumWords == 8)
        {
            /* The 8 word AU message includes 4 CNC counters.
              Each CNC counter use two 32 bit words.
              The descCtrlPtr->unreadCncCounters is count down variable
              that starts from number of CNC counters in a block (2048, 512 or 256).
              First uploaded counter has descCtrlPtr->unreadCncCounters % 4 = 0.
              Second one has descCtrlPtr->unreadCncCounters % 4 = 3.
              Third one has descCtrlPtr->unreadCncCounters % 4 = 2.
              Fourth one has descCtrlPtr->unreadCncCounters % 4 = 1.
              Use simple equation to get index of current counter in AU message:
               - index = (4 - (descCtrlPtr->unreadCncCounters % 4)) % 4  */
            firstCncWordIdx = ((4 - (descCtrlPtr->unreadCncCounters & 3)) & 3) * 2;
        }
        else
        {
            /* The 4 word AU message includes 2 CNC counters.
              Each CNC counter use two 32 bit words.
              First uploaded counter has even value of descCtrlPtr->unreadCncCounters.
              Second one has odd value of descCtrlPtr->unreadCncCounters. */
            firstCncWordIdx = (descCtrlPtr->unreadCncCounters & 1) * 2;
        }

        if (initOutCounters != GT_FALSE)
        {
            /* start with clean counter value */
            cpssOsMemSet(
                &(counterValuesPtr[ii]), 0,
                sizeof(CPSS_DXCH_CNC_COUNTER_STC));
        }

        /* swap HW values to get correct endian order of the CNC counter */
        word[0] = CPSS_32BIT_LE(auMemPtr[firstCncWordIdx]);
        firstCncWordIdx++;
        word[1] = CPSS_32BIT_LE(auMemPtr[firstCncWordIdx]);

        rc = prvCpssDxChCncCounterHwToSw(devNum,format, word, &cuppPortGroupCounter);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* summary the counter from all the port groups */
        counterValuesPtr[ii].byteCount =
            prvCpssMathAdd64(counterValuesPtr[ii].byteCount,
                cuppPortGroupCounter.byteCount);

        counterValuesPtr[ii].packetCount =
            prvCpssMathAdd64(counterValuesPtr[ii].packetCount,
                cuppPortGroupCounter.packetCount);

        descCtrlPtr->unreadCncCounters --;

        /* check if last CNC counters of AU message was read by check
           number of last AU word that was read - firstCncWordIdx. */
        if (firstCncWordIdx < (auMessageNumWords - 1))
        {
            /* this is not last CNC counter in a AU message.
               need to read next CNC counter in current AU message memory. */
            continue;
        }

        /* erase memory to be compatible with FU messages upload.
           point descPtr to the current AU descriptor in the queue */
        descPtr = (PRV_CPSS_AU_DESC_STC*)auMemPtr;
        AU_DESC_RESET_MAC(descPtr);

        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
        {
            /* get next <auMemPtr> and update the 'DB management' of shared port groups CNC */
            rc_nextDesc = sharedCncDescCtrlUpdateAndAddrGet(devNum, portGroupId , descCtrlPtr ,
                auMessageNumBytes ,&auMemPtr , GT_TRUE);
            if(rc_nextDesc == GT_NO_MORE)
            {
                /* will be used only after possible 'rewind' */
            }
            else
            if (rc != GT_OK)
            {
                return rc;
            }

            /* check if need to rewind */
            rc = sharedCncDescCtrlRewindCheck(devNum, portGroupId , descCtrlPtr,
                &needRewind);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(needRewind == GT_TRUE)
            {
                auMemPtr = (GT_U32 *)(descCtrlPtr->blockAddr);
            }
            else
            {
                /* no rewind needed */
                if(rc_nextDesc == GT_NO_MORE)
                {
                    /* no need to rewind but there are no more for current port group */

                    /* ii yet not incremented by loop header */
                    *numOfCounterValuesPtr = (ii + 1);

                    return /* it's not error for log */ GT_NO_MORE;
                }

                if(auMemPtr >= overTheTopAddrPtr)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                        "DB management ERROR: auMemPtr is over the allowed range by [%d] indexes",
                        1 + (((char*)auMemPtr - (char*)overTheTopAddrPtr)/auMessageNumBytes));
                }
            }

        }
        else
        {
            /* move to next AU message memory */
            descCtrlPtr->currDescIdx = ((descCtrlPtr->currDescIdx + 1) %
                                       descCtrlPtr->blockSize);
            auMemPtr = (GT_U32 *)(descCtrlPtr->blockAddr + (auMessageNumBytes * descCtrlPtr->currDescIdx));

            needRewind  = (descCtrlPtr->currDescIdx == 0) ? GT_TRUE : GT_FALSE;
        }

        /* reached the end of AU block */
        if (needRewind == GT_TRUE)
        {
            rc = prvCpssDxChFullQueueRewind(
                devNum,
                    cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ?
                        MESSAGE_QUEUE_PRIMARY_FUQ_E :
                        MESSAGE_QUEUE_CNC23_FUQ_E
                    , descCtrlPtr/*appDescCtrlPtr*/);
            /* ii yet not incremented by loop header */
            *numOfCounterValuesPtr = (ii + 1);

            if(rc == GT_OK && rc_nextDesc == GT_NO_MORE)
            {
                return /* it's not error for log */ GT_NO_MORE;
            }

            if(rc != GT_OK)
            {
                return rc;
            }


            /* on successful rewind ... continue ! (after minimal sleep) */
            /* this is 'MUST' for multi-port groups device (like Lion2)
               that called cpssDxChCncPortGroupUploadedBlockGet(...) with BMP
               of more than single port group

               because each port group can be in different position in it's FUQ
            */
            cpssOsTimerWkAfter(CNC_MINIMAL_SLEEP_TIME_CNS);

            continue;
        }

    }

    *numOfCounterValuesPtr = cncMaxCnt;
    return GT_OK;
}
/**
* @internal cncUploadedBlockGet_perPortGroup function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncUploadedBlockGet one more time to get rest of the block.
*
*         NOTE: AC5P : the function supports CNC0,1 from MG[0]  and CNC2,3 from MG[1]
*               there is logic of give all counters from CNC0,1 and when no more ,
*               to give from CNC2,3 until no more and then return to CNC0,1
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupId              - the port group Id. to support multi-port-groups device.
* @param[in] initOutCounters          - GT_TRUE - override the memory contents
*                                      - GT_FALSE - add to memory contents
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
static GT_STATUS cncUploadedBlockGet_perPortGroup
(
    IN     GT_U8                             devNum,
    IN     GT_U32                            portGroupId,
    IN     GT_BOOL                           initOutCounters,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
)
{
    GT_STATUS   rc;
    GT_STATUS   rc1;
    PRV_CPSS_AU_DESC_CTRL_STC   *descCtrlPtr;  /* pointer to the descriptors DB
                                                 of the device                  */
    GT_U32 orig_numOfCounterValues = *numOfCounterValuesPtr;
    GT_U32 curr_numOfCounterValues = orig_numOfCounterValues;
    GT_U32 alreadyGot_numOfCounterValues;
    PRV_CPSS_DXCH_UNIT_ENT  *lastCncUnitToServePtr;/* pointer to the last CNC units in the port group served */
    PRV_CPSS_DXCH_UNIT_ENT  cncUnit;
    GT_U32                 unreadCncCounters;/* the number of CNC counters still not read from the FUQ */
    GT_U32 mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */

    lastCncUnitToServePtr = &PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.cncUploadNextCncUnitInPortGroupToServe[portGroupId];
    cncUnit = *lastCncUnitToServePtr;

    /* get MG port group that serves the current CNC unit port group */
    rc = prvCpssDxChHwPortGroupBetweenUnitsConvert(devNum,
        cncUnit                   , portGroupId,
        PRV_CPSS_DXCH_UNIT_MG_E   , &mgUnit_portGroupId);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E)
    {
        descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId];
    }
    else /*PRV_CPSS_DXCH_UNIT_CNC_2_E*/
    {
        descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[mgUnit_portGroupId];
    }

    rc = cncUploadedBlockGet_perPortGroup_perCncUnit(devNum, portGroupId,
            descCtrlPtr,
            cncUnit , initOutCounters,
            format, &curr_numOfCounterValues, &counterValuesPtr[0]);
    /* update caller in case error will return */
    *numOfCounterValuesPtr = curr_numOfCounterValues;
    if (rc != GT_OK && rc != GT_NO_MORE)
    {
        return rc;
    }

    unreadCncCounters = descCtrlPtr->unreadCncCounters;

    rc1 = rc;/* save the rc for later use ! */

    *numOfCounterValuesPtr = curr_numOfCounterValues;

    if(PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits > 2 &&
       unreadCncCounters == 0)
    {
        /*
        *         NOTE: AC5P : the function supports CNC0,1 from MG[0]  and CNC2,3 from MG[1]
        *               there is logic of give all counters from CNC0,1 and when no more ,
        *               to give from CNC2,3 until no more and then return to CNC0,1
        */
        cncUnit = (cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E) ?
                PRV_CPSS_DXCH_UNIT_CNC_2_E :
                PRV_CPSS_DXCH_UNIT_CNC_0_E ;

        if(cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E)
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId];
        }
        else /*PRV_CPSS_DXCH_UNIT_CNC_2_E*/
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[mgUnit_portGroupId];
        }

        /* save the info about last CNC units in the port group served */
        *lastCncUnitToServePtr = cncUnit;

        alreadyGot_numOfCounterValues = curr_numOfCounterValues;
        curr_numOfCounterValues       = orig_numOfCounterValues - alreadyGot_numOfCounterValues;

        /* support the CNC upload of the CNC2,3 in the same 'pipe' */
        rc = cncUploadedBlockGet_perPortGroup_perCncUnit(devNum, portGroupId,
                descCtrlPtr,
                cncUnit , initOutCounters,
                format, &curr_numOfCounterValues, &counterValuesPtr[alreadyGot_numOfCounterValues]);
        /* update caller in case error will return */
        *numOfCounterValuesPtr = curr_numOfCounterValues + alreadyGot_numOfCounterValues;
        if (rc != GT_OK && rc != GT_NO_MORE)
        {
            return rc;
        }
    }
    else
    {
        rc = rc1;
    }

    unreadCncCounters = descCtrlPtr->unreadCncCounters;

    if(unreadCncCounters == 0)
    {
        /* save the info about the next port group served */
        /* always start from MG 0 , after no more counters in the 2 MGs */
        *lastCncUnitToServePtr = PRV_CPSS_DXCH_UNIT_CNC_0_E;
    }


    return rc;/* can be : GT_OK or GT_NO_MORE */
}
/**
* @internal internal_cpssDxChCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncUploadedBlockGet one more time to get rest of the block .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat3.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
static GT_STATUS internal_cpssDxChCncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E);

    return cpssDxChCncPortGroupUploadedBlockGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        numOfCounterValuesPtr, format, counterValuesPtr);
}

/**
* @internal cpssDxChCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat3;
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncUploadedBlockGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfCounterValuesPtr, format, counterValuesPtr));

    rc = internal_cpssDxChCncUploadedBlockGet(devNum, numOfCounterValuesPtr, format, counterValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfCounterValuesPtr, format, counterValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCountingEnableSet function
* @endinternal
*
* @brief   The function enables counting on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncUnit                  - selected unit for enable\disable counting
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCountingEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    IN  GT_BOOL                                 enable
)
{
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */
    GT_U32 bitOffset;   /* bit offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (cncUnit)
    {
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
            }
            bitOffset = 3;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
            }
            bitOffset = 4;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_EGRESS_FILTER_PASS_DROP_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.cncEvlanConfigs.vlanPassDropConfigs;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            bitOffset = 3;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PHA_E:
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not supports the PHA unit");
                }

                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PHA.pha_regs.generalRegs.PHACtrl;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            bitOffset = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = (enable == GT_FALSE) ? 0 : 1;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, bitOffset/*fieldOffset*/, 1/*fieldLength*/, data);
}

/**
* @internal cpssDxChCncCountingEnableSet function
* @endinternal
*
* @brief   The function enables counting on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncUnit                  - selected unit for enable\disable counting
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCountingEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCountingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cncUnit, enable));

    rc = internal_cpssDxChCncCountingEnableSet(devNum, cncUnit, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cncUnit, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCountingEnableGet function
* @endinternal
*
* @brief   The function gets enable counting status on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncUnit                  - selected unit for enable\disable counting
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCountingEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;       /* return code            */
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */
    GT_U32 bitOffset;   /* bit offset in register */

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (cncUnit)
    {
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
            }
            bitOffset = 3;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).countingModeConfig;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.cncCountMode;
            }
            bitOffset = 4;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_EGRESS_FILTER_PASS_DROP_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.cncEvlanConfigs.vlanPassDropConfigs;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            bitOffset = 3;
            break;
        case CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PHA_E:
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not supports the PHA unit");
                }

                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PHA.pha_regs.generalRegs.PHACtrl;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            bitOffset = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, bitOffset/*fieldOffset*/, 1/*fieldLength*/, &data);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChCncCountingEnableGet function
* @endinternal
*
* @brief   The function gets enable counting status on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncUnit                  - selected unit for enable\disable counting
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCountingEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCountingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cncUnit, enablePtr));

    rc = internal_cpssDxChCncCountingEnableGet(devNum, cncUnit, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cncUnit, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    return cpssDxChCncPortGroupCounterFormatSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, format);
}

/**
* @internal cpssDxChCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterFormatSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, format));

    rc = internal_cpssDxChCncCounterFormatSet(devNum, blockNum, format);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, format));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
static GT_STATUS internal_cpssDxChCncCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
)
{
    return cpssDxChCncPortGroupCounterFormatGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        blockNum, formatPtr);
}

/**
* @internal cpssDxChCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS cpssDxChCncCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCounterFormatGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, blockNum, formatPtr));

    rc = internal_cpssDxChCncCounterFormatGet(devNum, blockNum, formatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, blockNum, formatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncEgressQueueClientModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT mode
)
{
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */
    GT_U32 fieldLength; /* field length           */
    GT_U32 fieldOffset; /* field offset           */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
        fieldOffset = 16;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
        fieldOffset = 8;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.peripheralAccess.cncModes.modesRegister;
        fieldOffset = 8;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch (mode)
        {
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E:
                data = 2;
                break;
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
                data = 0;
                break;
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
                data = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        fieldLength = 2;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        switch (mode)
        {
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E:
                data = 0;
                break;
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
                data = 1;
                break;
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
                data = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        fieldLength = 2;
    }
    else
    {
        switch (mode)
        {
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
                data = 0;
                break;
            case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
                data = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        fieldLength = 1;
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, fieldOffset, fieldLength, data);
}

/**
* @internal cpssDxChCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressQueueClientModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncEgressQueueClientModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChCncEgressQueueClientModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncEgressQueueClientModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT *modePtr
)
{
    GT_STATUS rc;       /* return code            */
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */
    GT_U32 fieldLength; /* field length           */
    GT_U32 fieldOffset; /* field offset           */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
        fieldOffset = 16;
        fieldLength = 2;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.CNCModes.CNCModes;
        fieldOffset = 8;
        fieldLength = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 2 : 1;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.peripheralAccess.cncModes.modesRegister;
        fieldOffset = 8;
        fieldLength = 1;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, fieldOffset, fieldLength, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch (data)
        {
            case 2:
               *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;
                break;
            case 0:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
                break;
            case 1:
            default:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
                break;
        }
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        switch (data)
        {
            case 0:
               *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
                break;
            default:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
                break;
        }
    }
    else
    {
        switch (data)
        {
            case 0:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
                break;
            default:
                *modePtr = CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
                break;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressQueueClientModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncEgressQueueClientModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChCncEgressQueueClientModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCpuAccessStrictPriorityEnableSet function
* @endinternal
*
* @brief   The function enables strict priority of CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCpuAccessStrictPriorityEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */
    GT_U32    i;                /* loop index         */
    GT_STATUS rc;               /* return code        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    data = (enable == GT_FALSE) ? 0 : 1;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2; Caelum; Bobcat3 */
        for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, i).globalRegs.CNCGlobalConfigReg;
            rc = prvCpssHwPpSetRegField(
                devNum, regAddr, 10/*fieldOffset*/, 1/*fieldLength*/, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Not eArch devices */

    for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[i].globalCfg;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 10/*fieldOffset*/, 1/*fieldLength*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncCpuAccessStrictPriorityEnableSet function
* @endinternal
*
* @brief   The function enables strict priority of CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCpuAccessStrictPriorityEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCpuAccessStrictPriorityEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCncCpuAccessStrictPriorityEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncCpuAccessStrictPriorityEnableGet function
* @endinternal
*
* @brief   The function gets enable status of strict priority of
*         CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncCpuAccessStrictPriorityEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;       /* return code            */
    GT_U32 data;        /* data                   */
    GT_U32 regAddr;     /* register address       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, 0).globalRegs.CNCGlobalConfigReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[0].globalCfg;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 10/*fieldOffset*/, 1/*fieldLength*/, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChCncCpuAccessStrictPriorityEnableGet function
* @endinternal
*
* @brief   The function gets enable status of strict priority of
*         CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCpuAccessStrictPriorityEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncCpuAccessStrictPriorityEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCncCpuAccessStrictPriorityEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on fail due muxed clients
*
* @note In AC5X there are two pairs of clients which are muxed.
*       If one of the clients is enabled in a block, enabling the other will fail.
*       1) CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E
*       2) CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
)
{
    GT_STATUS   rc;           /* return code                  */
    GT_U32      data;         /* data                         */
    GT_U32      regAddr;      /* register address             */
    GT_U32      fieldOffset;  /* field Offset                 */
    GT_U32      portGroupId;  /* the port group Id            */
    GT_U32      mask;         /* field bit mask               */

    GT_BOOL     checkMuxedClient;   /* do AC5X logic        */
    GT_U32      muxRegAddr;         /* register address     */
    GT_U32      numOfBlocks;        /* number of the blocks in the device    */
    GT_U32      muxBitMask;         /* bit of the client in the mux register */
    GT_BOOL     updateEnableGet;    /* enable of other client */
    GT_U32      ii;
    CPSS_DXCH_CNC_CLIENT_ENT  muxedClient;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    /* do "Mux" logic only when enable = true */
    if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) == GT_TRUE  && updateEnable)
    {
        rc = prvCpssDxChCncMuxModeInfoGet(devNum, client, &checkMuxedClient, &muxedClient, &muxRegAddr, &muxBitMask, &data);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(checkMuxedClient)
        {
            /* iterate over all CNC blocks */
            numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks;
            for (ii = 0; ii< numOfBlocks ; ii++)
            {
                /* check if the other client is enabled in the block */
                rc = cpssDxChCncPortGroupBlockClientEnableGet(devNum, portGroupsBmp,
                                                              ii, muxedClient, &updateEnableGet);
                if (rc!=GT_OK)
                {
                    return rc;
                }
                if (updateEnableGet) /* the other client is enabled in the block */
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }

            rc = prvCpssHwPpWriteRegBitMask(devNum, muxRegAddr, muxBitMask, data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        if (prvCpssDxChCncEArchBlockClientToCfgAddr(
            devNum, portGroupsBmp,client, blockNum, GT_TRUE/*write*/,&regAddr, &mask) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* the 'mask' is actually the client number     */
            /* we currently manage single client at a time  */
            GT_U32  clientId = mask;

            if((updateEnable == GT_FALSE))
            {
                data = 0;
            }
            else
            {
                data = 1 | (clientId << 4)/*bits 4..8*/;
            }

            /* update the full register */
            mask = 0xFFFFFFFF;
        }
        else
        {
            data = (updateEnable == GT_FALSE) ? 0 : mask;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupWriteRegBitMask(
                devNum, portGroupId, regAddr, mask, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    /* Not eArch devices */

    if (prvCpssDxChCncBlockClientToCfgAddr(
        devNum, client, blockNum, &regAddr, &fieldOffset) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = (updateEnable == GT_FALSE) ? 0 : 1;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(
            devNum, portGroupId, regAddr,
            fieldOffset, 1/*fieldLength*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on fail due muxed clients
*
* @note In AC5X there are two pairs of clients which are muxed.
*       If one of the clients is enabled in a block, enabling the other will fail.
*       1) CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E
*       2) CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E and CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E
*/
GT_STATUS cpssDxChCncPortGroupBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockClientEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, client, updateEnable));

    rc = internal_cpssDxChCncPortGroupBlockClientEnableSet(devNum, portGroupsBmp, blockNum, client, updateEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, client, updateEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    GT_STATUS rc;             /* return code        */
    GT_U32    data;           /* data               */
    GT_U32    regAddr;        /* register address   */
    GT_U32    fieldOffset;    /* field Offset       */
    GT_U32    portGroupId;    /* the port group Id  */
    GT_U32    mask;           /* field bit mask     */
    GT_BOOL   checkMuxedClient;
    CPSS_DXCH_CNC_CLIENT_ENT muxedClient;
    GT_U32    muxRegBitValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    CPSS_NULL_PTR_CHECK_MAC(updateEnablePtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        if (prvCpssDxChCncEArchBlockClientToCfgAddr(
            devNum, portGroupsBmp,client, blockNum,GT_FALSE/*read*/, &regAddr, &mask) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* the 'mask' is actually the client number     */
            /* we currently manage single client at a time  */
            GT_U32  clientId = mask;
            GT_U32  read_clientId;

            *updateEnablePtr = ((data & 1) == 0) ? GT_FALSE : GT_TRUE;
            read_clientId    = (data >> 4) & 0x1f;/*bits 4..8*/
            if(read_clientId != clientId)
            {
                *updateEnablePtr = GT_FALSE;
            }
        }
        else
        {
            *updateEnablePtr = ((data & mask) == 0) ? GT_FALSE : GT_TRUE;
        }

        /* Mux client */
        if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) == GT_TRUE && *updateEnablePtr == GT_TRUE)
        {
            rc = prvCpssDxChCncMuxModeInfoGet(devNum, client, &checkMuxedClient, &muxedClient, &regAddr, &fieldOffset, &muxRegBitValue);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(checkMuxedClient == GT_TRUE)
            {
                rc = prvCpssHwPpReadRegBitMask(devNum, regAddr, fieldOffset, &data);
                if (rc != GT_OK)
                {
                    return rc;
                }

                *updateEnablePtr = (data == muxRegBitValue) ? GT_TRUE : GT_FALSE;
            }
        }
        return GT_OK;
    }

    /* Not eArch devices */
    if (prvCpssDxChCncBlockClientToCfgAddr(
        devNum, client, blockNum, &regAddr, &fieldOffset) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupGetRegField(
        devNum, portGroupId,
        regAddr, fieldOffset, 1/*fieldLength*/, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *updateEnablePtr = (data == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockClientEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, client, updateEnablePtr));

    rc = internal_cpssDxChCncPortGroupBlockClientEnableGet(devNum, portGroupsBmp, blockNum, client, updateEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, client, updateEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
* @param[in] indexRangesBmp           - bitmap of counter index ranges
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      indexRangesBmp
)
{
    GT_STATUS rc;            /* return code                 */
    GT_U32    regAddr;       /* register address            */
    GT_U32    fieldOffset;   /* field Offset                */
    GT_U32    portGroupId;   /* the port group Id           */
    GT_U32    unitNum;       /* number of CNC unit          */
    GT_U32    blockInUnit;   /* number of block in CNC unit */
    GT_U32    i;             /* loop index                  */
    GT_U32    clientInstace; /* sip6.30 - client instance   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) &&
        (client == CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E ||
         client == CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E ||
         client == CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(((client == CPSS_DXCH_CNC_CLIENT_TTI_E) ||
            (client == CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E) ||
            (client == CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E)) &&
        (blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
            devNum, blockNum, &unitNum, &blockInUnit))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            if (prvCpssDxChCncEArchBlockClientToCfgAddr(
                devNum, portGroupsBmp,client, blockNum, GT_TRUE/*write*/,
                    NULL/* indication to get clientInstace on next param */,
                    &clientInstace) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            clientInstace = 0;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            for (i = 0; (i < 2); i++)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                    rangeCount.CNCBlockRangeCountEnable[i][blockInUnit][clientInstace];

                rc = prvCpssHwPpPortGroupWriteRegister(
                    devNum, portGroupId, regAddr, indexRangesBmp.l[i]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    /* Not eArch devices */

    if (prvCpssDxChCncBlockClientToCfgAddr(
        devNum, client, blockNum, &regAddr, &fieldOffset) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (! PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat3 - 10 ranges of 2048 counters */

        /* check that indexRangesBmp does not contain bits 10-31 */
        if ((indexRangesBmp.l[0] > 0x3FF) || (indexRangesBmp.l[1] != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupSetRegField(
            devNum, portGroupId, regAddr,
            (fieldOffset + 1), 10/*fieldLength*/, indexRangesBmp.l[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    /* Lion2 - 64 ranges                   */
    /* The ranges mapped to this block for all clients  */

    /* The block configuration registers 1-2            */
    regAddr += 4;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupWriteRam(
        devNum, portGroupId, regAddr,
        2/*words*/, indexRangesBmp.l);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
* @param[in] indexRangesBmpPtr        - Pointer to
*                                       bitmap of counter index ranges
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      *indexRangesBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockClientRangesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, client, indexRangesBmpPtr));

    rc = internal_cpssDxChCncPortGroupBlockClientRangesSet(devNum, portGroupsBmp, blockNum, client, *indexRangesBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, client, *indexRangesBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) bitmap counter index ranges
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    GT_STATUS rc;            /* return code                 */
    GT_U32  regAddr;         /* register address            */
    GT_U32  fieldOffset;     /* field Offset                */
    GT_U32  portGroupId;     /* the port group Id           */
    GT_U32  unitNum;         /* number of CNC unit          */
    GT_U32  blockInUnit;     /* number of block in CNC unit */
    GT_U32  i;               /* loop index                  */
    GT_U32    clientInstace; /* sip6.30 - client instance   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    CPSS_NULL_PTR_CHECK_MAC(indexRangesBmpPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) &&
        (client == CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E ||
         client == CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E ||
         client == CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E))
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
            devNum, blockNum, &unitNum, &blockInUnit))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            if (prvCpssDxChCncEArchBlockClientToCfgAddr(
                devNum, portGroupsBmp,client, blockNum, GT_FALSE/*read*/,
                    NULL/* indication to get clientInstace on next param */,
                    &clientInstace) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            clientInstace = 0;
        }

        for (i = 0; (i < 2); i++)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                rangeCount.CNCBlockRangeCountEnable[i][blockInUnit][clientInstace];

            rc = prvCpssHwPpPortGroupReadRegister(
                devNum, portGroupId, regAddr, &(indexRangesBmpPtr->l[i]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Not eArch devices */

    if (prvCpssDxChCncBlockClientToCfgAddr(
        devNum, client, blockNum, &regAddr, &fieldOffset) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (! PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat3 - 10 ranges of 2048 counters */
        indexRangesBmpPtr->l[1] = 0;

        return prvCpssHwPpPortGroupGetRegField(
            devNum, portGroupId, regAddr,
            (fieldOffset + 1), 10/*fieldLength*/,
            &(indexRangesBmpPtr->l[0]));
    }

    /* Lion2 - 64 ranges                   */
    /* The ranges mapped to this block for all clients  */

    /* The block configuration registers 1-2            */
    regAddr += 4;


    return prvCpssHwPpPortGroupReadRam(
        devNum, portGroupId, regAddr,
        2/*words*/, indexRangesBmpPtr->l);
}

/**
* @internal cpssDxChCncPortGroupBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) bitmap counter index ranges
*                                      xCat3 devices has 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X, Falcon (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockClientRangesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, client, indexRangesBmpPtr));

    rc = internal_cpssDxChCncPortGroupBlockClientRangesGet(devNum, portGroupsBmp, blockNum, client, indexRangesBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, client, indexRangesBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] portGroupIdArr[]         (pointer to) array of port group Ids of
*                                      counters wrapped around.
*                                      The NULL pointer supported.
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters
*                                      wrapped around
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
static GT_STATUS internal_cpssDxChCncPortGroupCounterWraparoundIndexesGet
(
    IN    GT_U8                   devNum,
    IN     GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN    GT_U32                  blockNum,
    INOUT GT_U32                  *indexNumPtr,
    OUT   GT_U32                  portGroupIdArr[],
    OUT   GT_U32                  indexesArr[]
)
{
    GT_STATUS rc;               /* return code                 */
    GT_U32    regAddr;          /* register address            */
    GT_U32    data;             /* HW data                     */
    GT_U32    i;                /* loop index                  */
    GT_U32    j;                /* loop index                  */
    GT_U32    maxIndexes;       /* index  array size           */
    GT_U32    portGroupId;      /*the port group Id            */
    GT_U32    unitNum;          /* number of CNC unit          */
    GT_U32    blockInUnit;      /* number of block in CNC unit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    CPSS_NULL_PTR_CHECK_MAC(indexNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    if (*indexNumPtr == 0)
    {
        return GT_OK;
    }

    maxIndexes = *indexNumPtr;

    *indexNumPtr = 0;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
            devNum, blockNum, &unitNum, &blockInUnit))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            for (i = 0; (i < 4); i++)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
                    wraparound.CNCBlockWraparoundStatusReg[blockInUnit][i];

                rc = prvCpssHwPpPortGroupReadRegister(
                    devNum, portGroupId, regAddr, &data);
                if (rc != GT_OK)
                {
                    return rc;
                }

                for (j = 0; (j < 2); j++, data = (data >> 16))
                {
                    if (data & 1)
                    {
                        indexesArr[*indexNumPtr] = (data >> 1) & 0x0FFF;
                        if (portGroupIdArr != NULL)
                        {
                            portGroupIdArr[*indexNumPtr] = portGroupId;
                        }

                        (*indexNumPtr) ++;

                        if (*indexNumPtr == maxIndexes)
                        {
                            return GT_OK;
                        }
                    }
                }
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    /* Not eArch devices */
    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockWraparoundStatusAddrBase
        + (CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS * blockInUnit);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        for (i = 0; (i < 4); i++)
        {
            rc = prvCpssHwPpPortGroupReadRegister(
                devNum, portGroupId, (regAddr + (i * 4)), &data);
            if (rc != GT_OK)
            {
                return rc;
            }

            for (j = 0; (j < 2); j++, data = (data >> 16))
            {
                if (data & 1)
                {
                    indexesArr[*indexNumPtr] = (data >> 1) & 0x0FFF;
                    if (portGroupIdArr != NULL)
                    {
                        portGroupIdArr[*indexNumPtr] = portGroupId;
                    }

                    (*indexNumPtr) ++;

                    if (*indexNumPtr == maxIndexes)
                    {
                        return GT_OK;
                    }
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] portGroupIdArr[]         (pointer to) array of port group Ids of
*                                      counters wrapped around.
*                                      The NULL pointer supported.
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters
*                                      wrapped around
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
GT_STATUS cpssDxChCncPortGroupCounterWraparoundIndexesGet
(
    IN    GT_U8                   devNum,
    IN     GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN    GT_U32                  blockNum,
    INOUT GT_U32                  *indexNumPtr,
    OUT   GT_U32                  portGroupIdArr[],
    OUT   GT_U32                  indexesArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupCounterWraparoundIndexesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, indexNumPtr, portGroupIdArr, indexesArr));

    rc = internal_cpssDxChCncPortGroupCounterWraparoundIndexesGet(devNum, portGroupsBmp, blockNum, indexNumPtr, portGroupIdArr, indexesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, indexNumPtr, portGroupIdArr, indexesArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_U32    regAddr;          /* register address           */
    GT_U32    unitNum;          /* number of CNC unit          */
    GT_U32    blockInUnit;      /* number of block in CNC unit */

    /* other parameters checked in called function */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    /*******************************************************/
    /* NOTE: since SIP_5_20 : the counters are 'read only' */
    /*******************************************************/

    if((blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }
    PRV_CPSS_DXCH_CNC_COUNTER_INDEX_VALID_CHECK_MAC(devNum, index);

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
            memory.baseAddress[blockInUnit] + (index * 8);
    }
    else
    {
        /* Not eArch devices */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockCountersAddrBase
            + (blockInUnit * CNC_COUNTER_BLOCK_ADDR_INCREMENT_CNS) + (index * 8);
    }

    return prvCpssDxChCncPortGroupCounterValueAverageSet(
        devNum, portGroupsBmp,
        regAddr, format, counterPtr);
}

/**
* @internal cpssDxChCncPortGroupCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, index, format, counterPtr));

    rc = internal_cpssDxChCncPortGroupCounterSet(devNum, portGroupsBmp, blockNum, index, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, index, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_U32    regAddr;          /* register address  */
    GT_U32    unitNum;          /* number of CNC unit          */
    GT_U32    blockInUnit;      /* number of block in CNC unit */

    /* other parameters checked in called function */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    if((blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }
    PRV_CPSS_DXCH_CNC_COUNTER_INDEX_VALID_CHECK_MAC(devNum, index);

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).perBlockRegs.
            memory.baseAddress[blockInUnit] + (index * 8);
    }
    else
    {
        /* Not eArch devices */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockCountersAddrBase
            + (blockInUnit * CNC_COUNTER_BLOCK_ADDR_INCREMENT_CNS) + (index * 8);
    }

    return prvCpssDxChCncPortGroupCounterValueSummingGet(
        devNum, portGroupsBmp,
        regAddr, format, counterPtr);
}

/**
* @internal cpssDxChCncPortGroupCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, index, format, counterPtr));

    rc = internal_cpssDxChCncPortGroupCounterGet(devNum, portGroupsBmp, blockNum, index, format, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, index, format, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal cncSectorSet function
* @endinternal
*
* @brief  The CNC may hold selector at the MG unit or DFX server.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in] blockNum                 - CNC block number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS cncSectorSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
)
{
    GT_STATUS  rc;              /* return code                   */
    GT_U32     regAddr;         /* register address              */
    GT_U32     unitNum = 0;     /* number of CNC unit            */
    GT_U32     blockInUnit = 0; /* number of block in CNC unit   */
    GT_U32     portGroupId;     /* the port group Id - support multi-port-groups device */
    GT_U32     fieldValue;      /* field value                   */
    GT_U32     full_dfxPortGroupsBmp,dfxPortGroupsBmp;
    GT_U32     dfxPortGroupId;
    GT_U32     mask;

    if (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits <= 1)
    {
        /* no selector */
        return GT_OK;
    }

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {

        /* Switching Core/Centralized Counters 0/Global Registers/CNC Metal Fix Register */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[0/* in CNC0 !!!*/].CNCMetalFixRegister;

        rc = prvCpssHwPpSetRegField(
           devNum, regAddr, 16, 1, 1 - unitNum/* unit 0 : value = 1 , unit 1 : value = 0 */);
        if (rc != GT_OK)
        {
            return rc;
        }

        return GT_OK;
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
       PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* the CNC in bobcat3 must not be triggered in parallel on the 2 pipes
           because both pipe use the same MG unit with the same FUQ
        */
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) /*BC3,Aldrin2*/
        {
            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;

            /* bits 18,19,20 */
            /*20 rf_cnc_pipe_select */
            /*19 pipe1_rf_cnc_unit_dump_select */
            /*18 pipe0_rf_cnc_unit_dump_select */
            if(0 == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                portGroupId = 0;/* otherwise we get 0xFFFFFFFF from PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC */
            }
            else
            {
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);
            }

            fieldValue = (unitNum << 0)/* pipe 0 */ |
                         (unitNum << 1)/* pipe 1 */ |
                         (portGroupId  << 2);/* select the pipeId */

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                devNum, regAddr,
                18/*fieldOffset*/, 3/*fieldLength*/, fieldValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))/*AC5P*/
        {
            GT_U32 fieldOffset;

            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;

            fieldValue  = unitNum & 1;
            fieldOffset = 11 +  (unitNum >> 1);/* bit 11 for CNC0,1 , bit 12 for CNC 2,3 */

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                devNum, regAddr,
                fieldOffset, 1/*fieldLength*/, fieldValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else /*Falcon*/
        {
            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15;

            /* bits 28,29 */
            /*29 pipe1_rf_cnc_unit_dump_select */
            /*28 pipe0_rf_cnc_unit_dump_select */
            fieldValue = (unitNum << 0) /* pipe 0 */ |
                         (unitNum << 1);/* pipe 1 */

            full_dfxPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,full_dfxPortGroupsBmp,PRV_CPSS_DXCH_UNIT_DFX_SERVER_E);

            dfxPortGroupsBmp = 0;
            /* build dfxPortGroupsBmp that not hold duplications of port groups in portGroupsBmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
                devNum, portGroupsBmp, portGroupId)
            {
                dfxPortGroupId = portGroupId & (~1);/* the 'even' port group */
                if(full_dfxPortGroupsBmp & (3 << dfxPortGroupId))/* one of the 2 port groups belong to this DFX */
                {
                    dfxPortGroupsBmp |= 1 << dfxPortGroupId;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
                devNum, portGroupsBmp, portGroupId)

            /* NOTE: we need to update only the relevant DFX port groups (single per tile)
               so we can't use the 'prvCpssDrvHwPpResetAndInitControllerSetRegField'
               not get the 'portGroupId' , so we need to use prvCpssHwPpPortGroupSetRegField(...)
            */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
                devNum, dfxPortGroupsBmp, portGroupId)
            {
                if((1<<(portGroupId/2)) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
                {
                    /* check if the odd port group was in the orig bmp --- it reflects bit 28 */
                    mask  = ((1<<(portGroupId+1)) & portGroupsBmp) ? BIT_28 : 0;
                    /* check if the even port group was in the orig bmp --- it reflects bit 29 */
                    mask |= ((1<<(portGroupId+0)) & portGroupsBmp) ? BIT_29 : 0;
                }
                else
                {
                    /* check if the even port group was in the orig bmp --- it reflects bit 28 */
                    mask  = ((1<<(portGroupId+0)) & portGroupsBmp) ? BIT_28 : 0;
                    /* check if the odd port group was in the orig bmp --- it reflects bit 29 */
                    mask |= ((1<<(portGroupId+1)) & portGroupsBmp) ? BIT_29 : 0;
                }

                rc = prvCpssHwPpPortGroupWriteRegBitMask(
                    devNum, portGroupId, regAddr,
                    mask,fieldValue << 28);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
                devNum, dfxPortGroupsBmp, portGroupId)
        }
    }
    else /* BC2 .. Aldrin or GM (for all sip5 devices)*/
    {
        /*<MG_IP> MG_IP/MG Registers/Global Control/General Configurations*/
        /* Bit 28 - CNC Dump Selector.Dumping is done from CNC0 / CNC1 */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.generalConfigurations;

        rc = prvCpssHwPpSetRegField(
           devNum, regAddr, 28, 1, unitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


#ifdef GM_USED
/* flag to allow debug of : CNC-142 : Aldrin2 VERIFIER GM : triggering of CNCFastDumpTriggerReg (0x0e000030) crash the GM*/
#ifdef _WIN32
static GT_U32   debug_Jira_CNC_142 =  0;
#else /*!_WIN32*/
static GT_U32   debug_Jira_CNC_142 =  1;/* currently pass in LINUX ?! */
#endif /*!_WIN32*/
/* function to allow debug of the issue without re-compiling the code */
void debug_Jira_CNC_142_Set(GT_U32  allowTrigger)
{
    debug_Jira_CNC_142 = allowTrigger;
}
#endif /*GM_USED*/
/**
* @internal internal_cpssDxChCncPortGroupBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncPortGroupBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
)
{
    GT_STATUS              rc;               /* return code                       */
    GT_U32                 regAddr;          /* register address                  */
    GT_BOOL                actionCompleted;  /* Action Trigger Flag               */
    GT_BOOL                uploadEnable;     /* FU upload Enable                  */
    CPSS_FDB_ACTION_MODE_ENT    mode;        /* FDB action mode                   */
    PRV_CPSS_AU_DESC_STC        *descPtr;    /*pointer to the current descriptor */
    PRV_CPSS_AU_DESC_CTRL_STC   *descCtrlPtr;/* pointer to the descriptors DB
                                                 of the device                    */
    GT_U32                     *auMemPtr;/* pointer to the SDMA memory , to start of current message */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32                      auMessageNumBytes;
    GT_U32                      inProcessBlocksBmp[2]; /* bitmap of being upload blocks */
    GT_U32                      unitNum = 0;        /* number of CNC unit            */
    GT_U32                      blockInUnit = 0;    /* number of block in CNC unit   */
    GT_U32                      mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    GT_PORT_GROUPS_BMP          IN_inProcessBlocksBmp;/*needed to be check groups */
    GT_U32                      triggerId;/* the trigger Id of the current CNC upload operation */
    PRV_CPSS_DXCH_UNIT_ENT      cncUnit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    PRV_CNC_BLOCK_UPLOAD_SUPPORT_CHECK_MAC(devNum);

    /* the feartue needs separated AU and FU message queues */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.fuqUseSeparate == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (GT_OK != prvCpssDxChCncMultiUnitsBlockUnitGet(
        devNum, blockNum, &unitNum, &blockInUnit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits > 2 &&
        unitNum >= 2)
    {
        /* AC5P (and above) */
        /* CNC units 2,3 uses MG1 ... see function prvCpssDxChHwPortGroupBetweenUnitsConvert(...) */
        cncUnit = PRV_CPSS_DXCH_UNIT_CNC_2_E;
    }
    else
    {
        cncUnit = PRV_CPSS_DXCH_UNIT_CNC_0_E;
    }

    IN_inProcessBlocksBmp = portGroupsBmp;
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
    {
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            if(PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId)->otherSharedPortGroupsOnMyMgUnit & portGroupsBmp)
            {
                /* the CNC in bobcat3 must not be triggered in parallel on the 2 pipes
                   because both pipe use the same MG unit with the same FUQ
                */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Triggering upload from multiple CNC ports groups in parallel is not supported for portGroupId[%d] with port group bmp[0x%8.8x]",
                    portGroupId ,
                    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId)->otherSharedPortGroupsOnMyMgUnit);
            }

            /* we need to check that CNC is not activated on other 'shared' port group */

            /* the CNC in bobcat3 even if we want to trigger CNC of port group 1
                we need to wait for end of trigger of port group 0 as well
                (beside end of trigger of port group 0)
               because both pipe use the same MG unit with the same FUQ
            */
            IN_inProcessBlocksBmp |=
                PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId)->otherSharedPortGroupsOnMyMgUnit;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

    }

    auMessageNumBytes = 4 * PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;

    /* check that there is not not-finished FDB upload */
    rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &actionCompleted);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChBrgFdbUploadEnableGet(devNum, &uploadEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChBrgFdbActionModeGet(devNum, &mode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((uploadEnable == GT_TRUE) && (actionCompleted == GT_FALSE)
        && (mode == CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    inProcessBlocksBmp[0] = 0;
    inProcessBlocksBmp[1] = 0;
    rc = cpssDxChCncPortGroupBlockUploadInProcessGet(
        devNum, IN_inProcessBlocksBmp, inProcessBlocksBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((inProcessBlocksBmp[0] != 0 && cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E) ||
        (inProcessBlocksBmp[1] != 0 && cncUnit == PRV_CPSS_DXCH_UNIT_CNC_2_E))
    {
        #ifdef CPSS_LOG_ENABLE
        GT_U32  failedBmp = cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ? inProcessBlocksBmp[0] : inProcessBlocksBmp[1];
        GT_CHAR*  failedBmpStr = cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E ? "31..0" : "63..32";

        if(IN_inProcessBlocksBmp == portGroupsBmp)
        {
            if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "The Device not finished previous CNC upload \n"
                    "on CNC blocks bitmap [blocks %s] [0x%8.8x] ",
                    failedBmpStr,
                    failedBmp);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "The Device not finished previous upload on CNC port groups bitmap[0x%8.8x] \n"
                    "on CNC blocks bitmap [blocks %s] [0x%8.8x] ",
                    IN_inProcessBlocksBmp,
                    failedBmpStr,
                    failedBmp);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "The Device not finished previous upload on CNC port groups bitmap [0x%8.8x] \n"
                "on CNC blocks bitmap [blocks %s] [0x%8.8x] \n",
                IN_inProcessBlocksBmp ,
                failedBmpStr,
                failedBmp);
        }
        #else /*!CPSS_LOG_ENABLE*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,LOG_ERROR_NO_MSG);
        #endif /*!CPSS_LOG_ENABLE*/
    }

    /* loop on all port groups to check the configuration */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* get MG port group that serves the current CNC unit port group */
        rc = prvCpssDxChHwPortGroupBetweenUnitsConvert(devNum,
            cncUnit                   , portGroupId,
            PRV_CPSS_DXCH_UNIT_MG_E   , &mgUnit_portGroupId);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* used separate FU queue if enabled */
        if(cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E)
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId];
        }
        else /*PRV_CPSS_DXCH_UNIT_CNC_2_E*/
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[mgUnit_portGroupId];
        }

        /* check there are no FU messages in the queue */
        if (descCtrlPtr->unreadCncCounters == 0)
        {
            /* check that the FUQ initialized */
            PRV_CPSS_FUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum);

            /* the pointer to start of 'next message to handle'  */
            auMemPtr = (GT_U32 *)(descCtrlPtr->blockAddr + (auMessageNumBytes * descCtrlPtr->currDescIdx));

            /* pointer to the current "retrieved" descriptor */
            descPtr = (PRV_CPSS_AU_DESC_STC*)auMemPtr;
            /* found valid FU message in the queue */
            if (AU_DESC_IS_NOT_VALID(descPtr) != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    /* the CNC may hold selector at the MG unit or DFX server */
    rc = cncSectorSet(devNum,portGroupsBmp,blockNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* loop on all port groups to trigger the upload */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* get MG port group that serves the current CNC unit port group */
        rc = prvCpssDxChHwPortGroupBetweenUnitsConvert(devNum,
            cncUnit                   , portGroupId,
            PRV_CPSS_DXCH_UNIT_MG_E   , &mgUnit_portGroupId);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(cncUnit == PRV_CPSS_DXCH_UNIT_CNC_0_E)
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId];
        }
        else /*PRV_CPSS_DXCH_UNIT_CNC_2_E*/
        {
            descCtrlPtr = &PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[mgUnit_portGroupId];
        }

        /* set indication (if needed) about FUQ used by CNC and not by the FDB */
        rc = prvCpssDxChFuqOwnerSet(devNum , portGroupId , GT_TRUE /* used by CNC */,
            descCtrlPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* increase amount of queued counters */
        descCtrlPtr->unreadCncCounters +=
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlockNumEntries;

        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
        {
            /* get 'index' before the update (from MG port group)*/
            triggerId = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,mgUnit_portGroupId)->
                perMgUnit.triggeredNumOfCncUploads;

            /* state on my port group (NOT on MG port group) that this trigger is 'mine' */
            PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_SET_MAC(devNum,portGroupId,
                triggerId,1);

            /* register this trigger into the 'per MG info' */
            PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,mgUnit_portGroupId)->
                perMgUnit.triggeredNumOfCncUploads++;
        }

        /* trigger CNC counters upload */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {

            /* CNC-142 : Aldrin2 VERIFIER GM : triggering of CNCFastDumpTriggerReg (0x0e000030) crash the GM */
#ifdef GM_USED
            if((0 == debug_Jira_CNC_142) && PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                    "operation not allowed due to JIRA : CNC-142 : Aldrin2 VERIFIER GM : triggering of CNCFastDumpTriggerReg (0x0e000030) crash the GM");
            }
#endif /*GM_USED*/
            regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, unitNum).
                globalRegs.CNCFastDumpTriggerReg;
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId,regAddr,
                blockInUnit/*fieldOffset*/, 1/*fieldLength*/, 1);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[unitNum].blockUploadTriggers;
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId,regAddr,
                blockInUnit/*fieldOffset*/, 1/*fieldLength*/, 1);
        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncPortGroupBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncPortGroupBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockUploadTrigger);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum));

    rc = internal_cpssDxChCncPortGroupBlockUploadTrigger(devNum, portGroupsBmp, blockNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Checks all active port groups of the bitmap.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*                                      NOTE: this is array of 2 GT_U32 in next devices: (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.)
*                                            as the device supports 64 blocks (not only 32 as in legacy devices)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
    GT_STATUS   rc;               /* return code       */
    GT_U32      regAddr;          /* register address  */
    GT_U32      regData;          /* register data     */
    /* support multi-port-groups device */
    GT_U32      portGroupId;      /* the port group Id */
    GT_U32      i;                /* loop index        */
    GT_U32      cncBlocksPerUnit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    CPSS_NULL_PTR_CHECK_MAC(inProcessBlocksBmpPtr);
    PRV_CNC_BLOCK_UPLOAD_SUPPORT_CHECK_MAC(devNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        /* clear before accumulating */
        for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
        {
            if ((i & 1) == 0)
            {
                inProcessBlocksBmpPtr[i / 2] = 0;
            }
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, i).
                    globalRegs.CNCFastDumpTriggerReg;

                rc = prvCpssHwPpPortGroupReadRegister(
                    devNum, portGroupId, regAddr, &regData);
                if (rc != GT_OK)
                {
                    return rc;
                }

                inProcessBlocksBmpPtr[i / 2] |=
                    ((regData & 0xFFFF) << (((i & 1) == 0) ? 0 : 16));

            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    *inProcessBlocksBmpPtr = 0x0;
    cncBlocksPerUnit = PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks;
    /* Not eArch devices */
    for (i = 0; (i < PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits); i++)
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cncRegs[i].blockUploadTriggers;

        /* loop on all port groups to set the same configuration */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupGetRegField(
                devNum, portGroupId, regAddr,
                0/*fieldOffset*/, 12/*fieldLength*/,
                &regData);
            if (rc != GT_OK)
            {
                return rc;
            }
            *inProcessBlocksBmpPtr |= regData << (cncBlocksPerUnit * i);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Checks all active port groups of the bitmap.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*                                      NOTE: this is array of 2 GT_U32 in next devices: (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.)
*                                            as the device supports 64 blocks (not only 32 as in legacy devices)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupBlockUploadInProcessGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, inProcessBlocksBmpPtr));

    rc = internal_cpssDxChCncPortGroupBlockUploadInProcessGet(devNum, portGroupsBmp, inProcessBlocksBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, inProcessBlocksBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Getting messages from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
static GT_STATUS internal_cpssDxChCncPortGroupUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         counterValuesPtr[]
)
{
    GT_STATUS   rc; /* return code */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  portGroupNumOfCounters;/* number of counters requested and actually */
                                   /* retrieved for the current port group. */
    GT_U32  minNumOfCounters;      /* the minimum number of counters read per port */
                                   /* group from all port groups.   */
    GT_BOOL firstPortGroup;        /* is first Port Group the first */
    GT_STATUS   outOfSyncStatus = GT_OK;/* indication of out of synch between the multi-port groups */
    GT_U32  retryIteration;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    CPSS_NULL_PTR_CHECK_MAC(numOfCounterValuesPtr);

    PRV_CNC_COUNTER_FORMAT_CHECK_MAC(devNum, format);

    CPSS_NULL_PTR_CHECK_MAC(counterValuesPtr);
    PRV_CNC_BLOCK_UPLOAD_SUPPORT_CHECK_MAC(devNum);

    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
    {
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            if(PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId)->otherSharedPortGroupsOnMyMgUnit & portGroupsBmp)
            {
                /* the CPSS management to support read in parallel from 2 port
                   groups without getting into 'half' upload from one of the port
                   groups while the other is fully uploaded it too heavy.
                */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Getting messages from multiple CNC ports groups in parallel is not supported for portGroupId[%d] with port group bmp[0x%8.8x]",
                    portGroupId ,
                    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId)->otherSharedPortGroupsOnMyMgUnit);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

    }

    if (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        rc = GT_OK;

        minNumOfCounters = *numOfCounterValuesPtr;

        firstPortGroup = GT_TRUE;
        retryIteration = 1;

        /* sum the counters from all the port groups */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            retryBlockGet_lbl:/*retry to get the block of counters */

            portGroupNumOfCounters = *numOfCounterValuesPtr;
            /* all port groups have the same number of counters */
            rc = cncUploadedBlockGet_perPortGroup(
                devNum, portGroupId, firstPortGroup /*initOutCounters*/,
                format, &portGroupNumOfCounters, counterValuesPtr);

            if(rc == GT_NOT_READY && retryIteration)
            {
                /* this port group is not ready yet , let's give it another try
                   after minimal sleep */
                retryIteration = 0;
                cpssOsTimerWkAfter(CNC_MINIMAL_SLEEP_TIME_CNS);
                goto retryBlockGet_lbl;
            }

            retryIteration = 1;
            if(((GT_OK == rc) || (GT_NO_MORE == rc)) &&
                (firstPortGroup != GT_TRUE) &&
                (portGroupNumOfCounters != minNumOfCounters))
            {
                /* the non first port group hold different number of counters
                   than the previous port groups ! */
                outOfSyncStatus = GT_GET_ERROR;
            }

            if( portGroupNumOfCounters < minNumOfCounters )
            {
                minNumOfCounters = portGroupNumOfCounters;
            }

            if( GT_OK == rc || GT_NO_MORE == rc )
            {
                /* continue to next port group in case get number of requested counters */
                /* successful or this port group has no more counters */
            }
            else
            {
                return rc;
            }

            firstPortGroup = GT_FALSE;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        *numOfCounterValuesPtr = minNumOfCounters;

        if(outOfSyncStatus != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(outOfSyncStatus,
                "the port groups BMP[0x%x] are 'out of sync' about the number of available counters ",
                portGroupsBmp);
        }

        /* at this point : ( GT_OK == rc || GT_NO_MORE == rc) */
        return rc;
    }

    /* non multi-port-groups device */
    return cncUploadedBlockGet_perPortGroup(
        devNum, CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,
        GT_TRUE /*initOutCounters*/, format,
        numOfCounterValuesPtr,counterValuesPtr);
}

/**
* @internal cpssDxChCncPortGroupUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Getting messages from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncPortGroupUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         counterValuesPtr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupUploadedBlockGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, numOfCounterValuesPtr, format, counterValuesPtr));

    rc = internal_cpssDxChCncPortGroupUploadedBlockGet(devNum, portGroupsBmp, numOfCounterValuesPtr, format, counterValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, numOfCounterValuesPtr, format, counterValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortGroupCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    GT_STATUS rc;            /* return code                 */
    GT_U32    regAddr;       /* register address            */
    GT_U32    data;          /* HW data                     */
    GT_U32    offset;        /* HW data offset              */
    GT_U32    portGroupId;   /*the port group Id            */
    GT_U32    numBits;       /* number of bits for the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    if((blockNum >= 16) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <= 16) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        /* SIP5.15 only allows block numbers higher than 15 for
             TTI lookups 2 and 3, allow them even when cncBlocks <= 16. */
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum - 16);
    }
    else
    {
        PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    }

    switch (format)
    {
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E:
            data = 0;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E:
            data = 1;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E:
            data = 2;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                data = 3;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                data = 4;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                data = 5;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get info about count mode */
    rc = prvCpssDxChCncBlockClientToCountMode(devNum, blockNum,&regAddr,&offset,&numBits);
    if (rc != GT_OK)
    {
        return rc;
    }


    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(
            devNum, portGroupId, regAddr,
            offset, numBits/*length*/, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupCounterFormatSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, format));

    rc = internal_cpssDxChCncPortGroupCounterFormatSet(devNum, portGroupsBmp, blockNum, format);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, format));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortGroupCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
static GT_STATUS internal_cpssDxChCncPortGroupCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
)
{
    GT_STATUS rc;            /* return code                 */
    GT_U32    regAddr;       /* register address            */
    GT_U32    data;          /* HW data                     */
    GT_U32    offset;        /* HW data offset              */
    GT_U32    portGroupId;   /* the port group Id           */
    GT_U32    numBits;       /* number of bits for the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(devNum, blockNum);
    CPSS_NULL_PTR_CHECK_MAC(formatPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* get info about count mode */
    rc = prvCpssDxChCncBlockClientToCountMode(devNum, blockNum,&regAddr,&offset,&numBits);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpPortGroupGetRegField(
        devNum, portGroupId, regAddr,
        offset, numBits, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (data)
    {
        case 0:
            *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case 1:
            *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case 2:
            *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        case 3:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            break;
        case 4:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            break;
        case 5:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                *formatPtr = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChCncPortGroupCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS cpssDxChCncPortGroupCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortGroupCounterFormatGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, blockNum, formatPtr));

    rc = internal_cpssDxChCncPortGroupCounterFormatGet(devNum, portGroupsBmp, blockNum, formatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, blockNum, formatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncOffsetForNatClientSet function
* @endinternal
*
* @brief   Set the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] offset                   - CNC  for NAT client
*                                      (APPLICABLE RANGES: 0..65535)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out off range offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
static GT_STATUS internal_cpssDxChCncOffsetForNatClientSet
(
    IN   GT_U8    devNum,
    IN   GT_U32   offset
)
{
    GT_U32 regAddr;     /* the register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

    if (offset > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

   regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).NATConfig;

   return prvCpssHwPpSetRegField(devNum,
                                 regAddr,
                                 0 /*offset*/,
                                 16 /*length*/,
                                 offset);
}

/**
* @internal cpssDxChCncOffsetForNatClientSet function
* @endinternal
*
* @brief   Set the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] offset                   - CNC  for NAT client
*                                      (APPLICABLE RANGES: 0..65535)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out off range offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
GT_STATUS cpssDxChCncOffsetForNatClientSet
(
    IN   GT_U8    devNum,
    IN   GT_U32   offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncOffsetForNatClientSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, offset));

    rc = internal_cpssDxChCncOffsetForNatClientSet(devNum, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChCncOffsetForNatClientGet function
* @endinternal
*
* @brief   Get the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] offsetPtr                - (pointer to) CNC offset for NAT client
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
static GT_STATUS internal_cpssDxChCncOffsetForNatClientGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *offsetPtr
)
{
    GT_STATUS   rc;     /* return code            */
    GT_U32      regAddr;/* the register address   */
    GT_U32      value;  /* the value from regiter */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);

    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).NATConfig;

    rc = prvCpssHwPpGetRegField(devNum,
                                regAddr,
                                0/*fieldOffset*/,
                                16/*fieldLength*/,
                                &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *offsetPtr  = value;

    return GT_OK;
}

/**
* @internal cpssDxChCncOffsetForNatClientGet function
* @endinternal
*
* @brief   Get the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] offsetPtr                - (pointer to) CNC offset for NAT client
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
GT_STATUS cpssDxChCncOffsetForNatClientGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncOffsetForNatClientGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, offsetPtr));

    rc = internal_cpssDxChCncOffsetForNatClientGet(devNum, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncQueueStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of queue buffers consumption for triggering queue statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[in] queueLimit        - queue threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS internal_cpssDxChCncQueueStatusLimitSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN GT_U32                              queueLimit
)
{
    GT_U32 tableIndex; /* table index */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(tcQueue);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    if ( queueLimit > 0xFFFFF )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    tableIndex = (profileSet << 4) + tcQueue;

    rc = prvCpssDxChWriteTableEntryField(devNum,
    CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
    tableIndex,
    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,47,20,
    queueLimit);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS for queue status limit %d failed at index %d\n",
        queueLimit,16*(profileSet*16+tcQueue));
    }
    return rc;
}

/**
* @internal cpssDxChCncQueueStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of queue buffers consumption for triggering queue statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[in] queueLimit        - queue threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssDxChCncQueueStatusLimitSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN GT_U32                              queueLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncQueueStatusLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, queueLimit));

    rc = internal_cpssDxChCncQueueStatusLimitSet(devNum, profileSet,
                                                 tcQueue, queueLimit);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, queueLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncQueueStatusLimitGet function
* @endinternal
*
* @brief   Get the limit of queue buffers consumption for triggering queue statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[out] queueLimit       - (pointer to)queue threshold limit.
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS internal_cpssDxChCncQueueStatusLimitGet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    OUT GT_U32                             *queueLimitPtr
)
{
    GT_U32 tableIndex; /* table index */
    GT_STATUS rc;
    GT_U32 regData[4];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(tcQueue);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);
    CPSS_NULL_PTR_CHECK_MAC(queueLimitPtr);

    tableIndex = (profileSet << 4) + tcQueue;
    /* get queue status limit */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E, tableIndex, regData);
    if (rc != GT_OK)
        return rc;

    rc = prvCpssFieldValueGet(regData, 47, 20, queueLimitPtr);

    return rc;
}

/**
* @internal cpssDxChCncQueueStatusLimitGet function
* @endinternal
*
* @brief   Get the limit of queue buffers consumption for triggering queue statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[out] queueLimit       - (pointer to)queue threshold limit.
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssDxChCncQueueStatusLimitGet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    OUT GT_U32                             *queueLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncQueueStatusLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, queueLimitPtr));

    rc = internal_cpssDxChCncQueueStatusLimitGet(devNum, profileSet,
                                                 tcQueue, queueLimitPtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, queueLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of Port buffers consumption for triggering Port statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] portLimit         - port buffer threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS internal_cpssDxChCncPortStatusLimitSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U32                              portLimit
)
{
    GT_STATUS rc;
    GT_U32 regData[4];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    if ( portLimit > 0xFFFFF )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,
        regData);

    U32_SET_FIELD_IN_ENTRY_MAC(regData,72,20,portLimit);

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,
        regData);

    if(rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChWriteTableEntry failed\n");
    }

    return rc;
}

/**
* @internal cpssDxChCncPortStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of Port buffers consumption for triggering Port statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[in] portLimit         - port buffer threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssDxChCncPortStatusLimitSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U32                              portLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortStatusLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, portLimit));

    rc = internal_cpssDxChCncPortStatusLimitSet(devNum, profileSet,
                                                 portLimit);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, portLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCncPortStatusLimitGet function
* @endinternal
*
* @brief   Get the limit of port buffers consumption for triggering port statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[out] portLimit        - (pointer to)port threshold limit.
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCncPortStatusLimitGet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                             *portLimitPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 regData[4];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);
    CPSS_NULL_PTR_CHECK_MAC(portLimitPtr);

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,regData);

    if(rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChReadTableEntry failed");
    }

    rc = prvCpssFieldValueGet(regData,72,20,portLimitPtr);

    return rc;
}

/**
* @internal cpssDxChCncPortStatusLimitGet function
* @endinternal
*
* @brief   Get the limit of Port buffers consumption for triggering Port statistics counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum            - device number.
* @param[in] profileSet        - profile set.
* @param[out] portLimit        - (pointer to)port buffer threshold limit.
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssDxChCncPortStatusLimitGet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                             *portLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortStatusLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, portLimitPtr));

    rc = internal_cpssDxChCncPortStatusLimitGet(devNum, profileSet,
                                                 portLimitPtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, portLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncIngressPacketTypeClientHashModeEnableSet function
* @endinternal
*
* @brief  Enable/disable Hash mode for CNC Packet Type Cient.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @param[in] devNum                 - device number.
* @param[in] enable                 - status of the mode.
*                                       - GT_TRUE: to enable CNC Hash counting
*                                       - GT_FALSE: to disable CNC Hash counting
*
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - wrong device.
* @retval GT_HW_ERROR                 - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCncIngressPacketTypeClientHashModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
    fieldOffset = 0;
    fieldLength = 1;
    value = BOOL2BIT_MAC(enable);
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, value);
}

/**
* @internal cpssDxChCncIngressPacketTypeClientHashModeEnableSet function
* @endinternal
*
* @brief  Enable/disable Hash mode for CNC Packet Type Cient.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @param[in] devNum                 - device number.
* @param[in] enable                 - status of the mode.
*                                       - GT_TRUE: to enable CNC Hash counting
*                                       - GT_FALSE: to disable CNC Hash counting
*
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - wrong device.
* @retval GT_HW_ERROR                 - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncIngressPacketTypeClientHashModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncIngressPacketTypeClientHashModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCncIngressPacketTypeClientHashModeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCncIngressPacketTypeClientHashModeEnableGet function
* @endinternal
*
* @brief  Get Hash mode for CNC Packet Type Cient.
*
* @note APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[out] enablePtr         - (pointer to) status of the mode.
*                                   -GT_TRUE: CNC Hash counting enabled
*                                   -GT_FALSE: CNC Hash counting disabled
* @retval GT_OK                 - on success.
* @retval GT_BAD_PARAM          - wrong device or mode.
* @retval GT_HW_ERROR           - reading HW error
* @retval GT_BAD_PTR            - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCncIngressPacketTypeClientHashModeEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write from register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
    fieldOffset = 0;
    fieldLength = 1;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = (value) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}


/**
* @internal cpssDxChCncIngressPacketTypeClientHashModeEnableGet function
* @endinternal
*
* @brief  Get Hash mode for CNC Packet Type Cient.
*
* @note APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[out] enablePtr         - (pointer to) status of the mode.
*                                   -GT_TRUE: CNC Hash counting enabled
*                                   -GT_FALSE: CNC Hash counting disabled
*
* @retval GT_OK                 - on success.
* @retval GT_BAD_PARAM          - wrong device or mode.
* @retval GT_HW_ERROR           - reading HW error
* @retval GT_BAD_PTR            - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncIngressPacketTypeClientHashModeEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncIngressPacketTypeClientHashModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCncIngressPacketTypeClientHashModeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCncPortHashClientEnableSet function
* @endinternal
*
* @brief    Enable/disable CNC Hash counting for specific port.
*           Configures 'Flow Track Enable' field.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - port number
* @param[in] enable                 - status of the mode.
*                                       - GT_TRUE: to enable port CNC Hash counting
*                                       - GT_FALSE: to disable port CNC Hash counting
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCncPortHashClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_BOOL                   enable
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;    /* value to write to register   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    hwValue = BOOL2BIT_MAC(enable);

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            portNum,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);

    return rc;
}

/**
* @internal cpssDxChCncPortHashClientEnableSet function
* @endinternal
*
* @brief    Enable/disable CNC Hash counting for specific port.
*           Configures 'Flow Track Enable' field.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - status of the mode.
*                                       - GT_TRUE: to enable port CNC Hash counting
*                                       - GT_FALSE: to disable port CNC Hash counting
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortHashClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_BOOL                   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortHashClientEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCncPortHashClientEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortHashClientEnableGet function
* @endinternal
*
* @brief    Get the CNC Hash counting mode for specific port.
*           from 'Flow Track Enable' field.
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enable -             - (pointer to) status of the mode.
*                                       - GT_TRUE:  port CNC Hash counting enabled
*                                       - GT_FALSE: port CNC Hash counting disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCncPortHashClientEnableGet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    OUT   GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            portNum,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}


/**
* @internal cpssDxChCncPortHashClientEnableGet function
* @endinternal
*
* @brief    Get the CNC Hash counting status for specific port.
*           from 'Flow Track Enable' field.
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enable -             - (pointer to) status of the mode.
*                                       - GT_TRUE:  port CNC Hash counting enabled
*                                       - GT_FALSE: port CNC Hash counting disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortHashClientEnableGet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    OUT   GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortHashClientEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCncPortHashClientEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortQueueGroupBaseSet function
* @endinternal
*
* @brief    Configure Queue group base for specific port and CNC client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] portNum                    - port number
* @param[in] client                     - CNC client (APPLICABLE VALUES:
*                                                       CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E,
*                                                       CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E)
* @param[in] queueBase                  - Queue ID base value (APPLICABLE RANGES: 0-0x1FFF)
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range Queue ID base value
* @retval GT_BAD_PARAM              - on wrong device, port or client
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
static GT_STATUS internal_cpssDxChCncPortQueueGroupBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U32                      queueBase
)
{
    GT_STATUS   rc;
    SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_ENT   fieldName;
    GT_U32                                          groupOfQueuesIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    if (queueBase > 0x1FFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "cpssDxChCncPortQueueGroupBaseSet failed");
    }

    switch (client)
    {
        case (CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E): /* Queue Statistic Client */
            fieldName = SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_STATUS_PORT_BASE_E; /* field name */
            break;
        case (CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E): /* Transmit Queue Client */
            fieldName = SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_PASS_DROP_PORT_BASE_E; /* field name */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssDxChCncPortQueueGroupBaseSet failed");
            break;
    }

    /*convert physical port number to group of queues index ,let's read it from HW*/
    rc = prvCpssDxChTxqSip6GoQIndexGet(devNum,portNum,GT_TRUE,&groupOfQueuesIndex);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
            groupOfQueuesIndex,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            fieldName, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            queueBase);
    return rc;
}

/**
* @internal cpssDxChCncPortQueueGroupBaseSet function
* @endinternal
*
* @brief    Configure Queue group base for specific port and CNC client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] portNum                    - port number
* @param[in] client                     - CNC client (APPLICABLE VALUES:
*                                                       CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E,
*                                                       CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E)
* @param[in] queueBase                  - Queue ID base value (APPLICABLE RANGES: 0-0x1FFF)
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range Queue ID base value
* @retval GT_BAD_PARAM              - on wrong device, port or client
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
GT_STATUS cpssDxChCncPortQueueGroupBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U32                      queueBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortQueueGroupBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, queueBase, client));

    rc = internal_cpssDxChCncPortQueueGroupBaseSet(devNum, portNum, client, queueBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, client, queueBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCncPortQueueGroupBaseGet function
* @endinternal
*
* @brief    Get the Queue group base value for specific port and CNC client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
* @param[in] client         - CNC client (APPLICABLE VALUES
*                               CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E,
*                               CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E)
* @param[out] queueBasePtr  - (pointer to) Queue ID base value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, port or client
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on Null pointer
*/
static GT_STATUS internal_cpssDxChCncPortQueueGroupBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    OUT GT_U32                      *queueBasePtr
)
{
    GT_STATUS   rc;
    SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_ENT   fieldName;
    GT_U32                                          groupOfQueuesIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(queueBasePtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch (client)
    {
        case (CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E):
            fieldName = SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_STATUS_PORT_BASE_E; /* field name */
            break;
        case (CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E):
            fieldName = SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_PASS_DROP_PORT_BASE_E; /* field name */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssDxChCncPortQueueGroupBaseSet failed");
            break;
    }
    /*convert physical port number to group of queues index ,let's read it from HW*/
    rc = prvCpssDxChTxqSip6GoQIndexGet(devNum,portNum,GT_TRUE,&groupOfQueuesIndex);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
            groupOfQueuesIndex,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            fieldName, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            queueBasePtr);
    return rc;
}

/**
* @internal cpssDxChCncPortQueueGroupBaseGet function
* @endinternal
*
* @brief    Get the Queue group base value for specific port and CNC client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
* @param[in] client         - CNC client (APPLICABLE VALUES
*                               CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E,
*                               CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E)
* @param[out] queueBasePtr  - (pointer to) Queue ID base value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, port or client
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on Null pointer
*/
GT_STATUS cpssDxChCncPortQueueGroupBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    OUT GT_U32                      *queueBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCncPortQueueGroupBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, client, queueBasePtr));

    rc = internal_cpssDxChCncPortQueueGroupBaseGet(devNum, portNum, client, queueBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, client, queueBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
