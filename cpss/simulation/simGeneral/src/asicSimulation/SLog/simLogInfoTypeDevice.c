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
* @file simLogInfoTypeDevice.c
*
* @brief simulation logger device functions
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/skernel.h>

/*
 * macro to compare device object field
 */
#define SIM_LOG_DEVICE_COMPARE(field)                                       \
    {                                                                       \
        if ( memcmp(&old->field, &new->field, sizeof(old->field) != 0) )    \
        {                                                                   \
            if( GT_FALSE == changed )                                       \
            {                                                               \
                changed = GT_TRUE;                                          \
            }                                                               \
            simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_DEVICE_E,            \
                          "\n%55s:%9d%9d", #field, old->field, new->field); \
        }                                                                   \
    }


/**
* @internal simLogDevDescrPortGroupId function
* @endinternal
*
* @brief   log PortGroupId
*/
GT_VOID simLogDevDescrPortGroupId
(
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr
)
{
    simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_DEVICE_E, 
       "\n**************** devObjPtr->portGroupId = %d\n", devObjPtr->portGroupId);
}

/**
* @internal simLogDevObjCompare function
* @endinternal
*
* @brief   log changes between saved device object and given
*/
GT_VOID simLogDevObjCompare
(
    IN SKERNEL_DEVICE_OBJECT const *old,
    IN SKERNEL_DEVICE_OBJECT const *new,
    IN GT_CHAR               const *funcName
)
{
    /*GT_BOOL changed = GT_FALSE; */

    ASSERT_PTR(old);
    ASSERT_PTR(new);
    ASSERT_PTR(funcName);

    simLogMessage(NULL, NULL, 0, new, SIM_LOG_INFO_TYPE_DEVICE_E, 
       "**************** %30s devObj changes: ", funcName);

    /*
    SIM_LOG_DEVICE_COMPARE(byteCount);
    */
}


