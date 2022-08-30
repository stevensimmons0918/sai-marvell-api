// xpsAppQos.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAppQos.h"
#include "xpsAppL2.h"

uint8_t gAqmQProfileIdMap[MAX_AQM_PROFILES];

extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern xpsInterfaceId_t gLagIntfId[XP_MAX_DEVICES][MAX_NO_LAGS];
extern uint32_t
multicastVif[XP_MAX_DEVICES][XP_MULTICAST_TYPE_MAX][MAX_MULTICAST_ENTRIES];

XP_STATUS xpsAppConfigureQosSchedulerEntry(xpDevice_t devId,
                                           qosSchedulerDataEntry *qosSchdData)
{
    XP_STATUS retVal;
    int idx;

    if (!qosSchdData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    for (idx = 0; (idx < XP_MAX_QUEUES_PER_PORT) &&
         (qosSchdData->queueNum[idx] != -1); idx++)
    {
        // Configure the queue priority.
        if ((retVal = xpsQosSetQueueSchedulerSPPriority(devId, qosSchdData->devPortId,
                                                        qosSchdData->queueNum[idx], qosSchdData->priority)) != XP_NO_ERR)
        {
            printf("%s:Error: Failed to set Queue Scheduler Priority: error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }

        // Configure the scheduler type.
        if (qosSchdData->enableSP == 0)
        {
            // Configure the weights.
            if ((retVal = xpsQosSetQueueSchedulerDWRRWeight(devId, qosSchdData->devPortId,
                                                            qosSchdData->queueNum[idx], qosSchdData->weight)) != XP_NO_ERR)
            {
                printf("%s:Error: Failed to set Queue Scheduler DWRR Weight: error code: %d\n",
                       __FUNCTION__, retVal);
                return retVal;
            }

            // Scheduler type DWRR
            if ((retVal = xpsQosSetQueueSchedulerDWRR(devId, qosSchdData->devPortId,
                                                      qosSchdData->queueNum[idx], 1)) != XP_NO_ERR)
            {
                printf("%s:Error: Failed to set Queue Scheduler type to DWRR: error code: %d\n",
                       __FUNCTION__, retVal);
                return retVal;
            }
        }
        else if (qosSchdData->enableSP == 1)
        {
            // Scheduler type SP
            if ((retVal = xpsQosSetQueueSchedulerSP(devId, qosSchdData->devPortId,
                                                    qosSchdData->queueNum[idx], 1)) != XP_NO_ERR)
            {
                printf("%s:Error: Failed to set Queue Scheduler type to SP: error code: %d\n",
                       __FUNCTION__, retVal);
                return retVal;
            }
        }
        //printf ("PortId: %d, Queue#: %d, qPriotity: %d, Scheduler type: %s, Weight: %d\n", qosSchdData->devPortId, qosSchdData->queueNum[idx], qosSchdData->priority, qosSchdData->enableSP? "SP":"DWRR", qosSchdData->weight);
    }
    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureQosShaperPortEntry(xpDevice_t devId,
                                            qosShaperPortDataEntry *qosShaperPortData)
{
    XP_STATUS retVal;

    if (!qosShaperPortData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    // Configure shaping on a particular port
    if ((retVal = xpsQosShaperConfigurePortShaper(devId, qosShaperPortData->devPort,
                                                  qosShaperPortData->rateKbps, qosShaperPortData->maxBurstSize)) != XP_NO_ERR)
    {
        printf("%s:Error: Can't configure port shaper: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    // Configure port shaping enable/disable on a particular port
    if ((retVal = xpsQosShaperSetPortShaperEnable(devId, qosShaperPortData->devPort,
                                                  qosShaperPortData->portShaperEn)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Set Port Shaper Enable: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    // Configure port shaping feature
    if ((retVal = xpsQosShaperEnablePortShaping(devId,
                                                qosShaperPortData->featureEn)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Enable Port Shaping: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }
    printf("PortId: %d, maxBurstSize: %d, rateKbps: %ld, portShaperEn: %s, featureEn: %s\n",
           qosShaperPortData->devPort, qosShaperPortData->maxBurstSize,
           (long int)qosShaperPortData->rateKbps,
           qosShaperPortData->portShaperEn? "Enable":"Disable",
           qosShaperPortData->featureEn?"Enable":"Disable");

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureQosShaperQueueEntry(xpDevice_t devId,
                                             qosShaperQueueDataEntry *qosShaperQueueData)
{
    XP_STATUS retVal;

    if (!qosShaperQueueData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    // Configure a Queue Slow shaper
    if ((retVal = xpsQosShaperConfigureQueueSlowShaper(devId,
                                                       qosShaperQueueData->devPort, qosShaperQueueData->queueNum,
                                                       qosShaperQueueData->rateKbps, qosShaperQueueData->maxBurstSize)) != XP_NO_ERR)
    {
        printf("%s:Error: Can't Configure Queue Slow Shaper: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    // Configure queue slow shaping enable/disable
    if ((retVal = xpsQosShaperSetQueueSlowShaperEnable(devId,
                                                       qosShaperQueueData->devPort, qosShaperQueueData->queueNum,
                                                       qosShaperQueueData->qSlowShaperEn)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Set Queue Slow Shaper Enable: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    // Configure queue shaping feature
    if ((retVal = xpsQosShaperEnableQueueShaping(devId,
                                                 qosShaperQueueData->featureEn)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Enable Queue Shaping: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }
    printf("PortId: %d, Queue#: %d, maxBurstSize: %d, rateKbps: %ld, qSlowShaperEn: %s, featureEn: %s\n",
           qosShaperQueueData->devPort, qosShaperQueueData->queueNum,
           qosShaperQueueData->maxBurstSize, (long int)qosShaperQueueData->rateKbps,
           qosShaperQueueData->qSlowShaperEn?"Enable":"Disable",
           qosShaperQueueData->featureEn?"Enable":"Disable");

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureQosAqmQProfileEntry(xpDevice_t devId,
                                             qosAqmQProfileDataEntry *qosAqmQProfileData)
{
    XP_STATUS retVal;
    uint32_t profileId;

    if (!qosAqmQProfileData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    if (gAqmQProfileIdMap[qosAqmQProfileData->profileId] == 0)
    {
        //Create the profile
        if ((retVal = xpsQosAqmCreateAqmQProfile(devId, &profileId)) != XP_NO_ERR)
        {
            printf("%s:Error: Failed to Create Aqm QProfile: error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
        gAqmQProfileIdMap[qosAqmQProfileData->profileId] = profileId;
    }
    else
    {
        profileId = gAqmQProfileIdMap[qosAqmQProfileData->profileId];
    }

    //Configure Wred Mode
    if ((retVal = xpsQosAqmConfigureWredMode(devId, profileId,
                                             qosAqmQProfileData->wredMode)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Configure Wred Mode: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    if (qosAqmQProfileData->wredMode != XP_DISABLE_WRED)
    {
        //Configure Wred
        if ((retVal = xpsQosAqmConfigureWred(devId, profileId,
                                             qosAqmQProfileData->minThreshold, qosAqmQProfileData->maxThreshold,
                                             qosAqmQProfileData->weight, qosAqmQProfileData->maxDropProb)) != XP_NO_ERR)
        {
            printf("%s:Error: Failed to Configure Wred: error code: %d\n", __FUNCTION__,
                   retVal);
            return retVal;
        }
    }

    //Configure Tail Drop
    if ((retVal = xpsQosAqmConfigureQueuePageTailDropThresholdForProfile(devId,
                                                                         profileId, qosAqmQProfileData->lengthMaxThreshold)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Configure Queue Page Tail Drop Threshold: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    //Bind the profile to Queue
    if ((retVal = xpsQosAqmBindAqmQProfileToQueue(devId,
                                                  qosAqmQProfileData->devPort, qosAqmQProfileData->queueNum,
                                                  profileId)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to Bind Aqm QProfile To Queue: error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }
    printf("devPort%d,queueNum=%d,profileId=%d,minThreshold%d,maxThreshold%d,weight=%d,maxDropProb=%d,lengthMaxThreshold=%d,wredMode=%d\n",
           qosAqmQProfileData->devPort, qosAqmQProfileData->queueNum,
           qosAqmQProfileData->profileId, qosAqmQProfileData->minThreshold,
           qosAqmQProfileData->maxThreshold, qosAqmQProfileData->weight,
           qosAqmQProfileData->maxDropProb, qosAqmQProfileData->lengthMaxThreshold,
           qosAqmQProfileData->wredMode);

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureEgressQosMapEntry(xpDevice_t devId,
                                           qosEgressMapDataEntry *qosEgressMapData)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpEgressCosMapData_t map;
    xpsHashIndexList_t indexList;
    uint32_t enable = 1;
    xpsInterfaceId_t intfId;
    uint32_t gTnlVif = 0;
    void *r;

    if (!qosEgressMapData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    switch (qosEgressMapData->type)
    {
        case 0: //Port:
            if ((retVal = xpsPortGetPortIntfId(devId, qosEgressMapData->egressPort,
                                               &intfId)) != XP_NO_ERR)
            {
                printf("%s: Error: Failed to get the interface ID for device : %d, port : %d\n",
                       __FUNCTION__, devId, qosEgressMapData->egressPort);
                return retVal;
            }
            qosEgressMapData->egressVif = intfId;
            break;
        case 1: //Lag:
            qosEgressMapData->egressVif = gLagIntfId[devId][qosEgressMapData->idx];
            break;
        case 2: //XP_L2_ENCAP_VXLAN:
        case 3: //XP_L2_ENCAP_NVGRE:
            qosEgressMapData->egressVif = tnlVif[devId][qosEgressMapData->idx];
            break;
        case 4: //XP_L2_ENCAP_GENEVE:
            gTnlVif = qosEgressMapData->idx + qosEgressMapData->geneveTnlType;
            printf("QOS Map: Geneve Tunnel Vif : %d | eVif: %d \n", gTnlVif,
                   tnlVif[devId][gTnlVif]);
            qosEgressMapData->egressVif = tnlVif[devId][gTnlVif];
            break;
        case 5:
            qosEgressMapData->egressVif = pbbTnlVif[devId][qosEgressMapData->idx];
            break;
        case 6: //MPLS Tunnel
            r  = getAllocatedMplsTnlVif(devId, qosEgressMapData->idx);
            if (r == NULL)
            {
                printf("Error: mplsIdx : %d in not found in allocatedMplsTnlIvif \n",
                       qosEgressMapData->idx);
                return XP_ERR_INIT;
            }
            qosEgressMapData->egressVif  = (*(allocatedMplsTnlVifMap **)r)->tnlId;
            break;
        case 7: //Multicast

            qosEgressMapData->egressVif =
                multicastVif[devId][qosEgressMapData->mcastType][qosEgressMapData->idx];
            break;
        default:
            printf("Error: invalid type: %d\n", qosEgressMapData->type);
            break;
    }

    retVal = xpsQosSetEgressQosMapEn(devId, enable);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error: Failed to set Egress Qos Map Entry, error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    map.l2QosEn = qosEgressMapData->l2QosEn;
    map.l3QosEn = qosEgressMapData->l3QosEn;
    map.pcp = qosEgressMapData->pcp;
    map.dei = qosEgressMapData->dei;
    map.dscp = qosEgressMapData->dscp;

    if (map.l2QosEn == 1)
    {
        xpsQosSetPcpDeiRemarkEn(devId, enable);
    }
    if (map.l3QosEn == 1)
    {
        xpsQosSetDscpRemarkEn(devId, enable);
    }

    if ((retVal = xpsQosEgressQosMapAdd(devId, qosEgressMapData->egressVif,
                                        qosEgressMapData->tc, qosEgressMapData->dp, map, &indexList)) != XP_NO_ERR)
    {
        printf("%s:Error: Failed to add Egress Qos Map Entry, egressVif: %d, tc: %d, dp: %d, error code: %d\n",
               __FUNCTION__, qosEgressMapData->egressVif, qosEgressMapData->tc,
               qosEgressMapData->dp, retVal);
        return retVal;
    }

    printf("Egress Qos Map Entry, egressVif: %d, tc: %d, dp: %d, l2QosEn: %d, l3QosEn: %d, pcp: %d, dei: %d, dscp: %d,\n",
           qosEgressMapData->egressVif, qosEgressMapData->tc, qosEgressMapData->dp,
           qosEgressMapData->l2QosEn, qosEgressMapData->l3QosEn,
           qosEgressMapData->pcp, qosEgressMapData->dei, qosEgressMapData->dscp);

    return retVal;
}
