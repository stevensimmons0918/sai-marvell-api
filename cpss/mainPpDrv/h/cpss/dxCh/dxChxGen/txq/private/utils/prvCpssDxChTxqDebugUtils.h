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
* @file prvCpssDxChTxqDebugUtils.h
*
* @brief CPSS SIP6 TXQ debug
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqDebugUtils
#define __prvCpssDxChTxqDebugUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum{

    PRV_CPSS_TXQ_CIDER_INFO_ACTION_SHOW_ENT = 0,
    PRV_CPSS_TXQ_CIDER_INFO_ACTION_UPGRADE_ENT = 1,
    PRV_CPSS_TXQ_CIDER_INFO_ACTION_CHECK_LAST_ENT = 2
} PRV_CPSS_TXQ_CIDER_INFO_ACTION_ENT;

typedef enum{

    PRV_CPSS_TXQ_FAILURE_QUEUE_DRAIN_E = 0,
    PRV_CPSS_TXQ_FAILURE_MAC_FIFO_DRAIN_E = 1,
} PRV_CPSS_TXQ_FAILURE_TYPE_ENT;


typedef enum{

    PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_INCREASE_E = 0,
    PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_RESET_E = 1,
} PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_ENT;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_ALERTS
 *
 * @brief Failure alerts
*/
typedef struct{

      /**counts mac fifo drain failures*/
    GT_U32 macFifoDrainFailureCount;
    /*counts txq fifo drain alerts*/
    GT_U32 txQDrainFailureCount;
} PRV_CPSS_DXCH_TXQ_SIP_6_ALERT_COUNTERS;

GT_STATUS prvCpssTxqSip6DebugCiderVersionIsLast
(
    IN  GT_U8                   devNum,
    OUT GT_BOOL                 *isLastVersionPtr
);

GT_BOOL prvCpssSip6TxqUtilsDataPathEventLogGet
(
   GT_U8 devNum
);

/**
 * @internal prvCpssSip6TxqUtilsDumpSchedDbProfiles function
 * @endinternal
 *
 * @brief   Print  schedule profile attributes
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS prvCpssSip6TxqUtilsDumpSchedDbProfiles
(
    IN GT_U8 devNum
);

/**
 * @internal prvCpssSip6TxqUtilsDumpBindPortsToSchedDb function
 * @endinternal
 *
 * @brief   Print port to schedule profile binding
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS prvCpssSip6TxqUtilsDumpBindPortsToSchedDb
(
    IN GT_U8 devNum
);

GT_STATUS prvCpssTxqSip6DebugFailureCountSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_TXQ_FAILURE_TYPE_ENT failureType,
    IN  PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_ENT action
);

GT_VOID prvCpssTxqSip6DebugLog
(
    IN  GT_U8                       devNum,
    IN  const char *                file,
    IN  const char *                 function,
    IN  GT_U32                      line,
    IN  const char *                format,
    IN  GT_U32                      param0,
    IN  GT_U32                      param1,
    IN  GT_U32                      param2,
    IN  GT_U32                      param3
);

GT_VOID prvCpssTxqSip6DebugLogInit
(
    IN  GT_U8          devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqFcGopUtils */

