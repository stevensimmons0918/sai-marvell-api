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
* @file prvCpssPxPortMappingShadowDB.c
*
* @brief interface to port mapping shadow DB
*
* @version   1
********************************************************************************
*/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBEntryCheck function
* @endinternal
*
* @brief   Check port whether is allowed to use as index
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBEntryCheck
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum
)
{
    GT_U32 dbSize= sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr)/
                   sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[0]);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (physPortNum >= dbSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBClear function
* @endinternal
*
* @brief   Clear entry in port mapping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBClear
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    rc = prvCpssPxPortPhysicalPortMapShadowDBEntryCheck(devNum,physPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].valid = GT_FALSE;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType            = CPSS_PX_PORT_MAPPING_TYPE_INVALID_E;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.macNum                 = CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.dmaNum                 = CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txqNum                 = CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS;
    return GT_OK;
}

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBSet function
* @endinternal
*
* @brief   Clear entry in papping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
* @param[in] portMapPtr               - pointer to user mapping
* @param[in] portArgArr[CPSS_PX_PORT_MAPPING_TYPE_MAX_E] - pointer to detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBSet
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_PHYSICAL_PORT_NUM    physPortNum,
    IN CPSS_PX_PORT_MAP_STC   *portMapPtr,
    IN GT_U32                  portArgArr[CPSS_PX_PORT_MAPPING_TYPE_MAX_E]
)
{
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortPhysicalPortMapShadowDBEntryCheck(devNum,physPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    prvCpssPxPortPhysicalPortMapShadowDBClear(devNum,physPortNum); 
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].valid = GT_TRUE;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType      = portMapPtr->mappingType;
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.macNum           = portArgArr[PRV_CPSS_PX_PORT_TYPE_MAC_E];
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.dmaNum           = portArgArr[PRV_CPSS_PX_PORT_TYPE_RxDMA_E];
    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txqNum           = portArgArr[PRV_CPSS_PX_PORT_TYPE_TXQ_E];

    if (portMapPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[portArgArr[PRV_CPSS_PX_PORT_TYPE_MAC_E]] = physPortNum;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBGet function
* @endinternal
*
* @brief   get pointer to entry in mapping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @param[out] portMapShadowPtrPtr      - (pointer to) entry in mapping DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBGet
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum,
    OUT  CPSS_PX_DETAILED_PORT_MAP_STC **portMapShadowPtrPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortPhysicalPortMapShadowDBEntryCheck(devNum,physPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(portMapShadowPtrPtr);
    *portMapShadowPtrPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum];
    return GT_OK;

}


/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBInit function
* @endinternal
*
* @brief   init shadow mapping WB
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               macNum;
    GT_U32              arraySize;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    arraySize = sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr)/
                sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[0]);

    for (portNum = 0; portNum < arraySize; portNum++)
    {
        prvCpssPxPortPhysicalPortMapShadowDBClear(devNum,portNum);
    }

    /*-----------------------------------*
     * inverse MAC--> Physical port init *
     *-----------------------------------*/
    arraySize = sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr)
                      /sizeof(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[0]);
    for (macNum = 0; macNum < arraySize; macNum++)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[macNum] = CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortPhysicalPortIsCpu function
* @endinternal
*
* @brief   check whether given port is CPU port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @retval GT_TRUE                  - on success (CPU port)
* @retval GT_FALSE                 - (if not)
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortIsCpu
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortPhysicalPortMapShadowDBEntryCheck(devNum,physPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType ==
        CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}


