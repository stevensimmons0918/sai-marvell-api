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
* @file prvCpssDxChPortDynamicPizzaArbiter.c
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   93
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPATypeDef.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAGCDAlgo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_SHARED_PA_DB_VAR(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc._var)

#ifdef WIN32
    #define PIZZA_DEBUG_PRINT_DUMP         0
    #define __DYNAMIC_PIZZA_ARBITER_TEST   0
#else
    #define PIZZA_DEBUG_PRINT_DUMP         0
    #define __DYNAMIC_PIZZA_ARBITER_TEST   0
#endif


#if (PIZZA_DEBUG_PRINT_DUMP == 1)

    extern GT_STATUS gtBobcat2PortPizzaArbiterIfStateDump(/*IN*/GT_U8  devNum,
                                                         /*IN*/GT_U32 portGroupId);
    int g_printPizzaDump = 1;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unit,
    IN GT_BOOL               status
);
#ifdef __cplusplus
}
#endif /* __cplusplus */


GT_STATUS prvCpssDxChPortDynamicPizzaArbiterSpeedConv
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speedEnm,
    OUT GT_U32              *speedPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum); */
    /* GT_STATUS rc; */
    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    devNum  = devNum;
    portNum = portNum;
    if (speedPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (speedEnm == CPSS_PORT_SPEED_NA_E)
    {
        *speedPtr = 0;
        return GT_OK;
    }

    *speedPtr = paWsPtr->prv_speedEnt2MBitConvArr[speedEnm];
    if (*speedPtr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}






static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterCPUPort2PortDBAdd
(
    GT_U8               devNum,
    CPSS_PORT_SPEED_ENT cpuSpeedEnm
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL                      isCPUInitilized;
    GT_U32                       CPUspeedInMBit;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    maxPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        /* find local physical port number of CPU port */
        rc =  prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == portMapShadowPtr->valid)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isCPUInitilized,/*OUT*/&CPUspeedInMBit);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (isCPUInitilized == GT_FALSE)
                {
                    rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(devNum,portNum,cpuSpeedEnm,/*OUT*/&CPUspeedInMBit);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = prvCpssDxChPortDynamicPAPortSpeedDBSet(devNum,portNum,CPUspeedInMBit);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}


typedef struct
{
    GT_FLOAT64      prv_clientSpeedConfigArr         [CPSS_DXCH_PA_UNIT_MAX_E][PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    /* prv_specialTxQClientArr[] indexed by 'global txq queue-id' (BC3 -> 0..575)  */
    /* this array used for supporting of remote physical ports                     */
    /* the value is the 'TXQ unit' (CPSS_DXCH_PA_UNIT_TXQ_0_E..CPSS_DXCH_PA_UNIT_TXQ_5_E).*/
    /* value of CPSS_DXCH_PA_UNIT_UNDEFINED_E means 'not used'                      */
    CPSS_DXCH_PA_UNIT_ENT prv_specialTxQClientArr                             [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    GT_U32          prv_sliceNum2Config              [CPSS_DXCH_PA_UNIT_MAX_E];
    GT_PORT_NUM     prv_pizzaArray                   [CPSS_DXCH_PA_UNIT_MAX_E][PRV_CPSS_DXCH_DYNAMIC_PA_SLICE_NUM_CNS];
    GT_U32          prv_highSpeedPortNumber          [CPSS_DXCH_PA_UNIT_MAX_E];
    GT_U32          prv_highSpeedPortArr             [CPSS_DXCH_PA_UNIT_MAX_E][CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
    GT_FLOAT64      tmp_clientSpeedConfigArr[PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_SIZE_CNS];
    GT_PORT_NUM     tmp_pizzaArray[PRV_CPSS_DXCH_DYNAMIC_PA_SLICE_NUM_CNS];
} dynamicPaConfigList_STC;

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfInit
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      unitIdx;
    CPSS_DXCH_PA_UNIT_ENT unitType;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    rc = prvCpssDxChPortDynamicPAPortSpeedDBInit(devNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* init list of groups of special txq clients */
    rc = prvCpssDxChPortDynamicPATxQClientGroupListInit(devNum);
    if (GT_OK != rc)
    {
        return rc;
    }


    rc = prvCpssDxChPortDynamicPizzaArbiterWSInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDynamicPAClientBWListInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------*/
    /* set units BW */
    /*--------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWInit(devNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        /* array boundary check */
        if ((GT_U32)unitType >= CPSS_DXCH_PA_UNIT_MAX_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if(paWsPtr->prv_unit2PizzaAlgoFunArr[unitType] == NULL)
        {
            continue;
        }

        rc = bobcat2PizzaArbiterUnitDrvInit(devNum,unitType);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /*-------------------------------------------------------------------------*/
    /*   Add to port-DB CPU port (if mapped !!!) with speed of 1G              */
    /*   mapping is called at end of phase 1                                   */
    /*   Pizza arbiter is initialized at and of phase 2                        */
    /*   therefore it is already known whether CPU port is mapped or not       */
    /*-------------------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterCPUPort2PortDBAdd(devNum,paWsPtr->cpuSpeedEnm);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------*
     * set work conserving mode for TxQ unit                        *
     *     cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet() *
     *--------------------------------------------------------------*/
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) )
    {
        if (paWsPtr->workConservingModeOnUnitListPtr != NULL)
        {
            for ( unitIdx = 0 ; paWsPtr->workConservingModeOnUnitListPtr[unitIdx] !=  CPSS_DXCH_PA_UNIT_UNDEFINED_E ; unitIdx++)
            {
                rc = prvCpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum,paWsPtr->workConservingModeOnUnitListPtr[unitIdx],GT_TRUE);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nInit dev %2d",devNum);
            rc = gtBobcat2PortPizzaArbiterIfStateDump(devNum,0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

    #endif

    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet function
* @endinternal
*
* @brief   define BW of TM units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*                                      portNum  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  bwMbps
)
{
    GT_STATUS rc;
    OUT PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;
    GT_U32 i;

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0 ; paWsPtr->tmUnitClientListPtr[i].unit != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        rc = prvCpssDxChPortDynamicPAClientBWListUpdate(devNum,
                                                        paWsPtr->tmUnitClientListPtr[i].unit,
                                                        paWsPtr->tmUnitClientListPtr[i].clientId,bwMbps);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfCheckSupport function
* @endinternal
*
* @brief   Check whether Pizza Arbiter can be configured to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS rc;   /* return code */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_DXCH_PA_UNIT_ENT             *configuredUnitListPtr;
    GT_U32                             paMinSliceResolutionMbps;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;



    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE) /* port is not mapped, can't configure it */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType][portMapShadowPtr->portMap.trafficManagerEn];
    if (NULL == configuredUnitListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    paMinSliceResolutionMbps = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;
    if (portSpeedInMBit > 0) /* 0 for delete operation, minSliceResolution is irrelevant here */
    {
        if (portSpeedInMBit < paMinSliceResolutionMbps)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portSpeedInMBit % paMinSliceResolutionMbps != 0) /* shall be multiple of resolution */
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

static GT_U32 prvCpssDxChPortDynamicPizzaArbiteIsTxFifoUnit
(
    CPSS_DXCH_PA_UNIT_ENT                  unitType
)
{
    switch (unitType)
    {
        case CPSS_DXCH_PA_UNIT_TX_FIFO_0_E:
        case CPSS_DXCH_PA_UNIT_TX_FIFO_1_E:
        case CPSS_DXCH_PA_UNIT_TX_FIFO_2_E:
        case CPSS_DXCH_PA_UNIT_TX_FIFO_3_E:
        case CPSS_DXCH_PA_UNIT_TX_FIFO_4_E:
        case CPSS_DXCH_PA_UNIT_TX_FIFO_5_E:
        case CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E:
        case CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E:
        case CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E:
            return 1;
        default:
            break;
    }
    return 0;
}

/* return the TXQ unit (CPSS_DXCH_PA_UNIT_TXQ_0_E...CPSS_DXCH_PA_UNIT_TXQ_5_E)
   that hold the txqPort */
static CPSS_DXCH_PA_UNIT_ENT convertTxqPortToTxqUnit
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             txqPort
)
{
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if(txqPort >= (6*96))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(CPSS_DXCH_PA_UNIT_TXQ_0_E,
                "BC3 txqPort[%d] over the limit",
                txqPort);
        }

        return (txqPort/96) + CPSS_DXCH_PA_UNIT_TXQ_0_E;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if(txqPort >= (4*25))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(CPSS_DXCH_PA_UNIT_TXQ_0_E,
                "BC3 txqPort[%d] over the limit",
                txqPort);
        }

        return (txqPort/25) + CPSS_DXCH_PA_UNIT_TXQ_0_E;
    }

    return CPSS_DXCH_PA_UNIT_TXQ_0_E;
}

/* return the first global TXQ port of the txq unit (CPSS_DXCH_PA_UNIT_TXQ_0_E...CPSS_DXCH_PA_UNIT_TXQ_5_E)*/
static GT_U32   getFirstTxqPortInUnit(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT              txqUnitType
)
{
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        return (txqUnitType - CPSS_DXCH_PA_UNIT_TXQ_0_E) * 96;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        return (txqUnitType - CPSS_DXCH_PA_UNIT_TXQ_0_E) * 25;
    }

    return 0;
}

/*----------------------------------------------------------------*/
/* Build configuration according to stored DB(old configuration)  */
/*----------------------------------------------------------------*/

static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfBuildConfigByStoredDB
(
    IN  PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr,
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT              specialTxQClientArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS],  /*paListPtr->prv_specialTxQClientArr*/
    OUT GT_FLOAT64                         clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_MAX_E][PRV_CPSS_MAX_PP_PORTS_NUM_CNS]/*paListPtr->prv_clientSpeedConfigArr*/
)
{
    GT_STATUS                   rc;
    GT_BOOL                     isPortConfigured;
    GT_U32                      speedEnt;
    GT_U32                      txFifoSpeed;
    GT_FLOAT64                  speedFloat;
    GT_FLOAT64                  txFifoSpeedFloat;
    GT_FLOAT64                  txQspeedFloat;
    CPSS_DXCH_PA_UNIT_ENT *     configuredUnitListPtr;
    CPSS_DXCH_PA_UNIT_ENT       unitType;
    GT_U32                      unitIdx;
    GT_U32                      clientIdx;
    GT_PHYSICAL_PORT_NUM        maxPhysPortNum;
    GT_PHYSICAL_PORT_NUM        physPortNum;
    GT_U32                      specificPortNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32                      i;
    GT_U32                      portArgArr[CPSS_DXCH_PA_UNIT_MAX_E];
    GT_U32                      specialTxQClientsGroupListLen;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
    GT_U32                      clientIdx_array[6];/* supporting : CPSS_DXCH_PA_UNIT_TXQ_0_E .. CPSS_DXCH_PA_UNIT_TXQ_5_E*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /*--------------------*
     *  Init output array *
     *--------------------*/
    for (unitType = (CPSS_DXCH_PA_UNIT_ENT)0; unitType < CPSS_DXCH_PA_UNIT_MAX_E; unitType++)
    {
        for (specificPortNum = 0 ; specificPortNum< PRV_CPSS_MAX_PP_PORTS_NUM_CNS ; specificPortNum++)
        {
            clientSpeedConfigArr[unitType][specificPortNum] = 0;
        }
    }


    maxPhysPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (physPortNum = 0 ; physPortNum < maxPhysPortNum; physPortNum++)
    {
        rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(
            devNum,physPortNum,/*OUT*/&isPortConfigured,/*OUT*/&speedEnt);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* TX_FIFO adjusted speeds */
        rc = prvCpssDxChPortDynamicPAPortTxFifoAdjustedSpeedDBGet(
            devNum,physPortNum,/*OUT*/&isPortConfigured,/*OUT*/&txFifoSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == isPortConfigured)
        {
            speedFloat          = (GT_FLOAT64)speedEnt;
            txQspeedFloat       = speedFloat;
            txFifoSpeedFloat    = (GT_FLOAT64)txFifoSpeed;

            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,physPortNum,/*OUT*/&portMapShadowPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            if (portMapShadowPtr->valid == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "PA : according to Speed DB port %d is configured to speed %d Mbps, but mapping is invalid; inconsistent data; check CPU mapping !",physPortNum,speedEnt);
            }
            /* 30G WA */
            if (GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_PRV_CPSS_DXCH_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_WA_E.enabled)
            {
                if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
                {
                    if (speedFloat == 40000)
                    {
                        if (portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE)
                        {
                            txQspeedFloat = 30000;
                        }
                    }
                }
            }
            /*---------------------------------------------*
             * convert from mapping to array of units      *
             *---------------------------------------------*/
            rc = paWsPtr->mapping2unitConvFunPtr(devNum,portMapShadowPtr,portArgArr);
            if (rc != GT_OK)
            {
                return rc;
            }

            configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType][portMapShadowPtr->portMap.trafficManagerEn];
            if (NULL == configuredUnitListPtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                /* CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); */
            }


            for (unitIdx = 0 ;  configuredUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
            {
                unitType        = configuredUnitListPtr[unitIdx];
                /* array boundary check */
                if ((GT_U32)unitType >= CPSS_DXCH_PA_UNIT_MAX_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                specificPortNum = portArgArr[unitType];
                if (specificPortNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS) /* is Valid ? */
                {
                    if (prvCpssDxChPortDynamicPizzaArbiteIsTxFifoUnit(unitType))
                    {
                        clientSpeedConfigArr[unitType][specificPortNum] += txFifoSpeedFloat;
                    }
                    else
                    if (unitType < CPSS_DXCH_PA_UNIT_TXQ_0_E || unitType > CPSS_DXCH_PA_UNIT_TXQ_5_E)
                    {
                        clientSpeedConfigArr[unitType][specificPortNum] += speedFloat;
                    }
                    else
                    {
                        clientSpeedConfigArr[unitType][specificPortNum] += txQspeedFloat;
                    }
                }
            }
            /*-------------------------------------*
             * compute client for MPPM units       *
             *-------------------------------------*/
            if (paWsPtr->mppmClientCodingListPtr != NULL)
            {
                for (unitIdx = 0; paWsPtr->mppmClientCodingListPtr[unitIdx].unit != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
                {
                    unitType        = paWsPtr->mppmClientCodingListPtr[unitIdx].unit;
                    /* array boundary check */
                    if ((GT_U32)unitType >= CPSS_DXCH_PA_UNIT_MAX_E)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }

                    clientIdx       = paWsPtr->mppmClientCodingListPtr[unitIdx].mppmClientCode;
                    specificPortNum = portArgArr[unitType];
                    if (specificPortNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
                    {
                        if (prvCpssDxChPortDynamicPizzaArbiteIsTxFifoUnit(unitType))
                        {
                            clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_MPPM_E][clientIdx]
                                += txFifoSpeedFloat * paWsPtr->mppmClientCodingListPtr[unitIdx].clientWeight;
                        }
                        else
                        {
                            clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_MPPM_E][clientIdx]
                                += speedFloat * paWsPtr->mppmClientCodingListPtr[unitIdx].clientWeight;
                        }
                    }
                }
            }
        }
    }

    /*-----------------------------------------------------------------------------------*
     * now when all client BW is computed as sum, checked whether there is predefined BW *
     *-----------------------------------------------------------------------------------*/
    for (i = 0 ;  ; i++)
    {
        PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC * ptr;

        ptr = prvCpssDxChPortDynamicPAClientBWListEntryByIdxGet(devNum,i);
        if (ptr == NULL)
        {
            break;
        }
        /* overwrite pre-computed value */
        clientSpeedConfigArr[ptr->unitType][ptr->clinetId] = ptr->bwMbps;
    }

    /*------------------------------------------------------------*
     * now for TXQ clients group, add dummy client for each group *
     *------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPATxQClientGroupListLenGet(devNum,&specialTxQClientsGroupListLen);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* pass 1 : indicate not special ports */
    for (i = 0;  i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        /* state 'no-special' TXQ clients */
        specialTxQClientArr[i] = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
    }
    /* pass 2 : build indications of special TXQ clients */
    for (i = 0; i < specialTxQClientsGroupListLen; i++)
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet(devNum,i,/*OUT*/&groupPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (groupPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something is bad, group shall exists */
        }
        /*----------------------------------------------------------*
         * configure client when                                    *
         *   1. cascade port configured                             *
         *   2. bw of clients > 0  i.e. there is real clients       *
         *----------------------------------------------------------*/
        if (groupPtr->isExtendedCascadeExist == GT_TRUE && groupPtr->groupSpeedMbps > 0)
        {
            for (clientIdx = 0; clientIdx < groupPtr->clientListLen; clientIdx++)
            {
                /* save the TXQ unit that is holding the txqPort of the remote physical port (special port) */
                specialTxQClientArr[groupPtr->clientIdList[clientIdx].txqPort] =
                    convertTxqPortToTxqUnit(devNum,groupPtr->clientIdList[clientIdx].txqPort);
            }
        }
    }

    for (i = 0; i < 6; i++)
    {
        /* each TXQ unit hold different global txq-port range */
        clientIdx_array[i] = getFirstTxqPortInUnit(devNum,i + CPSS_DXCH_PA_UNIT_TXQ_0_E);
    }

    /* pass 3 : find empty clients that not use special ports */
    for (i = 0; i < specialTxQClientsGroupListLen; i++)
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet(devNum,i,/*OUT*/&groupPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (groupPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something is bad, group shall exists */
        }
        /*----------------------------------------------------------*
         * configure client when                                    *
         *   1. cascade port configured                             *
         *   2. bw of clients > 0  i.e. there is real clients       *
         *----------------------------------------------------------*/
        if (groupPtr->isExtendedCascadeExist == GT_TRUE && groupPtr->groupSpeedMbps > 0)
        {
            CPSS_DXCH_PA_UNIT_ENT  txqUnitType;
            GT_U32  representative_clientIdx = 0;/* this 'groupPtr' should hold txq-ports from the same TXQ unit */
            GT_U32  local_clientIdx;/* local txq-port in the TXQ unit */

            txqUnitType = specialTxQClientArr[groupPtr->clientIdList[representative_clientIdx].txqPort];
            if(txqUnitType > CPSS_DXCH_PA_UNIT_TXQ_5_E ||
               txqUnitType < CPSS_DXCH_PA_UNIT_TXQ_0_E )
            {
                /* unexpected ! should have been set few line above calling convertTxqPortToTxqUnit()*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected TXQ unit");
            }

            clientIdx       = clientIdx_array[txqUnitType-CPSS_DXCH_PA_UNIT_TXQ_0_E];
            local_clientIdx = clientIdx - getFirstTxqPortInUnit(devNum,txqUnitType);

            /* search first empty TxQ client from current place */
            for (; clientIdx < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; clientIdx++ ,local_clientIdx++)
            {
                if (clientSpeedConfigArr[txqUnitType][local_clientIdx] == 0 ) /* empty client ?*/
                {
                    if (specialTxQClientArr[clientIdx] == CPSS_DXCH_PA_UNIT_UNDEFINED_E) /* it is not appear in list of special TXQ clients ? */
                    {
                        break;  /* found empty place in the TXQ unit to hold this txq-port of the remote physical port */
                    }
                }
            }
            if (clientIdx == PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something bad,  we shall always find empty client */
            }

            /* save the index for next 'groupPtr' that may need to use the same 'txqUnitType' */
            clientIdx_array[txqUnitType-CPSS_DXCH_PA_UNIT_TXQ_0_E] = clientIdx;

            groupPtr->tmp_groupTxqClient = local_clientIdx; /* hold local number used with (paListPtr->prv_pizzaArray[txqUnitType][sliceIdx])*/
            clientSpeedConfigArr[txqUnitType][local_clientIdx] = groupPtr->groupSpeedMbps;
        }
    }
    return GT_OK;
}

static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfComputeUnitDistributions
(
    INOUT dynamicPaConfigList_STC        *paListPtr,
    IN PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr,
    IN GT_U8                              devNum,
    IN CPSS_DXCH_PA_UNIT_ENT              unitType
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               maxPhysPortNum;
    PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN algoDistribFun;
    GT_U32                             unitBWInGBit;
    GT_U32                             unitBWInMBit;
    GT_U32                             minSliceResolutionInMBps;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    maxPhysPortNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS; /* PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); */

    minSliceResolutionInMBps = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;
    algoDistribFun = paWsPtr->prv_unit2PizzaAlgoFunArr[unitType];
    if(NULL != algoDistribFun)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(devNum,unitType, /*OUT*/&unitBWInGBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        unitBWInMBit = unitBWInGBit*PRV_CPSS_DXCH_PA_BW_COEFF;
        rc = (paWsPtr->prv_unit2PizzaAlgoFunArr[unitType])(/*IN*/&paListPtr->prv_clientSpeedConfigArr[unitType][0],
                                                    /*IN*/ maxPhysPortNum,
                                                    /*IN*/ unitBWInMBit,
                                                    /*IN*/ minSliceResolutionInMBps,
                                                    /*IN*/ &(paWsPtr->prv_txQHighSpeedPortThreshParams),
                                                    /*IN*/ PRV_CPSS_DXCH_DYNAMIC_PA_SLICE_NUM_CNS,
                                                    /*OUT*/&paListPtr->prv_pizzaArray[unitType][0],
                                                    /*OUT*/&paListPtr->prv_sliceNum2Config[unitType],
                                                    /*OUT*/&paListPtr->prv_highSpeedPortNumber[unitType],
                                                    /*OUT*/&paListPtr->prv_highSpeedPortArr[unitType][0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfCorrectPizzaDistributionforSpecialTxQClient
(
    INOUT dynamicPaConfigList_STC   *paListPtr,
    GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32 groupIdx;
    GT_U32 sliceIdx;
    GT_U32 specialTxQClientsGroupListLen;
    GT_U32 txqClientIdx;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC* pDev;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    /*------------------------------------------------------------*
     * now for TXQ clients group, add dummy client for each group *
     *------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPATxQClientGroupListLenGet(devNum,&specialTxQClientsGroupListLen);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (groupIdx = 0; groupIdx < specialTxQClientsGroupListLen; groupIdx++)
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet(devNum,groupIdx,/*OUT*/&groupPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (groupPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something is bad, group shall exist */
        }
        /*----------------------------------------------------------*
         * configure client when                                    *
         *   1. cascade port configured                             *
         *   2. bw of clients > 0  i.e. there are real clients      *
         *----------------------------------------------------------*/
        if (groupPtr->isExtendedCascadeExist == GT_TRUE && groupPtr->groupSpeedMbps > 0)
        {
            CPSS_DXCH_PA_UNIT_ENT  txqUnitType;
            GT_U32  representative_clientIdx = 0;/* this 'groupPtr' should hold txq-ports from the same TXQ unit */
            GT_U32       sliceNum2Config;
            GT_U32       pizzaSliceIdx;

            /* clear array */
            for (txqClientIdx = 0 ; txqClientIdx < sizeof(paListPtr->tmp_clientSpeedConfigArr)/sizeof(paListPtr->tmp_clientSpeedConfigArr[0]); txqClientIdx++)
            {
                paListPtr->tmp_clientSpeedConfigArr[txqClientIdx] = 0;
            }
            /* build list of clients*/
            for (txqClientIdx = 0 ; txqClientIdx < groupPtr->clientListLen; txqClientIdx++)
            {
                paListPtr->tmp_clientSpeedConfigArr[txqClientIdx] = groupPtr->clientIdList[txqClientIdx].speedMbps/pDev->paData.paMinSliceResolutionMbps;
            }
            rc = BuildPizzaDistribution_NO_GCD
            (
                IN   &paListPtr->tmp_clientSpeedConfigArr[0],
                IN   groupPtr->clientListLen,
                IN   groupPtr->groupSpeedMbps/pDev->paData.paMinSliceResolutionMbps,
                IN   sizeof(paListPtr->tmp_pizzaArray)/sizeof(paListPtr->tmp_pizzaArray[0]), /* GT_U32       pizzaArraySize, */
                OUT  &paListPtr->tmp_pizzaArray[0],
                OUT  &sliceNum2Config
            );
            if (rc != GT_OK)
            {
                return rc;
            }

            txqUnitType = paListPtr->prv_specialTxQClientArr[groupPtr->clientIdList[representative_clientIdx].txqPort];
            if(txqUnitType > CPSS_DXCH_PA_UNIT_TXQ_5_E ||
               txqUnitType < CPSS_DXCH_PA_UNIT_TXQ_0_E )
            {
                /* unexpected ! should have been by prvCpssDxChPortDynamicPizzaArbiterIfBuildConfigByStoredDB(...)*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected TXQ unit");
            }

            {
                /* jira : CPSS-8738 : BC3+2*Pipes : cpss FAIL to set 20 remote physical ports of 10G + 1 port 100G native + SDMA 10G on single DP[1] */

                /* PATCH : to allow continue with configurations */
                /* in BC3 when in DQ[1] I use :
                    1. MAC 12 as 100G
                    2. MAC 16 represent 10 remote ports of 10G each
                    3. MAC 20 represent 10 remote ports of 10G each
                    4. SDMA CPU port 10G

                    --> we get :  sliceNum2Config = 100
                    while paListPtr->prv_sliceNum2Config[txqUnitType] = 31
                */
                if(sliceNum2Config > paListPtr->prv_sliceNum2Config[txqUnitType])
                {
                    /* we will surly fail in : if (sliceIdx == paListPtr->prv_sliceNum2Config[txqUnitType]) */
                    /* after groupPtr->clientListLen iterations in */
                    /* to overcome this , state next next loop */

                    if(groupPtr->groupSpeedMbps == 100000 /*100G*/)
                    {
                        sliceNum2Config = groupPtr->clientListLen;
                    }
                }
            }

            /* search first empty TxQ client from current place */
            sliceIdx = 0;
            for (pizzaSliceIdx = 0 ; pizzaSliceIdx < sliceNum2Config; pizzaSliceIdx++)
            {
                /* find first slice belogs to the group */
                while (sliceIdx < paListPtr->prv_sliceNum2Config[txqUnitType])
                {
                    if (paListPtr->prv_pizzaArray[txqUnitType][sliceIdx] == groupPtr->tmp_groupTxqClient)
                    {
                        break;
                    }
                    sliceIdx++;
                }
                if (sliceIdx == paListPtr->prv_sliceNum2Config[txqUnitType])
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something is bad : supposed to be enough slices */
                }
                /* replace found slice' dummy client to real one */
                txqClientIdx = paListPtr->tmp_pizzaArray[pizzaSliceIdx];
                paListPtr->prv_pizzaArray[txqUnitType][sliceIdx] = groupPtr->clientIdList[txqClientIdx].txqPort;
                sliceIdx++;
            }
        }
    }
    return GT_OK;
}

static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfConfigDistributionsInHW
(
    IN dynamicPaConfigList_STC        *paListPtr,
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr,
    GT_U8                              devNum,
    CPSS_DXCH_PA_UNIT_ENT              unitType
)
{
    GT_STATUS                          rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    if(NULL != paWsPtr->prv_unit2PizzaAlgoFunArr[unitType]) /* pizza was computed ? */
    {
        if (paWsPtr->prv_unit2HighSpeedPortConfFunArr[unitType] != NULL)
        {
            if (paListPtr->prv_highSpeedPortNumber[unitType] > CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
            {
                cpssOsPrintf("\n--> PA ERROR : currently TxQ support SINGLE High Speed Ports configuration");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            rc = (paWsPtr->prv_unit2HighSpeedPortConfFunArr[unitType])(devNum,paListPtr->prv_highSpeedPortNumber[unitType],&paListPtr->prv_highSpeedPortArr[unitType][0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = bobcat2PizzaArbiterUnitDrvSet(devNum,unitType,paListPtr->prv_sliceNum2Config[unitType],&paListPtr->prv_pizzaArray[unitType][0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterCompute function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] paListPtrPtr            - (pointer to)(pointer to)parameter structure with
*                                       all calculated values
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on null pointer parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterCompute
(
    IN  GT_U8                              devNum,
    dynamicPaConfigList_STC                **paListPtrPtr
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    CPSS_DXCH_PA_UNIT_ENT              unitType;
    GT_U32                             unitIdx;
    dynamicPaConfigList_STC   *paListPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(paListPtrPtr);

    *paListPtrPtr = NULL;

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Add CPU port to port-DB (if mapped !!!) with speed of 1G */
    rc = prvCpssDxChPortDynamicPizzaArbiterCPUPort2PortDBAdd(devNum,paWsPtr->cpuSpeedEnm);
    if (rc != GT_OK)
    {
        return rc;
    }

    paListPtr = cpssOsMalloc(sizeof(*paListPtr));
    if (paListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(paListPtr->prv_pizzaArray,0,sizeof(paListPtr->prv_pizzaArray));
    /*----------------------------------------------------------------*/
    /* Build configuration according to stored DB(old configuration)  */
    /*----------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfBuildConfigByStoredDB(
        paWsPtr, devNum, paListPtr->prv_specialTxQClientArr,
        /*OUT*/paListPtr->prv_clientSpeedConfigArr);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*----------------------------------*/
    /* now compute pizzas for all units */
    /*----------------------------------*/
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];

        rc = prvCpssDxChPortDynamicPizzaArbiterIfComputeUnitDistributions(paListPtr, paWsPtr,devNum, unitType);
        if (rc != GT_OK)
        {
            cpssOsFree(paListPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* correct TxQ clients for special TXQ group */
    rc = prvCpssDxChPortDynamicPizzaArbiterIfCorrectPizzaDistributionforSpecialTxQClient(paListPtr, devNum);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *paListPtrPtr = paListPtr;
    return GT_OK;
}

/* flag to allow optimization of the amount of times that 'PIZZA' configured
   for 'X' ports.
   assumption is that caller knows that no traffic should exists in the device
   while 'optimizing'
*/
/*allow application to optimize HW calls ... to reduce config time */
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimizeSet(IN GT_BOOL optimize)
{
    PRV_SHARED_PA_DB_VAR(
        prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimize) = optimize;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterComputeAndConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterComputeAndConfigure
(
    IN  GT_U8                              devNum
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    CPSS_DXCH_PA_UNIT_ENT              unitType;
    GT_U32                             unitIdx;
    dynamicPaConfigList_STC   *paListPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if (PRV_SHARED_PA_DB_VAR(
        prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimize) != GT_FALSE)
    {
        /* the caller will modify the flag on 'last port' */
        return GT_OK;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterCompute(devNum, &paListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*----------------------------------------------------------------------*/
    /* now all data was prepared , configure HW (High Speed Ports and Pizza)*/
    /*----------------------------------------------------------------------*/
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        rc = prvCpssDxChPortDynamicPizzaArbiterIfConfigDistributionsInHW(paListPtr, paWsPtr,devNum,unitType);
        if (rc != GT_OK)
        {
            goto error_restore_db;
        }
    }

    cpssOsFree(paListPtr);
    return GT_OK;

error_restore_db:
    cpssOsFree(paListPtr);
    return rc;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck function
* @endinternal
*
* @brief   Check Pizza Arbiter Bandwidth overbooking according to ports speeds in DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NO_RESOURCE           - on Pizza Arbiter Bandwidth overbooking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck
(
    IN  GT_U8                              devNum
)
{
    GT_STATUS rc;   /* return code */
    dynamicPaConfigList_STC   *paListPtr;

    paListPtr = NULL;
    rc = prvCpssDxChPortDynamicPizzaArbiterCompute(devNum, &paListPtr);
    if (paListPtr)
    {
        cpssOsFree(paListPtr);
    }
    return ((rc == GT_FAIL) ? GT_NO_RESOURCE : rc);
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS rc;   /* return code */

    CPSS_DXCH_DETAILED_PORT_MAP_STC   *portMapShadowPtr;
    CPSS_DXCH_PA_UNIT_ENT             *configuredUnitListPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC       *pDev;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    GT_BOOL isPortConfigured;
    GT_U32  speedDBMbpsOld = 0;



    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);


    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paWsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

#if 0  /* currently this check performed at If Not Remote / remote */
    /* check whether this type is supported */
    configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType][portMapShadowPtr->portMap.trafficManagerEn];
    if (NULL == configuredUnitListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
#endif


    if (portSpeedInMBit > 0) /* 0 for delete operation, minSliceResolution is irrelevant here */
    {
        if (portSpeedInMBit < pDev->paData.paMinSliceResolutionMbps)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portSpeedInMBit % pDev->paData.paMinSliceResolutionMbps != 0) /* shall be compain to resolution */
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /* check whether this type is supported */
        configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType][portMapShadowPtr->portMap.trafficManagerEn];
        if (NULL == configuredUnitListPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        /*-------------------------------*/
        /* get old speed                 */
        /*-------------------------------*/
        rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isPortConfigured,/*OUT*/&speedDBMbpsOld);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* configure new speed */
        rc = prvCpssDxChPortDynamicPAPortSpeedDBSet(devNum,portNum,portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* create the group if it does not exist */
        if (portMapShadowPtr->portMap.isExtendedCascadePort)
        {
            PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
            rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,portMapShadowPtr->portMap.macNum,/*OUT*/&groupPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portSpeedInMBit > 0)
            {
                if (groupPtr == NULL) /* group does not exist, use mac of extended-cascade port as group id  */
                {
                    rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(devNum,portMapShadowPtr->portMap.macNum);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                rc = prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet(devNum,portMapShadowPtr->portMap.macNum,GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else /* delete operation */
            {
                if (groupPtr != NULL) /* group does not exist, use mac of extended-cascade port as group id  */
                {
                    rc = prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet(devNum,portMapShadowPtr->portMap.macNum,GT_FALSE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something bad we can't delete something that does not exist ??? */
                }
            }
        }
    }
    else /* mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E */
    {
        PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
        /* create the group if it does not exist */
        rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,portMapShadowPtr->portMap.macNum,/*OUT*/&groupPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (groupPtr == NULL) /* group does not exist, use mac of remote port as group id  */
        {
            rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(devNum,portMapShadowPtr->portMap.macNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = prvCpssDxChPortDynamicPATxQClientGroupListClientAdd(devNum, portMapShadowPtr->portMap.macNum,
                                                                    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_REMOTE_PORT_E,
                                                                    portNum,
                                                                    portMapShadowPtr->portMap.txqNum,
                                                                    portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (paWsPtr->removeRemoteFromTxQWC == GT_TRUE)
        {
            GT_BOOL status;
            if (portSpeedInMBit > 0) /* alloc, remove port from WC mode */
            {
                status = GT_FALSE;
            }
            else                     /* delete port , restore WC status of TXQ port!!! */
            {
                status = GT_TRUE;
            }
            rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet(devNum,portMapShadowPtr->portMap.txqNum,status);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterComputeAndConfigure(devNum);
    if (rc != GT_OK)
    {
        goto error_restore_db;
    }
    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nConfigure dev %2d  port %2d speed = %5d",
                                devNum, portNum, portSpeedInMBit);
            rc = gtBobcat2PortPizzaArbiterIfStateDump(devNum,0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    #endif
    return GT_OK;
error_restore_db:
    /*------------------------------*/
    /* restore  DB in case of error */
    /*------------------------------*/
    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        GT_STATUS rc1;  /* use new status in order not to harm real status, usually this call returns without error */
        rc1 = prvCpssDxChPortDynamicPAPortSpeedDBSet(devNum,portNum,speedDBMbpsOld);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfCCFCClientConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for CCFC port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] macNum                   - mac Num
*                                      txq       - txq number
* @param[in] portSpeedInMBit          - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfCCFCClientConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  macNum,
    IN  GT_U32                  txqNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* Get the Work Space */
    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* create the group if it does not exists */
    rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,macNum,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr == NULL) /* group does not exists, use mac of CCFC port as group id  */
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(devNum,macNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    rc = prvCpssDxChPortDynamicPATxQClientGroupListClientAdd(devNum, macNum,
                                                                PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_REMOTE_PORT_E,
                                                                GT_NA,
                                                                txqNum,
                                                                portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paWsPtr->removeRemoteFromTxQWC == GT_TRUE)
    {
        GT_BOOL status;
        if (portSpeedInMBit > 0)
        {
            status = GT_FALSE;
        }
        else /* delete operataion */
        {
            status = GT_TRUE;
        }
        /* Set/Reset the Work Conserving status of the TxQ */
        rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet(devNum,txqNum,status);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterComputeAndConfigure(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nConfigure dev %2d  CCFC mac = %2d txq = %d speed = %5d",
                                devNum, macNum, txqNum, portSpeedInMBit);
            rc = gtBobcat2PortPizzaArbiterIfStateDump(devNum,0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    #endif
    return GT_OK;

}



/*----------------------------------------------------------------*/
/* Interlaken PA                                                  */
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* Build configuration according to stored DB(old configuration)  */
/*----------------------------------------------------------------*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelBuildConfigByStoredDB
(
    IN  GT_U8       devNum,
    OUT GT_FLOAT64  clientSpeedConfigArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_STATUS                   rc;
    GT_BOOL                     isPortConfigured;
    GT_U32                      speedEnt;
    GT_FLOAT64                  speedFloat;
    GT_PHYSICAL_PORT_NUM        maxPhysPortNum;
    GT_PHYSICAL_PORT_NUM        physPortNum;
    GT_U32                      specificChannelNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* Init output array */
    for (specificChannelNum = 0 ; specificChannelNum< PRV_CPSS_MAX_PP_PORTS_NUM_CNS ; specificChannelNum++)
    {
        clientSpeedConfigArr[specificChannelNum] = 0;
    }

    maxPhysPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (physPortNum = 0 ; physPortNum < maxPhysPortNum; physPortNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,physPortNum,/*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (GT_TRUE == portMapShadowPtr->valid)
        {
            rc = prvCpssDxChPortDynamicPAIlknChannelSpeedDBGet(devNum,physPortNum,/*OUT*/&isPortConfigured,/*OUT*/&speedEnt);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (GT_TRUE == isPortConfigured && portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E)
            {
                speedFloat = (GT_FLOAT64)speedEnt;
                clientSpeedConfigArr[portMapShadowPtr->portMap.ilknChannel] += speedFloat;
            }
        }
    }

    return GT_OK;
}


/*-------------------------------------------------------*/
/* now input portNum is configured by old configuration  */
/* correct it ports' speed on delta of new configuration */
/*-------------------------------------------------------*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelCorrectStoredConfigForSpecificPort
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit,
    INOUT GT_FLOAT64            clientSpeedConfigArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_STATUS                   rc;
    GT_BOOL                     isPortConfigured;
    GT_U32                      speedEnt;
    GT_FLOAT64                  speedFloat;
    GT_FLOAT64                  entrySpeedFloat;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32                      specificChannelNum;
    GT_U32                      speedResolutionMBbps;
    GT_U32                      unitBWInGBit;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    entrySpeedFloat = (GT_FLOAT64)portSpeedInMBit;

    rc = prvCpssDxChPortDynamicPAIlknChannelSpeedDBGet(devNum,portNum,/*OUT*/&isPortConfigured,/*OUT*/&speedEnt);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_TRUE == isPortConfigured)   /* correct speed on already configured value */
    {

        speedFloat = entrySpeedFloat - (GT_FLOAT64)speedEnt;
    }
    else
    {
        speedFloat = entrySpeedFloat;
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    clientSpeedConfigArr[portMapShadowPtr->portMap.ilknChannel] += speedFloat;

    /* PATCH : check whether speed resolution is greater than 40M : shall be chanhged in near future */
    rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(devNum,CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E, /*OUT*/&unitBWInGBit);
    if (rc != GT_OK)
    {
        return rc;
    }
    speedResolutionMBbps = unitBWInGBit*PRV_CPSS_DXCH_PA_BW_COEFF;
    for (specificChannelNum = 0 ; specificChannelNum< PRV_CPSS_MAX_PP_PORTS_NUM_CNS ; specificChannelNum++)
    {
        if (clientSpeedConfigArr[specificChannelNum] > 0)
        {
            speedResolutionMBbps = prvCpssDxChDynPAGCDAlgo((GT_U32)clientSpeedConfigArr[specificChannelNum],speedResolutionMBbps);
            if (speedResolutionMBbps < 40)
            {
                cpssOsPrintf("\nERROR : speed resolution smaller than 40MBps : channel %d",specificChannelNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}



GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS rc;   /* return code */

    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    dynamicPaConfigList_STC   *paListPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    paListPtr = cpssOsMalloc(sizeof(*paListPtr));
    if (paListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(paListPtr->prv_pizzaArray,0,sizeof(paListPtr->prv_pizzaArray));
    /*----------------------------------------------------------------*/
    /* Build configuration according to stored DB(old configuration)  */
    /*----------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelBuildConfigByStoredDB(devNum,/*OUT*/paListPtr->prv_clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E]);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    /*-------------------------------------------------------*/
    /* now input portNum is configured by old configuration  */
    /* correct it ports' speed on delta of new configuration */
    /*-------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelCorrectStoredConfigForSpecificPort(devNum, portNum,portSpeedInMBit,
                                                                                           /*OUT*/paListPtr->prv_clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E]);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    cpssOsFree(paListPtr);
    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS rc;   /* return code */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    /* PRV_CPSS_GEN_PP_CONFIG_STC        *pDev; */
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    dynamicPaConfigList_STC   *paListPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    paListPtr = cpssOsMalloc(sizeof(*paListPtr));
    if (paListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(paListPtr->prv_pizzaArray,0,sizeof(paListPtr->prv_pizzaArray));
    /*----------------------------------------------------------------*/
    /* Build configuration according to stored DB(old configuration)  */
    /*----------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelBuildConfigByStoredDB(devNum,/*OUT*/paListPtr->prv_clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E]);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    /*-------------------------------------------------------*/
    /* now input portNum is configured by old configuration  */
    /* correct it ports' speed on delta of new configuration */
    /*-------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelCorrectStoredConfigForSpecificPort(devNum, portNum,portSpeedInMBit,
                                                                                           /*OUT*/paListPtr->prv_clientSpeedConfigArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E]);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    /*----------------------------------*/
    /* now compute pizzas for all units */
    /*----------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfComputeUnitDistributions(paListPtr, paWsPtr,devNum,CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    /*----------------------------------------------------------------------*/
    /* now all data was prepared , configure HW (High Speed Ports and Pizza)*/
    /*----------------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfConfigDistributionsInHW(paListPtr, paWsPtr,devNum,CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }
    /*---------------------*/
    /* update DB           */
    /*---------------------*/
    rc = prvCpssDxChPortDynamicPAIlknChannelSpeedDBSet(devNum,portNum,portSpeedInMBit);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nConfigure dev %2d  port %2d speed = %5d",
                                devNum, portNum, portSpeedInMBit);
            rc = gtBobcat2PortPizzaArbiterIfStateDump(devNum,0);
            if (rc != GT_OK)
            {
                cpssOsFree(paListPtr);
                return rc;
            }
        }
    #endif

    cpssOsFree(paListPtr);
    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfDevStateGet function
* @endinternal
*
* @brief   Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Pizza Arbiter State which includes state of all
*         -
*         - state of slices
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
*
* @param[out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS   rc;         /* return code */
    CPSS_DXCH_PA_UNIT_ENT unitType;
    GT_U32                unitIdx;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC * unitStatePtrArr[CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    PRV_CPSS_GEN_PP_CONFIG_STC        *devPtr; /* pointer to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(pizzaDeviceStatePtr);

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    devPtr = PRV_CPSS_PP_MAC(devNum);
    pizzaDeviceStatePtr->devType = devPtr->devFamily;
    /* old interface , shall be disappear after lua will changed to a new interface, just for BC2 !!! */
    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E && devPtr->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
    {
        portGroupId = portGroupId;
        cpssOsMemSet(unitStatePtrArr, 0, sizeof(unitStatePtrArr));

        unitStatePtrArr[CPSS_DXCH_PA_UNIT_RXDMA_0_E       ] = &pizzaDeviceStatePtr->devState.bc2.rxDMA;
        unitStatePtrArr[CPSS_DXCH_PA_UNIT_TXDMA_0_E       ] = &pizzaDeviceStatePtr->devState.bc2.txDMA;
        unitStatePtrArr[CPSS_DXCH_PA_UNIT_TXQ_0_E         ] = &pizzaDeviceStatePtr->devState.bc2.TxQ;
        unitStatePtrArr[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E     ] = &pizzaDeviceStatePtr->devState.bc2.txFIFO;
        unitStatePtrArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E ] = &pizzaDeviceStatePtr->devState.bc2.ethFxFIFO;
        unitStatePtrArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E] = &pizzaDeviceStatePtr->devState.bc2.ilknTxFIFO;

        for (unitType = (CPSS_DXCH_PA_UNIT_ENT)0 ;  unitType < CPSS_DXCH_PA_UNIT_MAX_E; unitType++)
        {
            if(paWsPtr->prv_unit2PizzaAlgoFunArr[unitType] == NULL)
            {
                continue;
            }
            rc = bobcat2PizzaArbiterUnitDrvGet(devNum,unitType,/*OUT*/unitStatePtrArr[unitType]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*----------------------------------
     * copy Unit List
     *----------------------------------*/
    for (unitIdx = 0 ; unitIdx < sizeof(pizzaDeviceStatePtr->devState.bobK.unitList)/sizeof(pizzaDeviceStatePtr->devState.bobK.unitList[0]); unitIdx++)
    {
        pizzaDeviceStatePtr->devState.bobK.unitList[unitIdx] = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
    }
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        pizzaDeviceStatePtr->devState.bobK.unitList[unitIdx] = unitType;

        rc = bobcat2PizzaArbiterUnitDrvGet(devNum,unitType,/*OUT*/&pizzaDeviceStatePtr->devState.bobK.unitState[unitIdx]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}



#if (__DYNAMIC_PIZZA_ARBITER_TEST == 1)

    typedef struct
    {
        GT_PHYSICAL_PORT_NUM portNum;
        CPSS_PORT_SPEED_ENT  speed;
        GT_STATUS            res;
    }PRV_CPSS_DXCH_PORT_SPEED_STC;

    typedef struct
    {
        GT_CHAR                      * namePtr;
        PRV_CPSS_DXCH_PORT_SPEED_STC * listPtr;
        GT_U32                         size;
    }PRV_DYNPIZZA_TEST_CASE_STC;

    typedef struct
    {
        CPSS_PORT_SPEED_ENT   speedEnt;
        GT_CHAR              *speedStr;
    }PRV_SPEED2STR_STC;


    static PRV_CPSS_DXCH_PORT_SPEED_STC prv_testPortSpeedArrNotSupported[] =
    {
         {    0, CPSS_PORT_SPEED_10_E        , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_100_E       , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_12000_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_13600_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_15000_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_16000_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_75000_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_50000_E     , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_100G_E      , GT_NOT_SUPPORTED }
        ,{    0, CPSS_PORT_SPEED_140G_E      , GT_NOT_SUPPORTED }
    };

    static PRV_CPSS_DXCH_PORT_SPEED_STC prv_testPortSpeedArrNotMapped[] =
    {
         {   52, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM }
        ,{   53, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM }
        ,{   54, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM }
        ,{   55, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   59, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   60, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   61, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   62, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   64, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   65, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   66, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   67, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   68, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   69, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   70, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   71, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   72, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   73, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   74, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   75, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   76, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   77, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   78, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   79, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   81, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
        ,{   83, CPSS_PORT_SPEED_1000_E       , GT_BAD_PARAM  }
    };


    static PRV_CPSS_DXCH_PORT_SPEED_STC prv_testPortSpeedArr[] =
    {
        /* 1G */
         {    0, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    1, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    2, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    3, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    4, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    5, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    6, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    7, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    8, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{    9, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   10, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   11, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   12, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   13, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   14, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   15, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   16, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   17, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   18, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   19, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   20, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   21, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   22, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   23, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   24, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   25, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   26, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   27, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   28, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   29, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   30, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   31, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   32, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   33, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   34, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   35, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   36, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   37, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   38, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   39, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   40, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   41, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   42, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   43, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   44, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   45, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   46, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   47, CPSS_PORT_SPEED_1000_E      , GT_OK }
        ,{   48, CPSS_PORT_SPEED_1000_E      , GT_OK }
        /* 10 G */
        ,{   49, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   50, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   51, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   56, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   58, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   80, CPSS_PORT_SPEED_10000_E     , GT_OK }
        ,{   82, CPSS_PORT_SPEED_10000_E     , GT_OK }
        /* CPU port */
        ,{   63, CPSS_PORT_SPEED_10000_E     , GT_OK }
    };


    static PRV_DYNPIZZA_TEST_CASE_STC prv_dynPATestList[] =
    {
         {
             "NOT SUPPORTED"
             ,&prv_testPortSpeedArrNotSupported[0]
             ,sizeof(prv_testPortSpeedArrNotSupported)/sizeof(prv_testPortSpeedArrNotSupported[0])
         }
        ,{
             "NOT MAPPED PORTS"
             ,&prv_testPortSpeedArrNotMapped[0]
             ,sizeof(prv_testPortSpeedArrNotMapped)/sizeof(prv_testPortSpeedArrNotMapped[0])
         }
        ,{
             "Regular work"
            ,&prv_testPortSpeedArr[0]
            ,sizeof(prv_testPortSpeedArr)/sizeof(prv_testPortSpeedArr[0])
        }
    };

    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
    #include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
    #include <appDemo/sysHwConfig/appDemoDb.h>

    static PRV_SPEED2STR_STC prv_speedConvArr[] =
    {
         { CPSS_PORT_SPEED_10_E,      "  10M"  }
        ,{ CPSS_PORT_SPEED_100_E,     " 100M"  }
        ,{ CPSS_PORT_SPEED_1000_E,    "   1G"  }
        ,{ CPSS_PORT_SPEED_2500_E,    " 2.5G"  }
        ,{ CPSS_PORT_SPEED_5000_E,    "   5G"  }
        ,{ CPSS_PORT_SPEED_10000_E,   "  10G"  }
        ,{ CPSS_PORT_SPEED_12000_E,   "  12G"  }
        ,{ CPSS_PORT_SPEED_16000_E,   "  16G"  }
        ,{ CPSS_PORT_SPEED_15000_E,   "  15G"  }
        ,{ CPSS_PORT_SPEED_13600_E,   "13.6G"  }
        ,{ CPSS_PORT_SPEED_20000_E,   "  20G"  }
        ,{ CPSS_PORT_SPEED_40000_E,   "  40G"  }
        ,{ CPSS_PORT_SPEED_50000_E,   "  50G"  }
        ,{ CPSS_PORT_SPEED_75000_E,   "  75G"  }
        ,{ CPSS_PORT_SPEED_100G_E,    " 100G"  }
        ,{ CPSS_PORT_SPEED_140G_E,    " 140G"  }
        ,{ CPSS_PORT_SPEED_NA_E  ,    "  N/A"  }
    };

    GT_CHAR * prvTestFindSpeed(CPSS_PORT_SPEED_ENT speedEnm)
    {
        GT_U32 i;
        GT_U32 size;
        PRV_SPEED2STR_STC * ptr;
        ptr = &prv_speedConvArr[0];

        size = sizeof(prv_speedConvArr)/sizeof(prv_speedConvArr[0]);
        prv_speedConvArr[size-1].speedEnt = speedEnm;
        for (i = 0; ptr->speedEnt != speedEnm ; i++, ptr++);
        prv_speedConvArr[size-1].speedEnt = CPSS_PORT_SPEED_NA_E;
        return ptr->speedStr;
    }

    typedef enum
    {
        CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E,
        CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E,
        CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,
        CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,
        CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ALL_E

    }CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT;

    extern GT_STATUS appDemoTraceHwAccessEnable
    (
        IN GT_U8                                devNum,
        IN CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT    accessType,
        IN GT_BOOL                              enable
    );



    #define APP_INV_PORT_CNS ((GT_U32)(~0))
    typedef enum
    {
         PORT_LIST_TYPE_EMPTY = 0
        ,PORT_LIST_TYPE_INTERVAL
        ,PORT_LIST_TYPE_LIST
    }PortListType_ENT;

    typedef struct
    {
        PortListType_ENT             entryType;
        GT_PHYSICAL_PORT_NUM         portList[32];  /* depending on list type */
                                                    /* interval : 0 startPort     */
                                                    /*            1 stopPort      */
                                                    /*            2 step          */
                                                    /*            3 APP_INV_PORT_CNS */
                                                    /* list     : 0...x  ports     */
                                                    /*            APP_INV_PORT_CNS */
        CPSS_PORT_SPEED_ENT          speed;
        CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
    }PortInitList_STC;

    typedef struct
    {
        GT_PHYSICAL_PORT_NUM         portNum;
        CPSS_PORT_SPEED_ENT          speed;
        CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
    }PortInitInternal_STC;


    static CPSS_DXCH_PORT_MAP_STC bc2TmEnable_0_2_48_49_Map[] =
    { /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
         {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,   GT_NA,             GT_TRUE,            0 }
        ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,   GT_NA,             GT_TRUE,            2 }
        ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,   GT_NA,             GT_TRUE,           48 }
        ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,   GT_NA,             GT_TRUE,           49 }
    };

    static PortInitList_STC portInitlist_TmEnable_0_2_48_49[] =
    {
         { PORT_LIST_TYPE_LIST,      { 0,2,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
        ,{ PORT_LIST_TYPE_LIST,      {48,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_LIST,      {49,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };

    static char * prv_speed2str[] =
    {
        "10M  ",   /* 0 */
        "100M ",   /* 1 */
        "1G   ",   /* 2 */
        "10G  ",   /* 3 */
        "12G  ",   /* 4 */
        "2.5G ",   /* 5 */
        "5G   ",   /* 6 */
        "13.6G",   /* 7 */
        "20G  ",   /* 8 */
        "40G  ",   /* 9 */
        "16G  ",   /* 10 */
        "15G  ",   /* 11 */
        "75G  ",   /* 12 */
        "100G ",   /* 13 */
        "50G  ",   /* 14 */
        "140G "    /* 15 */
    };

    static char * prv_prvif2str[] =
    {
        "REDUCED_10BIT"   /* 0   CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E,    */
        ,"REDUCED_GMII"    /* 1   CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,     */
        ,"MII         "    /* 2   CPSS_PORT_INTERFACE_MODE_MII_E,              */
        ,"SGMII       "    /* 3   CPSS_PORT_INTERFACE_MODE_SGMII_E,            */
        ,"XGMII       "    /* 4   CPSS_PORT_INTERFACE_MODE_XGMII_E,            */
        ,"MGMII       "    /* 5   CPSS_PORT_INTERFACE_MODE_MGMII_E,            */
        ,"1000BASE_X  "    /* 6   CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,       */
        ,"GMII        "    /* 7   CPSS_PORT_INTERFACE_MODE_GMII_E,             */
        ,"MII_PHY     "    /* 8   CPSS_PORT_INTERFACE_MODE_MII_PHY_E,          */
        ,"QX          "    /* 9   CPSS_PORT_INTERFACE_MODE_QX_E,               */
        ,"HX          "    /* 10  CPSS_PORT_INTERFACE_MODE_HX_E,                */
        ,"RXAUI       "    /* 11  CPSS_PORT_INTERFACE_MODE_RXAUI_E,             */
        ,"100BASE_FX  "    /* 12  CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,        */
        ,"QSGMII      "    /* 13  CPSS_PORT_INTERFACE_MODE_QSGMII_E,            */
        ,"XLG         "    /* 14  CPSS_PORT_INTERFACE_MODE_XLG_E,               */
        ,"LOCAL_XGMII "    /* 15  CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,       */
        ,"KR          "    /* 16  CPSS_PORT_INTERFACE_MODE_KR_E,                */
        ,"HGL         "    /* 17  CPSS_PORT_INTERFACE_MODE_HGL_E,               */
        ,"CHGL_12     "    /* 18  CPSS_PORT_INTERFACE_MODE_CHGL_12_E,           */
        ,"ILKN12      "    /* 19  CPSS_PORT_INTERFACE_MODE_ILKN12_E,            */
        ,"SR_LR       "    /* 20  CPSS_PORT_INTERFACE_MODE_SR_LR_E,             */
        ,"ILKN16      "    /* 21  CPSS_PORT_INTERFACE_MODE_ILKN16_E,            */
        ,"ILKN24      "    /* 22  CPSS_PORT_INTERFACE_MODE_ILKN24_E,            */
        ,"NA_E        "    /* 23  CPSS_PORT_INTERFACE_MODE_NA_E                 */
    };

    static GT_CHAR * prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E] =
    {
         "ETHERNET"
        ,"CPU-SDMA"
        ,"ILKN-CHL"
        ,"REMOTE-P"
    };


    static GT_STATUS TestPort0x20x48x49_PortInterfaceInit
    (
        IN  GT_U8 dev
    )
    {
        GT_STATUS   rc;             /* return code */
        GT_U32      i;                  /* interator */
        CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                            *initPortsBmpPtr;/* pointer to bitmap */
        PortInitList_STC * portInitList;
        PortInitList_STC * portInitPtr;
        GT_U32         portIdx;
        GT_U32         maxPortIdx;
        GT_PHYSICAL_PORT_NUM portNum;
        static PortInitInternal_STC    portList[256];

        initPortsBmpPtr = &initPortsBmp;

        portInitList = &portInitlist_TmEnable_0_2_48_49[0];

        for (portIdx = 0 ; portIdx < sizeof(portList)/sizeof(portList[0]); portIdx++)
        {
            portList[portIdx].portNum       = APP_INV_PORT_CNS;
            portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
            portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        maxPortIdx = 0;
        portInitPtr = &portInitList[0];
        for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
        {
            for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
            {
                GT_BOOL _isValid;

                portNum = portInitPtr->portList[portIdx];
                rc = cpssDxChPortPhysicalPortMapIsValidGet(dev, portNum, &_isValid);
                if (rc == GT_OK && _isValid == GT_TRUE)
                {
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            }
        }

        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
        cpssOsPrintf("\n| #  | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |  res |");
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");

        for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
        {
            CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr;
            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(dev,portList[portIdx].portNum,/*OUT*/&portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsPrintf("\n| %2d | %4d | %s | %s |",portIdx,
                                                  portList[portIdx].portNum,
                                                  prv_speed2str[portList[portIdx].speed],
                                                  prv_prvif2str[portList[portIdx].interfaceMode]);
            cpssOsPrintf(" %-15s | %5d %5d %3d %3d %4d %3d |"
                                                ,prv_mappingTypeStr[portMapPtr->portMap.mappingType]
                                                ,portMapPtr->portMap.rxDmaNum
                                                ,portMapPtr->portMap.txDmaNum
                                                ,portMapPtr->portMap.macNum
                                                ,portMapPtr->portMap.txqNum
                                                ,portMapPtr->portMap.ilknChannel
                                                ,portMapPtr->portMap.tmPortIdx);


            PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
            CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portList[portIdx].portNum);
            appDemoTraceHwAccessEnable(dev, CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E, GT_TRUE);
            rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_TRUE,
                                            portList[portIdx].interfaceMode,
                                            portList[portIdx].speed);
            if(rc != GT_OK)
            {
                cpssOsPrintf("\n--> ERROR : cpssDxChPortModeSpeedSet() :rc=%d\n", rc);
                return rc;
            }
            appDemoTraceHwAccessEnable(dev, CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E, GT_FALSE);
            cpssOsPrintf("  OK  |");
        }
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
        cpssOsPrintf("\nPORT INTERFACE Init Done.");
        return GT_OK;
    }


    GT_STATUS TestPort0x20x48x49(GT_VOID)
    {
        GT_STATUS rc;
        GT_U32 initSerdesDefaults;
        GT_U8 dev = 0;

        initSerdesDefaults = 0;
        rc = appDemoDbEntryAdd("initSerdesDefaults", initSerdesDefaults);

        rc = cpssInitSystem(29,1,0);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortPhysicalPortMapSet(dev, sizeof(bc2TmEnable_0_2_48_49_Map)/sizeof(bc2TmEnable_0_2_48_49_Map[0]), &bc2TmEnable_0_2_48_49_Map[0]);
        if (GT_OK != rc)
        {
            return rc;
        }
        rc = TestPort0x20x48x49_PortInterfaceInit(dev);
        return rc;
    }

    GT_STATUS prvTestDynPA(GT_VOID)
    {
        GT_STATUS rc;
        GT_U32    testCase;
        GT_U32    idx;
        PRV_CPSS_DXCH_PORT_SPEED_STC * ptr;


        rc = cpssInitSystem(29,1,0);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*rc = prvCpssDxChPortDynamicPizzaArbiterIfInit(0);
          if (rc != GT_OK)
          {
              return rc;
          }
        */
        for (testCase = 0; testCase< sizeof(prv_dynPATestList)/sizeof(prv_dynPATestList[0]); testCase++)
        {
            printf("\n--------------------------------------");
            printf("\n  %s ",prv_dynPATestList[testCase].namePtr);
            printf("\n--------------------------------------");
            ptr = prv_dynPATestList[testCase].listPtr;
            for (idx = 0 ; idx < prv_dynPATestList[testCase].size; idx++, ptr++)
            {
                printf("\n %4d %2d : speed = %s exp-rc = %d ... ",idx,ptr->portNum, prvTestFindSpeed(ptr->speed), ptr->res);

                rc = prvCpssDxChPortPizzaArbiterIfCheckSupport(0,ptr->portNum,ptr->speed);
                printf("Check rc = %d   ", rc);
                if (ptr->res != rc)
                {
                    printf(".....Error !!!");
                    return rc;
                }

                rc = prvCpssDxChPortPizzaArbiterIfConfigure(0,ptr->portNum,ptr->speed);
                printf("Conf rc = %d", rc);

                if (ptr->res != rc)
                {
                    printf(".....Error !!! once more");
                    rc = prvCpssDxChPortPizzaArbiterIfConfigure(0,ptr->portNum,ptr->speed);
                    return rc;
                }
            }
        }
        return GT_OK;
    }

#endif

/************************************************************************************/
/* OVERRIDING PIZZA SLICES MAPPING                                                   */
/************************************************************************************/

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet function
* @endinternal
*
* @brief   Get Pizza Arbiter Unit Channel Speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in]  devNum                   - device number
* @param[in]  unitId                   - Id of unit containing pizza arbiter
* @param[in]  removeOtherChannels      - GT_TRUE - all new slice map specified in this function
*                                                  parameters channel arrays
*                                        GT_FALSE - update only speed of specified channels
*                                                speed of other channels left unchanged
* @param[in]  channelArrSize           - size of arrays for channels
* @param[out] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[out] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    OUT GT_U32                  *channelArrActualSizePtr,
    OUT GT_U32                  *channelNumArrPtr,
    OUT GT_U32                  *channelSpeedInMBitArrPtr
)
{
    GT_STATUS                    rc;
    GT_U32                       unitBWInGBit;
    GT_U32                       unitBWInMBit;
    GT_U32                       numOfSlices;
    GT_U32                       *slicesMapArrPtr;
    GT_U32                       sliceBWInMBit;
    GT_U32                       sliceId;
    GT_U32                       channel;
    GT_U32                       channelIdx;
    GT_U32                       numOfChannels;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(channelArrActualSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(channelNumArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(channelSpeedInMBitArrPtr);

    rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(devNum,unitId, /*OUT*/&unitBWInGBit);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    unitBWInMBit = unitBWInGBit*PRV_CPSS_DXCH_PA_BW_COEFF;

    /* retrieve amount of used slices only */
    rc = bobcat2PizzaArbiterUnitDrvSlicesMapGet(
        devNum, unitId, 0 /*slice2PortMapArrSize*/,
        &numOfSlices, NULL /*slice2PortMapArrPtr*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (numOfSlices == 0)
    {
        *channelArrActualSizePtr = 0;
        return GT_OK;
    }

    slicesMapArrPtr = cpssOsMalloc(sizeof(GT_U32) * numOfSlices);
    if (slicesMapArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* retrieve all slices mapping */
    rc = bobcat2PizzaArbiterUnitDrvSlicesMapGet(
        devNum, unitId, numOfSlices /*slice2PortMapArrSize*/,
        &numOfSlices, slicesMapArrPtr);
    if (rc != GT_OK)
    {
        cpssOsFree(slicesMapArrPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    sliceBWInMBit = (unitBWInMBit / numOfSlices);

    numOfChannels = 0;
    *channelArrActualSizePtr = 0;
    for (sliceId = 0; (sliceId < numOfSlices); sliceId++)
    {
        if (numOfChannels >= channelArrSize)
        {
            cpssOsFree(slicesMapArrPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        channel = slicesMapArrPtr[sliceId];
        if (channel == PA_INVALID_PORT_CNS) continue;

        /* look for channel */
        for (channelIdx = 0; (channelIdx < numOfChannels); channelIdx++)
        {
            if (channel == channelNumArrPtr[channelIdx]) break;
        }
        if (channelIdx < numOfChannels)
        {
            /* found channel - add BW only */
            channelSpeedInMBitArrPtr[channelIdx] += sliceBWInMBit;
        }
        else
        {
            /* new channel - initialize */
            channelNumArrPtr[numOfChannels]         = channel;
            channelSpeedInMBitArrPtr[numOfChannels] = sliceBWInMBit;
            numOfChannels ++;
            *channelArrActualSizePtr = numOfChannels;
        }
    }

    cpssOsFree(slicesMapArrPtr);
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedOverride function
* @endinternal
*
* @brief   Override Pizza Arbiter Unit Channel Speed (ingnoring previous state)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - Id of unit containing pizza arbiter
* @param[in] channelArrSize           - size of arrays of specified channels
* @param[in] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[in] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedOverride
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    IN  GT_U32                  *channelNumArrPtr,
    IN  GT_U32                  *channelSpeedInMBitArrPtr
)
{
    GT_STATUS               rc;
    GT_U32                  maxNumOfChannels;
    GT_U32                  channel;
    GT_U32                  channelIdx;
    GT_U32                  unitBWInGBit;
    GT_U32                  unitBWInMBit;
    GT_U32                  minSliceResolutionInMBps;
    GT_U32                  maxPizzaSlices;
    GT_U32                  *pizzaSlicesArrPtr;
    GT_U32                  sliceNum2Config;
    GT_U32                  highSpeedPortNumber;
    GT_U32                  highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];/*8*/
    GT_FLOAT64              *channelSpeedConfigArrPtr;
    PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN algoDistribFun;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             *paWsPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    if (channelArrSize != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(channelNumArrPtr);
        CPSS_NULL_PTR_CHECK_MAC(channelSpeedInMBitArrPtr);
    }

    maxNumOfChannels = PRV_CPSS_MAX_MAC_PORTS_NUM_CNS;

    rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(devNum, unitId, /*OUT*/&unitBWInGBit);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    unitBWInMBit = unitBWInGBit*PRV_CPSS_DXCH_PA_BW_COEFF;

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (paWsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = bobcat2PizzaArbiterUnitDrvSlicesMapMaxSizeGet(
        devNum, unitId, &maxPizzaSlices);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    minSliceResolutionInMBps = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;
    algoDistribFun = paWsPtr->prv_unit2PizzaAlgoFunArr[unitId];
    if (NULL == algoDistribFun)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    pizzaSlicesArrPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32) * maxPizzaSlices);
    if (NULL == pizzaSlicesArrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    channelSpeedConfigArrPtr = (GT_FLOAT64*)cpssOsMalloc(sizeof(GT_FLOAT64) * maxNumOfChannels);
    if (NULL == channelSpeedConfigArrPtr)
    {
        cpssOsFree(pizzaSlicesArrPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* build full array of channel speeds */
    for (channel = 0; (channel < maxNumOfChannels); channel++)
    {
        channelSpeedConfigArrPtr[channel] = 0;
    }
    for (channelIdx = 0; (channelIdx < channelArrSize); channelIdx++)
    {
        channel = channelNumArrPtr[channelIdx];
        if (channel >= maxNumOfChannels)
        {
            cpssOsFree(pizzaSlicesArrPtr);
            cpssOsFree(channelSpeedConfigArrPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        channelSpeedConfigArrPtr[channel] = channelSpeedInMBitArrPtr[channelIdx];
    }

    rc = algoDistribFun(
        /*IN*/channelSpeedConfigArrPtr,
        /*IN*/ maxNumOfChannels,
        /*IN*/ unitBWInMBit,
        /*IN*/ minSliceResolutionInMBps,
        /*IN*/ &(paWsPtr->prv_txQHighSpeedPortThreshParams),
        /*IN*/ maxPizzaSlices,
        /*OUT*/pizzaSlicesArrPtr,
        /*OUT*/&sliceNum2Config,
        /*OUT*/&highSpeedPortNumber,
        /*OUT*/highSpeedPortArr);
    cpssOsFree(channelSpeedConfigArrPtr);
    if (rc != GT_OK)
    {
        cpssOsFree(pizzaSlicesArrPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = bobcat2PizzaArbiterUnitDrvSet(
        devNum, unitId, sliceNum2Config, pizzaSlicesArrPtr);
    cpssOsFree(pizzaSlicesArrPtr);
    return rc;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedUpdate function
* @endinternal
*
* @brief   Update Pizza Arbiter Unit Channel Speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - Id of unit containing pizza arbiter
* @param[in] channelArrSize           - size of arrays of specified channels
* @param[in] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[in] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedUpdate
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    IN  GT_U32                  *channelNumArrPtr,
    IN  GT_U32                  *channelSpeedInMBitArrPtr
)
{
    GT_STATUS               rc;
    GT_U32                  maxNumOfChannels;
    GT_U32                  numOfChannels;
    GT_U32                  *finalChannelNumArr;
    GT_U32                  *finalChannelSpeedInMBitArr;
    GT_U32                  channel;
    GT_U32                  channelSrcIdx;
    GT_U32                  channelFinalIdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    if (channelArrSize != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(channelNumArrPtr);
        CPSS_NULL_PTR_CHECK_MAC(channelSpeedInMBitArrPtr);
    }

    maxNumOfChannels = PRV_CPSS_MAX_MAC_PORTS_NUM_CNS;

    /* merge old slice map with the given */
    finalChannelNumArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32) * maxNumOfChannels);
    if (finalChannelNumArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    finalChannelSpeedInMBitArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32) * maxNumOfChannels);
    if (finalChannelSpeedInMBitArr == NULL)
    {
        cpssOsFree(finalChannelNumArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet(
        devNum, unitId, maxNumOfChannels,
        &numOfChannels, finalChannelNumArr, finalChannelSpeedInMBitArr);
    if (rc != GT_OK)
    {
        cpssOsFree(finalChannelNumArr);
        cpssOsFree(finalChannelSpeedInMBitArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    for (channelSrcIdx = 0; (channelSrcIdx < channelArrSize); channelSrcIdx++)
    {
        channel = channelNumArrPtr[channelSrcIdx];
        for (channelFinalIdx = 0; (channelFinalIdx < numOfChannels); channelFinalIdx++)
        {
            if (finalChannelNumArr[channelFinalIdx] == channel) break;
        }
        if (channelFinalIdx < numOfChannels)
        {
            /* replace speed of found channel */
            finalChannelSpeedInMBitArr[channelFinalIdx] =
                channelSpeedInMBitArrPtr[channelSrcIdx];
        }
        else
        {
            /* add new channel */
            finalChannelNumArr[channelFinalIdx] = channel;
            finalChannelSpeedInMBitArr[numOfChannels] =
                channelSpeedInMBitArrPtr[channelSrcIdx];
            numOfChannels ++;
        }
        if (numOfChannels >= maxNumOfChannels)
        {
            cpssOsFree(finalChannelNumArr);
            cpssOsFree(finalChannelSpeedInMBitArr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedOverride(
        devNum, unitId, numOfChannels,
        finalChannelNumArr, finalChannelSpeedInMBitArr);
    cpssOsFree(finalChannelNumArr);
    cpssOsFree(finalChannelSpeedInMBitArr);
    return rc;
}

/**
* @internal prvCpssDxChPortPizzaArbiterTxFifoChannelSpeedFactorSet function
* @endinternal
*
* @brief   Set Bandwidth speed factor of TX_FIFO Unit Pizza Arbiter Channel
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port mapped to TX_FIFO Pizza Arbitter Channel
*                                       being configured
* @param[in] speedFactor              - TX_FIFO Pizza Arbiter Channel speed factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterTxFifoChannelSpeedFactorSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              speedFactor
)
{
    GT_STATUS                  rc;
    GT_U32                     oldSpeedFactor;
    GT_BOOL                    isInitilized;
    GT_U32                     speedInMBit;

    /* port and device validity checked inside called function */
    /* backup an old value to restore at fail case             */
    rc = prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet(
        devNum, portNum, &oldSpeedFactor);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet(
        devNum, portNum, speedFactor);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(
        devNum, portNum, &isInitilized, &speedInMBit);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if ((isInitilized == GT_FALSE) || (speedInMBit == 0))
    {
        /* port speed not configured - reconfiguration not needed */
        return GT_OK;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck(devNum);
    if (rc != GT_OK)
    {
        if (rc == GT_NO_RESOURCE)
        {
            cpssOsPrintf("\nNo Enough Bandwidth for required configuration\n");
        }
        /* restore saved value in DB */
        prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet(
            devNum, portNum, oldSpeedFactor);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterComputeAndConfigure(devNum);
    if (rc != GT_OK)
    {
        /* restore saved value in DB */
        prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet(
            devNum, portNum, oldSpeedFactor);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portSpeedGet function
* @endinternal
*
* @brief   Check Pizza Arbiter Bandwidth overbooking - get Port Speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[out] portSpeedInMBitPtr      - (pointer to))port speed in Megabit/Sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portSpeedGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U32                  *portSpeedInMBitPtr
)
{
    GT_STATUS                        rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  *portMapShadowPtr;
    GT_BOOL                          isPortConfigured;

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(
        devNum, portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        return prvCpssDxChPortDynamicPAPortSpeedDBGet(
            devNum, portNum, &isPortConfigured, portSpeedInMBitPtr);
    }

    /* CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E case */
    rc = prvCpssDxChRemotePortSpeedGet(
        devNum, portNum, portSpeedInMBitPtr);
    if (rc == GT_FAIL)
    {
        /* port not found in DB */
        *portSpeedInMBitPtr = 0;
        return GT_OK;
    }
    return rc;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portCfg function
* @endinternal
*
* @brief   Check Pizza Arbiter Bandwidth overbooking - configure port DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeedInMBit          - port speed in Megabit/Sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portCfg
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
)
{
    GT_STATUS                        rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  *portMapShadowPtr;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC   *paWsPtr;
    GT_U32                           portOldSpeedInMBit;
    GT_BOOL                          isPortConfigured;
    GT_BOOL                          reCfgCascadePort;

    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (paWsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(
        devNum, portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /* get old speed - important for cascade port */
        /* cascade port can and should be reconfigured */
        /* if exact one of old and new speed is zero   */
        rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(
            devNum, portNum, /*OUT*/&isPortConfigured, /*OUT*/&portOldSpeedInMBit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        reCfgCascadePort = GT_FALSE;
        if ((portOldSpeedInMBit == 0) && (portSpeedInMBit != 0))
        {
            reCfgCascadePort = GT_TRUE;
        }
        if ((portOldSpeedInMBit != 0) && (portSpeedInMBit == 0))
        {
            reCfgCascadePort = GT_TRUE;
        }

        /* configure new speed */
        rc = prvCpssDxChPortDynamicPAPortSpeedDBSet(
            devNum, portNum, portSpeedInMBit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* create the group if it does not exist */
        if (portMapShadowPtr->portMap.isExtendedCascadePort
            && (reCfgCascadePort == GT_TRUE))
        {
            PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
            rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(
                devNum, portMapShadowPtr->portMap.macNum, /*OUT*/&groupPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            if (portSpeedInMBit > 0)
            {
                if (groupPtr == NULL) /* group does not exist, use mac of extended-cascade port as group id  */
                {
                    rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(
                        devNum, portMapShadowPtr->portMap.macNum);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
                rc = prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet(
                    devNum, portMapShadowPtr->portMap.macNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else /* delete operation */
            {
                if (groupPtr != NULL) /* group does not exist, use mac of extended-cascade port as group id  */
                {
                    rc = prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet(
                        devNum, portMapShadowPtr->portMap.macNum, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    /* something bad we can't delete something that does not exist ??? */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    else /* mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E */
    {
        PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
        /* create the group if it does not exist */
        rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(
            devNum, portMapShadowPtr->portMap.macNum, /*OUT*/&groupPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (groupPtr == NULL) /* group does not exist, use mac of remote port as group id  */
        {
            rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(
                devNum, portMapShadowPtr->portMap.macNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        rc = prvCpssDxChPortDynamicPATxQClientGroupListClientAdd(
            devNum, portMapShadowPtr->portMap.macNum,
            PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_REMOTE_PORT_E,
            portNum,
            portMapShadowPtr->portMap.txqNum,
            portSpeedInMBit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck function
* @endinternal
*
* @brief   Check Pizza Arbiter Bandwidth overbooking according to new ports speeds
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] numOfPorts               - amount of ports
* @param[in] portNumArrPtr            - (pointer to) array of physical port numbers
* @param[in] portSpeedInMBitArrPtr    - (pointer to) array of port speeds in Megabit/Sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - on work memory allocation error
* @retval GT_NO_RESOURCE           - on Pizza Arbiter Bandwidth overbooking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  numOfPorts,
    IN  GT_PHYSICAL_PORT_NUM    *portNumArrPtr,
    IN  GT_U32                  *portSpeedInMBitArrPtr
)
{
    GT_STATUS                        rc;
    GT_STATUS                        finalRc = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_STC      *pDev;
    GT_U32                           *speedDBMbpsOldArrPtr;
    GT_U32                           ndx;
    GT_U32                           newPortSpeedInMbps;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if (numOfPorts == 0)
    {
        return prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck(devNum);
    }
    CPSS_NULL_PTR_CHECK_MAC(portNumArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(portSpeedInMBitArrPtr);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    speedDBMbpsOldArrPtr = (GT_U32*)cpssOsMalloc(numOfPorts * sizeof(GT_U32));
    if (speedDBMbpsOldArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    for (ndx = 0; (ndx < numOfPorts); ndx++)
    {
        /* save port speeds from DB to be restored */
        rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portSpeedGet(
            devNum, portNumArrPtr[ndx], /*OUT*/&(speedDBMbpsOldArrPtr[ndx]));
        if (rc != GT_OK)
        {
            cpssOsFree(speedDBMbpsOldArrPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* set new port speeds to ports DB */
    for (ndx = 0; (ndx < numOfPorts); ndx++)
    {
        /* new port speed aligned to PA resolution */
        newPortSpeedInMbps  = (portSpeedInMBitArrPtr[ndx] + pDev->paData.paMinSliceResolutionMbps - 1);
        newPortSpeedInMbps -= (newPortSpeedInMbps % pDev->paData.paMinSliceResolutionMbps);
        rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portCfg(
            devNum, portNumArrPtr[ndx], newPortSpeedInMbps);
        if (rc != GT_OK)
        {
            finalRc = rc;
            continue;
        }
    }

    if (finalRc == GT_OK)
    {
        finalRc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck(devNum);
    }

    /* restore DB */
    for (ndx = 0; (ndx < numOfPorts); ndx++)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck_portCfg(
            devNum, portNumArrPtr[ndx], speedDBMbpsOldArrPtr[ndx]);
        if (rc != GT_OK)
        {
            finalRc = GT_FAIL;
        }
    }
    cpssOsFree(speedDBMbpsOldArrPtr);
    if (finalRc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(finalRc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortsBitmapCheck function
* @endinternal
*
* @brief   Check Pizza Arbiter Bandwidth overbooking according to new speed of ports
*          given by bitmap.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap
* @param[in] portSpeed                - port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - on work memory allocation error
* @retval GT_NO_RESOURCE           - on Pizza Arbiter Bandwidth overbooking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortsBitmapCheck
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORTS_BMP_STC      *portsBmpPtr,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS               rc;
    GT_U32                  port;
    GT_U32                  numOfPorts;
    GT_PHYSICAL_PORT_NUM    *portNumArrPtr;
    GT_U32                  *portSpeedInMBitArrPtr;
    GT_U32                  portSpeedInMBit;
    GT_U32                  arrNdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);

    /* convert speed to Megabit/Sec                              */
    /* porNum is redundant parameter not used in called function */
    rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(
        devNum, 0/*portNum*/, portSpeed, /*OUT*/&portSpeedInMBit);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* count amount of ports */
    numOfPorts = 0;
    for (port = 0; (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        if (! CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, port)) continue;
        numOfPorts ++;
    }

    if (numOfPorts == 0)
    {
        return prvCpssDxChPortDynamicPizzaArbiterBWOverbookDeviceCheck(devNum);
    }

    /* allocate arrays */
    portNumArrPtr = (GT_PHYSICAL_PORT_NUM*)cpssOsMalloc(
        numOfPorts * sizeof(GT_PHYSICAL_PORT_NUM));
    if (portNumArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    portSpeedInMBitArrPtr = (GT_U32*)cpssOsMalloc(numOfPorts * sizeof(GT_U32));
    if (portSpeedInMBitArrPtr == NULL)
    {
        cpssOsFree(portNumArrPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    arrNdx = 0;
    for (port = 0; (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        if (! CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, port)) continue;
        portNumArrPtr[arrNdx]         = port;
        portSpeedInMBitArrPtr[arrNdx] = portSpeedInMBit;
        arrNdx ++;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortArrayCheck(
        devNum, numOfPorts, portNumArrPtr, portSpeedInMBitArrPtr);
    cpssOsFree(portNumArrPtr);
    cpssOsFree(portSpeedInMBitArrPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


