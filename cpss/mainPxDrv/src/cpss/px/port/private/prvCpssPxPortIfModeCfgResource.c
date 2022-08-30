/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxPortIfModeCfgResource.c
*
* @brief CPSS implementation for port resource configuration.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define INVALID_PORT_CNS  0xFFFF
/**
* @internal prvCpssPxPortMappingCPUPortGet function
* @endinternal
*
* @brief   Function checks and returns the number of SDMA CPU ports
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuPortNumPtr            -  array with the physical port bumbers of the SDMA CPU ports
* @param[out] numOfCpuPorts            - (pointer to) the number of SDMA CPU ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMappingCPUPortGet
(
     IN  GT_SW_DEV_NUM        devNum,
     OUT GT_PHYSICAL_PORT_NUM *cpuPortNumPtr, /*array of size CPSS_MAX_CPU_PORTS_CNS*/
     OUT GT_U32               *numOfCpuPorts
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_U32 index;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuPortNumPtr);

    maxPortNum = PRV_CPSS_PX_PORTS_NUM_CNS;

    index = 0;

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        /* find local physical port number of CPU port */
        rc =  prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == portMapShadowPtr->valid)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                cpuPortNumPtr[index] = portNum;
                index++;
            }
        }
    }
    *numOfCpuPorts = index;
    return GT_OK;
}

/* */


/*
//    +------------------------------------------------------------------------------------------------------------------------------+
//    |                             Port resources                                                                                   |
//    +----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------+-----------+
//    |    |      |          |       |              |                    |RXDMA|      TXDMA SCDMA            | TX-FIFO   |Eth-TX-FIFO|
//    |    |      |          |       |              |                    |-----|-----+-----+-----+-----------|-----+-----|-----+-----|
//    | #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF | TxQ |Burst|Burst| TX-FIFO   | Out | Pay | Out | Pay |
//    |    |      |          |       |              |                    |Width|Descr| All | Full|-----+-----|Going| Load|Going| Load|
//    |    |      |          |       |              |                    |     |     | Most|Thrsh| Hdr | Pay | Bus |Thrsh| Bus |Thrsh|
//    |    |      |          |       |              |                    |     |     | Full|     |Thrsh|Load |Width|     |Width|     |
//    |    |      |          |       |              |                    |     |     |Thrsh|     |     |Thrsh|     |     |     |     |
//    +----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//    | 16 |   16 | CPU-SDMA | 1G    | ------------ |    16  16    16 -1 | 64b |   2 |   2 |   2 |   2 |   4 |  1B |   2 |  -- |  -- |  As 1G
//    +----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------+-----------+

CPU  TxFIFO out-going-bus (DMA 16) shall be configured as 8byte

Regarding the TxDMA �Burst Almost Full Thr� and �Burst Full Thr� the configuration is irrelevant if we aren�t enabling the feature
/Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1
    http://cideril.marvell.com:81/Cider/link.php?str=11642643,7
    0x02003010 + p*0x20 : where p (0-16) represents SCDMA, bits 0-9
    (Enable Burst Limit SCDMA %p).
*/


typedef struct
{
    CPSS_PORT_SPEED_ENT speedEnm;
    GT_U32              speedMbps;
}PRV_CPSS_PX_SPEED_ENM_SPEED_Mbps_STC;

/**
* @internal prvCpssPxPortResourcesSpeedIndexGet function
* @endinternal
*
* @brief   Gets speed index in database
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] speed                    - port interface speed
*
* @param[out] speedIndexPtr            - (pointer to) speed DB
* @param[out] speedValuePtr            - (pointer to) speed value in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesSpeedIndexGet
(
    IN  CPSS_PORT_SPEED_ENT  speed,
    OUT GT_U32               *speedIndexPtr,
    OUT GT_U32               *speedValuePtr
)
{
    GT_U32                         i;
    GT_U32                         speedMbps;
    GT_U32                         speedIdx;
    const PRV_CPSS_PX_SPEED_PLACE_STC *tablePtr;

    CPSS_NULL_PTR_CHECK_MAC(speedIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedValuePtr);

    tablePtr = &(prv_PIPE_speedPlaceArr[0]);

    speedMbps = 0;
    speedIdx = 0;
    for (i = 0 ; (tablePtr[i].speedEnm != CPSS_PORT_SPEED_NA_E); i++)
    {
        if (tablePtr[i].speedEnm == speed)
        {
            speedMbps = tablePtr[i].speedMbps;
            speedIdx = tablePtr[i].placeInArr;
            break;
        }
    }
    if (speedIdx == 0) /* not found */
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *speedIndexPtr = speedIdx;
    *speedValuePtr = speedMbps;

    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesClockIndexGet function
* @endinternal
*
* @brief   Gets core clock index in database
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] coreClock                - device core clock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong clock
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortResourcesClockIndexGet
(
    IN  GT_U32          coreClock,
    OUT GT_U32          *clockIndexPtr
)
{
    GT_U32 i;
    const PRV_CPSS_PX_CORECLOCK_PLACE_STC * tablePtr;

    CPSS_NULL_PTR_CHECK_MAC(clockIndexPtr);

    tablePtr = &(prv_PIPE_coreClockPlaceArr[0]);

    for (i = 0 ; tablePtr[i].real_coreClockMHz != 0 ; i++)
    {
        if (tablePtr[i].real_coreClockMHz == coreClock)
        {
            *clockIndexPtr = tablePtr[i].placeInArr;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

typedef struct
{
    CPSS_PORT_SPEED_ENT                         speed;
    PRV_CPSS_PX_RXDMA_IfWidth_ENT               rxDmaIfWidth;
    PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_ENT     txFifoOutGoingBusWidth;
}PRV_CPSS_PX_SPEED_2_RES_STC;

PRV_CPSS_PX_SPEED_2_RES_STC prv_pipe_speed2resourceInitList[] =
{
      {  CPSS_PORT_SPEED_1000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_2500_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_5000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_10000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_11800_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_12500_E ,  PRV_CPSS_PX_RXDMA_IfWidth_64_E  , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_20000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_23600_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_25000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_26700_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_40000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_47200_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_50000_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_52500_E ,  PRV_CPSS_PX_RXDMA_IfWidth_256_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E }
     ,{  CPSS_PORT_SPEED_100G_E ,  PRV_CPSS_PX_RXDMA_IfWidth_512_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_102G_E ,  PRV_CPSS_PX_RXDMA_IfWidth_512_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_107G_E ,  PRV_CPSS_PX_RXDMA_IfWidth_512_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E }
     ,{    CPSS_PORT_SPEED_NA_E ,  PRV_CPSS_PX_RXDMA_IfWidth_MAX_E , PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_MAX_E }
};

/**
* @internal prvCpssPxPortResourcesConfigStaticBusWidthGet function
* @endinternal
*
* @brief   Gets static width of SDMA incoming and outgoing buses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @param[out] rxdmaScdmaIncomingBusWidthPtr - (pointer to) Incoming Bus Width
* @param[out] txfifoScdmaShiftersOutgoingBusWidthPtr - (pointer to) Outgoing Bus Width
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong clock
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigStaticBusWidthGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT speed,
    OUT GT_U32              *rxdmaScdmaIncomingBusWidthPtr,
    OUT GT_U32              *txfifoScdmaShiftersOutgoingBusWidthPtr
)
{
    CPSS_PX_DETAILED_PORT_MAP_STC    *portMapShadowPtr;
    PRV_CPSS_PX_SPEED_2_RES_STC      *listPtr;
    GT_U32                          coreClkDb;
    GT_U32 i;
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD    portMode;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    listPtr = &prv_pipe_speed2resourceInitList[0];

    for (i = 0 ; listPtr[i].speed != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (listPtr[i].speed == speed)
        {
            *rxdmaScdmaIncomingBusWidthPtr          = listPtr[i].rxDmaIfWidth;
            *txfifoScdmaShiftersOutgoingBusWidthPtr = listPtr[i].txFifoOutGoingBusWidth;

            /*--------------------------------------------------------------------*
             *  correct for CPU txFifo-outgoing bus width DMA has width 8Bytes *
             *--------------------------------------------------------------------*/
            if (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                *txfifoScdmaShiftersOutgoingBusWidthPtr = PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E;
            }
            else
            {
                rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMapShadowPtr->portMap.macNum, portMode, &curPortParams);
                if (GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in hwsPortModeParamsGetToBuffer, portMacMap = %d\n", portMapShadowPtr->portMap.macNum);
                }

                if (curPortParams.numOfActLanes == 1)
                {
                    switch (speed)
                    {
                    case CPSS_PORT_SPEED_20000_E:
                    case CPSS_PORT_SPEED_23600_E:
                    case CPSS_PORT_SPEED_25000_E:
                    case CPSS_PORT_SPEED_26700_E:
                        rc = cpssPxHwCoreClockGet(devNum, &coreClkDb);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                        if (coreClkDb >= 450)
                        {
                            /* Tx DMA bus width to 64b */
                            *txfifoScdmaShiftersOutgoingBusWidthPtr = PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/* just for test , manual configuration */
typedef struct
{
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_SPEED_ENT             speed;
    GT_U32                          txqCredits;
}PORT_MANULAL_TXQ_CREDIT_STC;


PORT_MANULAL_TXQ_CREDIT_STC prv_px_txqPortManualCredit = { (GT_U32)(~0),(CPSS_PORT_SPEED_ENT)(-1), 0};

GT_STATUS prvCpssPx_Port_TxQManualCreditSet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_U32                          txqCredits
)
{
    prv_px_txqPortManualCredit.portNum     = portNum;
    prv_px_txqPortManualCredit.speed       = speed;
    prv_px_txqPortManualCredit.txqCredits  = txqCredits;
    return GT_OK;
}

/**
* @internal prvCpssPxPortResourceRegAddrGet function
* @endinternal
*
* @brief   Pipr port resourse registers address get
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - port field code name
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] fieldOffsetPtr           - (pointer to) register field offset
* @param[out] fieldLenPtr              - (pointer to) register field length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortResourceRegAddrGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CPSS_PX_REG_FIELDS_ENT  fieldCode,
    OUT GT_U32                      *regAddrPtr,
    OUT GT_U32                      *fieldOffsetPtr,
    OUT GT_U32                      *fieldLenPtr
)
{
    GT_STATUS   rc;                  /* return code */
    GT_U32      rxDmaOnDP;           /* local rxDma index for RxDMA port */
    GT_U32      txDmaOnDP;           /* local txDma index for TxDMA port */
    GT_U32      txFifoOnDP;          /* local rxFifo index for TxFIFO port */
    GT_U32      txqDqIdx;            /* txqDq idx */
    GT_U32      txqNumOnDQ;          /* txq on Dq */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldLenPtr);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rxDmaOnDP     = portMapShadowPtr->portMap.dmaNum;
    txDmaOnDP     = portMapShadowPtr->portMap.dmaNum;
    txFifoOnDP    = portMapShadowPtr->portMap.dmaNum;

    rc = prvCpssPxPpResourcesTxqGlobal2LocalConvert(
        devNum,portMapShadowPtr->portMap.txqNum,/*OUT*/&txqDqIdx,&txqNumOnDQ);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch(fieldCode)
    {
        case PRV_CPSS_PX_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E:
            *regAddrPtr =
                PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.portDescCntr[portNum];

            *fieldOffsetPtr = 0;
            *fieldLenPtr = 16;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E:

            if (CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS == txqNumOnDQ)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *regAddrPtr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,txqDqIdx).
                global.creditCounters.txdmaPortCreditCounter[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 12;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E:

            if(CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS == txqNumOnDQ)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *regAddrPtr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,txqDqIdx).
                global.globalDQConfig.creditCountersReset;
            *fieldOffsetPtr = txDmaOnDP;
            *fieldLenPtr = 1;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMADebug.informativeDebug.SCDMAStatusReg1[txDmaOnDP];
            *fieldOffsetPtr = 10;
            *fieldLenPtr = 10;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMADebug.informativeDebug.SCDMAStatusReg1[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 10;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.SCDMAConfigs2[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 1;
            break;
        /*--------------------------------------------------*
         * resources                                        *
         *--------------------------------------------------*/
        case PRV_CPSS_PX_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_RXDMA_MAC(devNum).
                singleChannelDMAConfigs.SCDMAConfig0[rxDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 3;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 9;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 10;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[txDmaOnDP];
            *fieldOffsetPtr = 10;
            *fieldLenPtr = 10;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.burstLimiterSCDMA[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 16;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.burstLimiterSCDMA[txDmaOnDP];
            *fieldOffsetPtr =16;
            *fieldLenPtr = 16;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.SCDMAConfigs1[txDmaOnDP];
            *fieldOffsetPtr =20;
            *fieldLenPtr   = 10;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr    = 32;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAGlobalConfigs.srcSCDMASpeedTbl[txDmaOnDP/8];
            *fieldOffsetPtr = (txDmaOnDP%8)*4;
            *fieldLenPtr = 4;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).
                txDMAPerSCDMAConfigs.SCDMAConfigs1[txDmaOnDP];
            *fieldOffsetPtr = 16;
            *fieldLenPtr = 4;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TX_FIFO_MAC(devNum).
                txFIFOGlobalConfig.SCDMAPayloadThreshold[txFifoOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 7;
            break;

        case PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_TX_FIFO_MAC(devNum).
                txFIFOShiftersConfig.SCDMAShiftersConf[txFifoOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 3;
            break;

        case PRV_CPSS_PX_REG_FIELDS_XLG_MAC_FIFO_TX_WRITE_THRESHOLD_E:
            if (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if(PRV_CPSS_GE_PORT_GE_ONLY_E !=
                   PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, portMapShadowPtr->portMap.macNum))
                {
                    *regAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                        perPortRegs[portMapShadowPtr->portMap.macNum].xlgPortFIFOsThresholdsConfig;
                    *fieldOffsetPtr = 11;
                    *fieldLenPtr    = 5;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(
                        GT_BAD_PARAM, "required XLG_MAC refister for GE_ONLY port");
                }
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxResourcePortRegFieldSet function
* @endinternal
*
* @brief   Pipe port resourse register field set
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - field code name
* @param[in] fieldValue               - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxResourcePortRegFieldSet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  PRV_CPSS_PX_REG_FIELDS_ENT   fieldCode,
    IN  GT_U32                       fieldValue
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/

    rc = prvCpssPxPortResourceRegAddrGet(
        devNum, portNum, fieldCode,
        &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLen, fieldValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxResourcePortRegFieldGet function
* @endinternal
*
* @brief   Pipe port resourse register field get
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - field code name
*
* @param[out] fieldValuePtr            - (pointer to) field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssPxResourcePortRegFieldGet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  PRV_CPSS_PX_REG_FIELDS_ENT   fieldCode,
    OUT GT_U32                       *fieldValuePtr
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/

    CPSS_NULL_PTR_CHECK_MAC(fieldValuePtr);

    rc = prvCpssPxPortResourceRegAddrGet(
        devNum, portNum, fieldCode,
        &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLen, fieldValuePtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/* convert values for debug prints */
char* PRV_CPSS_PX_REG_FIELDS_enum_value_to_name
(
    IN PRV_CPSS_PX_REG_FIELDS_ENT enum_value
)
{
    static struct
    {
        PRV_CPSS_PX_REG_FIELDS_ENT   fieldCode;
        char*                        fieldName;
    } fieldsArr[] =
    {
        #define FLD_PAIR(_n){PRV_CPSS_PX_REG_FIELDS_##_n, #_n}
        FLD_PAIR(RXDMA_SCDMA_INCOMING_BUS_WIDTH_E),
        FLD_PAIR(TXQ_Q_PORT_DESC_COUNTER_E),
        FLD_PAIR(TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E),
        FLD_PAIR(TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E),
        FLD_PAIR(TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E),
        FLD_PAIR(TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E),
        FLD_PAIR(TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E),
        FLD_PAIR(TXDMA_DESC_CREDITS_SCDMA_E),
        FLD_PAIR(TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E),
        FLD_PAIR(TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E),
        FLD_PAIR(TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E),
        FLD_PAIR(TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E),
        FLD_PAIR(TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E),
        FLD_PAIR(TXDMA_SCDMA_BURST_FULL_THRESHOLD_E),
        FLD_PAIR(TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E),
        FLD_PAIR(TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E),
        FLD_PAIR(TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E),
        FLD_PAIR(TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E),
        FLD_PAIR(XLG_MAC_FIFO_TX_WRITE_THRESHOLD_E)
        #undef FLD_PAIR
    };
    static GT_U32 fieldsArrSize = (sizeof(fieldsArr) / sizeof(fieldsArr[0]));
    GT_U32 i;

    for (i = 0; (i < fieldsArrSize); i++)
    {
        if (fieldsArr[i].fieldCode == enum_value) return fieldsArr[i].fieldName;
    }
    return "unknown";
}

/**
* @internal prvCpssPxResourcePortRegFieldsDebugDump function
* @endinternal
*
* @brief   Pipe port resourse register fields dump
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssPxResourcePortRegFieldsDebugDump
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/
    GT_U32      fieldValue;     /*field value*/
    PRV_CPSS_PX_REG_FIELDS_ENT      field; /* field enum value */
    char*       name;           /* fiels name */

    for (field = 0; (field < PRV_CPSS_PX_REG_FIELDS_LAST); field++)
    {
        name = PRV_CPSS_PX_REG_FIELDS_enum_value_to_name(field);
        rc = prvCpssPxPortResourceRegAddrGet(
            devNum, portNum, field,
            &regAddr, &fieldOffset, &fieldLen);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "Error getting register address for %s\n",
                name);
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(
            CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLen, &fieldValue);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "Error reading register rc = 0x%X for %s\n",
                rc, name);
            continue;
        }
        cpssOsPrintf(
            "<0x%08X-%02d/%02d> 0x%08X %s\n",
            regAddr, fieldOffset, fieldLen, fieldValue, name);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxResourceAllPortsRegFieldsDebugDump function
* @endinternal
*
* @brief   Pipe all ports resourse register fields dump
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssPxResourceAllPortsRegFieldsDebugDump
(
    IN  GT_SW_DEV_NUM                devNum
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/
    GT_U32      fieldValue;     /*field value*/
    char*       name;           /* fiels name */
    PRV_CPSS_PX_REG_FIELDS_ENT    field;             /* field enum value    */
    GT_PHYSICAL_PORT_NUM          portNum;           /* port number         */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr; /* port DB entry       */
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;            /* port interface mode */
    CPSS_PORT_SPEED_ENT           speed;             /* port speed          */

    for (field = 0; (field < PRV_CPSS_PX_REG_FIELDS_LAST); field++)
    {
        name = PRV_CPSS_PX_REG_FIELDS_enum_value_to_name(field);
        cpssOsPrintf("field %s\n", name);
        for (portNum = 0 ; (portNum < PRV_CPSS_PX_PORTS_NUM_CNS); portNum++)
        {
            rc =  prvCpssPxPortPhysicalPortMapShadowDBGet(
                devNum,portNum, /*OUT*/&portMapShadowPtr);
            if (rc != GT_OK) { continue; }
            if (GT_FALSE == portMapShadowPtr->valid) { continue; }
            if (portMapShadowPtr->portMap.mappingType
                != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                rc = cpssPxPortInterfaceModeGet(
                    devNum, portNum, &ifMode);
                if (rc != GT_OK) { continue; }
                rc = cpssPxPortSpeedGet(
                    devNum, portNum,/*OUT*/&speed);
                if (rc != GT_OK) { continue; }
                if (speed == CPSS_PORT_SPEED_NA_E) { continue; }
            }

            rc = prvCpssPxPortResourceRegAddrGet(
                devNum, portNum, field,
                &regAddr, &fieldOffset, &fieldLen);
            if (rc != GT_OK)
            {
                cpssOsPrintf(
                    "Error getting register address for %s\n",
                    name);
                continue;
            }
            rc = prvCpssDrvHwPpGetRegField(
                CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLen, &fieldValue);
            if (rc != GT_OK)
            {
                cpssOsPrintf(
                    "Error reading register rc = 0x%X for %s\n",
                    rc, name);
                continue;
            }
            cpssOsPrintf(
                "port 0x%02X <0x%08X-%02d/%02d> 0x%08X\n",
                portNum, regAddr, fieldOffset, fieldLen, fieldValue);
        }
    }
    return GT_OK;
}

GT_BOOL g_px_doNotCheckCredits = GT_FALSE;

static PRV_CPSS_PX_RESOURCES_STATUS_STC  * prvCpssPxPortResourcesConfigDbGet
(
    GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    resourcesStatusPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus);
    return resourcesStatusPtr;
}

/**
* @internal prvCpssPxPortResourcesConfigDbInit function
* @endinternal
*
* @brief   Resource data structure initialization
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] maxDescCredits           - physical device number
* @param[in] maxHeaderCredits         - physical device number
* @param[in] maxPayloadCredits        - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbInit
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          maxDescCredits,
    IN  GT_U32          maxHeaderCredits,
    IN  GT_U32          maxPayloadCredits
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if(dpIndex >= PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);

    resourcesStatusPtr->usedDescCredits[dpIndex]         = 0;
    resourcesStatusPtr->maxDescCredits[dpIndex]          = maxDescCredits;
    resourcesStatusPtr->usedHeaderCredits[dpIndex]       = 0;
    resourcesStatusPtr->maxHeaderCredits[dpIndex]        = maxHeaderCredits;
    resourcesStatusPtr->usedPayloadCredits[dpIndex]      = 0;
    resourcesStatusPtr->maxPayloadCredits[dpIndex]       = maxPayloadCredits;
    resourcesStatusPtr->coreOverallSpeedSummary[dpIndex] = 0;

    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesConfigDbAdd function
* @endinternal
*
* @brief   Resource data structure DB add operation.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      speedValueMbps      - speed Mbps
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbAdd
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          txQDescrCredits,
    IN  GT_U32          txFifoHeaderCredits,
    IN  GT_U32          txFifoPayloadCredits
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);

    resourcesStatusPtr->usedDescCredits        [dpIndex] += txQDescrCredits;
    resourcesStatusPtr->usedHeaderCredits      [dpIndex] += txFifoHeaderCredits;
    resourcesStatusPtr->usedPayloadCredits     [dpIndex] += txFifoPayloadCredits;

    return GT_OK;
}


/**
* @internal prvCpssPxPortResourcesConfigDbDelete function
* @endinternal
*
* @brief   Resource data structure DB delete operation.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      speedValueMbps      - speed Mbps
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          txQDescrCredits,
    IN  GT_U32          txFifoHeaderCredits,
    IN  GT_U32          txFifoPayloadCredits
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);

    resourcesStatusPtr->usedDescCredits        [dpIndex] -= txQDescrCredits;
    resourcesStatusPtr->usedHeaderCredits      [dpIndex] -= txFifoHeaderCredits;
    resourcesStatusPtr->usedPayloadCredits     [dpIndex] -= txFifoPayloadCredits;

    return GT_OK;
}


GT_STATUS prvCpssPxPortResourcesConfigDbCoreOverallSpeedAdd
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          bandwidthMbps
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);
    resourcesStatusPtr->coreOverallSpeedSummary[dpIndex] += bandwidthMbps;
    return GT_OK;
}

GT_STATUS prvCpssPxPortResourcesConfigDbCoreOverallSpeedDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          bandwidthMbps
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);

    if (resourcesStatusPtr->coreOverallSpeedSummary[dpIndex] < bandwidthMbps)
    {
        #ifdef GM_USED
            return GT_OK;
        #else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        #endif
    }
    resourcesStatusPtr->coreOverallSpeedSummary[dpIndex] -= bandwidthMbps;
    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesConfigDbAvailabilityCheck function
* @endinternal
*
* @brief   Resource data structure limitations checks
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      txQDescCredits      - physical device number
* @param[in] txFifoHeaderCredits      - physical device number
* @param[in] txFifoPayloadCredits     - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbAvailabilityCheck
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          txQDescrCredits,
    IN  GT_U32          txFifoHeaderCredits,
    IN  GT_U32          txFifoPayloadCredits
)
{
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    if (dpIndex >= PRV_CPSS_PX_MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    resourcesStatusPtr = prvCpssPxPortResourcesConfigDbGet(devNum);

    if (g_px_doNotCheckCredits == GT_FALSE)  /* check credits */
    {
        if (txQDescrCredits + resourcesStatusPtr->usedDescCredits[dpIndex] > resourcesStatusPtr->maxDescCredits[dpIndex])
        {
            CPSS_LOG_INFORMATION_MAC(
                "\n--> ERROR : TXQ credit: number (%d) of credit exceeds limit (%d)\n",
                resourcesStatusPtr->usedDescCredits[dpIndex] + txQDescrCredits,
                resourcesStatusPtr->maxDescCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (txFifoHeaderCredits + resourcesStatusPtr->usedHeaderCredits[dpIndex] > resourcesStatusPtr->maxHeaderCredits[dpIndex])
        {
            CPSS_LOG_INFORMATION_MAC(
                "\n--> ERROR : TxFIFO headers credits: number (%d) of credit exceeds limit (%d)\n",
                resourcesStatusPtr->usedHeaderCredits[dpIndex] + txFifoHeaderCredits,
                resourcesStatusPtr->maxHeaderCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (txFifoPayloadCredits + resourcesStatusPtr->usedPayloadCredits[dpIndex] > resourcesStatusPtr->maxPayloadCredits[dpIndex])
        {
            CPSS_LOG_INFORMATION_MAC(
                "\n--> ERROR : TxFIFO payload credits: number (%d) of credit exceeds limit (%d)\n",
                resourcesStatusPtr->usedPayloadCredits[dpIndex] + txFifoPayloadCredits,
                resourcesStatusPtr->maxPayloadCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/*
From ResourcePerCoreClock_Compute.m

function  rateLimitIntegerPart = TXDMA_SCDMA_RateLimitIntegerPart(CORE_FREQ_MHz, port_speed_Gbps)
    port_speed_Mbps = port_speed_Gbps *1000;
    res = 128*8*CORE_FREQ_MHz/port_speed_Mbps;
    rateLimitIntegerPart = floor(res);
end

function  rateLimitResidueVector = TXDMA_SCDMA_RateLimitResidueVector(CORE_FREQ_MHz, port_speed_Gbps)
    port_speed_Mbps = port_speed_Gbps *1000;
    res = 128*8*CORE_FREQ_MHz/port_speed_Mbps;
    rateLimitResidueVector = floor((res-floor(res))*32);
end
*/

static GT_U32 prvCpssPxPipePortResources_TXDMA_SCDMA_RateLimitIntegerPart
(
    IN GT_U32      speedValueMbps,
    IN GT_U32      coreClockInMHz
)
{
    return ((128 * 8 * coreClockInMHz) / speedValueMbps);
}

static GT_U32 prvCpssPxPipePortResources_TXDMA_SCDMA_RateLimitResidueVector
(
    IN GT_U32      speedValueMbps,
    IN GT_U32      coreClockInMHz
)
{
    static GT_U32 enum_arr[] =
    {
        DIV_0_32,  DIV_1_32,  DIV_2_32,  DIV_3_32,  DIV_4_32,  DIV_5_32,  DIV_6_32,  DIV_7_32,
        DIV_8_32,  DIV_9_32,  DIV_10_32, DIV_11_32, DIV_12_32, DIV_13_32, DIV_14_32, DIV_15_32,
        DIV_16_32, DIV_17_32, DIV_18_32, DIV_19_32, DIV_20_32, DIV_21_32, DIV_22_32, DIV_23_32,
        DIV_24_32, DIV_25_32, DIV_26_32, DIV_27_32, DIV_28_32, DIV_29_32, DIV_30_32, DIV_31_32
    };
    GT_U32 rate_limit_mul32 =  ((32 * 128 * 8 * coreClockInMHz) / speedValueMbps);
    return enum_arr[rate_limit_mul32 % 32];
}

static GT_U32 prvCpssPxPipePortResources_XLG_MAC_TxReadThreshold
(
    IN GT_U32      speedValueMbps
)
{
    /* copied fro BC3_XLG_MAC_TxReadThreshold_Arr */
    return ((speedValueMbps < 1000000) ? 17 : 3);
}

/* same conversion used for both TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL */
/* and TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E fields                  */
static GT_U32 prvCpssPxPipePortResourcesSpeedToTxDmaScdmaSpeedIndex
(
    IN GT_U32 speedInMbps
)
{
    struct {
        GT_U32 speedIndex;
        GT_U32 speedValue;
    } speedValuesArr[] =
    {
        /*  values not described in Cider: */
        /*  11800 treated as  12000        */
        /* 107000 treated as 110000        */
        {0,   1000}, {1,    2500},  {2,    5000}, {3,   10000}, {4,   11800},
        {4,  12000}, {5,   20000},  {6,   24000}, {7,   25000},
        {8,  30000}, {9,   40000},  {10,  48000}, {11,  50000},
        {12, 60000}, {13, 100000},  {14, 107000}, {14, 110000}, {15, 120000}
    };
    static GT_U32 speedValuesArrSize = (sizeof(speedValuesArr) / sizeof(speedValuesArr[0]));
    GT_U32 i;

    for (i = 0; (i < speedValuesArrSize); i++)
    {
        if (speedInMbps <= speedValuesArr[i].speedValue) return speedValuesArr[i].speedIndex;
    }
    return speedValuesArr[speedValuesArrSize - 1].speedIndex; /* not found - use maximal index */
}

/**
* @internal prvCpssPxPipePortResourcesConfigStatic function
* @endinternal
*
* @brief   Pipe port resourse static configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPipePortResourcesConfigStatic
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaCreditValue;   /* TXDMA desscriptor credit number */
    GT_U32      txfifoHeaderCounterThresholdScdma;
    GT_U32      txfifoPayloadCounterThresholdScdma;
    GT_U32      txfifoScdmaPayloadThreshold;
    GT_U32      txfifoScdmaBurstFullThrshold;
    GT_U32      txfifoScdmaBurstAmostFullThrshold;
    GT_U32      txdmaScdmaRateLimitIntegerThr;
    GT_U32      txdmaScdmaRateLimitResidueVectorThr;
    GT_U32      txfifoScdmaShiftersOutgoingBusWidth;
    GT_U32      rxdmaScdmaIncomingBusWidth;
    GT_U32      xlgMacTxReadThresh;
    GT_U32      speedValueMbps;
    GT_U32      speedIndex;         /* index in speed DB */
    GT_U32      coreClockIndex;     /* index in clock DB */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32      dpIndexTxqDq;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* system recovery info*/
    ifMode = ifMode;

    /* Get Port Mapping DataBase */
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(
        devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    dpIndexTxqDq =
        (portMapShadowPtr->portMap.txqNum
         / PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp);

    /* convert port speed to DB index and speed value in Mbps */
    rc = prvCpssPxPortResourcesSpeedIndexGet(
        speed, &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* convert core clock to DB index */
    rc = prvCpssPxPortResourcesClockIndexGet(
        PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortResourcesConfigStaticBusWidthGet(
        devNum,portNum,ifMode,speed,/*OUT*/&rxdmaScdmaIncomingBusWidth,&txfifoScdmaShiftersOutgoingBusWidth);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    txdmaCreditValue                     = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txfifoHeaderCounterThresholdScdma    = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txfifoPayloadCounterThresholdScdma   = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txfifoScdmaPayloadThreshold          = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txfifoScdmaBurstFullThrshold         = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txfifoScdmaBurstAmostFullThrshold    = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txdmaScdmaRateLimitIntegerThr        = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    txdmaScdmaRateLimitResidueVectorThr  = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    xlgMacTxReadThresh                   = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;

    txdmaCreditValue                     = PIPE_TXDMA_SCDMA_TxQDescriptorCredit_Arr                   [coreClockIndex][speedIndex];
    txfifoHeaderCounterThresholdScdma    = PIPE_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr           [coreClockIndex][speedIndex];
    txfifoPayloadCounterThresholdScdma   = PIPE_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr          [coreClockIndex][speedIndex];
    txfifoScdmaPayloadThreshold          = PIPE_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr          [coreClockIndex][speedIndex];
    txfifoScdmaBurstFullThrshold         = PIPE_TXDMA_SCDMA_burstFullThreshold_Arr                    [coreClockIndex][speedIndex];
    txfifoScdmaBurstAmostFullThrshold    = PIPE_TXDMA_SCDMA_burstAlmostFullThreshold_Arr              [coreClockIndex][speedIndex];
    txdmaScdmaRateLimitIntegerThr        =
        prvCpssPxPipePortResources_TXDMA_SCDMA_RateLimitIntegerPart(
            speedValueMbps, PRV_CPSS_PP_MAC(devNum)->coreClock);
    txdmaScdmaRateLimitResidueVectorThr =
        prvCpssPxPipePortResources_TXDMA_SCDMA_RateLimitResidueVector(
            speedValueMbps, PRV_CPSS_PP_MAC(devNum)->coreClock);
    if (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        if(PRV_CPSS_GE_PORT_GE_ONLY_E !=
           PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, portMapShadowPtr->portMap.macNum))
        {
            xlgMacTxReadThresh            =
                prvCpssPxPipePortResources_XLG_MAC_TxReadThreshold(speedValueMbps);
        }
    }

    if ((prv_px_txqPortManualCredit.portNum == portNum)
        && (prv_px_txqPortManualCredit.speed) == speed)
    {
        txdmaCreditValue = prv_px_txqPortManualCredit.txqCredits;
    }

    rc = prvCpssPxPortResourcesConfigDbAvailabilityCheck(
        devNum, dpIndexTxqDq, txdmaCreditValue,
        txfifoHeaderCounterThresholdScdma, txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) && 
         (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) ||
        (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        /*
            1. TXDMA desc credits � each DMA, according to its speed, requires a certain amount of descriptors
               allocated space inside the TXDMA to be able to support FWS (desc_credits_scdma%p).

            TXDMA 1
            Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
            Fields:     BITS 0-8: Descriptor Credits SCDMA %p
            Limitation: SUM(DescriptorCreditsScdma_cc {PortSpeed,CoreFreq}) <= 144

        */

        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                              txdmaCreditValue);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E");
        }
        /*
            2. TXFIFO header and payload credits � each DMA, according to its speed,
               requires a certain amount of space allocated inside the TXDMA for both
               headers and payload to achieve FWS and avoid under-runs

            TXDMA 1
            Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
            Fields:     BITS 0-9: TxFIFO Header Counter Threshold SCDMA %p
                        BITS 10-19: TxFIFO Payload Counter Threshold SCDMA %p
            Limitation: SUM(Header {PortSpeed,CoreFreq}) <= 363
                        SUM(Payload {PortSpeed,CoreFreq}) <= 447

        */
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                              txfifoHeaderCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E");
        }

        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                              txfifoPayloadCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E");
        }

        /*----------------------------------------------------------------*
         *  burst almost full threhold and burts full threshold           *
         *     TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
                Fields:     BITS  0-15: TXDMA SCDMA burst full threhold %p
                            BITS 16-31: TXDMA SCDMA burst almost full threhold %p
         *----------------------------------------------------------------*/
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E,
                                              txfifoScdmaBurstFullThrshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E");
        }
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E,
                                              txfifoScdmaBurstAmostFullThrshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E");
        }
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
                                              txdmaScdmaRateLimitIntegerThr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E");
        }
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,
                                              txdmaScdmaRateLimitResidueVectorThr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E");
        }
        /* for CutThrough configurations  src and dst DMA speed */
        rc = prvCpssPxResourcePortRegFieldSet(
            devNum, portNum,
            PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E,
            prvCpssPxPipePortResourcesSpeedToTxDmaScdmaSpeedIndex(speedValueMbps));
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E");
        }

        rc = prvCpssPxResourcePortRegFieldSet(
            devNum, portNum,
            PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E,
            prvCpssPxPipePortResourcesSpeedToTxDmaScdmaSpeedIndex(speedValueMbps));
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E");
        }
        if (xlgMacTxReadThresh != PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS)
        {
            rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                                    PRV_CPSS_PX_REG_FIELDS_XLG_MAC_FIFO_TX_WRITE_THRESHOLD_E,xlgMacTxReadThresh);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_XLG_MAC_FIFO_TX_READ_THRESHOLD_E");
            }
        }

        /*
            3. Payload Threshold � to avoid under-runs due to variance in the
               latency of read responses from the MPPM � the TXFIFO is required
               to start transmitting the packet only after reaching a certain
               threshold. Each DMA speeds require a different threshold
               (scdma_%p_payload_threshold).
        */
        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
                                              txfifoScdmaPayloadThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /*
            4. TXFIFO Outgoing Bus Width � Each transmit DMA speed requires
               a different interface width to the GOP (outgoing_bus_%p_width).

            TXDMA 1
            Register:   <TXFIFO1_IP> TxFIFO IP Units/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
            Fields:     BITS 0-2: outgoing_bus_%p_width

            5. RXDMA Incoming Bus Width � Each receive DMA speed requires a different interface width to the GOP.

            TXDMA 1
            Register:   RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 0/if_width<<%n>>
            Fields:     BITS 0-2: outgoing_bus_%p_width
        */

        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
                                              txfifoScdmaShiftersOutgoingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                              PRV_CPSS_PX_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
                                              rxdmaScdmaIncomingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* update DB */
    rc = prvCpssPxPortResourcesConfigDbCoreOverallSpeedAdd(
        devNum, dpIndexTxqDq, speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortResourcesConfigDbAdd(
        devNum, dpIndexTxqDq, txdmaCreditValue,
        txfifoHeaderCounterThresholdScdma, txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesConfig function
* @endinternal
*
* @brief   Pipe port resourse configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfig
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc;                 /* return code */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL isSupported;

    if(speed >= CPSS_PORT_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* resources are computed on 1G base , so move the speed to 1G resolution */
    if (speed == CPSS_PORT_SPEED_10_E || speed == CPSS_PORT_SPEED_100_E)
    {
        speed = CPSS_PORT_SPEED_1000_E;
    }


    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        rc = prvCpssCommonPortInterfaceSpeedGet(
            CAST_SW_DEVNUM(devNum), portMapShadowPtr->portMap.macNum,
            ifMode,speed,/*OUT*/&isSupported);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (isSupported == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssPxPipePortResourcesConfigStatic(devNum, portNum, ifMode, speed);

    return rc;
}

typedef struct
{
    GT_U32 dpIdx;
    GT_U32 txqCredits;
    GT_U32 txFifoHeaders;
    GT_U32 txFifoPayloads;
}PRV_CPSS_PX_DP_RESOURCE_LIMIT_STC;

#define BAD_VALUE (GT_U32)(~0)
/* max for clock == 287 */
/* values from prvCpssPxPortIfModeCfgPipeResourceTables.c */
/* TXQ_CREDITS            for 10G -  9 for 25G - 14 */
#define PRV_TXQ_CREDITS_10G      9
#define PRV_TXQ_CREDITS_25G      14
/* TXFIFO_HEADERS_CREDITS for 10G - 12 for 25G - 20 */
#define PRV_TXFIFO_HEADERS_CREDITS_10G      12
#define PRV_TXFIFO_HEADERS_CREDITS_25G      20
/* TXFIFO_PAYLOAD_CREDITS for 10G - 14 for 25G - 23 */
#define PRV_TXFIFO_PAYLOAD_CREDITS_10G      12
#define PRV_TXFIFO_PAYLOAD_CREDITS_25G      20
/* DP0 - (10 * 10G) */
#define PRV_CPSS_PX_PIPE_DP0_TXQ_CREDITS_CNS               (10 * PRV_TXQ_CREDITS_10G)
#define PRV_CPSS_PX_PIPE_DP0_TXFIFO_HEADERS_CREDITS_CNS    (10 * PRV_TXFIFO_HEADERS_CREDITS_10G)
/* NOTE : use 13*10 instead of 12*10 because in 350CC the 10 ports of 10G need 13 each
    see PIPE_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr[]
    fix JIRA : CPSS-6400 : 350Mhz CC 10x10Gb cannot be initialized */
#define PRV_CPSS_PX_PIPE_DP0_TXFIFO_PAYLOAD_CREDITS_CNS    130
/* DP1 - ((2 * 10G) + (4 * 25G) + 10G)*/
#define PRV_CPSS_PX_PIPE_DP1_TXQ_CREDITS_CNS               \
    ((3 * PRV_TXQ_CREDITS_10G) + (4 * PRV_TXQ_CREDITS_25G))
#define PRV_CPSS_PX_PIPE_DP1_TXFIFO_HEADERS_CREDITS_CNS    \
    ((3 * PRV_TXFIFO_HEADERS_CREDITS_10G) + (4 * PRV_TXFIFO_HEADERS_CREDITS_25G))
#define PRV_CPSS_PX_PIPE_DP1_TXFIFO_PAYLOAD_CREDITS_CNS    \
    ((3 * PRV_TXFIFO_PAYLOAD_CREDITS_10G) + (4 * PRV_TXFIFO_PAYLOAD_CREDITS_25G))

PRV_CPSS_PX_DP_RESOURCE_LIMIT_STC pipe_dp_res[] =
{
     {         0,  PRV_CPSS_PX_PIPE_DP0_TXQ_CREDITS_CNS, PRV_CPSS_PX_PIPE_DP0_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_PX_PIPE_DP0_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         1,  PRV_CPSS_PX_PIPE_DP1_TXQ_CREDITS_CNS, PRV_CPSS_PX_PIPE_DP1_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_PX_PIPE_DP1_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,  BAD_VALUE,                           BAD_VALUE,                                          BAD_VALUE }
};

GT_STATUS prvCpssPxPipePortResourcesInit
(
    IN    GT_SW_DEV_NUM       devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_DP_RESOURCE_LIMIT_STC *listPtr;
    GT_U32 i;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);


    /*---------------------------------*/
    /* select list according to device */
    /*---------------------------------*/
    listPtr = &pipe_dp_res[0];

    for (i = 0 ; listPtr[i].dpIdx != BAD_VALUE; i++)
    {
        rc = prvCpssPxPortResourcesConfigDbInit(
            devNum, listPtr[i].dpIdx, listPtr[i].txqCredits,
            listPtr[i].txFifoHeaders, listPtr[i].txFifoPayloads);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "DP %d credits threshold config",listPtr[i].dpIdx);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesInit
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc = GT_OK; /*return code*/
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32 i, numOfCpuPorts;
    CPSS_PORT_SPEED_ENT cpuSpeed;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPipePortResourcesInit(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }
    rc = prvCpssPxPortMappingCPUPortGet(devNum,/*OUT*/cpuPortNumArr,&numOfCpuPorts);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; i < numOfCpuPorts; i++)
    {

         rc = prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet(devNum,/*OUT*/&cpuSpeed);
         if (rc != GT_OK)
         {
             return rc;
         }
         /* configure cpu port resources only if it mapped. It should be done also in Fast Boot*/
         /* cause cpu port resources could be not part of eeprom */
         rc = prvCpssPxPortResourcesConfig(
            devNum, cpuPortNumArr[i],
            CPSS_PORT_INTERFACE_MODE_NA_E,
            cpuSpeed);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
         }
    }

    return GT_OK;
}


/*--------------------------------------------------------------------
 *
 *--------------------------------------------------------------------
 */
GT_STATUS prvCpssPxPortPipeCreditsCheckSet(GT_BOOL val)
{
    switch (val)
    {
        case GT_FALSE:
            g_px_doNotCheckCredits = GT_TRUE;
            break;
        default:
            g_px_doNotCheckCredits = GT_FALSE;
            break;
    }
    CPSS_LOG_INFORMATION_MAC(
        "\nCredits control = %d\n",!g_px_doNotCheckCredits);
    return GT_OK;
}

/**
* @internal prvCpssPxPipePortResourcesReleaseStatic function
* @endinternal
*
* @brief   Remove static resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPipePortResourcesReleaseStatic
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaCreditValue;
    GT_U32      speedIndex;
    GT_U32      speedValueMbps;
    GT_U32      txfifoHeaderCounterThresholdScdma;
    GT_U32      txfifoPayloadCounterThresholdScdma;
    GT_U32      dpIndexTxqDq;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_PORT_SPEED_ENT speed;

    /* Get Port Mapping DataBase */
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    dpIndexTxqDq =
        (portMapShadowPtr->portMap.txqNum
         / PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp);

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
        &txdmaCreditValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
        &txfifoHeaderCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssPxResourcePortRegFieldGet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
        &txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /*
        1. In the TXQ - Reset the desc credits counters of the port (Credit_Counters_Reset_%n).
           (not self cleared - after reset should be restored to normal mode)
    */
    if (portMapShadowPtr->portMap.txqNum != GT_PX_NA) /* extended-cascade port has no TXQ , therefore no credit counter reset */
    {
        rc = prvCpssPxResourcePortRegFieldSet(
            devNum, portNum,
            PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
            1);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        /* Not self cleared - should be restored to 0 */
        rc = prvCpssPxResourcePortRegFieldSet(
            devNum, portNum,
            PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
            0);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    /*
        2. In the TXDMA - configure TXQ descriptor credits threshold to 0 to avoid
           sending credits to the TXQ (desc_credits_scdma%p).
    */

    rc = prvCpssPxResourcePortRegFieldSet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
        0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
        3. Reset the TXDMA scdma desc credit counter (reset_desc_credits_counter_scdma%p).
           (not self cleared - after reset should be restored to normal mode)
    */
    rc = prvCpssPxResourcePortRegFieldSet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
        1);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Not self cleared - should be restored to 0 */
    rc = prvCpssPxResourcePortRegFieldSet(
        devNum,portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
        0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* update DB (DP_1) */
    /* convert port speed to DB index and speed value in Mbps */
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMapShadowPtr->portMap.macNum);
    if (speed == CPSS_PORT_SPEED_29090_E)
    {
        speed = CPSS_PORT_SPEED_40000_E;
    }
    rc = prvCpssPxPortResourcesSpeedIndexGet(
        PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMapShadowPtr->portMap.macNum),
        &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortResourcesConfigDbCoreOverallSpeedDelete(
        devNum, dpIndexTxqDq, speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortResourcesConfigDbDelete(
        devNum, dpIndexTxqDq, txdmaCreditValue,
        txfifoHeaderCounterThresholdScdma,txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal prvCpssPxPortResourcesRelease function
* @endinternal
*
* @brief   Remove static resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesRelease
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
)
{
    return prvCpssPxPipePortResourcesReleaseStatic(devNum, portNum);
}

/**
* @internal prvCpssPxPortApHighLevelConfigurationSet function
* @endinternal
*
* @brief   Allocate/release high level port data path resources (pizza etc.) for
*         port where AP enabled, while physical port interface configuration made
*         by HWS in co-processor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portCreate               - GT_TRUE - port create;
*                                      GT_FALSE - port delete;
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortApHighLevelConfigurationSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         portCreate,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc; /* return code */
    GT_U32    portMacMap; /* number of mac mapped to this physical port */
    CPSS_PX_DETAILED_PORT_MAP_STC   *portMapShadowPtr;
    CPSS_PORT_INTERFACE_MODE_ENT    *portIfModePtr;
    CPSS_PORT_SPEED_ENT             *portSpeedPtr;
    GT_BOOL                         isCgUnitInUse;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacMap);

    portIfModePtr = &(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap));
    portSpeedPtr = &(PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap));

    isCgUnitInUse = GT_FALSE;
    rc = prvCpssPxIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portCreate == GT_FALSE)
    {
        rc = prvCpssPxPipePortResourcesReleaseStatic(devNum, portNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set FCA+PTP interface width */
        rc = prvCpssPxPortBusWidthSet(devNum, portNum, CPSS_PORT_SPEED_10_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssPxPortPizzaArbiterIfDelete(devNum, portNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;

        if(isCgUnitInUse == GT_TRUE)
        {
            /* update port DB with the default port type value in order to
                prevent using CG MAC when it is disabled  */
            /* init CG port register database */
            rc = prvCpssPxCgPortDbInvalidate(devNum, portMacMap, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxCgPortDbInvalidate");
            }
        }

        /* save new interface mode in DB */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        return GT_OK;
    }

    if(isCgUnitInUse == GT_TRUE)
    {
        /* init CG port register database */
        rc = prvCpssPxCgPortDbInvalidate(devNum, portMacMap, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssPxPortPizzaArbiterIfConfigure(devNum, portNum,
        ((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPipePortResourcesConfigStatic(devNum, portNum, ifMode, speed);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Set FCA+PTP interface width */
    rc = prvCpssPxPortBusWidthSet(devNum, portNum, speed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    prvCpssPxPortTypeSet(devNum, portNum, ifMode, speed);

    if (rc != GT_OK)
#ifndef GM_USED
    /* Configures type of the port connected to the LED */
    rc = prvCpssPxLedPortTypeConfig(devNum, portNum, portCreate);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif

    /* save new interface mode in DB */
    *portIfModePtr = ifMode;
    *portSpeedPtr = speed;

    return GT_OK;
}
/**
* @internal prvCpssPxPortApPortModeAckSet function
* @endinternal
*
* @brief   Send Ack to Service CPU indicate pizza resources allocate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*                                      mode       - mode in CPSS format
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS prvCpssPxPortApPortModeAckSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  phyPortNum; /* port number in local core */
    GT_U32                  portGroup;  /* local core number */
    MV_HWS_PORT_STANDARD    hwPortMode;


    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(
        devNum,portNum,phyPortNum);

    portGroup =
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
            CAST_SW_DEVNUM(devNum), portNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode,speed,&hwPortMode);

    if(rc != GT_OK)
    {
        return rc;
    }

    rc = mvHwsApPortCtrlSysAck(
        CAST_SW_DEVNUM(devNum),portGroup,phyPortNum,hwPortMode);

    return rc;
}

/**
* @internal prvCpssPxPipePortResourceConfigGet function
* @endinternal
*
* @brief   Get resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPipePortResourceConfigGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC          *resourcePtr
)
{
    GT_STATUS   rc;

    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    /* Get Port Mapping DataBase */
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    resourcePtr->rxdmaScdmaIncomingBusWidth               = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txqQPortDescriptorsCounter               = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txqDqPortCreditCounter                   = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txqDqCreditCountersReset                 = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txfifoScdmaPayloadStartTransmThreshold   = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txfifoScdmaShiftersOutgoingBusWidth      = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaResetDescCreditsCounter        = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaDescCreditScdma                     = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaHeaderTxfifoCounter            = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaPayloadTxfifoCounter           = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaHeaderTxfifoThrshold           = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaPayloadTxfifoThrshold          = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaBurstAmostFullThrshold         = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaBurstFullThrshold              = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaRateLimitIntegerThreshold      = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaRateLimitResidueVectThreshold  = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaGlobalSourceDmaSpeed                = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaGlobalTargetDmaSpeed                = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->xlgMacFifoTxWriteThreshold               = PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS;

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
        &(resourcePtr->rxdmaScdmaIncomingBusWidth));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E,
        &(resourcePtr->txqQPortDescriptorsCounter));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E,
        &(resourcePtr->txqDqPortCreditCounter));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
        &(resourcePtr->txqDqCreditCountersReset));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
        &(resourcePtr->txfifoScdmaPayloadStartTransmThreshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
        &(resourcePtr->txfifoScdmaShiftersOutgoingBusWidth));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
        &(resourcePtr->txdmaScdmaResetDescCreditsCounter));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
        &(resourcePtr->txdmaDescCreditScdma));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
        &(resourcePtr->txdmaScdmaHeaderTxfifoCounter));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
        &(resourcePtr->txdmaScdmaPayloadTxfifoCounter));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaHeaderTxfifoThrshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaPayloadTxfifoThrshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaBurstAmostFullThrshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaBurstFullThrshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaRateLimitIntegerThreshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,
        &(resourcePtr->txdmaScdmaRateLimitResidueVectThreshold));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E,
        &(resourcePtr->txdmaGlobalSourceDmaSpeed));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxResourcePortRegFieldGet(
        devNum, portNum,
        PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E,
        &(resourcePtr->txdmaGlobalTargetDmaSpeed));
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (GT_TRUE == portMapShadowPtr->valid)
    {
        if (portMapShadowPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            rc = prvCpssPxResourcePortRegFieldGet(
                devNum, portNum,
                PRV_CPSS_PX_REG_FIELDS_XLG_MAC_FIFO_TX_WRITE_THRESHOLD_E,
                &(resourcePtr->xlgMacFifoTxWriteThreshold));
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesRateLimitSet function
* @endinternal
*
* @brief    Jumbo with loopback issue:
*           Due to the loopback in the MAC, the S/D-PCS is no
*           longer limiting the rate of the port to exactly
*           10Gbps. The port speed is determined by the
*           MAC_CLK/Divider (in this case 833Mhz/4 * 64 =
*           13.33Gbps). Because the rate limiter of the loopback
*           port was enabled for exactly 10Gbps, the port got an
*           underrun (when rate limiter was disabled or shaped
*           to port speed of 13.33, there are no more issue).
*           Solution: As long as there is no Cut-Through traffic
*           towards the loopback port - disable the loopback
*           port rate limiter By writing 0x2 into TxDMA %a/TxDMA
*           Per SCDMA Configurations/SCDMA %p Configurations Reg
*           1/<SCDMA %p Rlimit Integer Thr>
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port interface speed
* @param[in] enable                   - enable rate limit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortResourcesRateLimitSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_SPEED_ENT             speed,
    IN GT_BOOL                         enable
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaScdmaRateLimitIntegerThr;
    GT_U32      speedValueMbps;
    GT_U32      speedIndex;         /* index in speed DB */

    if (enable)
    {
        /* convert port speed to DB index and speed value in Mbps */
        rc = prvCpssPxPortResourcesSpeedIndexGet(
            speed, &speedIndex, &speedValueMbps);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        txdmaScdmaRateLimitIntegerThr        =
            prvCpssPxPipePortResources_TXDMA_SCDMA_RateLimitIntegerPart(
                speedValueMbps, PRV_CPSS_PP_MAC(devNum)->coreClock);

    }
    else
    {    /* to disable rate limit per port we set value 2*/
        txdmaScdmaRateLimitIntegerThr = 2;
    }
    rc = prvCpssPxResourcePortRegFieldSet(devNum,portNum,
                                          PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
                                          txdmaScdmaRateLimitIntegerThr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxResourcePortRegFieldSet() : PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E");
    }
    return GT_OK;
}


