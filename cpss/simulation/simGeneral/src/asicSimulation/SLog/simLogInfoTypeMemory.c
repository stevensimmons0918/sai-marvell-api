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
* @file simLogInfoTypeMemory.c
*
* @brief simulation logger memory functions
*
* @version   4
********************************************************************************
*/
#include <string.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypeMemory.h>

#define PCI_CONFIG_CYCLE " PCI config cycle"
#define DFX_MEMORY_ACCESS " DFX memory access"

GT_BOOL  microInitFormat = GT_FALSE;
/**
* @internal simLogMemory function
* @endinternal
*
* @brief   log memory info
*
* @retval GT_OK                    - success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - error
*/
GT_STATUS simLogMemory
(
    IN SKERNEL_DEVICE_OBJECT     const *devObjPtr,
    IN SIM_LOG_MEMORY_ACTION_ENT action,
    IN SIM_LOG_MEMORY_SOURCE_ENT source,
    IN GT_U32                    address,
    IN GT_U32                    value,
    IN GT_U32                    oldValue
)
{
    GT_CHAR actionStr[50] = {0};
    GT_CHAR sourceStr[50] = {0};
    GT_U32  xor = 0;
    GT_U32  deviceId = 0;
    GT_U32  coreId   = 0;
    GT_BOOL opIsRead;

    if(!simLogIsOpenFlag)
    {
        return GT_OK;
    }

    ASSERT_PTR(devObjPtr);

    /* get source */
    switch(source)
    {
        case SIM_LOG_MEMORY_CPU_E:
            strcpy(sourceStr, "CPU");
            break;
        case SIM_LOG_MEMORY_DEV_E:
            strcpy(sourceStr, "DEV");
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* get device and core */
    if (devObjPtr->portGroupSharedDevObjPtr)
    {
        /* the device is core in the father device */
        /* so the devId should be of the father    */
        deviceId = devObjPtr->portGroupSharedDevObjPtr->deviceId;
        coreId = devObjPtr->portGroupId;
    }
    else
    {
        /*the device is not part of multi-cores device */
        deviceId = devObjPtr->deviceId;
        coreId = 0;/* not applicable*/
    }

    switch(action)
    {
        case SIM_LOG_MEMORY_PCI_READ_E:
            strcpy(actionStr, "read" PCI_CONFIG_CYCLE);
            opIsRead = GT_TRUE;
            break;
        case SIM_LOG_MEMORY_READ_E:
            opIsRead = GT_TRUE;
            strcpy(actionStr, "read" );
            break;
        case SIM_LOG_MEMORY_DFX_READ_E:
            opIsRead = GT_TRUE;
            strcpy(actionStr, "read" DFX_MEMORY_ACCESS);
            break;
        case SIM_LOG_MEMORY_PCI_WRITE_E:
            opIsRead = GT_FALSE;
            strcpy(actionStr, "write" PCI_CONFIG_CYCLE);
            break;
        case SIM_LOG_MEMORY_WRITE_E:
            opIsRead = GT_FALSE;
            strcpy(actionStr, "write" );
            break;
        case SIM_LOG_MEMORY_DFX_WRITE_E:
            opIsRead = GT_FALSE;
            strcpy(actionStr, "write" DFX_MEMORY_ACCESS);
            break;
        default:
            return GT_BAD_PARAM;
    }

    if(opIsRead == GT_TRUE)
    {
        if(microInitFormat==GT_FALSE)
        {
            /* print read row*/
            simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
                  "%6s %6s %3d %5d 0x%08X 0x%08X %10s %9s\n",
                  actionStr, sourceStr, deviceId, coreId, address, value,
                  "NA", "NA");
        }
    }
    else
    {
        xor = value ^ oldValue;

        if(microInitFormat==GT_FALSE)
        {
             simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
                "%6s %6s %3d %5d 0x%08X 0x%08X 0x%08X 0%08X\n",
                actionStr, sourceStr, deviceId, coreId, address, value,
                oldValue, xor);
        }
        else
        {
            /* print write row */
            simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
                  "wrd 0 0 0x%08X 0x%08X 0xffffffff\n",
                   address, value);
        }
    }

    return GT_OK;
}


/**
* @internal simGenericHiddenLogMemory function
* @endinternal
*
* @brief   log memory info
* @param[in] deviceObj               - pointer to device object.
* @param[in] action                  - the action access type.
* @param[in] source                  - the source access type.
* @param[in] memName                 - the memory name.
* @param[in] memInstance             - the memory instance ID.
* @param[in] entryIndex              - the entry index in the memory.
* @param[in] value                   - the value to write.
* @param[in] oldValue                - the value that exists before the write.
*
* @retval GT_OK                    - success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - error
*/
GT_STATUS simGenericHiddenLogMemory
(
    IN SKERNEL_DEVICE_OBJECT     const *devObjPtr,
    IN SIM_LOG_MEMORY_ACTION_ENT action,
    IN SIM_LOG_MEMORY_SOURCE_ENT source,
    IN GT_CHAR*                  memName,
    IN GT_U32                    memInstance,
    IN GT_U32                    entryIndex,
    IN GT_U32                    value,
    IN GT_U32                    oldValue
)
{
    GT_CHAR actionStr[50] = {0};
    GT_CHAR sourceStr[50] = {0};
    GT_U32  xor = 0;
    GT_U32  deviceId = 0;
    GT_U32  coreId   = 0;
    GT_BOOL opIsRead;

    if(!simLogIsOpenFlag)
    {
        return GT_OK;
    }

    ASSERT_PTR(devObjPtr);

    /* get source */
    switch(source)
    {
        case SIM_LOG_MEMORY_CPU_E:
            strcpy(sourceStr, "CPU");
            break;
        case SIM_LOG_MEMORY_DEV_E:
            strcpy(sourceStr, "DEV");
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* get device and core */
    if (devObjPtr->portGroupSharedDevObjPtr)
    {
        /* the device is core in the father device */
        /* so the devId should be of the father    */
        deviceId = devObjPtr->portGroupSharedDevObjPtr->deviceId;
        coreId = devObjPtr->portGroupId;
    }
    else
    {
        /*the device is not part of multi-cores device */
        deviceId = devObjPtr->deviceId;
        coreId = 0;/* not applicable*/
    }

    switch(action)
    {
        case SIM_LOG_MEMORY_READ_E:
            opIsRead = GT_TRUE;
            strcpy(actionStr, "read" );
            break;
        case SIM_LOG_MEMORY_WRITE_E:
            opIsRead = GT_FALSE;
            strcpy(actionStr, "write" );
            break;
        default:
            return GT_BAD_PARAM;
    }

    if(opIsRead == GT_TRUE)
    {
        /* print read row */
        simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
              "%6s %6s %3d %5d %s 0x%08X %10s %9s [instance %d][index 0x%8.8x]\n",
              actionStr,
                   sourceStr,
                       deviceId,
                           coreId,
                               memName,
                                  value, "NA", "NA",
                                                            memInstance,
                                                                      entryIndex);
    }
    else
    {
        xor = value ^ oldValue;

        /* print write row */
        simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
              "%6s %6s %3d %5d %s 0x%08X 0x%08X 0%08X [instance %d][index 0x%8.8x]\n",
              actionStr,
                   sourceStr,
                       deviceId,
                           coreId,
                               memName,
                                  value, oldValue, xor,
                                                                memInstance,
                                                                            entryIndex);
    }

    return GT_OK;
}

