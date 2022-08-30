// xpSaiUdf.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiUdf.h"

XP_SAI_LOG_REGISTER_API(SAI_API_UDF);

static sai_udf_api_t* _xpSaiUdfApi;

/**
 *  xpSaiUdfCreate - implements sai_create_udf_fn
 */
sai_status_t xpSaiUdfCreate(sai_object_id_t *udf_id, sai_object_id_t switch_id,
                            uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfCreate.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfRemove - implements sai_remove_udf_fn
 */
static sai_status_t xpSaiUdfRemove(sai_object_id_t udf_id)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfRemove.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfAttributeSet - implements set_udf_attribute
 */
sai_status_t xpSaiUdfAttributeSet(sai_object_id_t udf_id,
                                  const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfAttributeSet.\n");

    switch (attr->id)
    {
        case SAI_UDF_ATTR_MATCH_ID:
        case SAI_UDF_ATTR_GROUP_ID:
        case SAI_UDF_ATTR_BASE:
        case SAI_UDF_ATTR_OFFSET:
        case SAI_UDF_ATTR_HASH_MASK:
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

/**
 *  xpSaiUdfAttributeGet - implements sai_get_udf_attribute_fn
 */
sai_status_t xpSaiUdfAttributeGet(sai_object_id_t udf_id, uint32_t attr_count,
                                  sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfAttributeGet.\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_UDF_ATTR_MATCH_ID:
            case SAI_UDF_ATTR_GROUP_ID:
            case SAI_UDF_ATTR_BASE:
            case SAI_UDF_ATTR_OFFSET:
            case SAI_UDF_ATTR_HASH_MASK:
                {
                    saiRetVal = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                    saiRetVal = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                    break;
                }
        }
    }

    return saiRetVal;
}

/**
 *  xpSaiUdfMatchCreate - implements sai_create_udf_match_fn
 */
sai_status_t xpSaiUdfMatchCreate(sai_object_id_t *udf_match_id,
                                 sai_object_id_t switch_id,
                                 uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfMatchCreate.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfMatchRemove - implements sai_remove_udf_match_fn
 */
static sai_status_t xpSaiUdfMatchRemove(sai_object_id_t udf_match_id)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfMatchRemove.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfMatchAttributeSet - implements sai_set_udf_match_attribute_fn
 */
sai_status_t xpSaiUdfMatchAttributeSet(sai_object_id_t udf_match_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfMatchAttributeSet.\n");

    switch (attr->id)
    {
        case SAI_UDF_MATCH_ATTR_L2_TYPE:
        case SAI_UDF_MATCH_ATTR_L3_TYPE:
        case SAI_UDF_MATCH_ATTR_GRE_TYPE:
        case SAI_UDF_MATCH_ATTR_PRIORITY:
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

/**
 *  xpSaiUdfMatchAttributeGet - implements sai_get_udf_match_attribute_fn
 */
sai_status_t xpSaiUdfMatchAttributeGet(sai_object_id_t udf_match_id,
                                       uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfMatchAttributeGet.\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_UDF_MATCH_ATTR_L2_TYPE:
            case SAI_UDF_MATCH_ATTR_L3_TYPE:
            case SAI_UDF_MATCH_ATTR_GRE_TYPE:
            case SAI_UDF_MATCH_ATTR_PRIORITY:
                {
                    XP_SAI_LOG_ERR("Unsupported attribute %d\n", attr_list[count].id);
                    saiRetVal = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                    saiRetVal = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                    break;
                }
        }
    }

    return saiRetVal;
}

/**
 *  xpSaiUdfGroupCreate - implements sai_create_udf_group_fn
 */
sai_status_t xpSaiUdfGroupCreate(sai_object_id_t *udf_group_id,
                                 sai_object_id_t switch_id,
                                 uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfGroupCreate.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfGroupRemove - implements sai_remove_udf_group_fn
 */
static sai_status_t xpSaiUdfGroupRemove(sai_object_id_t udf_group_id)
{
    XP_SAI_LOG_DBG("Calling xpSaiUdfGroupRemove.\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

/**
 *  xpSaiUdfGroupAttributeSet - implements sai_set_udf_group_attribute_fn
 */
sai_status_t xpSaiUdfGroupAttributeSet(sai_object_id_t udf_group_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfGroupAttributeSet.\n");

    switch (attr->id)
    {
        case SAI_UDF_GROUP_ATTR_UDF_LIST:
        case SAI_UDF_GROUP_ATTR_TYPE:
        case SAI_UDF_GROUP_ATTR_LENGTH:
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

/**
 *  xpSaiUdfGroupAttributeGet - implements sai_get_udf_group_attribute_fn
 */
sai_status_t xpSaiUdfGroupAttributeGet(sai_object_id_t udf_group_id,
                                       uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfGroupAttributeGet.\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_UDF_GROUP_ATTR_UDF_LIST:
            case SAI_UDF_GROUP_ATTR_TYPE:
            case SAI_UDF_GROUP_ATTR_LENGTH:
                {
                    XP_SAI_LOG_ERR("Unsupported attribute %d\n", attr_list[count].id);
                    saiRetVal = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                    saiRetVal = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                    break;
                }
        }
    }

    return saiRetVal;
}

XP_STATUS xpSaiUdfApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUdfApiInit.\n");

    _xpSaiUdfApi = (sai_udf_api_t *)xpMalloc(sizeof(sai_udf_api_t));
    if (NULL == _xpSaiUdfApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiUdfApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiUdfApi->create_udf = xpSaiUdfCreate;
    _xpSaiUdfApi->remove_udf = xpSaiUdfRemove;
    _xpSaiUdfApi->set_udf_attribute = xpSaiUdfAttributeSet;
    _xpSaiUdfApi->get_udf_attribute = xpSaiUdfAttributeGet;
    _xpSaiUdfApi->create_udf_match = xpSaiUdfMatchCreate;
    _xpSaiUdfApi->remove_udf_match = xpSaiUdfMatchRemove;
    _xpSaiUdfApi->set_udf_match_attribute = xpSaiUdfMatchAttributeSet;
    _xpSaiUdfApi->get_udf_match_attribute = xpSaiUdfMatchAttributeGet;
    _xpSaiUdfApi->create_udf_group = xpSaiUdfGroupCreate;
    _xpSaiUdfApi->remove_udf_group = xpSaiUdfGroupRemove;
    _xpSaiUdfApi->set_udf_group_attribute = xpSaiUdfGroupAttributeSet;
    _xpSaiUdfApi->get_udf_group_attribute = xpSaiUdfGroupAttributeGet;

    saiRetVal = xpSaiApiRegister(SAI_API_UDF, (void*)_xpSaiUdfApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register SAI UDF API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

XP_STATUS xpSaiUdfApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiUdfApiDeinit.\n");

    xpFree(_xpSaiUdfApi);
    _xpSaiUdfApi = NULL;

    return  retVal;
}
