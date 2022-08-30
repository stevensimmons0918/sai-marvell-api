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
* @file prvCpssPxHwLogicalTables.c
*
* @brief API implementation for logical tables.
* state the relations between the logical tables to HW tables.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssPxTableHwMaxIndexGet function
* @endinternal
*
* @brief   Function returns the number of entries,lines that HW table hold
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - the HW table
*
* @param[out] maxNumEntriesPtr         - (pointer to) the number of entries supported by the table
* @param[out] maxNumLinesPtr           - (pointer to) the number of lines   supported by the table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not covered by this function
*/
GT_STATUS prvCpssPxTableHwMaxIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PX_TABLE_ENT                   hwTable,
    OUT GT_U32                              *maxNumEntriesPtr,
    OUT GT_U32                              *maxNumLinesPtr
)
{
    GT_STATUS   rc;
    GT_U32  maxNumLines;
    GT_U32  maxNumEntries;
    GT_U32  ratio;
    GT_BOOL  isMultiple;

    maxNumLines = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.maxNumOfEntries;

    rc = prvCpssPxTableEngineToHwRatioGet(devNum,hwTable,&ratio,&isMultiple);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(ratio == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Get ratio == 0 (should have not happened)");
    }

    maxNumEntries = maxNumLines;

    if(ratio != 1)
    {
        if(isMultiple == GT_TRUE)
        {
            /* ratio is 'multiple' (number of entries in single line) */
            maxNumEntries *= ratio;
        }
        else
        {
            /* ratio is 'fraction' (number of lines  in single entry) */
            maxNumEntries /= ratio;
        }
    }

    if(maxNumEntriesPtr)
    {
        *maxNumEntriesPtr = maxNumEntries;
    }
    if(maxNumLinesPtr)
    {
        *maxNumLinesPtr   = maxNumLines;
    }

    return GT_OK;
}

/**
* @internal checkIsDevSupportHwTable function
* @endinternal
*
* @brief   Function checks if the device supports the HW table
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in] hwTableType              - the HW table
*
* @retval GT_TRUE                  - the device   supports the table
* @retval GT_FALSE                 - the device NOT supports the table
*/
static GT_BOOL  checkIsDevSupportHwTable(
    IN GT_U8                devNum ,
    IN CPSS_PX_TABLE_ENT  hwTableType
)
{
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */

    if((GT_U32)hwTableType >=  PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum))
    {
        /* the HW table is not valid for this device */
        return GT_FALSE;
    }

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTableType);

    if(tableInfoPtr->directAccessInfo.maxNumOfEntries == 0)
    {
        /* the HW table is not valid for this device */
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal prvCpssPxTableCheckIsDevSupportHwTable function
* @endinternal
*
* @brief   Function checks if the device supports the HW table
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
*                                      hwTable     - the HW table
* @param[in] entryIndex               - entry index
*
* @retval GT_TRUE                  - the device   supports the table
* @retval GT_FALSE                 - the device NOT supports the table or entry
*/
GT_BOOL  prvCpssPxTableCheckIsDevSupportHwTable(
    IN GT_U8                devNum ,
    IN CPSS_PX_TABLE_ENT  hwTableType,
    IN GT_U32               entryIndex
)
{
    GT_STATUS   rc;
    GT_U32  maxLines;
    GT_U32  maxEntries;

    if(GT_FALSE == checkIsDevSupportHwTable(devNum,hwTableType))
    {
        /* the HW table is not valid for this device */
        return GT_FALSE;
    }

    /* get number of entries in the table */
    rc = prvCpssPxTableHwMaxIndexGet(devNum,hwTableType,&maxEntries,&maxLines);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }

    if(entryIndex >= maxEntries)
    {
        /* index is above valid range of this table ... ignore it !!! */
        return GT_FALSE;
    }

    return GT_TRUE;
}




