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
* @file prvCpssPxPortDynamicPAPortSpeedDB.c
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter Port Speed DB
*
* @version   1
********************************************************************************
*/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>



/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBInit function
* @endinternal
*
* @brief   init port speed DB for given device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBInit
(
    IN GT_SW_DEV_NUM           devNum
)
{
    GT_PHYSICAL_PORT_NUM i;
    PRV_CPSS_PX_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    portDBPtr = &PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB;

    portDBPtr->numInitPorts = 0;
    for (i = 0 ; i < sizeof(portDBPtr->prv_portDB)/sizeof(portDBPtr->prv_portDB[0]); i++)
    {
        portDBPtr->prv_portDB[i].physicalPortNum = i;
        portDBPtr->prv_portDB[i].isInitilized    = GT_FALSE;
        portDBPtr->prv_portDB[i].portSpeedInMBit = 0;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBSet function
* @endinternal
*
* @brief   set port BW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speedInMBit              - speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBSet
(
    IN    GT_SW_DEV_NUM               devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32                      speedInMBit
)
{
    PRV_CPSS_PX_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);


    portDBPtr = &PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB;
    if (portNum >= sizeof(portDBPtr->prv_portDB)/sizeof(portDBPtr->prv_portDB[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (speedInMBit > 0)
    {
        portDBPtr->prv_portDB[portNum].isInitilized = GT_TRUE;
        portDBPtr->prv_portDB[portNum].portSpeedInMBit  = speedInMBit;
        portDBPtr->numInitPorts ++;
    }
    else
    {
        portDBPtr->prv_portDB[portNum].isInitilized = GT_FALSE;
        portDBPtr->prv_portDB[portNum].portSpeedInMBit  = 0;
        portDBPtr->numInitPorts --;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBGet function
* @endinternal
*
* @brief   set port BW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isInitilizedPtr          - (pointer to)whether port is initialized
* @param[out] speedInMBitPtr           - (pointer to)speed in Mbps in case when  initialized
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBGet
(
    IN    GT_SW_DEV_NUM              devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
)
{
    PRV_CPSS_PX_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(isInitilizedPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedInMBitPtr);

    portDBPtr = &PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB;

    if (portNum >= sizeof(portDBPtr->prv_portDB)/sizeof(portDBPtr->prv_portDB[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *isInitilizedPtr = portDBPtr->prv_portDB[portNum].isInitilized;;
    *speedInMBitPtr  = portDBPtr->prv_portDB[portNum].portSpeedInMBit;
    return GT_OK;
}



