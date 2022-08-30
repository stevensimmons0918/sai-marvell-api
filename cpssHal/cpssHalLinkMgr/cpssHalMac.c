/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalMac.c
//


#include "cpssHalMac.h"
#include "cpssHalPort.h"
#include "xpsPort.h"
#include "xpsInit.h"
#include "xpsLock.h"
#include "cpssHalUtil.h"
#include "gtEnvDep.h"
#include "cpssDxChPortStat.h"
#include "cpssHalDevice.h"
#include "cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h"
#include "xpsMac.h"
#include "xpsLink.h"
#include "openXpsEnums.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include "xpsAcl.h"


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define CPSS_MAC_COUNTER_TO_UINT64_CONVERT(_cpssCounter) \
    (((uint64_t)(_cpssCounter.l[1]) << 32) + _cpssCounter.l[0])        //Used for converting members of cpss stat attributes structure(CPSS_PORT_MAC_COUNTER_SET_STC) from type GT_U64 to uint64_t
//(Taken from cpssHalDumpHalPort.c file)
#else
#define CPSS_MAC_COUNTER_TO_UINT64_CONVERT(_cpssCounter) \
    (((uint64_t)(_cpssCounter.l[0]) << 32) + _cpssCounter.l[1])        //Used for converting members of cpss stat attributes structure(CPSS_PORT_MAC_COUNTER_SET_STC) from type GT_U64 to uint64_t

#endif
// A wrapper for
XP_STATUS cpssHalMacGetCounterStats(xpsDevice_t devId, uint32_t portNum,
                                    xp_Statistics *stat)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    memset(stat, 0, sizeof(xp_Statistics));

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (GT_FALSE == PRV_CPSS_SIP_6_CHECK_MAC(cpssDevId))
    {
        CPSS_PORT_MAC_COUNTER_SET_STC stats;
        CPSS_PORT_MAC_COUNTER_SET_STC *cpssStatistics = &stats;
        memset(cpssStatistics, 0, sizeof(CPSS_PORT_MAC_COUNTER_SET_STC));

        if ((cpssRet = cpssDxChPortMacCountersOnPortGet(cpssDevId,
                                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, cpssStatistics)) != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            return ret;
        }

        stat->octetsRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                             cpssStatistics->goodOctetsRcv) + CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                             cpssStatistics->badOctetsRcv);
        stat->frameRxUnicastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssStatistics->ucPktsRcv);
        stat->frameRxOk =  CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                               cpssStatistics->goodPktsRcv);
        stat->frameRxAnyErr =  CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                   cpssStatistics->macRcvError);
        stat->frameRxBroadcastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssStatistics->brdcPktsRcv);
        stat->frameRxMulticastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssStatistics->mcPktsRcv);
        /* goodPktsRcv value is not supported for DxCh.
         * Hence frameRxUnicastAddr, frameRxBroadcastAddr, frameRxBroadcastAddr are used to calculate frameRxOk.*/
        if (stat->octetsRx !=0 && stat->frameRxOk == 0)
        {
            stat->frameRxOk = stat->frameRxUnicastAddr + stat->frameRxBroadcastAddr +
                              stat->frameRxMulticastAddr;
        }

        stat->octetsTransmittedTotal = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                           cpssStatistics->goodOctetsSent);
        stat->framesTransmittedUnicast = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssStatistics->ucPktsSent);
        stat->frameTransmittedOk = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssStatistics->brdcPktsSent)+ CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssStatistics->mcPktsSent)+ CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssStatistics->ucPktsSent);
        stat->frameTransmittedWithErr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                            cpssStatistics->macTransmitErr); //doubt
        stat->framesTransmittedBroadcast0 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                cpssStatistics->brdcPktsSent);
        stat->framesTransmittedMulticast = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                               cpssStatistics->mcPktsSent);
        stat->fragmentsRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                cpssStatistics->fragmentsPkts);
        stat->frameRxOversize = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssStatistics->oversizePkts);
        stat->frameRxFcsErr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                  cpssStatistics->badCrc);
        stat->frameRxUndersize = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                     cpssStatistics->undersizePkts);
        stat->jabberFrameRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                  cpssStatistics->jabberPkts);
        stat->frameRxLength64 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssStatistics->pkts64Octets);  //pkts64Octets contains both Tx and Rx packets
        stat->frameRxLength65To127 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssStatistics->pkts65to127Octets); //pkts65to127Octets contains both Tx and Rx packets
        stat->frameRxLength128To255 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                          cpssStatistics->pkts128to255Octets); //pkts128to255Octets contains both Tx and Rx packets
        stat->frameRxLength256To511 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                          cpssStatistics->pkts256to511Octets); //pkts256to511Octets contains both Tx and Rx packets
        stat->frameRxLength512To1023 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                           cpssStatistics->pkts512to1023Octets); //pkts512to1023Octets contains both Tx and Rx packets
        stat->frameRxLength1024To1518 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                            cpssStatistics->pkts1024to1518Octets); //pkts1024to1518Octets contains both Tx and Rx packets
        /*
           stat->frameRxLength1519To2047
           stat->frameRxLength2048To4095                    //There's only one struct member pkts1519ToMaxOctets adding all of these
           stat->frameRxLength4096To8191
           stat->frameRxLength8192To9215
        */
        stat->framesTransmittedLength64 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssStatistics->pkts64Octets);  //pkts64Octets contains both Tx and Rx packets
        stat->framesTransmittedLength65to127 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                   cpssStatistics->pkts65to127Octets); //pkts65to127Octets contains both Tx and Rx packets
        stat->framesTransmittedLength128to255 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                    cpssStatistics->pkts128to255Octets); //pkts128to255Octets contains both Tx and Rx packets
        stat->framesTransmittedLength256to511 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                    cpssStatistics->pkts256to511Octets); //pkts256to511Octets contains both Tx and Rx packets
        stat->framesTransmittedLength512to1023 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                     cpssStatistics->pkts512to1023Octets); //pkts512to1023Octets contains both Tx and Rx packets
        stat->framesTransmittedLength1024to1518 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                      cpssStatistics->pkts1024to1518Octets); //pkts1024to1518Octets contains both Tx and Rx packets
        /*
           stat->framesTransmittedLength1519to2047
           stat->frameRxLength2048To4095                   //There's only one struct member pkts1519ToMaxOctets adding all of these
           stat->frameRxLength4096To8191
           stat->frameRxLength8192To9215
        */
    }
    else  /* SIP-6 */
    {
        CPSS_PORT_MAC_MTI_COUNTER_SET_STC cpssMtiStatistics;

        memset(&cpssMtiStatistics, 0, sizeof(CPSS_PORT_MAC_MTI_COUNTER_SET_STC));

        if ((cpssRet = cpssDxChPortMacCountersOnMtiPortGet(cpssDevId,
                                                           (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssMtiStatistics)) != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            return ret;
        }

        stat->octetsRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                             cpssMtiStatistics.rxCounterStc.etherStatsOctets);
        stat->frameRxOk =  CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                               cpssMtiStatistics.rxCounterStc.aFramesReceivedOK);
        stat->frameRxAnyErr =  CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                   cpssMtiStatistics.rxCounterStc.ifInErrors);
        stat->frameRxUnicastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssMtiStatistics.rxCounterStc.ifInUcastPkts);
        stat->frameRxBroadcastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssMtiStatistics.rxCounterStc.ifInBroadcastPkts);
        stat->frameRxMulticastAddr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssMtiStatistics.rxCounterStc.ifInMulticastPkts);
        // HW does not support
        stat->frameRxUnknownProtocol = 0;
        stat->frameRxFcsErr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                  cpssMtiStatistics.rxCounterStc.aFrameCheckSequenceErrors);
        stat->frameRxUndersize = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                     cpssMtiStatistics.rxCounterStc.etherStatsUndersizePkts);
        stat->frameRxOversize = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.etherStatsOversizePkts);
        stat->fragmentsRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                cpssMtiStatistics.rxCounterStc.etherStatsFragments);
        stat->jabberFrameRx = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                  cpssMtiStatistics.rxCounterStc.etherStatsJabbers);

        stat->frameRxLength64 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.etherStatsPkts64Octets);
        stat->frameRxLength65To127 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                         cpssMtiStatistics.rxCounterStc.etherStatsPkts65to127Octets);
        stat->frameRxLength128To255 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                          cpssMtiStatistics.rxCounterStc.etherStatsPkts128to255Octets);
        stat->frameRxLength256To511 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                          cpssMtiStatistics.rxCounterStc.etherStatsPkts256to511Octets);
        stat->frameRxLength512To1023 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                           cpssMtiStatistics.rxCounterStc.etherStatsPkts512to1023Octets);
        stat->frameRxLength1024To1518 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                            cpssMtiStatistics.rxCounterStc.etherStatsPkts1024to1518Octets);
        stat->framesRxLength1519to2047 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.rxCounterStc.etherStatsPkts1519toMaxOctets);
        stat->framesRxLength2048to4095 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.rxCounterStc.etherStatsPkts1519toMaxOctets);
        /* FOR SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS, so set 8192to9215 to 0 */
        stat->framesRxLength4096to8191 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.rxCounterStc.etherStatsPkts1519toMaxOctets);
        stat->framesRxLength8192to9215 = 0;
        stat->framesRxLength9216to16383 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.rxCounterStc.etherStatsPkts1519toMaxOctets);
        stat->octetsTransmittedTotal = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                           cpssMtiStatistics.txCounterStc.ifOutOctets);
        stat->frameTransmittedOk = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                       cpssMtiStatistics.txCounterStc.aFramesTransmittedOK);
        stat->frameTransmittedWithErr = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                            cpssMtiStatistics.txCounterStc.ifOutErrors);
        stat->framesTransmittedUnicast = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.ifOutUcastPkts);
        stat->framesTransmittedBroadcast0 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                cpssMtiStatistics.txCounterStc.ifOutBroadcastPkts);
        stat->framesTransmittedMulticast = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                               cpssMtiStatistics.txCounterStc.ifOutMulticastPkts);

        stat->framesTransmittedLength64 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts64Octets);
        stat->framesTransmittedLength65to127 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                   cpssMtiStatistics.txCounterStc.etherStatsPkts65to127Octets);
        stat->framesTransmittedLength128to255 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                    cpssMtiStatistics.txCounterStc.etherStatsPkts128to255Octets);
        stat->framesTransmittedLength256to511 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                    cpssMtiStatistics.txCounterStc.etherStatsPkts256to511Octets);
        stat->framesTransmittedLength512to1023 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                     cpssMtiStatistics.txCounterStc.etherStatsPkts512to1023Octets);
        stat->framesTransmittedLength1024to1518 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                      cpssMtiStatistics.txCounterStc.etherStatsPkts1024to1518Octets);
        stat->framesTransmittedLength1519to2047 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                      cpssMtiStatistics.txCounterStc.etherStatsPkts1519toMaxOctets);
        stat->framesTransmittedLength2048to4095 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                      cpssMtiStatistics.txCounterStc.etherStatsPkts1519toMaxOctets);
        /* FOR SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS, so set 8192to9215 to 0 */
        stat->framesTransmittedLength4096to8191 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                      cpssMtiStatistics.txCounterStc.etherStatsPkts1519toMaxOctets);
        stat->framesTransmittedLength8192to9215 = 0;
        stat->framesTransmittedLength9216to16383 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                                       cpssMtiStatistics.txCounterStc.etherStatsPkts1519toMaxOctets);
        stat->framesTransmittedOversize = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts64Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts65to127Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts128to255Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts256to511Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts512to1023Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts1024to1518Octets) -
                                          CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                              cpssMtiStatistics.txCounterStc.etherStatsPkts1519toMaxOctets);

        stat->framesRxPriNum0 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_0);
        stat->framesRxPriNum1 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_1);
        stat->framesRxPriNum2 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_2);
        stat->framesRxPriNum3 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_3);
        stat->framesRxPriNum4 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_4);
        stat->framesRxPriNum5 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_5);
        stat->framesRxPriNum6 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_6);
        stat->framesRxPriNum7 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                    cpssMtiStatistics.rxCounterStc.aCBFCPAUSEFramesReceived_7);

        stat->framesTransmittedPriNum0 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_0);
        stat->framesTransmittedPriNum1 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_1);
        stat->framesTransmittedPriNum2 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_2);
        stat->framesTransmittedPriNum3 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_3);
        stat->framesTransmittedPriNum4 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_4);
        stat->framesTransmittedPriNum5 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_5);
        stat->framesTransmittedPriNum6 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_6);
        stat->framesTransmittedPriNum7 = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                             cpssMtiStatistics.txCounterStc.aCBFCPAUSEFramesTransmitted_7);

        stat->frameRxTypePause = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                     cpssMtiStatistics.rxCounterStc.aPAUSEMACCtrlFramesReceived);
        stat->framesTransmittedPause  = CPSS_MAC_COUNTER_TO_UINT64_CONVERT(
                                            cpssMtiStatistics.txCounterStc.aMACControlFramesTransmitted);
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalMacSetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable)
{

    XP_STATUS ret =  XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    GT_BOOL cpssEnable = GT_TRUE;
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    if (enable==0)
    {
        cpssEnable = GT_FALSE;
    }
    if ((cpssRet = cpssDxChPortMacCountersClearOnReadSet(cpssDevNum,
                                                         (GT_PHYSICAL_PORT_NUM)cpssPortNum, cpssEnable)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set clear on read for mac counters failed");
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPrintPortStat(int devId, uint32_t portNum,
                                  xp_Statistics *stat)
{
    if (!stat)
    {
        return XP_ERR_NULL_POINTER;
    }

    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxOk        0x%-16llx   TxOK        0x%-16llx\n", stat->frameRxOk,
             stat->frameTransmittedOk);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxAll       0x%-16llx   TxAll       0x%-16llx\n", stat->frameRxAll,
             stat->frameTransmittedAll);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxOctGood   0x%-16llx   TxOctGood   0x%-16llx\n", stat->octetsRxGoodFrame,
             stat->octetsTransmittedWithoutErr);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxOct       0x%-16llx   TxOct       0x%-16llx\n", stat->octetsRx,
             stat->octetsTransmittedTotal);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxUC        0x%-16llx   TxUC        0x%-16llx\n", stat->frameRxUnicastAddr,
             stat->framesTransmittedUnicast);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxMC        0x%-16llx   TxMC        0x%-16llx\n", stat->frameRxMulticastAddr,
             stat->framesTransmittedMulticast);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxBC        0x%-16llx   TxBC        0x%-16llx\n", stat->frameRxBroadcastAddr,
             stat->framesTransmittedBroadcast0);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPause     0x%-16llx   TxPause     0x%-16llx\n", stat->frameRxTypePause,
             stat->framesTransmittedPause);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriPause  0x%-16llx   TxPriPause  0x%-16llx\n", stat->framesRxPriorityPause,
             stat->framesTransmittedPriPause);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxLen<64    0x%-16llx   TxLen<64    0x%-16llx\n",
             stat->framesRxLengthLessThan64, stat->framesTransmittedLengthLessThan64);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxLen64     0x%-16llx   TxLen64     0x%-16llx\n", stat->frameRxLength64,
             stat->framesTransmittedLength64);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx65-127    0x%-16llx   Tx65-127    0x%-16llx\n", stat->frameRxLength65To127,
             stat->framesTransmittedLength65to127);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx128-255   0x%-16llx   Tx128-255   0x%-16llx\n", stat->frameRxLength128To255,
             stat->framesTransmittedLength128to255);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx256-511   0x%-16llx   Tx256-511   0x%-16llx\n", stat->frameRxLength256To511,
             stat->framesTransmittedLength256to511);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx512-1023  0x%-16llx   Tx512-1023  0x%-16llx\n", stat->frameRxLength512To1023,
             stat->framesTransmittedLength512to1023);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx1024-1518 0x%-16llx   Tx1024-1518 0x%-16llx\n",
             stat->frameRxLength1024To1518, stat->framesTransmittedLength1024to1518);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx1519-2047 0x%-16llx   Tx1519-2047 0x%-16llx\n",
             stat->framesRxLength1519to2047, stat->framesTransmittedLength1519to2047);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx2048-4095 0x%-16llx   Tx2048-4095 0x%-16llx\n",
             stat->framesRxLength2048to4095, stat->framesTransmittedLength2048to4095);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx4096-8191 0x%-16llx   Tx4096-8191 0x%-16llx\n",
             stat->framesRxLength4096to8191, stat->framesTransmittedLength4096to8191);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx8192-9215 0x%-16llx   Tx8192-9215 0x%-16llx\n",
             stat->framesRxLength8192to9215, stat->framesTransmittedLength8192to9215);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "Rx9216-Up   0x%-16llx   Tx9216-Up   0x%-16llx\n",
             stat->framesRxLength9216AndUp, stat->framesTransmittedLength9216AndUp);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum0   0x%-16llx   TxPriNum0   0x%-16llx\n", stat->framesRxPriNum0,
             stat->framesTransmittedPriNum0);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum1   0x%-16llx   TxPriNum1   0x%-16llx\n", stat->framesRxPriNum1,
             stat->framesTransmittedPriNum1);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum2   0x%-16llx   TxPriNum2   0x%-16llx\n", stat->framesRxPriNum2,
             stat->framesTransmittedPriNum2);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum3   0x%-16llx   TxPriNum3   0x%-16llx\n", stat->framesRxPriNum3,
             stat->framesTransmittedPriNum3);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum4   0x%-16llx   TxPriNum4   0x%-16llx\n", stat->framesRxPriNum4,
             stat->framesTransmittedPriNum4);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum5   0x%-16llx   TxPriNum5   0x%-16llx\n", stat->framesRxPriNum5,
             stat->framesTransmittedPriNum5);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum6   0x%-16llx   TxPriNum6   0x%-16llx\n", stat->framesRxPriNum6,
             stat->framesTransmittedPriNum6);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPriNum7   0x%-16llx   TxPriNum7   0x%-16llx\n", stat->framesRxPriNum7,
             stat->framesTransmittedPriNum7);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr0Pau1us 0x%-16llx   TxPr0Pau1us 0x%-16llx\n",
             stat->framesRxPriNum0Pause1us, stat->framesTransmitPriNum0Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr1Pau1us 0x%-16llx   TxPr1Pau1us 0x%-16llx\n",
             stat->framesRxPriNum1Pause1us, stat->framesTransmitPriNum1Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr2Pau1us 0x%-16llx   TxPr2Pau1us 0x%-16llx\n",
             stat->framesRxPriNum2Pause1us, stat->framesTransmitPriNum2Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr3Pau1us 0x%-16llx   TxPr3Pau1us 0x%-16llx\n",
             stat->framesRxPriNum3Pause1us, stat->framesTransmitPriNum3Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr4Pau1us 0x%-16llx   TxPr4Pau1us 0x%-16llx\n",
             stat->framesRxPriNum4Pause1us, stat->framesTransmitPriNum4Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr5Pau1us 0x%-16llx   TxPr5Pau1us 0x%-16llx\n",
             stat->framesRxPriNum5Pause1us, stat->framesTransmitPriNum5Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr6Pau1us 0x%-16llx   TxPr6Pau1us 0x%-16llx\n",
             stat->framesRxPriNum6Pause1us, stat->framesTransmitPriNum6Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxPr7Pau1us 0x%-16llx   TxPr7Pau1us 0x%-16llx\n",
             stat->framesRxPriNum7Pause1us, stat->framesTransmitPriNum7Pause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxFrmAnyEr  0x%-16llx   TxErr       0x%-16llx\n", stat->frameRxAnyErr,
             stat->frameTransmittedWithErr);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxFCSEr     0x%-16llx   TxVlan      0x%-16llx\n", stat->frameRxFcsErr,
             stat->framesTransmittedVLAN);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "rxtoolong   0x%-16llx   FsigCrcErr  0x%-16llx\n", stat->framesRxTooLong,
             stat->fsigCrcErr);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxCrcErrInv 0x%-16llx   FrmTruncat  0x%-16llx\n", stat->framesRxCRCError,
             stat->framesTruncated);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxFifoFull  0x%-16llx   RxStdPau1us 0x%-16llx\n",
             stat->frameDroppedFromRxFIFOFullCondition, stat->framesRxStandardPause1us);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxLenEr     0x%-16llx   RxUndSize   0x%-16llx\n", stat->frameRxLengthErr,
             stat->frameRxUndersize);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxOverSize  0x%-16llx   RxFragment  0x%-16llx\n", stat->frameRxOversize,
             stat->fragmentsRx);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "RxJabFram   0x%-16llx   RxInvPream  0x%-16llx\n", stat->jabberFrameRx,
             stat->framesRxInvalidPreamble);

    return XP_NO_ERR;

}

void cpssHalMacPrintPortInfo(xpsDevice_t devId, int portNum)
{
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    uint8_t isLoopback=0;
    xpMacConfigMode macConfigMode = MAC_MODE_MAX_VAL;
    bool linkStatus, faultStatus, serdesStatus;
    xpFecMode fecMode = MAX_FEC_MODE;
    GT_STATUS cpssRet = GT_OK;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    char macLoopbackStatus[4] = {'-', '-', '-', '\0'};
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    XP_STATUS ret;

    const char *xpMacConfigModeStr[MAC_MODE_MAX_VAL] =
    {
        "MAC_MODE_4X1GB",
        "MAC_MODE_MIX_4_CHANNEL_10G",
        "MAC_MODE_1GB",
        "MAC_MODE_4X10GB",
        "MAC_MODE_2X40GB",
        "MAC_MODE_1X40GB",
        "MAC_MODE_4X100GB",
        "MAC_MODE_1X25GB",
        "MAC_MODE_50GB",
        "MAC_MODE_1X10GB",
        "MAC_MODE_1000BASE_R",
        "MAC_MODE_1000BASE_X",
        "MAC_MODE_MIX_SGMII",
        "MAC_MODE_MIX_1000BASE_R",
        "MAC_MODE_50GB",
        "MAC_MODE_10M",
        "MAC_MODE_100M",
        "MAC_MODE_4X200GB",
        "MAC_MODE_8X400GB",
        "MAC_MODE_2X100GB"
    };

    const char *xpFecModeStr[4] =
    {
        "RS_FEC",
        "FC_FEC",
        "RS_544_514",
        "DISABLE"
    };

    cpssRet = cpssDxChPortPhysicalPortDetailedMapGet(cpssDevNum, cpssPortNum,
                                                     portMapPtr);
    if (cpssRet != GT_OK)
    {
        return;
    }

    cpssHalMacGetPortStatus(devId, portNum, &linkStatus, &faultStatus,
                            &serdesStatus);

    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "   %6d    ", portNum);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, " %3d/%3d  ", cpssDevNum,
             cpssPortNum);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, ":[%6d ]:",
             portMapPtr->portMap.macNum);
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%9s ", linkStatus?"UP":"DOWN");
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%9s ", faultStatus?"FALSE":"OK");
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%9s ", serdesStatus?"OK":"FALSE");

    //get mac Mode.

    ret = xpsMacConfigModeGet(devId, portNum, &macConfigMode);
    if (ret == XP_NO_ERR)
    {
        LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%22s",
                 xpMacConfigModeStr[macConfigMode]);
    }
    else
    {
        LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%22s", "n/a");
    }
    //get FEC mode

    ret = xpsMacFecModeGet(devId, portNum, &fecMode);
    if (ret == XP_NO_ERR)
    {
        LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "  %10s   ",
                 xpFecModeStr[fecMode]);
    }
    else
    {
        LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "%22s", "n/a");
    }

    xpsMacMacLoopbackGet(devId, portNum, MAC_LEVEL_LOOPBACK, &isLoopback);
    if (isLoopback)
    {
        macLoopbackStatus[0] = 'M';
    }
    xpsMacMacLoopbackGet(devId, portNum, MAC_PCS_LEVEL_LOOPBACK, &isLoopback);
    if (isLoopback)
    {
        macLoopbackStatus[1] = 'P';
    }
    xpsMacPortLoopbackEnableGet(devId, portNum, &isLoopback);
    if (isLoopback)
    {
        macLoopbackStatus[2] = 'S';
    }

    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "  %6s   ", macLoopbackStatus);

    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "\n");
}

void cpssHalMacPrintAllPortStatus(xpsDevice_t devId)
{
    int maxPortNum = 0;
    int portId = 0;

    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "=====================================================================================================================================================\n");
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "ChassisPort# CNX-Port# :[PTGNum ]:   Link     Fault    PTG-signalOK       MAC-Mode         FEC-Mode     Mac Loopback   \n",
             "                   ");
    LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT,
             "=====================================================================================================================================================\n");

    cpssHalSetDeviceSwitchId(devId);
    cpssHalGetMaxGlobalPorts(&maxPortNum);

    XPS_GLOBAL_PORT_ITER(portId, maxPortNum)
    {
        cpssHalMacPrintPortInfo(devId, portId);
    }
}

XP_STATUS cpssHalMacStatCounterReset(xpsDevice_t devId, uint32_t portNum)
{

    XP_STATUS ret =  XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    GT_BOOL  enable;
    CPSS_PORT_MAC_COUNTER_SET_STC cpssStatistics;
    CPSS_PORT_MAC_MTI_COUNTER_SET_STC cpssMtiStatistics;
    //get the counter clear on read status
    cpssRet = cpssDxChPortMacCountersClearOnReadGet(cpssDevId, cpssPortNum,
                                                    &enable);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    if (enable == GT_FALSE)
    {
        cpssRet = cpssDxChPortMacCountersClearOnReadSet(cpssDevId,
                                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, GT_TRUE);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortMacCountersClearOnReadSet dev %d port %d failed(%d)", devId,
                  portNum, cpssRet);
            return ret;
        }
    }
    if (GT_FALSE == PRV_CPSS_SIP_6_CHECK_MAC(cpssDevId))
    {
        cpssRet = cpssDxChPortMacCountersOnPortGet(cpssDevId,
                                                   (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssStatistics);
    }
    else
    {
        cpssRet = cpssDxChPortMacCountersOnMtiPortGet(cpssDevId,
                                                      (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssMtiStatistics);

    }
    //restore clear on read status back if changed
    if (enable==GT_FALSE)
    {
        cpssRet = cpssDxChPortMacCountersClearOnReadSet(cpssDevId,
                                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, GT_FALSE);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortMacCountersClearOnReadSet dev %d port %d failed(%d)", devId,
                  portNum, cpssRet);
            return ret;
        }

    }
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalStatCounterReset dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }
    else
    {
        LOGENTRY(XP_SUBMOD_LINK_MGR, XP_LOG_DEFAULT, "MAC Counter reset successful\n");
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacStatCounterCreateAclRuleForV4V6(xpsDevice_t devId,
                                                    uint32_t portNum,
                                                    uint32_t *counterId, xpsAclStage_e direction, xpsAclKeyFlds_t keyType)
{

    XP_STATUS ret =  XP_NO_ERR;
    GT_U8                                       cpssDevId = xpsGlobalIdToDevId(
                                                                devId, portNum);
    GT_U32                                      cpssPortNum =
        xpsGlobalPortToPortnum(devId, portNum);
    xpsVirtualTcamRuleData_t                    ruleData = {};
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes = {};
    xpsVirtualTcamRuleType_t                    tcamRuleType = {};
    xpsPclRuleFormat_t                          pcl_pattern = {};
    xpsPclRuleFormat_t                          pcl_mask    = {};
    CPSS_DXCH_PCL_ACTION_STC                    pcl_action  = {};
    xpsAclTableContextDbEntry_t                 dummy_tableIdDbEntry = {};
    xpsAclkeyField_t                            fldList[2] = {};
    uint32_t                                    tmpValue[ARRAY_SIZE(fldList)] = {};
    uint32_t                                    tmpMask[ARRAY_SIZE(fldList)]  = {};
    xpsAclkeyFieldList_t                        fieldList = {};
    CPSS_PP_FAMILY_TYPE_ENT                     devType = cpssHalDevPPFamilyGet(
                                                              devId);
    xpsScope_t                                  scopeId = 0;
    uint32_t                                    parallelLookupNum =
        XPS_ACL_PARALLEL_LOOKUP_3;
    uint32_t                                    portListBmp  = 0;
    uint32_t                                    portListMask = 0;
    uint32_t                                    portGroup = 0;
    uint32_t                                    offset = 0;
    uint32_t                                    tcamId = 0;

    if (NULL == counterId)
    {
        return XP_ERR_INVALID_DATA;
    }

    if (direction >= XPS_PCL_STAGE_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid pcl direction\n ");
        return XP_ERR_INVALID_VALUE;
    }
    if (keyType != XPS_PCL_IS_IPV4 && keyType != XPS_PCL_IS_IPV6)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid v4v6 pcl key type\n ");
        return XP_ERR_INVALID_VALUE;
    }

    for (int i = 0; i < ARRAY_SIZE(fldList); i++)
    {
        fldList[i].value = (uint8_t *)&tmpValue[i];
        fldList[i].mask  = (uint8_t *)&tmpMask[i];
    }

    /*Bind port for acl*/
    portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
    if (portGroup >= XPS_PORT_LIST_PORT_GROUP_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", cpssPortNum);
        return XP_ERR_INVALID_VALUE;
    }
    portListBmp |= (portGroup << XPS_PORT_LIST_OFFSET) | offset;
    portListMask = ~(portListBmp);
    portListMask&=0x00FFFFFF;
    portListMask|=0x0F000000;
    fldList[0].keyFlds = XP_PCL_PORT_BIT_MAP;
    memcpy(fldList[0].value, &(portListBmp), sizeof(uint32_t));
    memcpy(fldList[0].mask,  &(portListMask), sizeof(uint32_t));

    /*Configure v4/v6 rule*/
    fldList[1].keyFlds = keyType;
    fldList[1].value[0] = 1;
    fldList[1].mask[0] = 1;

    fieldList.fldList = fldList;
    fieldList.isValid = true;
    fieldList.numFlds = 2;

    if ((ret = xpsUdbKeySet(&fieldList, &pcl_pattern, &pcl_mask,
                            keyType == XPS_PCL_IS_IPV4? XPS_PCL_IPV4_L4_KEY:XPS_PCL_IPV6_L4_KEY,
                            direction)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsUdbKeySet failed with err : %d ", ret);
        return ret;
    }

    dummy_tableIdDbEntry.stage = direction;
    dummy_tableIdDbEntry.isMirror = false;

    /*Create counter id for acl rule*/
    ret = xpsAclCreateNewCounter(devId, scopeId, devType, &dummy_tableIdDbEntry,
                                 parallelLookupNum, counterId);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAclCreateNewCounter failed with err : %d\n ", ret);
        return ret;
    }

    pcl_action.matchCounter.enableMatchCount = GT_TRUE;
    pcl_action.matchCounter.matchCounterIndex = *counterId;
    pcl_action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    pcl_action.egressPolicy = (direction == XPS_PCL_STAGE_INGRESS)? GT_FALSE:
                              GT_TRUE;

    if (direction == XPS_PCL_STAGE_INGRESS)
    {
        if (keyType == XPS_PCL_IS_IPV4)
        {
            tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
            tcamId = XPS_PORT_V4_ING_STG_TABLE_ID_0;
        }
        else
        {
            tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
            tcamId = XPS_PORT_V6_ING_STG_TABLE_ID_0;
        }
    }
    else
    {
        if (keyType == XPS_PCL_IS_IPV4)
        {
            tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
            tcamId = XPS_PORT_V4_ERG_STG_TABLE_ID_0;
        }
        else
        {
            tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
            tcamId = XPS_PORT_V6_ERG_STG_TABLE_ID_0;
        }
    }

    tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

    ruleData.valid               = GT_TRUE;
    ruleData.rule.pcl.patternPtr = (CPSS_DXCH_PCL_RULE_FORMAT_UNT *)&pcl_pattern;
    ruleData.rule.pcl.maskPtr    = (CPSS_DXCH_PCL_RULE_FORMAT_UNT *)&pcl_mask;
    ruleData.rule.pcl.actionPtr  = &pcl_action;
    ruleAttributes.priority = cpssDevId << 24 | cpssPortNum;

    /* writing rule to hardware */
    if ((ret = cpssDxChVirtualTcamRuleWrite(XPS_GLOBAL_TACM_MGR, tcamId,
                                            cpssDevId << 24 | cpssPortNum, &ruleAttributes,
                                            &tcamRuleType, &ruleData)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "ACL write failed, err %d \n ", ret);
        return xpsConvertCpssStatusToXPStatus(ret);
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalMacStatCounterRemoveAclRuleForV4V6(xpsDevice_t devId,
                                                    uint32_t portNum,
                                                    uint32_t counterId, xpsAclStage_e direction, xpsAclKeyFlds_t keyType)
{
    XP_STATUS                           ret =  XP_NO_ERR;
    GT_U8                               cpssDevId = xpsGlobalIdToDevId(devId,
                                                                       portNum);
    GT_U32                              cpssPortNum = xpsGlobalPortToPortnum(devId,
                                                                             portNum);
    CPSS_PP_FAMILY_TYPE_ENT             devType = cpssHalDevPPFamilyGet(devId);
    xpsAclTableContextDbEntry_t         dummy_tableIdDbEntry = {};
    xpsScope_t                          scopeId = 0;
    uint32_t                            tcamId = 0;

    if (direction >= XPS_PCL_STAGE_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid pcl direction\n ");
        return XP_ERR_INVALID_VALUE;
    }
    if (keyType != XPS_PCL_IS_IPV4 && keyType != XPS_PCL_IS_IPV6)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid v4v6 pcl key type\n ");
        return XP_ERR_INVALID_VALUE;
    }

    dummy_tableIdDbEntry.isMirror = false;
    dummy_tableIdDbEntry.stage = direction;

    if (direction == XPS_PCL_STAGE_INGRESS)
    {
        tcamId = (keyType == XPS_PCL_IS_IPV4)? XPS_PORT_V4_ING_STG_TABLE_ID_0 :
                 XPS_PORT_V6_ING_STG_TABLE_ID_0;
    }
    else
    {
        tcamId = (keyType == XPS_PCL_IS_IPV4)? XPS_PORT_V4_ERG_STG_TABLE_ID_0 :
                 XPS_PORT_V6_ERG_STG_TABLE_ID_0;
    }

    /*Delete acl rule*/
    cpssDxChVirtualTcamRuleDelete(XPS_GLOBAL_TACM_MGR, tcamId,
                                  cpssDevId << 24 | cpssPortNum);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "ACL delete failed, err %d \n ", ret);
        return xpsConvertCpssStatusToXPStatus(ret);
    }

    /*Release acl counter*/
    ret = xpsAclRemoveCounter(devId, scopeId, devType, &dummy_tableIdDbEntry,
                              counterId);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAclCreateNewCounter failed with err : %d\n ", ret);
        return ret;
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalMacGetCounterV4V6Stats(xpsDevice_t devId, uint32_t portNum,
                                        uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                        uint32_t counterId_Egress_v4, uint32_t counterId_Egress_v6,
                                        uint64_t *v4TxPkts,  uint64_t *v4TxBytes,  uint64_t *v4RxPkts,
                                        uint64_t *v4RxBytes,
                                        uint64_t *v6TxPkts,  uint64_t *v6TxBytes,  uint64_t *v6RxPkts,
                                        uint64_t *v6RxBytes)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_U8 cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    /*Get egress v4 statistic*/
    ret = xpsAclGetCounterValueExt(cpssDevId, counterId_Egress_v4, v4TxPkts,
                                   v4TxBytes);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclGetCounterValue: returned with error %d\n", ret);
        return ret;
    }
    /*Get ingress v4 statistic*/
    ret = xpsAclGetCounterValueExt(cpssDevId, counterId_Ingress_v4, v4RxPkts,
                                   v4RxBytes);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclGetCounterValue: returned with error %d\n", ret);
        return ret;
    }
    /*Get egress v6 statistic*/
    ret = xpsAclGetCounterValueExt(cpssDevId, counterId_Egress_v6, v6TxPkts,
                                   v6TxBytes);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclGetCounterValue: returned with error %d\n", ret);
        return ret;
    }
    /*Get ingress v6 statistic*/
    ret = xpsAclGetCounterValueExt(cpssDevId, counterId_Ingress_v6, v6RxPkts,
                                   v6RxBytes);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclGetCounterValue: returned with error %d\n", ret);
        return ret;
    }

    return XP_NO_ERR;
}

