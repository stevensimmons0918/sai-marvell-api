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
* @file appDemoDxChPortUtils.c
*
* @brief App demo DxCh PORT API.
*
* @version   3
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/generic/labservices/port/gop/silicon/alleycat3/mvHwsAlleycat3PortIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

/* every symbol error is 10 bits */
#define CPSS_SYMBOL_ERROR_CONVERT_TO_BITS_CNST 10

extern GT_STATUS prvCpssDxChPortImplementWaSGMII2500
(
    IN  GT_U8                           devNum,
    GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal util_appDemoDxChDeviceCoreClockGet function
* @endinternal
*
* @brief   Get core clock in Herz of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] coreClockPtr             - (pointer to)core clock in Herz
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
static GT_STATUS util_appDemoDxChDeviceCoreClockGet
(
    IN GT_U8          dev,
    OUT GT_U32       *coreClockPtr
)
{
    /* device must be active */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(coreClockPtr);

    if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        /* Falcon use separated clock for MG Time Stamp */
        *coreClockPtr = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(dev).sip6MgCoreClock;
        return GT_OK;
    }

    switch (PRV_CPSS_PP_MAC(dev)->coreClock)
    {
        case 222: *coreClockPtr = 222222222; break;
        case 167: *coreClockPtr = 166666667; break;
        case 362: *coreClockPtr = 362500000; break;
        default: *coreClockPtr = PRV_CPSS_PP_MAC(dev)->coreClock * 1000000;
            break;
    }
    return GT_OK;
}


/**
* @internal util_appDemoDxChTimeStampsRateGet function
* @endinternal
*
* @brief   Get rate of events by VNT Time Stamps.
*
* @param[in] dev                      - device number
* @param[in] startTimeStamp           - start counting time stamp
* @param[in] endTimeStamp             - end counting time stamp
* @param[in] eventCount               - events count between end and start time stamps
*                                       events rate in events per second
*/
static GT_U32 util_appDemoDxChTimeStampsRateGet
(
    IN  GT_U8          dev,
    IN  GT_U32         startTimeStamp,
    IN  GT_U32         endTimeStamp,
    IN  GT_U32         eventCount
)
{
    GT_STATUS rc;           /* return code*/
    GT_U32    coreClock;    /* device core clock in HZ */
    double    diff;         /* difference between end and start time stamps */
    double    rate;         /* rate of events */
    GT_U32    rateOut;      /* integer value of rate */

    /* get core closk in HZ */
    rc = util_appDemoDxChDeviceCoreClockGet(dev,&coreClock);
    if (rc != GT_OK)
    {
        return 0xFFFFFFFF;
    }

    /* calculate difference between end and start */
    if (startTimeStamp > endTimeStamp)
    {
       /* take care of wraparound of end time */
       diff = 0xFFFFFFFFU;
       diff = diff + 1 + endTimeStamp - startTimeStamp;
    }
    else
       diff = endTimeStamp - startTimeStamp;

   /* time stamp is in device core clocks.
     event rate is number of events divided by time between them.
     the time is core clock period multiplied on number of core clocks.
     rate = eventCount / time, where time = diff * (1/coreClock) */
   rate = eventCount;
   rate = (rate * coreClock) / diff;

   /* convert float point value to unsigned integer */
   rateOut = (GT_U32)rate;

   return rateOut;
}


/**
* @internal util_appDemoDxChPortFWSRatesTableGet function
* @endinternal
*
* @brief   Calculates and stores RX/TX rate for all ports
*
* @param[out] rateDbPtr                - (pointer to) DB of rates
*
* @retval GT_OK                    - on success
*/
GT_STATUS util_appDemoDxChPortFWSRatesTableGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 arrSize,
    OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
)
{
    GT_U64      cntrValue;  /* value of counters */
    GT_PHYSICAL_PORT_NUM       portNum;              /* port number */
    GT_STATUS   rc;                   /* return code */
    GT_U32      startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for RX */
    GT_U32      startTimeStampTxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for TX */
    GT_U32      endTimeStamp;   /* VNT time stamp start */
    GT_U32      rxRate;         /* VNT time stamp based RX rate */
    GT_U32      txRate;         /* VNT time stamp based TX rate */
    GT_U32      macPortNum;     /* MAC port number */
    CPSS_PORT_MAC_COUNTERS_ENT        cntrNameArr1[3] = {CPSS_BRDC_PKTS_RCV_E,  CPSS_MC_PKTS_RCV_E,  CPSS_GOOD_UC_PKTS_RCV_E}; /* names of counters */
    CPSS_PORT_MAC_COUNTERS_ENT        cntrNameArr2[3] = {CPSS_BRDC_PKTS_SENT_E, CPSS_MC_PKTS_SENT_E, CPSS_GOOD_UC_PKTS_SENT_E}; /* names of counters */
    GT_U32      numOfCntrs = 3; /* number of counters */
    GT_U32      cntrNameIdx;          /* iterator */
    GT_U64      cntrValueArr1;  /* value of rx counters */
    GT_U64      cntrValueArr2;  /* value of tx counters */


    cpssOsMemSet(rateDbPtr, 0, arrSize * sizeof(CMD_PORT_FWS_RATE_STC));
    cpssOsMemSet(startTimeStampRxArr, 0, sizeof(startTimeStampRxArr));
    cpssOsMemSet(startTimeStampTxArr, 0, sizeof(startTimeStampTxArr));

    portNum = 0;

    while (1)
    {
        if (portNum >= arrSize)
        {
            break;
        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macPortNum);
            if(rc != GT_OK)
            {
                portNum++;
                continue;
            }
        }
        else
        {
            if(PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) != GT_TRUE)
            {
                portNum++;
                continue;
            }
        }

        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs; cntrNameIdx++)
        {
            rc = cpssDxChMacCounterGet(devNum, portNum, cntrNameArr1[cntrNameIdx], &cntrValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs; cntrNameIdx++)
        {
            rc = cpssDxChMacCounterGet(devNum, portNum, cntrNameArr2[cntrNameIdx], &cntrValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &startTimeStampTxArr[portNum]);
        if (rc != GT_OK)
        {
            return rc;
        }

        portNum++;
    }

    /* sleep */
    cpssOsTimerWkAfter(1000);

    portNum = 0;

    /* For device go over all available physical ports. */
    while (1)
    {
        if (portNum >= arrSize)
        {
            break;
        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macPortNum);
            if(rc != GT_OK)
            {
                portNum++;
                continue;
            }
        }
        else
        {
            if(PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) != GT_TRUE)
            {
                portNum++;
                continue;
            }
        }

        cntrValueArr1.l[0] = 0;
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs; cntrNameIdx++)
        {
            rc = cpssDxChMacCounterGet(devNum, portNum, cntrNameArr1[cntrNameIdx], &cntrValue);
            if(rc != GT_OK)
            {
                return rc;
            }
            cntrValueArr1.l[0] += cntrValue.l[0];
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        rxRate = util_appDemoDxChTimeStampsRateGet(devNum, startTimeStampRxArr[portNum], endTimeStamp, cntrValueArr1.l[0]);

        cntrValueArr2.l[0] = 0;
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs; cntrNameIdx++)
        {
            rc = cpssDxChMacCounterGet(devNum, portNum, cntrNameArr2[cntrNameIdx], &cntrValue);
            if(rc != GT_OK)
            {
                return rc;
            }
            cntrValueArr2.l[0] += cntrValue.l[0];
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        txRate = util_appDemoDxChTimeStampsRateGet(devNum, startTimeStampTxArr[portNum], endTimeStamp, cntrValueArr2.l[0]);

        rateDbPtr[portNum].rxPacketRate = rxRate;
        rateDbPtr[portNum].txPacketRate = txRate;

        portNum++;
    }

    return GT_OK;
}


/**
* @internal appDemoDxChPortPacketRateGet function
* @endinternal
*
* @brief   This function calculates RX/TX packet rate on specific port.
*
* @param[in] rxPacketRate             - (pointer to) RX packet rate
* @param[in] txPacketRate             - (pointer to) TX packet rate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS appDemoDxChPortPacketRateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *rxPacketRate,
    OUT GT_U32                  *txPacketRate
)
{
    GT_STATUS rc;
    static CMD_PORT_FWS_RATE_STC rateDb[CPSS_MAX_PORTS_NUM_CNS];

    CPSS_NULL_PTR_CHECK_MAC(rxPacketRate);
    CPSS_NULL_PTR_CHECK_MAC(txPacketRate);

    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    rc = util_appDemoDxChPortFWSRatesTableGet(devNum, CPSS_MAX_PORTS_NUM_CNS, rateDb);
    if(GT_OK != rc)
    {
        return rc;
    }

    *rxPacketRate = rateDb[portNum].rxPacketRate;
    *txPacketRate = rateDb[portNum].txPacketRate;

    return GT_OK;
}

/**
* @internal appDemoDxChPortMicroInitConfigSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; xCat3; AC5; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*                                      portsBmp  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To be used with MicroInit tool.
*
*/
GT_STATUS appDemoDxChPortMicroInitConfigSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc = GT_OK;

    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_PORT_STANDARD    portMode;
    PRV_CPSS_DXCH_PORT_STATE_STC  originalPortStateStc;
    GT_U32                  firstInQuadruplet;  /* mac number of first port in quadruplet */
    MV_HWS_REF_CLOCK_SUP_VAL refClock = MHz_25;
    GT_U32                  curLanesList[96];


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &originalPortStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }
        originalPortStateStc.portEnableState = GT_TRUE;
        if (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            /* Set FCA interface width */
            rc = prvCpssDxChPortBcat2FcaBusWidthSet(devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortBcat2FcaBusWidthSet, portNum = %d\n", portNum);
            }
            /* Set PTP interface width */
            rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortBcat2PtpInterfaceWidthSelect, portNum = %d\n", portNum);
            }

            /* set the default Tx and CTLE(Rx) parameters on port - done in
                mvHwsAvagoSerdesArrayPowerCtrlImpl/mvHwsAvagoSerdesPowerCtrlImpl */

            /* set P Sync Bypass */
            CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, 0, portMacMap, portMode));

            /* Configures the DP/Core Clock Selector on port */
            CHECK_STATUS(mvHwsClockSelectorConfig(devNum, 0, portMacMap, portMode));

            /* Configure MAC/PCS */
            CHECK_STATUS(mvHwsPortModeCfg(devNum, 0, portMacMap, portMode, NULL));

            /* Un-Reset the port */
            CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, portMacMap, portMode));

        }
        else
        {
            if(CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode)
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                firstInQuadruplet = portNum & 0xFFFFFFFC;
            }
            else
            {
                firstInQuadruplet = portNum;
            }
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, 0, firstInQuadruplet, portMode, curLanesList));

            /* power up the serdes */
            CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, 0, firstInQuadruplet, portMode, refClock, PRIMARY_LINE_SRC, &(curLanesList[0])));

            /* set the default Tx parameters on port */
            CHECK_STATUS(mvHwsPortTxDefaultsSet(devNum, 0, firstInQuadruplet, portMode));

            /* set P Sync Bypass */
            CHECK_STATUS(mvHwsAlleycat3PortPsyncBypassCfg(devNum, 0, firstInQuadruplet, portMode));

            /* Configures the DP/Core Clock Selector on port */
            CHECK_STATUS(mvHwsClockSelectorConfig(devNum, 0, firstInQuadruplet, portMode));

            /* Configure MAC/PCS */
            CHECK_STATUS(mvHwsPortModeCfg(devNum, 0, firstInQuadruplet, portMode, NULL));

            /* Un-Reset the port */
            CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, firstInQuadruplet, portMode));
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        if (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
            rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum,ifMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChHwRegAddrPortMacUpdate, portNum = %d\n", portNum);
            }
        }
        else
        {
            /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
            rc = prvCpssDxChHwXcat3RegAddrPortMacUpdate(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChHwXcat3RegAddrPortMacUpdate, portNum = %d\n", portNum);
            }
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        rc = cpssDxChPortMruSet(devNum, portNum, 1522);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in cpssDxChPortMruSet, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortStateRestore(devNum, portNum, &originalPortStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
        }
    }

    return rc;
}

/**
* @internal appDemoDxChPortSymbolErrorRateCommonParamsGet
*           function
* @endinternal
*
* @brief    utility function to calculate common data for SER
*           funcions
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; Caelum;
*                                   AC5P;Aldrin; AC3X; xCat3;
*                                   AC5; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] sleepTime             - time to measure rs_fec
*                                    errors (milliseconds)
* @param[OUT] speedInBits           - (pointer to) convert speed
*                                    fron Gb to bits
* @param[OUT] elaspedTime           - (pointer to) measure time
*                                    of sleep time
* @param[OUT] rsfecStatus           - (pointer to) read rs_fec
*                                       status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS appDemoDxChPortSymbolErrorRateCommonParamsGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      sleepTime, /* in milliseconds */
    OUT long double                 *speedInBits,
    OUT long double                 *elapsedTime,
    OUT CPSS_DXCH_RSFEC_STATUS_STC  *rsfecStatus

)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portMacNum, secondsStart = 0, nanoSecondsStart = 0, secondsEnd = 0, nanoSecondsEnd = 0, seconds = 0, nanoSec = 0;
    CPSS_PORT_SPEED_ENT         speed;
    CPSS_DXCH_PORT_FEC_MODE_ENT fecMode;

    /* arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(speedInBits);
    CPSS_NULL_PTR_CHECK_MAC(elapsedTime);
    CPSS_NULL_PTR_CHECK_MAC(rsfecStatus);

    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }
    if ( CPSS_PP_FAMILY_DXCH_FALCON_E != PRV_CPSS_PP_MAC(devNum)->devFamily )
    {
        return GT_NOT_SUPPORTED;
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    osMemSet(rsfecStatus, 0, sizeof(CPSS_DXCH_RSFEC_STATUS_STC));

    rc = cpssDxChPortFecModeGet(devNum, portNum, &fecMode);
    if ((fecMode != CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) && (fecMode != CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("FEC type not supported"));
    }

    /* convert speed from Giga bits to bits */
    speed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    switch ( speed )
    {
    case CPSS_PORT_SPEED_25000_E:
        *speedInBits = 25 * 1000000000UL;
        break;
    case CPSS_PORT_SPEED_50000_E:
        *speedInBits = 50 * 1000000000UL;
        break;
    case CPSS_PORT_SPEED_100G_E:
        *speedInBits = 100 * 1000000000UL;
        break;
    case CPSS_PORT_SPEED_200G_E:
        *speedInBits = 200 * 1000000000UL;
        break;
    case CPSS_PORT_SPEED_400G_E:
        *speedInBits = 400 * 1000000000UL;
        break;
    case CPSS_PORT_SPEED_NA_E:
        return GT_NOT_INITIALIZED;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        break;
    }

    /* clear counters on read */
    rc = cpssDxChRsFecStatusGet(devNum, portNum, rsfecStatus);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* check align lock */
    if ( rsfecStatus->fecAlignStatus != 1 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "no align lock");
    }

    /* read RT clock in nanosec resolution and sleep */
    rc = cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
    if(rc != GT_OK)
    {
        return rc;
    }
    cpssOsTimerWkAfter(sleepTime);

    /* calculate duration time (nanosec resolution) */
    rc = cpssOsTimeRT(&secondsEnd, &nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }
    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000UL - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    *elapsedTime = seconds + nanoSec/1000000000UL; /* seconds resolution */

    /* read counters */
    rc = cpssDxChRsFecStatusGet(devNum, portNum, rsfecStatus);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal appDemoDxChPortSymbolErrorRateGet function
* @endinternal
*
* @brief    prints the Symbol Error Rate given by :
*           SER = (symbol_error_in_bits * rs_fec_errors_counted)
*                 / (speed_in_bits * sleep_time)
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; Caelum;
*                                   AC5P;Aldrin; AC3X; xCat3;
*                                   AC5; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] sleepTime             - time to measure rs_fec
*                                    errors (milliseconds)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS appDemoDxChPortSymbolErrorRateGet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               sleepTime /* in milliseconds */
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      i;
    long double                 fecSymbolErrRate = 0, speedInBits = 0, symbolErr = 0, elapsedTime = 0;
    CPSS_DXCH_RSFEC_STATUS_STC  rsfecStatus;

    /* check arguments */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }
    if ( CPSS_PP_FAMILY_DXCH_FALCON_E != PRV_CPSS_PP_MAC(devNum)->devFamily )
    {
        return GT_NOT_SUPPORTED;
    }
    osMemSet(&rsfecStatus, 0, sizeof(rsfecStatus));

    rc = appDemoDxChPortSymbolErrorRateCommonParamsGet(devNum, portNum, sleepTime, &speedInBits, &elapsedTime, &rsfecStatus);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    for ( i = 0; i < CPSS_DXCH_PORT_MAC_RSFEC_STATUS_SYMBOL_ERROR_CNS; i+=2 )
    {
        symbolErr += rsfecStatus.symbolError[i] + (rsfecStatus.symbolError[i+1] << 16);
    }

    fecSymbolErrRate = (CPSS_SYMBOL_ERROR_CONVERT_TO_BITS_CNST * symbolErr) / (speedInBits * elapsedTime);
    printf("RS_FEC Symbol Error Rate = %2.3Le\n", fecSymbolErrRate);

    return rc;
}

/**
* @internal appDemoDxChPortSymbolErrorRateGet function
* @endinternal
*
* @brief    prints the Symbol Error Rate (per lane) given by :
*           SER = (symbol_error_in_bits * rs_fec_errors_counted)
*                 / (speed_in_bits * sleep_time)
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; Caelum;
*                                   AC5P;Aldrin; AC3X; xCat3;
*                                   AC5; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[in] portGroup             - port group
* @param[in] portNum               - physical port number
* @param[in] sleepTime             - time to measure rs_fec
*                                    errors (milliseconds)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS appDemoDxChPortSymbolErrorRatePerLaneGet
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroup,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               sleepTime /* in milliseconds */
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          i, portMacNum;
    long double                     fecSymbolErrRate = 0, speedInBits = 0, symbolErrLane = 0, elapsedTime = 0;
    CPSS_DXCH_RSFEC_STATUS_STC      rsfecStatus;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;

    /* check arguments */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }
    if ( CPSS_PP_FAMILY_DXCH_FALCON_E != PRV_CPSS_PP_MAC(devNum)->devFamily )
    {
        return GT_NOT_SUPPORTED;
    }

    osMemSet(&rsfecStatus, 0, sizeof(rsfecStatus));
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);

    /* extract num of active lanes */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "hwsPortModeParamsGetToBuffer returned null");
    }

    rc = appDemoDxChPortSymbolErrorRateCommonParamsGet(devNum, portNum, sleepTime, &speedInBits, &elapsedTime, &rsfecStatus);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    printf("RS_FEC Symbol Error Rate per lane :\n");
    for ( i = 0; i < (2 * curPortParams.numOfActLanes) && (i < CPSS_DXCH_PORT_MAC_RSFEC_STATUS_SYMBOL_ERROR_CNS) ; i += 2 )
    {
        symbolErrLane = 0;  /* reset for each lane */
        symbolErrLane += rsfecStatus.symbolError[i] + (rsfecStatus.symbolError[i+1] << 16);
        fecSymbolErrRate = (CPSS_SYMBOL_ERROR_CONVERT_TO_BITS_CNST * symbolErrLane) / (speedInBits * elapsedTime);
        printf("lane %d : Rate = %2.3Le\n", i/2, fecSymbolErrRate);
    }

    return rc;
}
