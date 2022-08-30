// xpSaiAppQos.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAppQos.h"
XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

sai_object_id_t gAqmQProfileIdMap[XPSAI_MAX_AQM_PROFILES];

XP_STATUS xpSaiAppConfigureIngressQosMapEntry(xpDevice_t devId,
                                              sai_object_id_t switchOid, qosIngressMapDataEntry *pIngressMapData)
{
    sai_object_id_t port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t dot1p_to_tc_map_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t dscp_to_tc_map_id = SAI_NULL_OBJECT_ID;
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_attribute_t sai_port_attr;
    sai_attribute_t sai_qos_map_attr[2];
    sai_qos_map_list_t qos_map_list;
    sai_uint8_t idx;
    uint8_t deviceMaxPort = 0;

    qos_map_list.count = 1;
    qos_map_list.list = (sai_qos_map_t *)calloc(qos_map_list.count,
                                                sizeof(sai_qos_map_t));
    if (qos_map_list.list == NULL)
    {
        printf("Error allocating memory for qos_map_list.list\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(qos_map_list.list, 0x0, qos_map_list.count * sizeof(sai_qos_map_t));

    /* L2Qos Trust Profile */
    if (pIngressMapData->l2QosEn)
    {
        qos_map_list.list[0].key.dot1p = pIngressMapData->pcp;
        qos_map_list.list[0].value.tc =  pIngressMapData->tc;

        sai_qos_map_attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
        sai_qos_map_attr[0].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC;

        sai_qos_map_attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
        sai_qos_map_attr[1].value.qosmap.count = qos_map_list.count;
        sai_qos_map_attr[1].value.qosmap.list = qos_map_list.list;

        /* Create the DOT1P To TC and Color map */
        if ((retVal = xpSaiQosMapApi->create_qos_map(&dot1p_to_tc_map_id, switchOid, 2,
                                                     sai_qos_map_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to create qos map |retVal: %d, Line# %d\n",
                           __FUNCTION__, retVal, __LINE__);
            return saiStatus2XpsStatus(retVal);
        }
    }

    /* L3Qos Trust Profile */
    if (pIngressMapData->l3QosEn)
    {
        qos_map_list.list[0].key.dscp = pIngressMapData->dscp;
        qos_map_list.list[0].value.tc = pIngressMapData->tc;

        sai_qos_map_attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
        sai_qos_map_attr[0].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC;

        sai_qos_map_attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
        sai_qos_map_attr[1].value.qosmap.count = qos_map_list.count;
        sai_qos_map_attr[1].value.qosmap.list = qos_map_list.list;

        /* Create the DSCP to TC and Color map */
        if ((retVal = xpSaiQosMapApi->create_qos_map(&dscp_to_tc_map_id, switchOid, 2,
                                                     sai_qos_map_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to create qos map |retVal: %d, Line# %d\n",
                           __FUNCTION__, retVal, __LINE__);
            free(qos_map_list.list);
            return saiStatus2XpsStatus(retVal);
        }
    }

    /* Free the allocated memory */
    free(qos_map_list.list);

    if ((retVal = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &deviceMaxPort)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("%s:Get system mac port failed. Error = %d", __FUNCTION__,
                       retVal);
        return saiStatus2XpsStatus(retVal);
    }

    for (idx = 0; (pIngressMapData->portList[idx] != -1) &&
         (idx < deviceMaxPort); idx++)
    {
        /* Get port object Id */
        if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                       (sai_uint64_t)pIngressMapData->portList[idx], &port_id)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(retVal);
        }

        /* Attach l2qos map to port */
        sai_port_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
        sai_port_attr.value.oid = dot1p_to_tc_map_id;
        if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                       &sai_port_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to set port attribute, DOT1P to TC & COLOR MAP |retVal: %d\n",
                           __FUNCTION__, retVal);
            return saiStatus2XpsStatus(retVal);
        }

        /* Attach l3qos map to port */
        sai_port_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
        sai_port_attr.value.oid = dscp_to_tc_map_id;
        if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                       &sai_port_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to set port attribute, DSCP to TC & COLOR MAP |retVal: %d\n",
                           __FUNCTION__, retVal);
            return saiStatus2XpsStatus(retVal);
        }

        /* Set the Default TC for port */
        sai_port_attr.id = SAI_PORT_ATTR_QOS_DEFAULT_TC;
        sai_port_attr.value.u32 = pIngressMapData->defaultTC;
        if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                       &sai_port_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to set port attribute, DEFAULT TC |retVal: %d\n",
                           __FUNCTION__, retVal);
            return saiStatus2XpsStatus(retVal);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureEgressQosMapEntry(xpDevice_t devId,
                                             sai_object_id_t switchOid, qosEgressMapDataEntry *pEgressMapData)
{
    sai_object_id_t tc_to_dot1p_map_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t tc_to_dscp_map_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t port_id = SAI_NULL_OBJECT_ID;
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_attribute_t sai_qos_map_attr[2];
    sai_qos_map_list_t qos_map_list;
    sai_attribute_t sai_port_attr;

    qos_map_list.count = 1;
    qos_map_list.list = (sai_qos_map_t *)calloc(qos_map_list.count,
                                                sizeof(sai_qos_map_t));
    if (qos_map_list.list == NULL)
    {
        printf("Error allocating memory for qos_map_list.list\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(qos_map_list.list, 0x0, qos_map_list.count*sizeof(sai_qos_map_t));

    if (pEgressMapData->l2QosEn)
    {
        qos_map_list.list[0].key.tc = pEgressMapData->tc;
        qos_map_list.list[0].key.color = (sai_packet_color_t)(pEgressMapData->dp);
        qos_map_list.list[0].value.dot1p = pEgressMapData->pcp;

        sai_qos_map_attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
        sai_qos_map_attr[0].value.s32 = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P;

        sai_qos_map_attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
        sai_qos_map_attr[1].value.qosmap.count = qos_map_list.count;
        sai_qos_map_attr[1].value.qosmap.list = qos_map_list.list;

        /* Create the TC and Color to DOT1P mapping */
        if ((retVal = xpSaiQosMapApi->create_qos_map(&tc_to_dot1p_map_id, switchOid, 2,
                                                     sai_qos_map_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to create qos map |retVal: %d, Line# %d\n",
                           __FUNCTION__, retVal, __LINE__);
            return saiStatus2XpsStatus(retVal);
        }
    }

    if (pEgressMapData->l3QosEn)
    {
        qos_map_list.list[0].key.tc = pEgressMapData->tc;
        qos_map_list.list[0].key.color = (sai_packet_color_t)(pEgressMapData->dp);
        qos_map_list.list[0].value.dscp = pEgressMapData->dscp;

        sai_qos_map_attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
        sai_qos_map_attr[0].value.s32 = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP;

        sai_qos_map_attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
        sai_qos_map_attr[1].value.qosmap.count = qos_map_list.count;
        sai_qos_map_attr[1].value.qosmap.list = qos_map_list.list;

        /* Create the TC and Color to DOT1P mapping */
        if ((retVal = xpSaiQosMapApi->create_qos_map(&tc_to_dscp_map_id, switchOid, 2,
                                                     sai_qos_map_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to create qos map |retVal: %d, Line# %d\n",
                           __FUNCTION__, retVal, __LINE__);
            free(qos_map_list.list);
            return saiStatus2XpsStatus(retVal);
        }
    }
    /* Free the allocated memory */
    free(qos_map_list.list);

    /* Get port object Id */
    if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                   (sai_uint64_t)pEgressMapData->egressVif, &port_id)) != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI object can not be created.\n");
        return saiStatus2XpsStatus(retVal);
    }

    /* Enable TC AND COLOR -> DOT1P MAP */
    sai_port_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;
    sai_port_attr.value.oid = tc_to_dot1p_map_id;
    if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                   &sai_port_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to set port attribute, TC & COLOR to DOT1P MAP |retVal: %d\n",
                       __FUNCTION__, retVal);
        return saiStatus2XpsStatus(retVal);
    }

    /* Enable TC AND COLOR -> DSCP MAP */
    sai_port_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
    sai_port_attr.value.oid = tc_to_dscp_map_id;
    if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                   &sai_port_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to set port attribute, TC & COLOR to DSCP MAP |retVal: %d\n",
                       __FUNCTION__, retVal);
        return saiStatus2XpsStatus(retVal);
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureQueueSchedulerEntry(xpDevice_t devId,
                                               sai_object_id_t switchOid, saiSchedulerDataEntry *pSchedulerData)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t queue_id;
    sai_object_id_t scheduler_id;
    sai_attribute_t sai_q_attr;
    sai_attribute_t sai_schd_attr[5];
    sai_uint8_t pIdx, qIdx;
    uint8_t deviceMaxPort = 0;

    /* Scheduler type */
    sai_schd_attr[0].id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    if (pSchedulerData->enableSP == 1)
    {
        /* Enable SP Scheduler */
        sai_schd_attr[0].value.s32 = SAI_SCHEDULING_TYPE_STRICT;
    }
    else
    {
        /* Enable DWRR Scheduler */
        sai_schd_attr[0].value.s32 = SAI_SCHEDULING_TYPE_DWRR;
    }

    /* Queue weight */
    sai_schd_attr[1].id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    sai_schd_attr[1].value.u8 = pSchedulerData->weight;

    /* Shaper type */
    sai_schd_attr[2].id = SAI_SCHEDULER_ATTR_METER_TYPE;
    sai_schd_attr[2].value.s32 = SAI_METER_TYPE_BYTES;

    /* Bandwidth shape rate [bytes/sec]. Value 0 to No Limit */
    sai_schd_attr[3].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    sai_schd_attr[3].value.u64 = pSchedulerData->bandwidthRate;

    /* Burst for Bandwidth shape rate [Bytes] */
    sai_schd_attr[4].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    sai_schd_attr[4].value.u64 = pSchedulerData->burstSize;

    /* Create scheduler profile */
    if ((retVal = xpSaiSchedulerApi->create_scheduler(&scheduler_id, switchOid, 5,
                                                      sai_schd_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to create scheduler profile |retVal: %d, Line# %d\n",
                       __FUNCTION__, retVal, __LINE__);
        return saiStatus2XpsStatus(retVal);
    }

    if ((retVal = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &deviceMaxPort)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("%s:Get system mac port failed. Error = %d", __FUNCTION__,
                       retVal);
        return saiStatus2XpsStatus(retVal);
    }

    for (pIdx = 0; (pIdx < XP_MAX_QUEUES_PER_PORT &&
                    pSchedulerData->portList[pIdx] != -1) &&
         (pIdx < deviceMaxPort); pIdx++)
    {
        for (qIdx = 0; (qIdx < XP_MAX_QUEUES_PER_PORT &&
                        pSchedulerData->queueNum[qIdx] != -1) &&
             (qIdx < XP_MAX_QUEUES_PER_PORT); qIdx++)
        {
            /* Get queue object Id */
            xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, pSchedulerData->portList[pIdx],
                                     pSchedulerData->queueNum[qIdx], &queue_id);

            /* Attach scheduler profile to queue */
            sai_q_attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
            sai_q_attr.value.oid = scheduler_id;
            if ((retVal = xpSaiQueueApi->set_queue_attribute(queue_id,
                                                             &sai_q_attr)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("%s:Failed to set queue attribute scheduler profile id |retVal: %d\n",
                               __FUNCTION__, retVal);
                return saiStatus2XpsStatus(retVal);
            }
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureShaperPortEntry(xpDevice_t devId,
                                           sai_object_id_t switchOid, qosShaperPortDataEntry *pShaperPortData)
{
    sai_object_id_t port_id;
    sai_object_id_t scheduler_id;
    sai_attribute_t sai_port_attr;
    sai_attribute_t sai_schd_attr[4];
    sai_uint64_t bandwidthRate;
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    /* Shaper type, Packets/Bytes */
    sai_schd_attr[0].id = SAI_SCHEDULER_ATTR_METER_TYPE;
    sai_schd_attr[0].value.s32 = SAI_METER_TYPE_BYTES;

    /* Convert Kbps to Bytes */
    bandwidthRate = ((pShaperPortData->rateKbps * XP_KBPS_TO_BITS_FACTOR) /
                     XP_BITS_IN_BYTE);

    /* Guaranteed Bandwidth shape rate [bytes/sec]. Value 0 to No Limit */
    sai_schd_attr[1].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    sai_schd_attr[1].value.u64 = bandwidthRate;

    /* Guaranteed Burst for Bandwidth shape rate [Bytes] */
    sai_schd_attr[2].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    sai_schd_attr[2].value.u64 = pShaperPortData->maxBurstSize;

    /* Scheduler type */
    sai_schd_attr[3].id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    sai_schd_attr[3].value.s32 = SAI_SCHEDULING_TYPE_DWRR;

    /* Create Scheduler Profile */
    if ((retVal = xpSaiSchedulerApi->create_scheduler(&scheduler_id, switchOid, 4,
                                                      sai_schd_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to create scheduler profile |retVal: %d, Line# %d\n",
                       __FUNCTION__, retVal, __LINE__);
        return saiStatus2XpsStatus(retVal);
    }

    /* Get port object Id */
    if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                   pShaperPortData->devPort, &port_id)) != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI object can not be created.\n");
        return saiStatus2XpsStatus(retVal);
    }

    /* Attach scheduler profile to port */
    sai_port_attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    sai_port_attr.value.oid = scheduler_id;
    if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                   &sai_port_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to set port attribute, SCHEDULER PROFILE ID |retVal: %d\n",
                       __FUNCTION__, retVal);
        return saiStatus2XpsStatus(retVal);
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureWredEntry(xpDevice_t devId,
                                     sai_object_id_t switchOid, qosAqmQProfileDataEntry *pWredData)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_attribute_t sai_wred_attr[6];
    sai_attribute_t sai_q_attr;
    sai_object_id_t queue_id;
    sai_object_id_t wred_id;

    if (gAqmQProfileIdMap[pWredData->profileId] == 0)
    {
        if (pWredData->wredMode == XP_DISABLE_WRED)
        {
            sai_wred_attr[0].id = SAI_WRED_ATTR_GREEN_ENABLE;
            sai_wred_attr[0].value.booldata = false;

            sai_wred_attr[1].id = SAI_WRED_ATTR_ECN_MARK_MODE;
            sai_wred_attr[1].value.s32 = SAI_ECN_MARK_MODE_NONE;
        }
        else
        {
            sai_wred_attr[0].id = SAI_WRED_ATTR_GREEN_ENABLE;
            sai_wred_attr[0].value.booldata = true;

            if (pWredData->wredMode == XP_ENABLE_WRED_MARK)
            {
                sai_wred_attr[1].id = SAI_WRED_ATTR_ECN_MARK_MODE;
                sai_wred_attr[1].value.s32 = SAI_ECN_MARK_MODE_GREEN;
            }
            else
            {
                sai_wred_attr[1].id = SAI_WRED_ATTR_ECN_MARK_MODE;
                sai_wred_attr[1].value.s32 = SAI_ECN_MARK_MODE_NONE;
            }
        }

        /* WRED Average Queue Length Minimum Threshold */
        /* Convert average queue length min threshold from pages to bytes */
        sai_wred_attr[2].id = SAI_WRED_ATTR_GREEN_MIN_THRESHOLD;
        sai_wred_attr[2].value.s32 = (pWredData->minThreshold *
                                      XPSAI_BUFFER_PAGE_SIZE_BYTES);

        /* WRED Average Queue Length Maximum Threshold */
        /* Convert average queue length max threshold from pages to bytes */
        sai_wred_attr[3].id = SAI_WRED_ATTR_GREEN_MAX_THRESHOLD;
        sai_wred_attr[3].value.s32 = (pWredData->maxThreshold *
                                      XPSAI_BUFFER_PAGE_SIZE_BYTES);

        /* WRED Max Drop Probability Percentage */
        sai_wred_attr[4].id = SAI_WRED_ATTR_GREEN_DROP_PROBABILITY;
        sai_wred_attr[4].value.s32 = pWredData->maxDropProb;

        /* WRED Average Queue Size Weight */
        sai_wred_attr[5].id = SAI_WRED_ATTR_WEIGHT;
        sai_wred_attr[5].value.s32 = pWredData->weight;

        /* Create WRED object */
        if ((retVal = xpSaiWredApi->create_wred(&wred_id, switchOid, 6,
                                                sai_wred_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to create wred profile |retVal: %d, Line# %d\n",
                           __FUNCTION__, retVal, __LINE__);
            return saiStatus2XpsStatus(retVal);
        }
        gAqmQProfileIdMap[pWredData->profileId] = wred_id;
    }
    else
    {
        wred_id = gAqmQProfileIdMap[pWredData->profileId];
    }

    /* Get the queue object id */
    xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, pWredData->devPort,
                             pWredData->queueNum, &queue_id);

    /* Applying the wred to a queue */
    sai_q_attr.id = SAI_QUEUE_ATTR_WRED_PROFILE_ID;
    sai_q_attr.value.oid = wred_id;
    if ((retVal = xpSaiQueueApi->set_queue_attribute(queue_id,
                                                     &sai_q_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to set queue attribute wred profile id |retVal: %d\n",
                       __FUNCTION__, retVal);
        return saiStatus2XpsStatus(retVal);
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigurePolicerEntry(xpDevice_t devId,
                                        sai_object_id_t switchOid, policerDataEntry *pPolicerData)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_attribute_t sai_policer_attr[9];
    sai_attribute_t sai_port_attr;
    sai_object_id_t policer_id;
    sai_object_id_t port_id;
    sai_uint32_t idx;
    uint8_t deviceMaxPort = 0;

    /* Metering is done based on bytes */
    sai_policer_attr[0].id = SAI_POLICER_ATTR_METER_TYPE;
    sai_policer_attr[0].value.s32 = SAI_METER_TYPE_BYTES;

    /* Two Rate Three color marker, CIR, CBS, PIR and PBS, G, Y and R */
    sai_policer_attr[1].id = SAI_POLICER_ATTR_MODE;
    sai_policer_attr[1].value.s32 = SAI_POLICER_MODE_TR_TCM;

    /* Color */
    sai_policer_attr[2].id = SAI_POLICER_ATTR_COLOR_SOURCE;
    if (pPolicerData->color == 0)
    {
        /* Previous coloring schemes are ignored */
        sai_policer_attr[2].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
    }
    else
    {
        /* Previous coloring schemes are used */
        sai_policer_attr[2].value.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;
    }

    /* Committed burst size */
    sai_policer_attr[3].id = SAI_POLICER_ATTR_CBS;
    sai_policer_attr[3].value.u64 = pPolicerData->cbs;

    /* Committed information rate */
    sai_policer_attr[4].id = SAI_POLICER_ATTR_CIR;
    sai_policer_attr[4].value.u64 = pPolicerData->cir;

    /* Peak burst size */
    sai_policer_attr[5].id = SAI_POLICER_ATTR_PBS;
    sai_policer_attr[5].value.u64 = pPolicerData->pbs;

    /* Peak information rate */
    sai_policer_attr[6].id = SAI_POLICER_ATTR_PIR;
    sai_policer_attr[6].value.u64 = pPolicerData->pir;

    /* Action to take for YELLOW color packets */
    sai_policer_attr[7].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
    if (pPolicerData->yellowPktAction == 0)
    {
        /* Drop Packet */
        sai_policer_attr[7].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        /* Forward Packet */
        sai_policer_attr[7].value.s32 = SAI_PACKET_ACTION_FORWARD;
    }

    /* Action to take for RED color packets */
    sai_policer_attr[8].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    if (pPolicerData->redPktAction == 0)
    {
        /* Drop Packet */
        sai_policer_attr[8].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        /* Forward Packet */
        sai_policer_attr[8].value.s32 = SAI_PACKET_ACTION_FORWARD;
    }

    /* Create Policer object */
    if ((retVal = xpSaiPolicerApi->create_policer(&policer_id, switchOid, 9,
                                                  sai_policer_attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:Failed to create policer |retVal: %d\n", __FUNCTION__,
                       retVal);
        return saiStatus2XpsStatus(retVal);
    }

    if ((retVal = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &deviceMaxPort)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("%s:Get system mac port failed. Error = %d", __FUNCTION__,
                       retVal);
        return saiStatus2XpsStatus(retVal);
    }

    for (idx = 0; (pPolicerData->portList[idx] != -1) &&
         (idx < deviceMaxPort); idx++)
    {
        /* Get port object Id */
        if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                       (sai_uint64_t)pPolicerData->portList[idx], &port_id)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(retVal);
        }

        /* Attach policer to port */
        sai_port_attr.id = SAI_PORT_ATTR_POLICER_ID;
        sai_port_attr.value.oid = policer_id;
        if ((retVal = xpSaiPortApi->set_port_attribute(port_id,
                                                       &sai_port_attr)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s:Failed to set port attribute, POLICER ID |retVal: %d\n",
                           __FUNCTION__, retVal);
            return saiStatus2XpsStatus(retVal);
        }
    }
    return XP_NO_ERR;
}
