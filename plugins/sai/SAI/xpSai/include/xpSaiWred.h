// xpSaiWred.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiWred_h_
#define _xpSaiWred_h_

#include "xpSai.h"
#include "cpssDxChPortEcn.h"


#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_WRED_COLOUR_GREEN   (0)
#define XPSAI_WRED_COLOUR_YELLOW  (1)
#define XPSAI_WRED_COLOUR_RED     (2)
#define XPSAI_WRED_GREEN_PFL_OFFSET  (0)
#define XPSAI_WRED_GREEN_PFL_LEN     (8)
#define XPSAI_WRED_YELLOW_PFL_OFFSET (8)
#define XPSAI_WRED_YELLOW_PFL_LEN    (8)
#define XPSAI_WRED_RED_PFL_OFFSET    (16)
#define XPSAI_WRED_RED_PFL_LEN       (8)

#define XPSAI_WRED_GREEN_PFL_VAL_MASK   (((1 << XPSAI_WRED_GREEN_PFL_LEN) -1) << XPSAI_WRED_GREEN_PFL_OFFSET)
#define XPSAI_WRED_YELLOW_PFL_VAL_MASK  (((1 << XPSAI_WRED_YELLOW_PFL_LEN) -1) << XPSAI_WRED_YELLOW_PFL_OFFSET)
#define XPSAI_WRED_RED_PFL_VAL_MASK     (((1 << XPSAI_WRED_RED_PFL_LEN) -1) << XPSAI_WRED_RED_PFL_OFFSET)

#define XPSAI_WRED_IS_ENABLE_VALID(x)             ((((x) >= 0) && ((x) <= 1)) ? 1 : 0)
#define XPSAI_WRED_IS_THRESHOLD_VALID(x)          ((((x) >= 0) && ((x) <= XPSAI_BUFFER_TOTAL_BUFFER_BYTES)) ? 1 : 0)
#define XPSAI_WRED_IS_DROP_PROBABILITY_VALID(x)   ((((x) >= 0) && ((x) <= 100)) ? 1 : 0)
#define XPSAI_WRED_IS_WEIGHT_VALID(x)             ((((x) >= 0) && ((x) <= 15)) ? 1 : 0)
#define XPSAI_WRED_IS_ECN_MARK_MODE_VALID(x)      ((((x) >= SAI_ECN_MARK_MODE_NONE) && ((x) <= SAI_ECN_MARK_MODE_ALL)) ? 1 : 0)

#define XPSAI_WRED_LIST_SIZE              (2048) /*(256 ports*8 queues)   - TBD replace with dynamic allocation*/

typedef struct _xpSaiWredAttributesT
{
    sai_attribute_value_t greenEnable;
    sai_attribute_value_t greenMinThreshold;
    sai_attribute_value_t greenMaxThreshold;
    sai_attribute_value_t greenDropProbability;
    sai_attribute_value_t yellowEnable;
    sai_attribute_value_t yellowMinThreshold;
    sai_attribute_value_t yellowMaxThreshold;
    sai_attribute_value_t yellowDropProbability;
    sai_attribute_value_t redEnable;
    sai_attribute_value_t redMinThreshold;
    sai_attribute_value_t redMaxThreshold;
    sai_attribute_value_t redDropProbability;
    sai_attribute_value_t weight;
    sai_attribute_value_t ecnMarkMode;
} xpSaiWredAttributesT;

typedef struct xpSaiWred_t
{
    sai_object_id_t     wredObjId;          /* Wred profile object id */
    xpsDevice_t         xpDevId;            /* Device Id*/

    bool                greenEnable;        /* Enable/Disable wred on green color */
    sai_uint32_t
    greenMinThreshold;  /* Min threshold for green color in bytes */
    sai_uint32_t
    greenMaxThreshold;  /* Max threshold for green color in bytes */
    sai_uint32_t
    greenDropProb;      /* Drop probability for green color in percentage(0 to 100). Default 100% */

    bool
    yellowEnable;       /* Enable/Disable wred on yellow color */
    sai_uint32_t
    yellowMinThreshold; /* Min threshold for yellow color in bytes */
    sai_uint32_t
    yellowMaxThreshold; /* Max threshold for yellow color in bytes */
    sai_uint32_t
    yellowDropProb;     /* Drop probability for yellow color in percentage(0 to 100). Default 100% */

    bool                redEnable;          /* Enable/Disable wred on red color */
    sai_uint32_t
    redMinThreshold;    /* Min threshold for yellow color in bytes */
    sai_uint32_t
    redMaxThreshold;    /* Max threshold for yellow color in bytes */
    sai_uint32_t
    redDropProb;        /* Drop probability for yellow color in percentage(0 to 100). Default 100% */

    sai_uint32_t        weight;             /* Weight(0 to 15), Default 0 */
    sai_ecn_mark_mode_t
    ecnMarkMode;        /* Enable/Disable ECN marking, Default is SAI_ECN_MARK_MODE_NONE */
    sai_uint32_t
    objCount;           /* Count number of objects to which wed is assigned */
    sai_object_id_t     objList[XPSAI_WRED_LIST_SIZE]; /* List of queue objects */
} xpSaiWred_t;

XP_STATUS xpSaiWredApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiWredApiDeinit();
XP_STATUS xpSaiWredInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiWredDeInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiWredObjIdValueGet(sai_object_id_t wredObjId,
                                    sai_uint32_t *pGreenProfileId, sai_uint32_t *pYellowProfileId,
                                    sai_uint32_t *pRedProfileId);

sai_status_t xpSaiMaxCountWredAttribute(uint32_t *count);
sai_status_t xpSaiCountWredObjects(uint32_t *count);
sai_status_t xpSaiGetWredObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list);
sai_status_t xpSaiGetWredAttribute(sai_object_id_t wred_id, uint32_t attr_count,
                                   sai_attribute_t *attr_list);
sai_status_t xpSaiCreateWred(sai_object_id_t *wred_id,
                             sai_object_id_t switch_id, uint32_t attr_count,
                             const sai_attribute_t *attr_list);
sai_status_t xpSaiWredValidateWredObject(sai_object_id_t wredObjId);
sai_status_t xpSaiWredBindQueueToWred(sai_object_id_t wredObjId,
                                      sai_object_id_t queueObjId);
sai_status_t xpSaiWredUnbindQueueToWred(sai_object_id_t wredObjId,
                                        sai_object_id_t queueObjId);

XP_STATUS xpSaiWredSai2CpssProfileGet
(
    IN  xpSaiWred_t *pWredInfo,
    OUT CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC * pCpssWredInfo,
    OUT CPSS_PORT_QUEUE_ECN_PARAMS_STC                  * pCpssEcnInfo
);

XP_STATUS xpSaiWredGetState(sai_object_id_t wredObjId,
                            xpSaiWred_t **ppWredInfo);



#ifdef __cplusplus
}
#endif

#endif //_xpSaiWred_h_
