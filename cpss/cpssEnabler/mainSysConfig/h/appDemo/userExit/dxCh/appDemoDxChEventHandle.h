/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChEventHandle.h
*
* @brief this library contains the implementation of the event handling functions
* for the DXCH device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* @version   3
********************************************************************************
*/

#ifndef __appDemoDxChEventHandleh
#define __appDemoDxChEventHandleh

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/userExit/userEventHandler.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal appDemoDxChNetRxPacketCbRegister function
* @endinternal
*
* @brief   register a CB function to be called on every RX packet to CPU
*         NOTE: function is implemented in the 'AppDemo' (application)
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*/
GT_STATUS appDemoDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);


/**
* @internal appDemoDxChNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- DXCH function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktHandle
(
    IN GT_UINTPTR                           evHandler,
    IN RX_EV_HANDLER_EV_TYPE_ENT            evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
);

/**
* @internal appDemoDxChNetRxPktGet function
* @endinternal
*
* @brief   application routine to get the packets from the queue and
*         put it in msgQ. -- DXCH function
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] msgQId                   - Message queue Id.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktGet
(
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN CPSS_OS_MSGQ_ID  msgQId
);

/**
* @internal appDemoDxChNetRxPktTreat function
* @endinternal
*
* @brief   application routine to treat the packets . -- DXCH function
*
* @param[in] rxParamsPtr              - (pointer to) rx paacket params
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktTreat
(
    IN APP_DEMO_RX_PACKET_PARAMS  *rxParamsPtr
);

/**
* @enum APP_DEMO_FEC_COUNTER_TYPES_ENT
 *
 * @brief FEC counter types
*/
typedef enum {
    /* FEC correctable error counter type */
    APP_DEMO_FEC_CE_COUNTER_TYPE_E,
    /* FEC non-correctable error counter type */
    APP_DEMO_FEC_NCE_COUNTER_TYPE_E,
    /* must be last */
    APP_DEMO_FEC_LAST_COUNTER_TYPE_E
}APP_DEMO_FEC_COUNTER_TYPES_ENT;

/* Maximal virtual lanes per port */
#define MAX_VIRTUAL_LANES_CNS       4

/**
* @internal appDemoDxChFecEventPortCounterGet function
* @endinternal
*
* @brief  Get the FEC counters per physical port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] counterType            - FEC counter type
*
* @param[out] countersArray         - array of the counters per physical port
*                                     Every element of array represents counter per one virtual lane for the physical port.
*                                     1 lane    - for ports 10G-R1 and 25G-R1
*                                     4 lanes   - for ports 40G-R4, 50G-R2 and 40G-R2
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - on bad devNum or counter type.
*
* @note none
*
*/
GT_STATUS appDemoDxChFecEventPortCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType,
    OUT GT_U32                          countersArray[MAX_VIRTUAL_LANES_CNS]
);

/**
* @internal appDemoDxChFecEventPortCounterClear function
* @endinternal
*
* @brief  Clear the FEC counters per physical port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - on wrong port number or device.
* @retval GT_NOT_SUPPORTED          - the counter type is not supported.
*
* @note none
*
*/
GT_STATUS appDemoDxChFecEventPortCounterClear
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
);

/**
* @internal appDemoDxChPortFecLaneIndexGet function
* @endinternal
*
* @brief    Gets FEC lane index per physical port
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; Caelum;
*                                   AC5P;Aldrin; AC3X; xCat3;
*                                   AC5; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in]    devNum              - physical device number
* @param[in]    portNum             - physical port number
* @param[out]   fecLaneIndexPtr     - (pointer to) first FEC virtual lane global index per physical port
* @param[out]   fecLanesNumbersPtr  - (pointer to) total FEC virtual lanes per physical port
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong port number or device
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_SUPPORTED          - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_PTR                - on NULL pointer.
*
*/
GT_STATUS appDemoDxChPortFecLaneIndexGet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_U32                              *fecLaneIndexPtr,
    OUT GT_U32                              *fecLanesNumbersPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoDxChEventHandleh */

