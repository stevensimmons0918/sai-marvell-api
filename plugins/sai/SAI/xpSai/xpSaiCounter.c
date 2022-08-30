// xpSaiCounter.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiCounter.h"

XP_SAI_LOG_REGISTER_API(SAI_API_COUNTER);

static sai_counter_api_t* _xpSaiCounterApi;

/**
 *  xpSaiCounterCreate - implements sai_create_counter_fn
 */
sai_status_t xpSaiCounterCreate(sai_object_id_t *counter_id,
                                sai_object_id_t switch_id,
                                uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterCreate.\n");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 *  xpSaiCounterRemove - implements sai_remove_counter_fn
 */
sai_status_t xpSaiCounterRemove(sai_object_id_t counter_id)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterRemove.\n");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 *  xpSaiCounterAttributeSet - implements set_counter_attribute
 */
sai_status_t xpSaiCounterAttributeSet(sai_object_id_t counter_id,
                                      const sai_attribute_t *attr)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterAttributeSet.\n");
    return SAI_STATUS_NOT_IMPLEMENTED;

}

/**
 *  xpSaiCounterAttributeGet - implements sai_get_counter_attribute_fn
 */
sai_status_t xpSaiCounterAttributeGet(sai_object_id_t counter_id,
                                      uint32_t attr_count,
                                      sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterAttributeGet.\n");
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t xpSaiCounterStatsGet(sai_object_id_t counter_id,
                                  uint32_t number_of_counters,
                                  const sai_stat_id_t *counter_ids, uint64_t *counters)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterStatsGet.\n");
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t xpSaiCounterStatsExtGet(sai_object_id_t counter_id,
                                     uint32_t number_of_counters,
                                     const sai_stat_id_t *counter_ids,
                                     sai_stats_mode_t mode, uint64_t *counters)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterStatsGet.\n");
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t xpSaiCounterStatsClear(sai_object_id_t counter_id,
                                    uint32_t number_of_counters,
                                    const sai_stat_id_t *counter_ids)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterStatsClear.\n");
    return SAI_STATUS_NOT_IMPLEMENTED;
}
/**
 *  xpSaiCounterMatchCreate - implements sai_create_counter_match_fn
 */
sai_status_t xpSaiCounterMatchCreate(sai_object_id_t *counter_match_id,
                                     sai_object_id_t switch_id,
                                     uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterMatchCreate.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiCounterGroupCreate - implements sai_create_counter_group_fn
 */
sai_status_t xpSaiCounterGroupCreate(sai_object_id_t *counter_group_id,
                                     sai_object_id_t switch_id,
                                     uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiCounterGroupCreate.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

XP_STATUS xpSaiCounterApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiCounterApiInit.\n");

    _xpSaiCounterApi = (sai_counter_api_t *)xpMalloc(sizeof(sai_counter_api_t));
    if (NULL == _xpSaiCounterApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiCounterApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiCounterApi->create_counter = xpSaiCounterCreate;
    _xpSaiCounterApi->remove_counter = xpSaiCounterRemove;
    _xpSaiCounterApi->set_counter_attribute = xpSaiCounterAttributeSet;
    _xpSaiCounterApi->get_counter_attribute = xpSaiCounterAttributeGet;
    _xpSaiCounterApi->get_counter_stats = xpSaiCounterStatsGet;
    _xpSaiCounterApi->get_counter_stats_ext = xpSaiCounterStatsExtGet;
    _xpSaiCounterApi->clear_counter_stats = xpSaiCounterStatsClear;

    saiRetVal = xpSaiApiRegister(SAI_API_COUNTER, (void*)_xpSaiCounterApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register SAI COUNTER API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

XP_STATUS xpSaiCounterApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiCounterApiDeinit.\n");

    xpFree(_xpSaiCounterApi);
    _xpSaiCounterApi = NULL;

    return  retVal;
}
