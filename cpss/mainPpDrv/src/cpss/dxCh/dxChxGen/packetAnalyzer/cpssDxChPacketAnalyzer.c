/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChPacketAnalyzer.c
*
* @brief Packet Analyzer APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebugAuto.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzerLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**************************** Constants definitions *****************************/

/**************************** Private macro definitions ***********************/
#define PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(_manager)                                   \
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.packetAnalyzerSrc.paMngDB[_manager])

/* check that the managerId was initialized */
#define PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId)                                         \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)                         \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);                                      \
    }                                                                                                       \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->numOfDevices == 0)              \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                      \
    }

/* check that the actionId was initialized */
#define PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId)                                 \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] == NULL)  \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);                                      \
    }

/* check that the groupId was initialized */
#define PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId)                                   \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] == NULL)   \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);                                      \
    }

/* check that the field assignment mode is valid */
#define PRV_CPSS_PACKET_ANALYZER_FIELD_MODE_CHECK_MAC(fieldMode)                        \
    switch (fieldMode)                                                                  \
        {                                                                               \
        case CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E:              \
        case CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E:          \
            /* do nothing */                                                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
        }

/* check that the paField is in range */
#define PRV_CPSS_PACKET_ANALYZER_FIELD_ID_CHECK_MAC(paField)                            \
    if (paField >= CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)                              \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/****************************** Enum definitions ********************************/

/********************************* Structures definitions ***********************/

/**************************** Private apis  *************************************/
/* convert idebug field to packet analyzer field */
static CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT idebugFieldToPacketAnalyzerFieldConv(GT_U32 managerId ,const GT_CHAR_PTR iDebugField)
{
    GT_U32                                      i;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    for(i=0; i<PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_UDF_NUM_CNS; i++)
    {
        if(cpssOsStrCmp(iDebugField,paMngDbPtr->udfArr[i].iDebugField) == 0)
        {
            return (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)(i+(PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1));
        }
    }

    for(i=0; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E; i++)
    {
        if(cpssOsStrCmp(iDebugField,packetAnalyzerFieldToIdebugFieldArr[i]) == 0)
        {
            return (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)i;
        }
    }

    return CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
}

/**
* @internal
*           prvCpssDxChPacketAnalyzerSamplingEnableCheck
*           function
* @endinternal
*
* @brief   Check that sampling on action is not enabled.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId              - manager identification
*                                   (APPLICABLE RANGES: 1..10)
* @param[in] actionId               - action identification
*                                   (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - on invalid parameter
* @retval GT_NOT_FOUND             - not found action
*
*/
static GT_STATUS prvCpssDxChPacketAnalyzerSamplingEnableCheck
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       paActDbPtr = NULL;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);

    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];
    if (paActDbPtr->action.samplingEnable == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: Can't change configuration while sampling is enabled for actionId [%d]",actionId);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerRuleActivateEnableSet function
* @endinternal
*
* @brief   Enable/disable packet analyzer rule activation.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] paRuleDbPtr              - (pointer to) rule identification
* @param[in] enable                   - enable/disable rule activation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPacketAnalyzerRuleActivateEnableSet
(
    IN  GT_U32                                      managerId,
    IN  PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr,
    IN  GT_BOOL                                     enable
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       paActDbPtr = NULL;
    GT_U32                                      i,j,keyId;
    GT_U32                                      numOfFields,numOfFieldsDb;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;
    GT_STATUS                                   rc = GT_OK;
    GT_U8                                       devNum;
    GT_CHAR                                     interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         paFieldName;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    *fieldsValueArr = NULL;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    *idebugFieldsValueArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         *fieldsArr = NULL;
    GT_U32                                      id,muxValidBmp;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(paRuleDbPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[paRuleDbPtr->paActId];

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFieldsDb);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* array allocation */
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    idebugFieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsDb);
    fieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsDb);
    if (fieldsArr == NULL || idebugFieldsValueArr == NULL || fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /*mark valid stages*/
    rc = prvCpssDxChPacketAnalyzerMarkMuxedStagesSet(managerId,devNum,paKeyDbPtr->numOfStages,paKeyDbPtr->stagesArr,&muxValidBmp);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        /*This stage was whiped out due to mux constaints*/
        if((muxValidBmp&(1<<i))==0)
            continue;

        stageId = paKeyDbPtr->stagesArr[i];
        if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        {
            id = stageId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

            /* for uds check that it is valid */
            if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
                continue;
        }

        /* Get keyId fields list per stage for bounded interface */
        numOfFields = numOfFieldsDb; /* maximum allocated number of fields for stageId */
        rc = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stageId,&numOfFields,fieldsArr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        /* clean the array */
        cpssOsMemSet(idebugFieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsDb);

        cpssOsStrCpy(interfaceId,paDevDbPtr->stagesArr[stageId].boundInterface.interfaceId);

        if (enable == GT_TRUE)
        {
            /* delete interface from rule's content (Data and Mask) */
            rc = prvCpssDxChIdebugClearLongestInterface(managerId,devNum,interfaceId);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }

            /* prepare array with rule's content (Data and Mask) */
            for (j=0; j<numOfFields; j++)
            {
                paFieldName = fieldsArr[j];

                if (paFieldName >= CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
                {
                    rc = GT_BAD_PARAM;
                    goto exit_cleanly_lbl;
                }

                if ((paFieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (paFieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
                {
                    id = paFieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

                    if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",paFieldName);

                    cpssOsStrCpy(idebugFieldsValueArr[j].fieldName,paMngDbPtr->udfArr[id].iDebugField);
                }
                else
                {
                    cpssOsStrCpy(idebugFieldsValueArr[j].fieldName,packetAnalyzerFieldToIdebugFieldArr[paFieldName]);
                }

                if (enable == GT_TRUE)
                {
                    cpssOsMemCpy(idebugFieldsValueArr[j].data, paRuleDbPtr->fieldsValueArr[paFieldName].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
                    cpssOsMemCpy(idebugFieldsValueArr[j].mask, paRuleDbPtr->fieldsValueArr[paFieldName].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
                }
            }

            /* configure bounded interface with rule's content (Data and Mask) */
            rc = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId,numOfFields,idebugFieldsValueArr);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }

        /* configure action attributes */
        rc = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId,&(paActDbPtr->action));
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    cpssOsFree(idebugFieldsValueArr);
    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);
    return rc;
}

/**
* @internal
*           prvCpssDxChPacketAnalyzerRuleActionSampelingEnableSet
*           function
* @endinternal
*
* @brief   Enable/disable packet analyzer rule's action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] paRuleDbPtr              - (pointer to) rule identification
* @param[in] enable                   - enable/disable rule's
*                                       action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPacketAnalyzerRuleActionSampelingEnableSet
(
    IN  GT_U32                                      managerId,
    IN  PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr,
    IN  GT_BOOL                                     enable
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      i,keyId,id;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;
    GT_STATUS                                   rc = GT_OK;
    GT_U8                                       devNum;
    GT_CHAR                                     interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         *fieldsArr = NULL;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(paRuleDbPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: PA is not enabled on device [%d]",devNum);
    }

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        stageId = paKeyDbPtr->stagesArr[i];

        if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        {
            id = stageId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);
            /* check if uds is valid */
            if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
                continue;
        }

        cpssOsStrCpy(interfaceId,paDevDbPtr->stagesArr[stageId].boundInterface.interfaceId);

        /* update sampling for interface */
        rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId,enable);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    cpssOsFree(fieldsArr);

    return rc;
}

/**
* @internal prvCpssDxChPacketAnalyzerActionGroupAdd function
* @endinternal
*
* @brief   Add packet analyzer group to action DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
static GT_STATUS prvCpssDxChPacketAnalyzerActionGroupAdd
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          groupId,
    IN  GT_U32                                          actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR                    paActDbPtr = NULL;
    GT_U32                                                   i;
    GT_BOOL                                                  found;
    GT_U8                                                    devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);

    /* update action in DB */
    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];
    found = GT_FALSE;
    for (i=1; i<=paActDbPtr->numOfGrps; i++)
    {
        if (*paActDbPtr->paGrpPtrArr[i] == groupId)
        {
            found = GT_TRUE;
            break;
        }
    }
    if (found == GT_FALSE)
    {
        paActDbPtr->paGrpPtrArr[paActDbPtr->numOfGrps+1] = (GT_U32*)cpssOsMalloc(sizeof(GT_U32));
        if (paActDbPtr->paGrpPtrArr[paActDbPtr->numOfGrps+1] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to add groupId [%d] to actionId [%d]",groupId,actionId);
        }
        *paActDbPtr->paGrpPtrArr[paActDbPtr->numOfGrps+1] = groupId;
        paActDbPtr->numOfGrps++;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerActionGroupDelete function
* @endinternal
*
* @brief   Delete packet analyzer group from action DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChPacketAnalyzerActionGroupDelete
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      groupId,
    IN  GT_U32                                      actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       paActDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i,j,counter;
    GT_U32                                      *tempPaGrpPtrArr;
    GT_U8                                       devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);

    /* check that only single rule in groupId is bounded to old actionId; then delete this action from DB */
    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
    counter = 0;
    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        if (paGrpDbPtr->paRulePtrArr[i]->paActId == actionId)
            counter++;
    }

    if (counter == 1)
    {
        /* delete actionId from DB */
        paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];
        for (i=1; i<=paActDbPtr->numOfGrps; i++)
        {
            if (*paActDbPtr->paGrpPtrArr[i] == groupId)
            {
                tempPaGrpPtrArr = paActDbPtr->paGrpPtrArr[i];
                /* copy all the remaining groups */
                for (j=i; j<paActDbPtr->numOfGrps; j++)
                {
                    paActDbPtr->paGrpPtrArr[j] = paActDbPtr->paGrpPtrArr[j+1];
                }
                cpssOsFree(tempPaGrpPtrArr);
                paActDbPtr->paGrpPtrArr[paActDbPtr->numOfGrps] = 0;
                paActDbPtr->numOfGrps--;
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerCreate
*           function
* @endinternal
*
* @brief   Create Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_BAD_PTR               - on NULL pointer value.
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerCreate
(
    IN  GT_U32                                              managerId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr = NULL;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);

    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) != NULL)
    {
        /* can't redefine an packet analyzer manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: managerId [%d] already exist",managerId);
    }

    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) = (PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC));
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate managerId [%d]",managerId);
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    cpssOsMemSet(paMngDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC));
    paMngDbPtr->paMngId = managerId;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerManagerCreate function
* @endinternal
*
* @brief   Create Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_BAD_PTR               - on NULL pointer value.
*/
GT_STATUS cpssDxChPacketAnalyzerManagerCreate
(
    IN  GT_U32                                              managerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId));

    rc = internal_cpssDxChPacketAnalyzerManagerCreate(managerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerDelete
*           function
* @endinternal
*
* @brief   Delete Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerDelete
(
    IN  GT_U32                                  managerId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR      paMngDbPtr = NULL;
    GT_U32                                     i;
    GT_STATUS                                  rc;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    if (paMngDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, i))
        {
            rc = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,(GT_U8)i);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* release memory*/
    cpssOsMemSet(paMngDbPtr,0,sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC));
    cpssOsFree(paMngDbPtr);
    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) = 0;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerManagerDelete function
* @endinternal
*
* @brief   Delete Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDelete
(
    IN  GT_U32                                  managerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId));

    rc = internal_cpssDxChPacketAnalyzerManagerDelete(managerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerDeviceAdd
*           function
* @endinternal
*
* @brief   Add device to Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_FULL                  - if array is full
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerDeviceAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    GT_U32                                      stageId,i;
    GT_STATUS                                   rc1,rc = GT_OK;
    GT_BOOL                                     samplingDisable = GT_FALSE;
    GT_BOOL                                     packetMarkNeeded = GT_FALSE;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC *boundedInterfacesArray = NULL;
    GT_U32                                      boundedInterfacesArraySize=0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    if (paMngDbPtr->numOfDevices >= PRV_CPSS_MAX_PP_DEVICES_CNS) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "Error: numOfDevices [%d] for managerId [%d] is full",paMngDbPtr->numOfDevices,managerId);
    }

    if (paMngDbPtr->numOfDevices)
    {
        /* there is at least one device in the packet analyzer manager prior to calling this function */

        /* Check device family */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != paMngDbPtr->devFamily)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Error: All manager's devices should be from the same family [%d]",paMngDbPtr->devFamily);
        }

         /* check that the device wasn't added already */
        if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, devNum))
        {
            /* the device exists */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: device already exist",devNum);
        }
    }
    else
    {
        /* there are no devices in the packet analyzer manager prior to calling this function */

        /* update device family */
        paMngDbPtr->devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

        /* open XML file */
        rc = prvCpssDxChIdebugFileOpen(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* set power save mode on devNum bounded to managerId */
    rc = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,!(paMngDbPtr->devsPaEnable));
    if (rc != GT_OK)
    {
        return rc;
    }

    boundedInterfacesArray = (PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC)*
                                                                                              (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1));
    if (boundedInterfacesArray == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate boundedInterfacesArray");
    }

    rc = prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet(devNum,
            boundedInterfacesArray,&boundedInterfacesArraySize);
    if (rc != GT_OK)
    {
        if (boundedInterfacesArray != NULL)
            cpssOsFree(boundedInterfacesArray);
        return rc;
    }

    /* from sip6_10 devices sampling is disabled by default */
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        if (paMngDbPtr->devsPaEnable)
        {
            /* disable sampling on all used interfaces in devNum */
            rc = prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable(managerId,devNum);
            if (rc != GT_OK)
            {
                if (boundedInterfacesArray != NULL)
                    cpssOsFree(boundedInterfacesArray);
                return rc;
            }
        }
        else
        {
            /* need to disable sampling on all interfaces in devNum when DFX power is on */
            samplingDisable = GT_TRUE;
        }
    }
    else
    {
        packetMarkNeeded = GT_TRUE;
    }

    /* allocate device */
    paMngDbPtr->paDevPtrArr[paMngDbPtr->numOfDevices] = (PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC));
    if (paMngDbPtr->paDevPtrArr[paMngDbPtr->numOfDevices] == NULL)
    {
        if (boundedInterfacesArray != NULL)
            cpssOsFree(boundedInterfacesArray);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocated device [%d] in managerId [%d]",devNum,managerId);
    }
    paDevDbPtr = paMngDbPtr->paDevPtrArr[paMngDbPtr->numOfDevices];
    cpssOsMemSet(paDevDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC));
    paDevDbPtr->paDevId = devNum;

    /* Add device to manager's bitmap */
    PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, devNum);
    paMngDbPtr->numOfDevices++;

    /* bounded stages array allocation */
    paDevDbPtr->stagesArr = (PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1));
    if (paDevDbPtr->stagesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated device [%d] stages array in managerId [%d]",devNum,managerId);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(paDevDbPtr->stagesArr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1));

    paMngDbPtr->udfArr = (PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC)*PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_UDF_NUM_CNS);
    paMngDbPtr->udsArr = (PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC)*CPSS_DXCH_PACKET_ANALYZER_MAX_UDS_NUM_CNS);
    if (paMngDbPtr->udfArr == NULL || paMngDbPtr->udsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated device [%d] uds/udf arrays in managerId [%d]",devNum,managerId);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(paMngDbPtr->udfArr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC)*PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_UDF_NUM_CNS);
    cpssOsMemSet(paMngDbPtr->udsArr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC)*CPSS_DXCH_PACKET_ANALYZER_MAX_UDS_NUM_CNS);

    for (i=0; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        paDevDbPtr->stagesArr[i].stageName = i;
    }

    /* update if need to disable all interfaces later */
    paDevDbPtr->samplingDisable = samplingDisable;
    /* update if need to set packet marking later */
    paDevDbPtr->packetMarkNeeded = packetMarkNeeded;

    for (i=0; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E; i++)
    {
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,i) == GT_FALSE)
                continue;

        stageId = i;

        /* update bounded interface in PA DB */
        rc = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundedInterfacesArray[stageId]);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    if (paMngDbPtr->devsPaEnable)
    {
        rc = prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet(managerId,devNum,boundedInterfacesArray,
                boundedInterfacesArraySize);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_TRUE)
        {
            /* Enable Descriptor Setting */
            rc = prvCpssDxChIdebugPacketMarkingSet(devNum);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
    }

    if (boundedInterfacesArray != NULL)
            cpssOsFree(boundedInterfacesArray);

    return GT_OK;

exit_cleanly_lbl:

    if (boundedInterfacesArray != NULL)
            cpssOsFree(boundedInterfacesArray);

    rc1 = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
    if (rc1 != GT_OK)
    {
        return rc1;
    }
    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerManagerDeviceAdd function
* @endinternal
*
* @brief   Add device to Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_FULL                  - if array is full
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerDeviceAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, devNum));

    rc = internal_cpssDxChPacketAnalyzerManagerDeviceAdd(managerId, devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, devNum));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerDeviceRemove
*           function
* @endinternal
*
* @brief   Remove device from Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerDeviceRemove
(

    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i,j,k,keyId,groupId;
    GT_STATUS                                   rc;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that the device was added to bitmap */
    if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, devNum))
    {
        if (paMngDbPtr->devsPaEnable == GT_TRUE)
        {
            /* disable sampling on all used interfaces in devNum */
            rc = prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable(managerId,devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* delete all keys bounded to this device */
        for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS; i++)
        {
            keyId = i;

            /* check that the key was added to bitmap */
            if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_KEY_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId))
            {
                /* get pointer to key in packet analyzer manager DB */
                rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* Need to delete all bounded rules to this key */
                for (k=1; k<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; k++)
                {
                    groupId = k;
                    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
                    if (paGrpDbPtr == NULL)
                        continue;

                    for (j=1; j<=paGrpDbPtr->numOfRules; j++)
                    {
                        /* delete this regular rule; it is bounded to keyId */
                        if (paGrpDbPtr->paRulePtrArr[j]->paKeyId == keyId)
                        {
                            rc = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,paGrpDbPtr->paRulePtrArr[j]->paRuleId ,groupId);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }
                    }
                }

                /* delete the key */
                rc = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        if (paDevDbPtr->stagesArr != NULL)
        {
            cpssOsFree(paDevDbPtr->stagesArr);
        }
        if (paMngDbPtr->udfArr != NULL)
            cpssOsFree(paMngDbPtr->udfArr);
        if (paMngDbPtr->udsArr != NULL)
            cpssOsFree(paMngDbPtr->udsArr);
        cpssOsFree(paDevDbPtr);
        paMngDbPtr->paDevPtrArr[devNum] = 0;

        /* Remove device from bitmap */
        PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_CLEAR_MAC(paMngDbPtr->devsBitmap, devNum);

        if(paMngDbPtr->numOfDevices == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: didn't finish to delete devNum [%d] from manageId [%d]",devNum);
        }

        /* update devices counter */
        paMngDbPtr->numOfDevices--;
    }

    /* set power save mode on devNum removed from managerId */
    rc = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paMngDbPtr->numOfDevices == 0)
    {
        /* close XML file */
        rc = prvCpssDxChIdebugFileClose(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        paMngDbPtr->devFamily = 0;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerManagerDeviceRemove function
* @endinternal
*
* @brief   Remove device from Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceRemove
(

    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerDeviceRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, devNum));

    rc = internal_cpssDxChPacketAnalyzerManagerDeviceRemove(managerId, devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, devNum));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerEnableSet
*           function
* @endinternal
*
* @brief   Enable/Disable the packet analyzer on all devices
*          added to manager id.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] enable                   - enable/disable PA on
*                                       manager id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_BOOL                                 enable
)
{

    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    GT_U32                                      i;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC *boundedInterfacesArray = NULL;
    GT_U32                                      boundedInterfacesArraySize=0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* enable all the devices bounded to managerId */
    paMngDbPtr->devsPaEnable = enable;

    if (paMngDbPtr->numOfDevices == 0)
        return GT_OK;

    /* set power save mode on all the devices bounded to managerId */
    for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, i))
        {
            rc = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet((GT_U8)i,!enable);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* disable sampling on all interfaces in devices bounded to managerId that their sampling are still enabled due to power save mode */
    if (paMngDbPtr->devsPaEnable == GT_TRUE)
    {
        for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, i))
            {
                /* get pointer to device in packet analyzer manager DB */
                rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,(GT_U8)i,&paDevDbPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if ((paDevDbPtr->samplingDisable == GT_TRUE) || (paDevDbPtr->packetMarkNeeded == GT_TRUE) )
                {
                    if (paDevDbPtr->samplingDisable == GT_TRUE)
                    {
                        /* disable sampling on all used interfaces in devNum */
                        rc = prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable(managerId,(GT_U8)i);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                    boundedInterfacesArray = (PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC)*
                                                                                                              (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1));
                    if (boundedInterfacesArray == NULL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate boundedInterfacesArray");
                    }

                    rc = prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet((GT_U8)i,
                            boundedInterfacesArray,&boundedInterfacesArraySize);
                    if (rc != GT_OK)
                    {
                        if (boundedInterfacesArray != NULL)
                            cpssOsFree(boundedInterfacesArray);
                        return rc;
                    }

                    rc = prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet(managerId,(GT_U8)i,boundedInterfacesArray,
                            boundedInterfacesArraySize);

                    if (rc != GT_OK)
                    {
                        if (boundedInterfacesArray != NULL)
                            cpssOsFree(boundedInterfacesArray);
                        return rc;
                    }

                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_TRUE)
                    {
                        /* Enable Descriptor Setting */
                        rc = prvCpssDxChIdebugPacketMarkingSet(devNum);
                        if (rc != GT_OK)
                        {
                            if (boundedInterfacesArray != NULL)
                                cpssOsFree(boundedInterfacesArray);
                            return rc;
                        }
                    }
                    paDevDbPtr->samplingDisable = GT_FALSE;
                    paDevDbPtr->packetMarkNeeded = GT_FALSE;
                }
            }
        }
    }
    if (boundedInterfacesArray != NULL)
        cpssOsFree(boundedInterfacesArray);
    return GT_OK;
}


/**
* @internal cpssDxChPacketAnalyzerManagerEnableSet function
* @endinternal
*
* @brief   Enable/Disable the packet analyzer on all devices
*          added to manager id.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] enable                   - enable/disable PA on
*                                       manager id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerManagerEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_BOOL                                 enable
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, enable));

    rc = internal_cpssDxChPacketAnalyzerManagerEnableSet(managerId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerManagerDevicesGet
*           function
* @endinternal
*
* @brief   Get packet analyzer manager information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[out] deviceStatePtr          - (pointer to) whether PA
*                                       is enabled for manager
*                                       id's devices
* @param[in,out] numOfDevicesPtr      - in: (pointer to)
*                                       allocated number of
*                                       devices for managerId
*                                       out: (pointer to) actual
*                                       number of devices for
*                                       managerId
* @param[out] devicesArr              - devices array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerDevicesGet
(
    IN    GT_U32                                            managerId,
    OUT   GT_BOOL                                           *deviceStatePtr,
    INOUT GT_U32                                            *numOfDevicesPtr,
    OUT   GT_U8                                             devicesArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr = NULL;
    GT_U32 i=0,j=0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(deviceStatePtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfDevicesPtr);
    CPSS_NULL_PTR_CHECK_MAC(devicesArr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    *deviceStatePtr = paMngDbPtr->devsPaEnable;

    if (*numOfDevicesPtr < paMngDbPtr->numOfDevices) {
        *numOfDevicesPtr = paMngDbPtr->numOfDevices;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: managerId [%d], has numDevices [%d]",managerId,paMngDbPtr->numOfDevices);
    }

    *numOfDevicesPtr = paMngDbPtr->numOfDevices;
    for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, i))
        {
            devicesArr[j] = (GT_U8)i;
            j++;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerManagerDevicesGet function
* @endinternal
*
* @brief   Get packet analyzer manager information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[out] deviceStatePtr          - (pointer to) whether PA
*                                       is enabled for manager
*                                       id's devices
* @param[in,out] numOfDevicesPtr      - in: (pointer to)
*                                       allocated number of
*                                       devices for managerId
*                                       out: (pointer to) actual
*                                       number of devices for
*                                       managerId
* @param[out] devicesArr              - devices array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDevicesGet
(
    IN    GT_U32                                            managerId,
    OUT   GT_BOOL                                           *deviceStatePtr,
    INOUT GT_U32                                            *numOfDevicesPtr,
    OUT   GT_U8                                             devicesArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerDevicesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, deviceStatePtr, numOfDevicesPtr, devicesArr));

    rc = internal_cpssDxChPacketAnalyzerManagerDevicesGet(managerId, deviceStatePtr, numOfDevicesPtr, devicesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, deviceStatePtr, numOfDevicesPtr, devicesArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerManagerResetToDefaults
*           function
* @endinternal
*
* @brief   Reset Packet Trace Manager configuration to default
*          settings.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerManagerResetToDefaults
(
    IN  GT_U32                                  managerId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       paActDbPtr = NULL;
    GT_U32                                      i;
    GT_STATUS                                   rc;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    if (paMngDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    for (i=1 ; i <= PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
    {
        paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[i];
        if (paGrpDbPtr!=NULL)
        {
            rc = cpssDxChPacketAnalyzerGroupDelete(managerId,paGrpDbPtr->paGrpId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (i=1 ; i <= PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_ACTIONS_NUM_CNS; i++)
    {
        paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[i];
        if (paActDbPtr!=NULL)
        {
            rc = cpssDxChPacketAnalyzerActionDelete(managerId,paActDbPtr->paActId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, i))
        {
            rc = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,(GT_U8)i);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    paMngDbPtr->devsPaEnable = GT_FALSE;
    paMngDbPtr->devFamily = 0;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerManagerResetToDefaults function
* @endinternal
*
* @brief   Reset Packet Trace Manager configuration to default
*          settings.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerResetToDefaults
(
    IN  GT_U32                                  managerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerManagerResetToDefaults);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId));

    rc = internal_cpssDxChPacketAnalyzerManagerResetToDefaults(managerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerFieldSizeGet function
* @endinternal
*
* @brief   Get packet analyzer field's size.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[out] lengthPtr               - (pointer to) fields
*                                       length in bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                      managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName,
    OUT GT_U32                                     *lengthPtr
)
{
    GT_CHAR                                     iDebugFieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;
    GT_U32                                      id;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if (fieldName >= CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: field not valid");
    }

    if ((fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
    {
        id = fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",fieldName);

        cpssOsStrCpy(iDebugFieldName,paMngDbPtr->udfArr[id].iDebugField);
    }
    else
    {
        cpssOsStrCpy(iDebugFieldName,packetAnalyzerFieldToIdebugFieldArr[fieldName]);
    }

    rc = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,iDebugFieldName,lengthPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal cpssDxChPacketAnalyzerFieldSizeGet function
* @endinternal
*
* @brief   Get packet analyzer field's size.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[out] lengthPtr               - (pointer to) fields
*                                       length in bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                      managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName,
    OUT GT_U32                                     *lengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerFieldSizeGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, fieldName, lengthPtr));

    rc = internal_cpssDxChPacketAnalyzerFieldSizeGet(managerId, fieldName, lengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, fieldName, lengthPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssDxChPacketAnalyzerLogicalKeyCreate
*           function
* @endinternal
*
* @brief   Create logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] keyAttrPtr               - (pointer to) key attributes
* @param[in] numOfStages              - number of stages
* @param[in] stagesArr                - (pointer to) applicable
*                                       stages list array
* @param[in] fieldMode                - field mode assignment
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_NOT_FOUND             - not found manager
* @retval GT_FULL                  - if array is full
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyCreate
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    IN  GT_U32                                              numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stagesArr[],
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT fieldMode,
    IN  GT_U32                                              numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldsArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                    paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyDbPtr = NULL;
    GT_U32                                                   i,j,k;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              stageId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *stagesArrGet = NULL;
    GT_U32                                                   numOfStagesArrGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      fieldName;
    GT_BOOL                                                  stageFound;
    GT_STATUS                                                rc = GT_OK;
    GT_U8                                                    devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(keyAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(stagesArr);
    if (numOfFields != 0)
        CPSS_NULL_PTR_CHECK_MAC(fieldsArr);
    PRV_CPSS_PACKET_ANALYZER_FIELD_MODE_CHECK_MAC(fieldMode);

    if (numOfStages > CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of stages is [%d]",CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    }

    if (numOfFields > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of fields is [%d]",CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    /* if stageArr == ALL_STAGES numOfStages must be 1 */
    for (i=0; i<numOfStages; i++)
    {
        if  (((stagesArr[i] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E) ||
             (stagesArr[i] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E) ||
             (stagesArr[i] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E)) &&
             (numOfStages != 1)
            )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: for ALL_STAGES num of stages must be 1");
        }

        if (((stagesArr[i] != CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E) &&
             (stagesArr[i] != CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E) &&
             (stagesArr[i] != CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E))
            )
        {
            PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stagesArr[i]);
        }
    }

    if (stagesArr[0] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E)
    {
        numOfStages = 0;
        /* add all ingress stages to array */
        for (stageId=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_FIRST_E; stageId<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_LAST_E; stageId++)
        {
            if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,stageId) == GT_FALSE)
                continue;

            stagesArr[numOfStages] = stageId;
            numOfStages++;
        }
    }

    if (stagesArr[0] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E)
    {
        numOfStages = 0;
        /* add all egress stages to array */
        for (stageId=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E; stageId<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_LAST_E; stageId++)
        {
            if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,stageId) == GT_FALSE)
                continue;

            stagesArr[numOfStages] = stageId;
            numOfStages++;
        }
    }

    if (stagesArr[0] == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E)
    {
        numOfStages = 0;
        /* add all ingress & egress stages to array */
        for (stageId=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_FIRST_E; stageId<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; stageId++)
        {
            if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,stageId) == GT_FALSE)
                continue;

            stagesArr[numOfStages] = stageId;
            numOfStages++;
        }
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if (paMngDbPtr->numOfKeys >= PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "Error: managerId [%d] has max logical keys [%d]",managerId,paMngDbPtr->numOfKeys);
    }

    /* check that the key wasn't allocated already */
    if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_KEY_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId))
    {
        /* the key exists */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: keyId [%d] already exists in managerId [%d]",keyId,managerId);
    }

    /* allocate key */
    paMngDbPtr->paKeyPtrArr[keyId] = (PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_STC));
    if (paMngDbPtr->paKeyPtrArr[keyId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate keyId [%d] for managerId [%d]",keyId,managerId);
    }

    paKeyDbPtr = paMngDbPtr->paKeyPtrArr[keyId];
    cpssOsMemSet(paKeyDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_STC));

    paKeyDbPtr->stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E + 1));
    if (paKeyDbPtr->stagesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocate stages array for keyId [%d] in managerId [%d]",keyId,managerId);
        goto exit_cleanly_lbl;

    }
    cpssOsMemSet(paKeyDbPtr->stagesArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E + 1));

    paKeyDbPtr->fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (paKeyDbPtr->fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocate stages array for keyId [%d] in managerId [%d]",keyId,managerId);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(paKeyDbPtr->fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    paKeyDbPtr->perStagesFieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT**)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E + 1));
    if (paKeyDbPtr->perStagesFieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocate per stage fields array for keyId [%d] in managerId [%d]",keyId,managerId);
        goto exit_cleanly_lbl;
    }
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        paKeyDbPtr->perStagesFieldsArr[i] = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        if (paKeyDbPtr->perStagesFieldsArr[i] == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to allocate per stage fields array for keyId [%d] in managerId [%d]",keyId,managerId);
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(paKeyDbPtr->perStagesFieldsArr[i],0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    paKeyDbPtr->perStageNumOfFields = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1));
    if (paKeyDbPtr->perStageNumOfFields == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocate per stage fields array for keyId [%d] in managerId [%d]",keyId,managerId);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(paKeyDbPtr->perStageNumOfFields,0,sizeof(GT_U32)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1));

    /* array allocation */
    stagesArrGet = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (stagesArrGet == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    if (fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E)
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];
            numOfStagesArrGet = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
            rc = cpssDxChPacketAnalyzerFieldStagesGet(managerId,fieldName,&numOfStagesArrGet,stagesArrGet);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
            /* check if field's stagesArrGet contains ALL stages in input stagesArr */
            for (j=0; j<numOfStages; j++)
            {
                stageId = stagesArr[j];
                stageFound = GT_FALSE;

                for (k=0; k<numOfStagesArrGet; k++)
                {
                    if (stagesArrGet[k] == stageId)
                    {
                        stageFound = GT_TRUE;
                        break;
                    }
                }

                if (stageFound == GT_FALSE)
                {
                    rc = GT_FAIL;
                    goto exit_cleanly_lbl;
                }
            }
        }

        /* add logical key to DB */
        paKeyDbPtr->paKeyId = keyId;
        paKeyDbPtr->fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E;
        cpssOsMemCpy(&(paKeyDbPtr->paKeyAttr),keyAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        paKeyDbPtr->numOfStages = numOfStages;
        cpssOsMemCpy(paKeyDbPtr->stagesArr,stagesArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numOfStages);
        paKeyDbPtr->numOfFields = numOfFields;
        cpssOsMemCpy(paKeyDbPtr->fieldsArr,fieldsArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numOfFields);
        PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId);
        paMngDbPtr->numOfKeys++;
    }
    else /* fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E */
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];
            numOfStagesArrGet = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
            rc = cpssDxChPacketAnalyzerFieldStagesGet(managerId,fieldName,&numOfStagesArrGet,stagesArrGet);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }

            /* check which stages from input stagesArr exist in field's stagesArrGet */
            for (j=0; j<numOfStages; j++)
            {
                stageId = stagesArr[j];

                for (k=0; k<numOfStagesArrGet; k++)
                {
                    if (stagesArrGet[k] == stageId)
                    {
                        /* add logical key to DB */
                        paKeyDbPtr->perStagesFieldsArr[stageId][(paKeyDbPtr->perStageNumOfFields[stageId])] = fieldName;
                        paKeyDbPtr->perStageNumOfFields[stageId]++;
                        break;
                    }
                }
            }
        }

        /* add logical key to DB */
        paKeyDbPtr->paKeyId = keyId;
        paKeyDbPtr->fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        cpssOsMemCpy(&(paKeyDbPtr->paKeyAttr),keyAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        paKeyDbPtr->numOfStages = numOfStages;
        cpssOsMemCpy(paKeyDbPtr->stagesArr,stagesArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numOfStages);
        paKeyDbPtr->numOfFields = numOfFields;
        cpssOsMemCpy(paKeyDbPtr->fieldsArr,fieldsArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numOfFields);
        PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId);
        paMngDbPtr->numOfKeys++;
    }

    cpssOsFree(stagesArrGet);
    return GT_OK;

exit_cleanly_lbl:

    /* delete key allocation */
    if (paKeyDbPtr->perStageNumOfFields != NULL)
        cpssOsFree(paKeyDbPtr->perStageNumOfFields);
    if (paKeyDbPtr->perStagesFieldsArr != NULL)
    {
        for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
        {
            if (paKeyDbPtr->perStagesFieldsArr[i] != NULL)
            {
                cpssOsFree(paKeyDbPtr->perStagesFieldsArr[i]);
            }
        }
        cpssOsFree(paKeyDbPtr->perStagesFieldsArr);
    }
    if (paKeyDbPtr->fieldsArr != NULL)
        cpssOsFree(paKeyDbPtr->fieldsArr);
    if (paKeyDbPtr->stagesArr != NULL)
        cpssOsFree(paKeyDbPtr->stagesArr);

    cpssOsFree(paKeyDbPtr);
    paMngDbPtr->paKeyPtrArr[keyId] = 0;
    cpssOsFree(stagesArrGet);

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyCreate function
* @endinternal
*
* @brief   Create logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] keyAttrPtr               - (pointer to) key attributes
* @param[in] numOfStages              - number of stages
* @param[in] stagesArr                - (pointer to) applicable
*                                       stages list array
* @param[in] fieldMode                - field mode assignment
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_NOT_FOUND             - not found manager
* @retval GT_FULL                  - if array is full
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyCreate
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    IN  GT_U32                                              numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stagesArr[], /*arrSizeVarName=numOfStages*/
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT fieldMode,
    IN  GT_U32                                              numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldsArr[] /*arrSizeVarName=numOfFields*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, keyAttrPtr, numOfStages, stagesArr, fieldMode, numOfFields, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId, keyAttrPtr, numOfStages, stagesArr, fieldMode, numOfFields, fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, keyAttrPtr, numOfStages, stagesArr, fieldMode, numOfFields, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerLogicalKeyFieldsAdd
*           function
* @endinternal
*
* @brief   Add fields to existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyFieldsAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR               paKeyDbPtr = NULL;
    GT_U32                                              i,j,k,id=0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         *stagesArr = NULL;
    GT_U32                                              numOfStages,numOfAddedFields=0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldName;
    GT_BOOL                                             stageFound;
    GT_STATUS                                           rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 *addedFieldsArr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr = NULL;
    GT_BOOL                                             fieldExists = GT_FALSE;
    GT_U32                                              numOfInterfaces;
    GT_U8                                               devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if (numOfFields > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of fields is [%d]",CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChIdebugDbNumInterfacesGet(devNum,&numOfInterfaces);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsArr[i];
        if ((fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
        {
            id = fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);
            if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",fieldName);
        }
    }

    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    addedFieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if(stagesArr == NULL || addedFieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(stagesArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    cpssOsMemSet(addedFieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    if (paKeyDbPtr->fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E)
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];

            /* check if field alrady exists in logical key DB */
            fieldExists = GT_FALSE;
            for (j=0; j<paKeyDbPtr->numOfFields; j++) {
                if (paKeyDbPtr->fieldsArr[j] == fieldName)
                {
                    fieldExists = GT_TRUE;
                    break;
                }
            }

            if (fieldExists)
                continue;

            numOfStages = numOfInterfaces;
            rc = cpssDxChPacketAnalyzerFieldStagesGet(managerId,fieldName,&numOfStages,stagesArr);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
            /* check if field's stagesArr contains ALL stages in stagesArr of logical key DB */
            for (j=0; j<paKeyDbPtr->numOfStages; j++)
            {
                stageId = paKeyDbPtr->stagesArr[j];
                stageFound = GT_FALSE;
                for (k=0; k<numOfStages; k++)
                {
                    if (stagesArr[k] == stageId)
                    {
                        stageFound = GT_TRUE;
                        break;
                    }
                }
                if (stageFound == GT_FALSE)
                {
                    rc = GT_FAIL;
                    if (fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_FIRST_E )
                    {
                        CPSS_LOG_ERROR_MAC("Error: user defined field [%d] doesn't exist all stages in paKeyDbPtr->stagesArr",fieldName);
                    }
                    else
                    {
                        CPSS_LOG_ERROR_MAC("Error: field [%s] doesn't exist in all stages in paKeyDbPtr->stagesArr", paFieldStrArr[fieldName]);
                    }
                    goto exit_cleanly_lbl;
                }
            }
            /* add field to array */
            addedFieldsArr[numOfAddedFields] = fieldName;
            numOfAddedFields++;
        }

        /* update logical key in DB */
        cpssOsMemCpy(&(paKeyDbPtr->fieldsArr[paKeyDbPtr->numOfFields]),addedFieldsArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numOfAddedFields);
        paKeyDbPtr->numOfFields += numOfAddedFields;

    }
    else /* fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E */
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];

            /* check if field alrady exists in logical key DB */
            fieldExists = GT_FALSE;
            for (j=0; j<paKeyDbPtr->numOfFields; j++) {
                if (paKeyDbPtr->fieldsArr[j] == fieldName)
                {
                    fieldExists = GT_TRUE;
                    break;
                }
            }

            if (fieldExists)
                continue;

            numOfStages = numOfInterfaces;
            rc = cpssDxChPacketAnalyzerFieldStagesGet(managerId,fieldName,&numOfStages,stagesArr);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
            /* check which stages from stagesArr in DB exist in field's stagesArr */
            stageFound = GT_FALSE;
            for (j=0; j<paKeyDbPtr->numOfStages; j++)
            {
                stageId = paKeyDbPtr->stagesArr[j];
                for (k=0; k<numOfStages; k++)
                {
                    if (stagesArr[k] == stageId)
                    {
                        /* add logical key to DB */
                        paKeyDbPtr->perStagesFieldsArr[stageId][(paKeyDbPtr->perStageNumOfFields[stageId])] = fieldName;
                        paKeyDbPtr->perStageNumOfFields[stageId]++;
                        stageFound = GT_TRUE;
                        break;
                    }
                }
            }
            if (stageFound == GT_TRUE)
            {
                /* add field to array */
                addedFieldsArr[numOfAddedFields] = fieldName;
                numOfAddedFields++;
            }
        }

        /* need to check if the key is regular and with the new fields it becames extended return error*/

        /* update logical key in DB */
        cpssOsMemCpy(&(paKeyDbPtr->fieldsArr[paKeyDbPtr->numOfFields]),addedFieldsArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numOfAddedFields);
        paKeyDbPtr->numOfFields += numOfAddedFields;
    }

exit_cleanly_lbl:

    cpssOsFree(stagesArr);
    cpssOsFree(addedFieldsArr);

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsAdd function
* @endinternal
*
* @brief   Add fields to existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[] /*arrSizeVarName=numOfFields*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyFieldsAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, numOfFields, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId, keyId, numOfFields, fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, numOfFields, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerLogicalKeyFieldsRemove
*           function
* @endinternal
*
* @brief   Remove fields from existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyFieldsRemove
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR           paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR           paGrpDbPtr = NULL;
    GT_U32                                          i,j,k,m;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             fieldName;
    GT_BOOL                                         fieldFound;
    GT_U32                                          groupId;
    GT_STATUS                                       rc;
    GT_U8                                           devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    if (numOfFields > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of fields is [%d]",CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    /* check that key id isn't bounded to any packet analyzer rule */
    for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
    {
        groupId = i;
        paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
        if (paGrpDbPtr == NULL)
            continue;

        for (j=1; j<=paGrpDbPtr->numOfRules; j++)
        {
            if (paGrpDbPtr->paRulePtrArr[j]->paKeyId == keyId)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: can't remove fields from keyId [%d]; keyId is bounded to ruleId [%d] in groupId [%d]",
                                              keyId,paGrpDbPtr->paRulePtrArr[j]->paRuleId,groupId);
        }
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paKeyDbPtr->fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E)
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];

            /* check if field exists in logical key DB */
            for (j=0; j<paKeyDbPtr->numOfFields; j++) {
                if (paKeyDbPtr->fieldsArr[j] == fieldName)
                {
                    /* delete the field and copy all the remaining array */
                    paKeyDbPtr->fieldsArr[j] = 0; /* if this is the last field */
                    for (k=j; k<=paKeyDbPtr->numOfFields-1; k++)
                    {
                        /* update logical key in DB */
                        paKeyDbPtr->fieldsArr[k] = paKeyDbPtr->fieldsArr[k+1];
                    }
                    paKeyDbPtr->numOfFields--;
                    break;
                }
            }
        }
    }
    else /* fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E */
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsArr[i];

            fieldFound = GT_FALSE;
            /* check if field exists in logical key DB */
            for (j=0; j<paKeyDbPtr->numOfFields; j++)
            {
                if (paKeyDbPtr->fieldsArr[j] == fieldName)
                {
                    fieldFound = GT_TRUE;

                    /* delete the field and copy all the remaining array */
                    paKeyDbPtr->fieldsArr[j] = 0; /* if this is the last field */
                    for (k=j; k<=paKeyDbPtr->numOfFields-1; k++)
                    {
                        /* update logical key in DB */
                        paKeyDbPtr->fieldsArr[k] = paKeyDbPtr->fieldsArr[k+1];
                    }
                    paKeyDbPtr->numOfFields--;
                    break;
                }
            }

            if (fieldFound == GT_TRUE)
            {
                for (j=0; j<paKeyDbPtr->numOfStages; j++)
                {
                    stageId = paKeyDbPtr->stagesArr[j];
                    for (k=0; k<paKeyDbPtr->perStageNumOfFields[stageId]; k++)
                    {
                        if (paKeyDbPtr->perStagesFieldsArr[stageId][k] == fieldName)
                        {
                            /* delete the field and copy all the remaining array */
                            paKeyDbPtr->perStagesFieldsArr[stageId][k] = 0; /* if this is the last field */
                            for (m=k; m<=paKeyDbPtr->perStageNumOfFields[stageId]-1; m++)
                            {
                                /* update logical key in DB */
                                paKeyDbPtr->perStagesFieldsArr[stageId][m] = paKeyDbPtr->perStagesFieldsArr[stageId][m+1];
                            }
                            paKeyDbPtr->perStageNumOfFields[stageId]--;
                            break;
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsRemove function
* @endinternal
*
* @brief   Remove fields from existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsRemove
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[] /*arrSizeVarName=numOfFields*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyFieldsRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, numOfFields, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId, keyId, numOfFields, fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, numOfFields, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerLogicalKeyDelete
*           function
* @endinternal
*
* @brief   Delete existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i,j;
    GT_U32                                      groupId;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);

    /* check that key id isn't bounded to any packet analyzer rule */
    for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
    {
        groupId = i;
        paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
        if (paGrpDbPtr == NULL)
            continue;

        for (j=1; j<=paGrpDbPtr->numOfRules; j++)
        {
            if (paGrpDbPtr->paRulePtrArr[j]->paKeyId == keyId)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: can't delete keyId [%d], it is bounded to ruleId [%d]",keyId,paGrpDbPtr->paRulePtrArr[j]->paRuleId);
        }
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paKeyDbPtr->paKeyId != keyId)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: wrong state keyPtrId [%d] != keyId [%d]",paKeyDbPtr->paKeyId,keyId);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* delete keyId */
    PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_CLEAR_MAC(paMngDbPtr->keysBitmap, keyId);
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
        cpssOsFree(paKeyDbPtr->perStagesFieldsArr[i]);
    cpssOsFree(paKeyDbPtr->perStageNumOfFields);
    cpssOsFree(paKeyDbPtr->perStagesFieldsArr);
    cpssOsFree(paKeyDbPtr->fieldsArr);
    cpssOsFree(paKeyDbPtr->stagesArr);
    cpssOsFree(paKeyDbPtr);
    paMngDbPtr->paKeyPtrArr[keyId] = 0;
    paMngDbPtr->numOfKeys--;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyDelete function
* @endinternal
*
* @brief   Delete existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyDelete(managerId, keyId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerLogicalKeyInfoGet
*           function
* @endinternal
*
* @brief   Get existing logical key configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyAttrPtr              - (pointer to) key
*                                       attributes
* @param[out] fieldModePtr            - (pointer to) field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyInfoGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT *fieldModePtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(keyAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldModePtr);

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paKeyDbPtr->paKeyId != keyId)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: wrong state keyPtrId [%d] != keyId [%d]",paKeyDbPtr->paKeyId,keyId);

    cpssOsMemCpy(keyAttrPtr,&(paKeyDbPtr->paKeyAttr),sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
    *fieldModePtr = paKeyDbPtr->fieldMode;

    return GT_OK;

}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyInfoGet function
* @endinternal
*
* @brief   Get existing logical key configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyAttrPtr              - (pointer to) key
*                                       attributes
* @param[out] fieldModePtr            - (pointer to) field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyInfoGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT *fieldModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyInfoGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, keyAttrPtr, fieldModePtr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId, keyId, keyAttrPtr, fieldModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, keyAttrPtr, fieldModePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerLogicalKeyStagesGet
*           function
* @endinternal
*
* @brief   Get existing logical key stages list.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - out: (pointer to) stages
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyStagesGet
(
    IN  GT_U32                                              managerId,
    IN    GT_U32                                            keyId,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[] /*arrSizeVarName=numOfStagesPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(numOfStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(stagesArr);

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paKeyDbPtr->paKeyId != keyId)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: wrong state keyPtrId [%d] != keyId [%d]",paKeyDbPtr->paKeyId,keyId);

    if (paKeyDbPtr->numOfStages > *numOfStagesPtr){
        *numOfStagesPtr = paKeyDbPtr->numOfStages;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: for keyId [%d] num of stages [%d]",keyId,paKeyDbPtr->numOfStages);
    }

    *numOfStagesPtr = paKeyDbPtr->numOfStages;
    cpssOsMemCpy(stagesArr,paKeyDbPtr->stagesArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*paKeyDbPtr->numOfStages);

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyStagesGet function
* @endinternal
*
* @brief   Get existing logical key stages list.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - out: (pointer to) stages
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyStagesGet
(
    IN  GT_U32                                              managerId,
    IN    GT_U32                                            keyId,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[] /*arrSizeVarName=numOfStagesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyStagesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, numOfStagesPtr, stagesArr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId, keyId, numOfStagesPtr, stagesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, numOfStagesPtr, stagesArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*           function
* @endinternal
*
* @brief   Get existing logical key fields list per stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
* @param[in] stagesId                 - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) field
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong state
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
     PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paKeyDbPtr->paKeyId != keyId)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: wrong state keyPtrId [%d] != keyId [%d]",paKeyDbPtr->paKeyId,keyId);

    if (paKeyDbPtr->fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E)
    {
        if (paKeyDbPtr->numOfFields > *numOfFieldsPtr){
            *numOfFieldsPtr = paKeyDbPtr->numOfFields;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: for keyId [%d] num of fields [%d]",keyId,paKeyDbPtr->numOfFields);
        }

        *numOfFieldsPtr = paKeyDbPtr->numOfFields;
        cpssOsMemCpy(fieldsArr,paKeyDbPtr->fieldsArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*paKeyDbPtr->numOfFields);
    }
    else /* paKeyDbPtr->fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E) */
    {
        if (paKeyDbPtr->perStageNumOfFields[stageId] > *numOfFieldsPtr){
            *numOfFieldsPtr = paKeyDbPtr->perStageNumOfFields[stageId];
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: for keyId [%d] stageId [%d] num of fields [%d]",keyId,stageId,paKeyDbPtr->perStageNumOfFields[stageId]);
        }

        *numOfFieldsPtr = paKeyDbPtr->perStageNumOfFields[stageId];
        cpssOsMemCpy(fieldsArr,paKeyDbPtr->perStagesFieldsArr[stageId],sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*paKeyDbPtr->perStageNumOfFields[stageId]);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet function
* @endinternal
*
* @brief   Get existing logical key fields list per stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
* @param[in] stagesId                 - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) field
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, stageId, numOfFieldsPtr, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId, keyId, stageId, numOfFieldsPtr, fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, stageId, numOfFieldsPtr, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerStagesGet function
* @endinternal
*
* @brief   Get applicable stages and Valid stages - stages that are not muxed for current configuration
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in,out] numOfApplicStagesPtr - in: (pointer to)
*                                       size of allocated applicStagesListArr
*                                       out: (pointer to) actual
*                                       number of applicable stages in applicStagesListArr
* @param[out] applicStagesListArr     - (pointer to) applicable stages list array
* @param[in,out] numOfValidStagesPtr  - in: (pointer to)
*                                       size of allocated ValidStagesListArr
*                                       out: (pointer to) actual
*                                       number of valid stages in ValidStagesListArr
* @param[out] ValidStagesListArr      - (pointer to) valid stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStagesGet
(
    IN    GT_U32                                         managerId,
    INOUT GT_U32                                         *numOfApplicStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    applicStagesListArr[], /*arrSizeVarName=numOfApplicStagesPtr*/
    INOUT GT_U32                                         *numOfValidStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    validStagesListArr[] /*arrSizeVarName=numOfValidStagesPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC        *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR           paDevDbPtr = NULL;
    GT_STATUS                                       rc = GT_TRUE;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId = PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;
    GT_CHAR                                         interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                          ii,applicCounter,validCounter,offset;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   boundInterfaceGet;
    GT_U8                                           devNum;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     tmpApplicStagesListArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     tmpValidStagesListArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1];

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfApplicStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(applicStagesListArr);
    CPSS_NULL_PTR_CHECK_MAC(numOfValidStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(validStagesListArr);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    applicCounter=0;
    validCounter=0;
    offset = 0;
    for (ii = 0; ii <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; ii++)
    {
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,ii) == GT_FALSE){
            offset++;
            continue;
        }
        rc = prvCpssDxChPacketAnalyzerStageOrderGet(managerId,devNum,ii,offset,&stageId);
        if(rc != GT_OK)
        {
            return rc;
        }
        /*if stageId is uds ,check that it is valid */
        if ( (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) &&
             (stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
        {
            rc = prvCpssDxChPacketAnalyzerUserDefinedValidCheck(managerId,stageId);
            if (rc != GT_OK)
                continue;
        }
        rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId, devNum, stageId, &boundInterfaceGet);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsStrCpy(interfaceId,boundInterfaceGet.info.interfaceId);

        tmpApplicStagesListArr[applicCounter] = stageId;
        applicCounter++;

        stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
        if (stageDbPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%s] of devNum [%d]",paStageStrArr[stageId],devNum);

        /* interface is bounded */
        if (GT_TRUE == stageDbPtr->isValid)
        {
            tmpValidStagesListArr[validCounter]=stageId;
            validCounter++;
        }
    }

    if (*numOfApplicStagesPtr < applicCounter){
        *numOfApplicStagesPtr = applicCounter;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: min number of applicable stages is [%d]", applicCounter);
    }

    if (*numOfValidStagesPtr < validCounter){
        *numOfValidStagesPtr  = validCounter;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: min number of valid stages is [%d]", validCounter);
    }
    cpssOsMemCpy(applicStagesListArr,tmpApplicStagesListArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*applicCounter);
    cpssOsMemCpy(validStagesListArr,tmpValidStagesListArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*validCounter);
    *numOfApplicStagesPtr = applicCounter;
    *numOfValidStagesPtr  = validCounter;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerStagesGet function
* @endinternal
*
* @brief   Get applicable stages and Valid stages - stages that are not muxed for current configuration
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in,out] numOfApplicStagesPtr - in: (pointer to)
*                                       size of allocated applicStagesListArr
*                                       out: (pointer to) actual
*                                       number of applicable stages in applicStagesListArr
* @param[out] applicStagesListArr     - (pointer to) applicable stages list array
* @param[in,out] numOfValidStagesPtr  - in: (pointer to)
*                                       size of allocated ValidStagesListArr
*                                       out: (pointer to) actual
*                                       number of valid stages in ValidStagesListArr
* @param[out] ValidStagesListArr      - (pointer to) valid stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerStagesGet
(
    IN    GT_U32                                         managerId,
    INOUT GT_U32                                         *numOfApplicStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    applicStagesListArr[], /*arrSizeVarName=numOfApplicStagesPtr*/
    INOUT GT_U32                                         *numOfValidStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    validStagesListArr[] /*arrSizeVarName=numOfValidStagesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStagesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId,numOfApplicStagesPtr, applicStagesListArr, numOfValidStagesPtr, validStagesListArr));

    rc = internal_cpssDxChPacketAnalyzerStagesGet(managerId,numOfApplicStagesPtr, applicStagesListArr, numOfValidStagesPtr, validStagesListArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, managerId, numOfApplicStagesPtr, applicStagesListArr, numOfValidStagesPtr, validStagesListArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerMuxStagesGet function
* @endinternal
*
* @brief   Get mux stages array for specific stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfMuxStagesPtr    - in: (pointer to)
*                                       allocated number of
*                                       muxed stages
*                                       out: (pointer to) actual
*                                       number of muxed
*                                       stages
* @param[out] muxStagesListArr        - (pointer to) muxed
*                                       stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerMuxStagesGet
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId,
    INOUT GT_U32                                         *numOfMuxStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    muxStagesListArr[] /*arrSizeVarName=numOfMuxStagesPtr*/
)
{
    GT_STATUS                                       rc = GT_TRUE;
    GT_U32                                          ii,jj,kk,counter = 0;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC boundInterfaceGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *tempMuxStagesListArr = NULL;
    GT_CHAR_PTR                                     *instancesNameArr=NULL;
    GT_CHAR_PTR                                     *tempInstancesNameArr=NULL;
    GT_U32                                          maxnumOfInstances,numOfInstances,tempNumOfInstances;
    GT_U8                                           devNum;
    GT_BOOL                                         found = GT_FALSE;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfMuxStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(muxStagesListArr);

    rc = prvCpssDxChIdebugDbNumInstancesGet(devNum,&maxnumOfInstances);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* array allocation */
    tempMuxStagesListArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    instancesNameArr = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS);
    tempInstancesNameArr = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS);
    if (tempMuxStagesListArr == NULL || instancesNameArr == NULL || tempInstancesNameArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated array for device [%d] in managerId [%d]",devNum,managerId);
        goto exit_cleanly_lbl;
    }

    for (ii=0; ii<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS; ii++)
    {
        instancesNameArr[ii] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
        tempInstancesNameArr[ii] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
        if (instancesNameArr[ii] == NULL || tempInstancesNameArr[ii] == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }
    }

    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum, stageId,&boundInterfaceGet);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    numOfInstances = maxnumOfInstances;
    rc = prvCpssDxChIdebugInterfaceInstancesGet(managerId,devNum,
                                                boundInterfaceGet.info.interfaceId,&numOfInstances,instancesNameArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (ii = PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; ii <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; ii++)
    {
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,ii) == GT_FALSE)
            continue;

        if (ii == (GT_U32)stageId)
            continue;

        /*if stageId is uds ,check that it is valid */
        if ( (ii >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) &&
             (ii <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
        {
            rc = prvCpssDxChPacketAnalyzerUserDefinedValidCheck(managerId,ii);
            if (rc != GT_OK)
            {
                continue;
            }
        }
        rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,ii,&boundInterfaceGet);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        tempNumOfInstances = maxnumOfInstances;
        rc = prvCpssDxChIdebugInterfaceInstancesGet(managerId,devNum,
                                                    boundInterfaceGet.info.interfaceId,&tempNumOfInstances,tempInstancesNameArr);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        found=GT_FALSE;
        for (jj=0;jj<numOfInstances;jj++)
        {
            if(found==GT_TRUE)
                break;
            for (kk=0; kk<tempNumOfInstances; kk++)
            {
                if (cpssOsStrCmp(instancesNameArr[jj],tempInstancesNameArr[kk]) == 0)
                {
                    tempMuxStagesListArr[counter]=ii;
                    counter++;
                    found = GT_TRUE;
                    break;
                }
            }
        }
    }

    if (*numOfMuxStagesPtr < counter)
    {
        *numOfMuxStagesPtr = counter;
        rc = GT_BAD_SIZE ;
        CPSS_LOG_ERROR_MAC("Error:rc [%d] num of muxed stages is [%d]",rc,counter);
        goto exit_cleanly_lbl;
    }

    *numOfMuxStagesPtr = counter;
    cpssOsMemCpy(muxStagesListArr,tempMuxStagesListArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*counter);
    rc = GT_OK;
exit_cleanly_lbl:

    if (tempMuxStagesListArr != NULL)
        cpssOsFree(tempMuxStagesListArr);

    if  (instancesNameArr != NULL)
    {
        for (ii=0; ii<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS; ii++)
        {
            if (instancesNameArr[ii] != NULL)
                cpssOsFree(instancesNameArr[ii]);
        }
        cpssOsFree(instancesNameArr);
    }
    if  (tempInstancesNameArr != NULL)
    {
        for (ii=0; ii<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS; ii++)
        {
            if  (tempInstancesNameArr[ii] != NULL)
                cpssOsFree(tempInstancesNameArr[ii]);
        }
        cpssOsFree(tempInstancesNameArr);
    }
    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerMuxStagesGet function
* @endinternal
*
* @brief   Get mux stages array for specific stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfMuxStagesPtr    - in: (pointer to)
*                                       allocated number of
*                                       muxed stages
*                                       out: (pointer to) actual
*                                       number of muxed
*                                       stages
* @param[out] muxStagesListArr        - (pointer to) muxed
*                                       stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStagesGet
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId,
    INOUT GT_U32                                         *numOfMuxStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    muxStagesListArr[] /*arrSizeVarName=numOfMuxStagesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerMuxStagesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId,stageId, numOfMuxStagesPtr, muxStagesListArr));

    rc = internal_cpssDxChPacketAnalyzerMuxStagesGet(managerId,stageId, numOfMuxStagesPtr, muxStagesListArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, managerId, stageId, numOfMuxStagesPtr, muxStagesListArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPacketAnalyzerGroupCreate function
* @endinternal
*
* @brief   Create packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupAttrPtr             - (pointer to) group
*                                       attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupCreate
(
    IN GT_U32                                               managerId,
    IN GT_U32                                               groupId,
    IN CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC       *groupAttrPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR                    paGrpDbPtr = NULL;
    GT_U8                                                    devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    CPSS_NULL_PTR_CHECK_MAC(groupAttrPtr);

    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] != NULL)
    {
        /* can't redefine an packet analyzer group DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: groupId [%d] already exists",groupId);
    }

    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] = (PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC));
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocated memory for groupId [%d]",groupId);
    }
    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
    cpssOsMemSet(paGrpDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC));

    paGrpDbPtr->paGrpId = groupId;
    cpssOsMemCpy(&(paGrpDbPtr->paGrpAttr),groupAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));

    paGrpDbPtr->stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*(PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1));
    if (paGrpDbPtr->stagesArr == NULL)
    {
        cpssOsFree(paGrpDbPtr);
        PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] = 0;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocated memory for stages array in groupId [%d]",groupId);
    }
    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerGroupCreate function
* @endinternal
*
* @brief   Create packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupAttrPtr             - (pointer to) group
*                                       attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerGroupCreate
(
    IN GT_U32                                               managerId,
    IN GT_U32                                               groupId,
    IN CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC       *groupAttrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, groupId, groupAttrPtr));

    rc = internal_cpssDxChPacketAnalyzerGroupCreate(managerId, groupId, groupAttrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, groupId, groupAttrPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerGroupDelete function
* @endinternal
*
* @brief   Delete packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupDelete
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   groupId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];

    /* delete all group's rule from DB */
    for (i=1; i<=paGrpDbPtr->numOfRules;)
    {
        /*paGrpDbPtr->numOfRegRules gets decrease inside function */
        rc = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,paGrpDbPtr->paRulePtrArr[i]->paRuleId,groupId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsFree(paGrpDbPtr->stagesArr);
    cpssOsFree(paGrpDbPtr);
    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId] = NULL;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerGroupDelete function
* @endinternal
*
* @brief   Delete packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group
*/
GT_STATUS cpssDxChPacketAnalyzerGroupDelete
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   groupId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, groupId));

    rc = internal_cpssDxChPacketAnalyzerGroupDelete(managerId, groupId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, groupId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerGroupRuleAdd function
* @endinternal
*
* @brief   Add packet analyzer rule to packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] ruleAttrPtr              - (pointer to) rule
*                                       attributes
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
* @retval GT_FULL                  - if array is full
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupRuleAdd
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          groupId,
    IN  GT_U32                                          ruleId,
    IN  CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   *ruleAttrPtr,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                          actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR                    paGrpDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR                   paRuleDbPtr = NULL;
    GT_U8                                                    devNum;
    GT_U32                                                   i,j;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              stageId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      fieldName;
    GT_STATUS                                                rc;
    GT_BOOL                                                  found;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    CPSS_NULL_PTR_CHECK_MAC(ruleAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);


    if (numOfFields > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of fields is [%d]",CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check allocation of rules */
    if (paGrpDbPtr->numOfRules >= PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "Error: max number of regular rules [%d]",PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS);
    }

    /* check that ruleId doesn't exist in groupId */
    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId]->paRulePtrArr[i]->paRuleId == ruleId)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: ruleId [%d] already exists in groupId [%d]",ruleId,groupId);
        }
    }

    /* check if ruleId can co-exist with other rules in groupId */
    /* 1. rule A and rule B can't share the same keyId */
    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        if (paGrpDbPtr->paRulePtrArr[i]->paKeyId == keyId)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Error: keyId [%d] already bounded to ruleId [%d]",keyId, paGrpDbPtr->paRulePtrArr[i]->paRuleId);
        }
    }

    /* 2. key a and key b can't share the same stageId */
    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        stageId = paKeyDbPtr->stagesArr[i];
        for (j=0; j<paGrpDbPtr->numOfStages; j++)
        {
            if (paGrpDbPtr->stagesArr[j] == stageId)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: stageId [%d] already exists in groupId [%d]",stageId, groupId);
            }
        }
    }

    /* 3. check that all the fields in fieldsValueArr[] are part of the keyId's fields */
    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsValueArr[i].fieldName;
        found = GT_FALSE;
        for (j=0; j<paKeyDbPtr->numOfFields; j++)
        {
            /* if field from rule exists in the key continue*/
            if (paKeyDbPtr->fieldsArr[j] == fieldName)
            {
                found = GT_TRUE;
                break;
            }
        }
        if (found == GT_FALSE)
        {
            if (fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Found param udf [%d] in ruleId [%d] groupId [%d] that doesn't exist in keyId [%d]",
                                              fieldName,ruleId,groupId,keyId);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Found param [%s] in ruleId [%d] groupId [%d] that doesn't exist in keyId [%d]",
                                              packetAnalyzerFieldToIdebugFieldArr[fieldName],ruleId,groupId,keyId);
            }
        }
    }

    /* 4. check that no overlapping fields in fieldsValueArr[] */
    rc = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* allocate rule */
    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId]->paRulePtrArr[paGrpDbPtr->numOfRules+1] =
                                    (PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC));
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId]->paRulePtrArr[paGrpDbPtr->numOfRules+1] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate regular keyId [%d]", keyId);
    }
    paRuleDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId]->paRulePtrArr[paGrpDbPtr->numOfRules+1];
    paGrpDbPtr->numOfRules++;

    cpssOsMemSet(paRuleDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC));

    if (numOfFields)
    {
        paRuleDbPtr->fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        if (paRuleDbPtr->fieldsValueArr == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to allocate fields value array for keyId [%d]", keyId);
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(paRuleDbPtr->fieldsValueArr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

        paRuleDbPtr->isFieldValueSetArr = (GT_BOOL*)cpssOsMalloc(sizeof(GT_BOOL)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        if (paRuleDbPtr->isFieldValueSetArr == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to allocate fields value array for keyId [%d]", keyId);
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(paRuleDbPtr->isFieldValueSetArr, 0, sizeof(GT_BOOL)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

        /* by diffault all the field's are ignored */
        for (i=0; i<paKeyDbPtr->numOfFields; i++)
        {
            fieldName = paKeyDbPtr->fieldsArr[i];
            paRuleDbPtr->fieldsValueArr[fieldName].fieldName = fieldName;
            cpssOsMemSet(paRuleDbPtr->fieldsValueArr[fieldName].msk, 0, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        }
    }

    /* update rule in DB */
    cpssOsMemCpy(&(paRuleDbPtr->paRuleAttr),ruleAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
    paRuleDbPtr->paRuleId = ruleId;
    paRuleDbPtr->paKeyId = keyId;
    paRuleDbPtr->paActId = actionId;
    paRuleDbPtr->numOfFields = numOfFields;

    /* update all the rule's values */
    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsValueArr[i].fieldName;
        paRuleDbPtr->fieldsValueArr[fieldName].fieldName = fieldName;
        cpssOsMemCpy(paRuleDbPtr->fieldsValueArr[fieldName].data, fieldsValueArr[i].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        cpssOsMemCpy(paRuleDbPtr->fieldsValueArr[fieldName].msk, fieldsValueArr[i].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        paRuleDbPtr->isFieldValueSetArr[fieldName] = GT_TRUE;
    }

    /* update action in DB */
    rc = prvCpssDxChPacketAnalyzerActionGroupAdd(managerId,groupId,actionId);
    if (rc != GT_OK)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /* update group in DB */
    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        paGrpDbPtr->stagesArr[paGrpDbPtr->numOfStages] = paKeyDbPtr->stagesArr[i];
        paGrpDbPtr->numOfStages++;
    }

    return GT_OK;

exit_cleanly_lbl:

    cpssOsFree(paRuleDbPtr->fieldsValueArr);
    cpssOsFree(paRuleDbPtr->isFieldValueSetArr);
    cpssOsFree(paRuleDbPtr);

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerGroupRuleAdd function
* @endinternal
*
* @brief   Add packet analyzer rule to packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] ruleAttrPtr              - (pointer to) rule
*                                       attributes
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
* @retval GT_FULL                  - if array is full
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleAdd
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          groupId,
    IN  GT_U32                                          ruleId,
    IN  CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   *ruleAttrPtr,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                          actionId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupRuleAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, keyId, groupId, ruleId, ruleAttrPtr, numOfFields, fieldsValueArr, actionId));

    rc = internal_cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, ruleAttrPtr, numOfFields, fieldsValueArr, actionId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, keyId, groupId, ruleId, ruleAttrPtr, numOfFields, fieldsValueArr, actionId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerGroupRuleUpdate
*           function
* @endinternal
*
* @brief   Replace content of packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupRuleUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId,
    IN  GT_U32                                      numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                      actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      i,j,numOfFieldsTotal = 0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName;
    GT_STATUS                                   rc;
    GT_BOOL                                     found;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   *fieldsValueArrTemp = NULL;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);
    if (numOfFields > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Max num of fields is [%d]",CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    }

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get devNum according to keyId */
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,paRuleDbPtr->paKeyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,paRuleDbPtr->paKeyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that all the fields in fieldsValueArr[] are part of the keyId's fields */
    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsValueArr[i].fieldName;
        found = GT_FALSE;
        for (j=0; j<paKeyDbPtr->numOfFields; j++)
        {
            /* if field from rule exists in the key continue*/
            if (paKeyDbPtr->fieldsArr[j] == fieldName)
            {
                found = GT_TRUE;
                break;
            }
        }
        if (found == GT_FALSE)
        {
            if (fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Found param udf [%d] in ruleId [%d] groupId [%d] that doesn't exist in keyId [%d]",
                                              fieldName,ruleId,groupId,paRuleDbPtr->paKeyId);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Found param [%s] in ruleId [%d] groupId [%d] that doesn't exist in keyId [%d]",
                                              packetAnalyzerFieldToIdebugFieldArr[fieldName],ruleId,groupId,paRuleDbPtr->paKeyId);
            }
        }
    }

    fieldsValueArrTemp = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsValueArrTemp == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate fields array for manager [%d]",managerId);
    cpssOsMemSet(fieldsValueArrTemp, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    /* get all rules fields */
    for (i=0; i<paKeyDbPtr->numOfFields; i++)
    {
        fieldName = paKeyDbPtr->fieldsArr[i];
        if (paRuleDbPtr->isFieldValueSetArr[fieldName] == GT_TRUE)
        {
            fieldsValueArrTemp[numOfFieldsTotal].fieldName = fieldName;
            numOfFieldsTotal++;
        }
    }
    /* add all new fields */
    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsValueArr[i].fieldName;
        if (paRuleDbPtr->isFieldValueSetArr[fieldName] == GT_FALSE)
        {
            fieldsValueArrTemp[numOfFieldsTotal].fieldName = fieldName;
            numOfFieldsTotal++;
        }
    }

    /* check that no overlapping fields in fieldsValueArrTemp[] */
    rc = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,paRuleDbPtr->paKeyId,numOfFieldsTotal,fieldsValueArrTemp);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldsValueArrTemp);
        return rc;
    }

    /* verify that action is changed */
    if (paRuleDbPtr->paActId != actionId)
    {
        /* delete old actionId from DB */
        rc = prvCpssDxChPacketAnalyzerActionGroupDelete(managerId,groupId,paRuleDbPtr->paActId);
        if (rc != GT_OK)
        {
            cpssOsFree(fieldsValueArrTemp);
            return rc;
        }

        /* add new actionId to DB */
        rc = prvCpssDxChPacketAnalyzerActionGroupAdd(managerId,groupId,actionId);
        if (rc != GT_OK)
        {
            cpssOsFree(fieldsValueArrTemp);
            return rc;
        }
    }

    /* update rule in DB */
    paRuleDbPtr->paActId = actionId;

    for (i=0; i<numOfFields; i++)
    {
        fieldName = fieldsValueArr[i].fieldName;

        paRuleDbPtr->fieldsValueArr[fieldName].fieldName = fieldName ;
        cpssOsMemCpy(paRuleDbPtr->fieldsValueArr[fieldName].data, fieldsValueArr[i].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        cpssOsMemCpy(paRuleDbPtr->fieldsValueArr[fieldName].msk, fieldsValueArr[i].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
    }

    cpssOsFree(fieldsValueArrTemp);
    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerGroupRuleUpdate function
* @endinternal
*
* @brief   Replace content of packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId,
    IN  GT_U32                                      numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                      actionId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupRuleUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, numOfFields, fieldsValueArr, actionId));

    rc = internal_cpssDxChPacketAnalyzerGroupRuleUpdate(managerId, ruleId, groupId, numOfFields, fieldsValueArr, actionId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, numOfFields, fieldsValueArr, actionId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerGroupRuleDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group or rule
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupRuleDelete
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U32                                      i,j,k;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   *tmpFieldsValueArr;
    GT_BOOL                                     *tmpIsFieldValueSetArr;
    GT_U32                                      tmpNumOfFields;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* delete rule's configuration from HW */
    rc = prvCpssDxChPacketAnalyzerRuleActivateEnableSet(managerId,paRuleDbPtr,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* delete rule's actionId from DB */
    rc = prvCpssDxChPacketAnalyzerActionGroupDelete(managerId,groupId,paRuleDbPtr->paActId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,paRuleDbPtr->paKeyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,paRuleDbPtr->paKeyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* delete rule's stages from group in DB */
    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        for (j=0; j<paGrpDbPtr->numOfStages; j++)
        {
            if (paGrpDbPtr->stagesArr[j] == paKeyDbPtr->stagesArr[i])
            {
                /* delete the stage and copy all the remaining array */
                paGrpDbPtr->stagesArr[j] = 0; /* if this is the last stage */
                for (k=j; k<paGrpDbPtr->numOfStages-1; k++)
                {
                    /* update group's stages array in DB */
                    paGrpDbPtr->stagesArr[k] = paGrpDbPtr->stagesArr[k+1];
                }
                paGrpDbPtr->numOfStages--;
                break;
            }
        }
    }

    /* delete rule from DB */
    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        if (paGrpDbPtr->paRulePtrArr[i]->paRuleId == ruleId)
        {
            /*save deleted pointer*/
            tmpNumOfFields = paGrpDbPtr->paRulePtrArr[i]->numOfFields;
            tmpFieldsValueArr = paGrpDbPtr->paRulePtrArr[i]->fieldsValueArr;
            tmpIsFieldValueSetArr = paGrpDbPtr->paRulePtrArr[i]->isFieldValueSetArr;

            /* copy all the remaining rules */
            for (j=i; j<=paGrpDbPtr->numOfRules; j++)
            {
                cpssOsMemCpy(paGrpDbPtr->paRulePtrArr[j],paGrpDbPtr->paRulePtrArr[j+1],sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC));
            }
            if (tmpNumOfFields)
            {
                cpssOsFree(tmpFieldsValueArr);
                cpssOsFree(tmpIsFieldValueSetArr);
            }

            cpssOsFree(paGrpDbPtr->paRulePtrArr[paGrpDbPtr->numOfRules]);
            paGrpDbPtr->paRulePtrArr[paGrpDbPtr->numOfRules] = 0;
            paGrpDbPtr->numOfRules--;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, " Error: failed to delete ruleId [%d] in groupId [%d]",ruleId,groupId);
}

/**
* @internal cpssDxChPacketAnalyzerGroupRuleDelete function
* @endinternal
*
* @brief   Delete packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group or rule
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleDelete
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupRuleDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId));

    rc = internal_cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerGroupRuleGet function
* @endinternal
*
* @brief   Get packet analyzer rule attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyIdPtr                - (pointer to) logical
*                                       key identification
* @param[out] ruleAttrPtr             - (pointer to) rule
*                                       attributes
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for rule
*                                       identification
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       rule identification
* @param[out] fieldsValueArr          - rule content array
*                                       (Data and Mask)
* @param[out] actionIdPtr             - (pointer to) rule action
*                                       identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or rule
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupRuleGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    OUT   GT_U32                                        *keyIdPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC *ruleAttrPtr,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     fieldsValueArr[], /*arrSizeVarName=numOfFieldsPtr*/
    OUT   GT_U32                                        *actionIdPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR       paKeyDbPtr = NULL;
    GT_U32                                      i;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    CPSS_NULL_PTR_CHECK_MAC(keyIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    CPSS_NULL_PTR_CHECK_MAC(actionIdPtr);

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    *keyIdPtr = paRuleDbPtr->paKeyId;
    cpssOsMemCpy(ruleAttrPtr,&(paRuleDbPtr->paRuleAttr),sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));

    /* get devNum according to keyId */
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,paRuleDbPtr->paKeyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,paRuleDbPtr->paKeyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (paRuleDbPtr->numOfFields > *numOfFieldsPtr){
        *numOfFieldsPtr = paRuleDbPtr->numOfFields;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: number of fields is [%d] in ruleId [%d] groupId [%d]",paRuleDbPtr->numOfFields,ruleId,groupId);
    }

    *numOfFieldsPtr = paRuleDbPtr->numOfFields;
    *actionIdPtr = paRuleDbPtr->paActId;

    for (i=0; i<paRuleDbPtr->numOfFields; i++)
    {
        fieldName = paKeyDbPtr->fieldsArr[i];
        fieldsValueArr[i].fieldName = paRuleDbPtr->fieldsValueArr[fieldName].fieldName;
        cpssOsMemCpy(fieldsValueArr[i].data, paRuleDbPtr->fieldsValueArr[fieldName].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        cpssOsMemCpy(fieldsValueArr[i].msk, paRuleDbPtr->fieldsValueArr[fieldName].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);

    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerGroupRuleGet function
* @endinternal
*
* @brief   Get packet analyzer rule attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyIdPtr                - (pointer to) logical
*                                       key identification
* @param[out] ruleAttrPtr             - (pointer to) rule
*                                       attributes
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for rule
*                                       identification
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       rule identification
* @param[out] fieldsValueArr          - rule content array
*                                       (Data and Mask)
* @param[out] actionIdPtr             - (pointer to) rule action
*                                       identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or rule
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    OUT   GT_U32                                        *keyIdPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC *ruleAttrPtr,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     fieldsValueArr[], /*arrSizeVarName=numOfFieldsPtr*/
    OUT   GT_U32                                        *actionIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupRuleGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, keyIdPtr, ruleAttrPtr, numOfFieldsPtr, fieldsValueArr, actionIdPtr));

    rc = internal_cpssDxChPacketAnalyzerGroupRuleGet(managerId, ruleId, groupId, keyIdPtr, ruleAttrPtr, numOfFieldsPtr, fieldsValueArr, actionIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, keyIdPtr, ruleAttrPtr, numOfFieldsPtr, fieldsValueArr, actionIdPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerActionCreate function
* @endinternal
*
* @brief   Create packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerActionCreate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR           paActDbPtr = NULL;
    GT_U8                                           devNum;
    GT_STATUS   rc;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    rc = prvCpssDxChIdebugActionCheck(actionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] != NULL)
    {
        /* can't redefine an packet analyzer action DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: actionId [%d] already exists",actionId);
    }

    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] = (PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC));
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocated memory for actionId [%d]",actionId);
    }
    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];
    cpssOsMemSet(paActDbPtr, 0, sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC));

    paActDbPtr->paActId = actionId;
    cpssOsMemCpy(&(paActDbPtr->action),actionPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerActionCreate function
* @endinternal
*
* @brief   Create packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerActionCreate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerActionCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, actionId, actionPtr));

    rc = internal_cpssDxChPacketAnalyzerActionCreate(managerId, actionId, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, actionId, actionPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerActionUpdate function
* @endinternal
*
* @brief   Update packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerActionUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR           paActDbPtr = NULL;
    GT_U8                                           devNum;
    GT_STATUS   rc;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    rc = prvCpssDxChIdebugActionCheck(actionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];

    cpssOsMemCpy(&(paActDbPtr->action),actionPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerActionUpdate function
* @endinternal
*
* @brief   Update packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
*/
GT_STATUS cpssDxChPacketAnalyzerActionUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerActionUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, actionId, actionPtr));

    rc = internal_cpssDxChPacketAnalyzerActionUpdate(managerId, actionId, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, actionId, actionPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerActionDelete function
* @endinternal
*
* @brief   Delete packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found action
*
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerActionDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  actionId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       paActDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i,j,groupId;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);

    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];

    for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
    {
        if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[i] == NULL)
            continue;
        groupId = i;

        paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];

        /* action that is connected to a rule can not be deleted */
        for (j=1; j<=paGrpDbPtr->numOfRules; j++)
        {
            if (paGrpDbPtr->paRulePtrArr[j]->paActId == actionId)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: can't delete actionId [%d] that is bounded to ruleId [%d]",actionId, paGrpDbPtr->paRulePtrArr[j]->paRuleId);
        }
    }

    cpssOsMemSet(paActDbPtr,0,sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC));
    cpssOsFree(paActDbPtr);
    PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] = 0;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerActionDelete function
* @endinternal
*
* @brief   Delete packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found action
*
*/
GT_STATUS cpssDxChPacketAnalyzerActionDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  actionId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerActionDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, actionId));

    rc = internal_cpssDxChPacketAnalyzerActionDelete(managerId, actionId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, actionId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerActionGet function
* @endinternal
*
* @brief   Get packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[out] actionPtr            - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC       *actionPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR           paActDbPtr = NULL;
    GT_U8                                           devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];

    cpssOsMemCpy(actionPtr, &(paActDbPtr->action),sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerActionGet function
* @endinternal
*
* @brief   Get packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[out] actionPtr            - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
*/
GT_STATUS cpssDxChPacketAnalyzerActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC       *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerActionGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, actionId, actionPtr));

    rc = internal_cpssDxChPacketAnalyzerActionGet(managerId, actionId, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, actionId, actionPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerSampledDataCountersClear
*           function
* @endinternal
*
* @brief   Clear Sampling Data and Counters for packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerSampledDataCountersClear
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR                   paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR                    paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                    paMngDbPtr = NULL;
    GT_U32                                                   i,keyId, value,id;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              stageId;
    GT_STATUS                                                rc = GT_OK;
    GT_U8                                                    devNum;
    GT_CHAR                                                  interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                                   numOfFields;


     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for rule */
    rc = prvCpssDxChPacketAnalyzerSamplingEnableCheck(managerId,paRuleDbPtr->paActId);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFields);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        stageId = paKeyDbPtr->stagesArr[i];

        if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        {
            id = stageId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

            /* for uds check that it is valid */
            if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
                continue;
        }

        cpssOsStrCpy(interfaceId,paDevDbPtr->stagesArr[stageId].boundInterface.interfaceId);
        rc = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceId,GT_TRUE,&value,NULL);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChIdebugPortGroupInterfaceResetSamplingTriger(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerSampledDataCountersClear function
* @endinternal
*
* @brief   Clear Sampling Data and Counters for packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
*/
GT_STATUS cpssDxChPacketAnalyzerSampledDataCountersClear
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerSampledDataCountersClear);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId));

    rc = internal_cpssDxChPacketAnalyzerSampledDataCountersClear(managerId, ruleId, groupId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerActionSamplingEnableSet
*           function
* @endinternal
*
* @brief   Enable/disable sampling for packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] actionId                 - action identification
*                                       (APPLICABLE RANGES: 1..128)
* @param[in] enable                   - enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found action
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerActionSamplingEnableSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  GT_BOOL                                     enable
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR           paActDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR          paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR           paGrpDbPtr = NULL;
    GT_U32                                          i,j;
    GT_STATUS                                       rc;
    GT_U8                                           devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_VALID_CHECK_MAC(managerId,actionId);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];
    paActDbPtr->action.samplingEnable = enable;

    /* update all {groupId,ruleId} that are bounded to this actionId */
    for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
    {
        paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[i];
        if (paGrpDbPtr == NULL)
            continue;
        for (j=1; j<=paGrpDbPtr->numOfRules; j++)
        {
            paRuleDbPtr = paGrpDbPtr->paRulePtrArr[j];
            if (paRuleDbPtr->paActId == actionId)
            {
                rc = prvCpssDxChPacketAnalyzerRuleActionSampelingEnableSet(managerId,paRuleDbPtr,enable);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerActionSamplingEnableSet function
* @endinternal
*
* @brief   Enable/disable sampling for packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] actionId                 - action identification
*                                       (APPLICABLE RANGES: 1..128)
* @param[in] enable                   - enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found action
*/
GT_STATUS cpssDxChPacketAnalyzerActionSamplingEnableSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  GT_BOOL                                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerActionSamplingEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, actionId, enable));

    rc = internal_cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId, actionId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, actionId, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerGroupActivateEnableSet
*           function
* @endinternal
*
* @brief   Enable/disable packet analyzer group activation.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] enable                   - enable/disable group activation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
* @retval GT_BAD_STATE             - on invalid parameter
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerRuleMatchStagesGet,cpssDxChPacketAnalyzerRuleMatchDataGet,
*       cpssDxChPacketAnalyzerRuleMatchDataAllFieldsGet
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerGroupActivateEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  groupId,
    IN  GT_BOOL                                 enable
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;
    GT_U32                                      i;
    GT_STATUS                                   rc;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* need to verify that no group is activated */
    if (enable == GT_TRUE)
    {
        for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS; i++)
        {
            paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[i];
            if (paGrpDbPtr == NULL)
                continue;

            if (paGrpDbPtr->activateEnable == GT_TRUE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "ERROR: groupId [%d] is activated",i);
        }
    }

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];

    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        paRuleDbPtr = paGrpDbPtr->paRulePtrArr[i];
        rc = prvCpssDxChPacketAnalyzerRuleActivateEnableSet(managerId,paRuleDbPtr,enable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    paGrpDbPtr->activateEnable = enable;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerGroupActivateEnableSet function
* @endinternal
*
* @brief   Enable/disable packet analyzer group activation.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] enable                   - enable/disable group activation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
* @retval GT_BAD_STATE             - on invalid parameter
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerRuleMatchStagesGet,cpssDxChPacketAnalyzerRuleMatchDataGet,
*       cpssDxChPacketAnalyzerRuleMatchDataAllFieldsGet
*/
GT_STATUS cpssDxChPacketAnalyzerGroupActivateEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  groupId,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerGroupActivateEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, groupId, enable));

    rc = internal_cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, groupId, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerRuleMatchStagesGet
*           function
* @endinternal
*
* @brief   Get stages where packet analyzer rule was matched.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfMatchedStagesPtr- in: (pointer to)
*                                        allocated number of
*                                        matched stages
*                                        out: (pointer to)
*                                        actual number of
*                                        matched stages
* @param[out] matchedStagesArr         - (pointer to) matched
*                                        stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerRuleMatchStagesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U32                                            ruleId,
    IN    GT_U32                                            groupId,
    INOUT GT_U32                                            *numOfMatchedStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       matchedStagesArr[] /*arrSizeVarName=numOfMatchedStagesPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR                   paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR                    paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                    paMngDbPtr = NULL;
    GT_U32                                                   i,j,keyId,value,id;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              stageId;
    GT_STATUS                                                rc = GT_OK;
    GT_U8                                                    devNum;
    GT_CHAR                                                  interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC                 *fieldsValueArr = NULL;
    GT_U32                                                   tempNumOfMatchedStages = 0;
    GT_BOOL                                                  tempMatchedStagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E+1] = {GT_FALSE};
    GT_U32                                                   numOfFields,numOfFieldsArr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      *fieldsArr = NULL;
    GT_U32                                                    activeMult=0;
    GT_U32                                                    muxedBmp;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    CPSS_NULL_PTR_CHECK_MAC(numOfMatchedStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(matchedStagesArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for rule */
    rc = prvCpssDxChPacketAnalyzerSamplingEnableCheck(managerId,paRuleDbPtr->paActId);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to key in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFieldsArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* array allocation */
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    fieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsArr);

    if (fieldsArr == NULL || fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    rc = prvCpssDxChPacketAnalyzerMarkMuxedStagesSet(managerId,devNum,paKeyDbPtr->numOfStages,paKeyDbPtr->stagesArr,&muxedBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<paKeyDbPtr->numOfStages; i++)
    {
        stageId = paKeyDbPtr->stagesArr[i];

        if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        {
            id = stageId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

            /* for uds check that it is valid */
            if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
                continue;
        }

        /* Get keyId fields list per stage for bounded interface */
        numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E; /* maximum allocated number of fields for stageId */
        rc = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stageId,&numOfFields,fieldsArr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        cpssOsStrCpy(interfaceId,paDevDbPtr->stagesArr[stageId].boundInterface.interfaceId); /* bounded interface */
        if(muxedBmp&(1<<i))
        {
            /* get number of hits for bounded interface */
            rc = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceId,GT_FALSE,&value,&activeMult);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
        else
        {
            value = 0;
        }
        /* there is match counter hit for this interface; add stageId to array */
        if (value)
        {
            if (tempMatchedStagesArr[stageId] == GT_FALSE)
            {
                tempMatchedStagesArr[stageId] = GT_TRUE;
                tempNumOfMatchedStages++;
            }
        }
    }

    if (*numOfMatchedStagesPtr < tempNumOfMatchedStages)
    {
        *numOfMatchedStagesPtr = tempNumOfMatchedStages;
        rc = GT_BAD_SIZE;
        CPSS_LOG_ERROR_MAC("Error: number of matched stages is [%d]",tempNumOfMatchedStages);
        goto exit_cleanly_lbl;
    }

    *numOfMatchedStagesPtr = tempNumOfMatchedStages;
    if (*numOfMatchedStagesPtr)
    {
        j=0;
        for (i=0; i<paKeyDbPtr->numOfStages; i++)
        {
            stageId = paKeyDbPtr->stagesArr[i];
            if (tempMatchedStagesArr[stageId] == GT_TRUE)
            {
                matchedStagesArr[j] = stageId;
                j++;
            }
        }
    }

exit_cleanly_lbl:

    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerRuleMatchStagesGet function
* @endinternal
*
* @brief   Get stages where packet analyzer rule was matched.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfMatchedStagesPtr- in: (pointer to)
*                                        allocated number of
*                                        matched stages
*                                        out: (pointer to)
*                                        actual number of
*                                        matched stages
* @param[out] matchedStagesArr         - (pointer to) matched
*                                        stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPacketAnalyzerRuleMatchStagesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U32                                            ruleId,
    IN    GT_U32                                            groupId,
    INOUT GT_U32                                            *numOfMatchedStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       matchedStagesArr[] /*arrSizeVarName=numOfMatchedStagesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerRuleMatchStagesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, numOfMatchedStagesPtr, matchedStagesArr));

    rc = internal_cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId, ruleId, groupId, numOfMatchedStagesPtr, matchedStagesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, numOfMatchedStagesPtr, matchedStagesArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPacketAnalyzerStageMatchDataGet function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes by search
*          attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[in,out] sampleFieldsValueArr - in: (pointer to)
*                                       requested fields for
*                                       sample data array
*                                       out: (pointer to) sample
*                                       data array values for
*                                       requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPacketAnalyzerStageMatchDataGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    IN    GT_U32                                        metaData,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    INOUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      id=0,i,keyId,value,numOfSampleFields = 0;
    GT_CHAR                                     interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    *fieldsValueArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         pAFieldName;
    GT_U8                                       devNum;
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      requiredMultInd=0;
    GT_U32                                      muxedBmp,numOfFields;
    GT_U32                                      numberOfStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E];

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfHitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfSampleFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(sampleFieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for rule */
    rc = prvCpssDxChPacketAnalyzerSamplingEnableCheck(managerId,paRuleDbPtr->paActId);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numberOfStages,stagesArr);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = prvCpssDxChPacketAnalyzerMarkMuxedStagesSet(managerId,devNum,numberOfStages,stagesArr,&muxedBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Check if stage is muxed*/
    for(i=0;i<numberOfStages;i++)
    {
        if(stageId==stagesArr[i])
        {
            if((muxedBmp&(1<<i))==0)
            {
                /*muxed stage = no hit*/
                *numOfHitsPtr=0;
                return GT_OK;
            }
        }
    }

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFields);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* array allocation */
    fieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFields);
    if (fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(fieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFields);

    cpssOsStrCpy(interfaceId,stageDbPtr->boundInterface.interfaceId);/* bounded interface */
    /* get number of hits for bounded interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,metaData,interfaceId,GT_FALSE,&value,&requiredMultInd);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    *numOfHitsPtr = value;

    /* prepare the array */
    for (i=0; i<*numOfSampleFieldsPtr; i++)
    {
        if ((sampleFieldsValueArr[i].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) &&
            (sampleFieldsValueArr[i].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
        {
            id = sampleFieldsValueArr[i].fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);
            if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",sampleFieldsValueArr[i].fieldName);
            cpssOsStrCpy(fieldsValueArr[i].fieldName,paMngDbPtr->udfArr[id].iDebugField);
        }
        else
            cpssOsStrCpy(fieldsValueArr[i].fieldName,packetAnalyzerFieldToIdebugFieldArr[sampleFieldsValueArr[i].fieldName]);
    }
    /* get all required interface fields even fields that are mux and not valid */
    rc = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId,*numOfSampleFieldsPtr,fieldsValueArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* copy only valid fields to array */
    for (i=0; i<*numOfSampleFieldsPtr; i++)
    {
        if (fieldsValueArr[i].isValid == GT_TRUE)
        {
            pAFieldName = idebugFieldToPacketAnalyzerFieldConv(managerId,fieldsValueArr[i].fieldName);
            if (pAFieldName == CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
            {
                rc = GT_BAD_PARAM;
                CPSS_LOG_ERROR_MAC("Error: iDebug field [%s] doesn't exist in pa",fieldsValueArr[i].fieldName);
                goto exit_cleanly_lbl;
            }

            sampleFieldsValueArr[numOfSampleFields].fieldName = pAFieldName;
            cpssOsMemCpy(sampleFieldsValueArr[numOfSampleFields].data,fieldsValueArr[i].data,sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
            cpssOsMemCpy(sampleFieldsValueArr[numOfSampleFields].msk,fieldsValueArr[i].mask,sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
            numOfSampleFields++;
        }
    }

    *numOfSampleFieldsPtr = numOfSampleFields;

exit_cleanly_lbl:

    cpssOsFree(fieldsValueArr);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerStageMatchDataGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[in,out] sampleFieldsValueArr - in: (pointer to)
*                                       requested fields for
*                                       sample data array
*                                       out: (pointer to) sample
*                                       data array values for
*                                       requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageMatchDataGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    INOUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
{
    return prvCpssDxChPacketAnalyzerStageMatchDataGet(managerId, ruleId, groupId, stageId,
        META_DATA_ALL_CNS,numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);
}

/**
* @internal cpssDxChPacketAnalyzerStageMatchDataGet function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[in,out] sampleFieldsValueArr - in: (pointer to)
*                                       requested fields for
*                                       sample data array
*                                       out: (pointer to) sample
*                                       data array values for
*                                       requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    INOUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageMatchDataGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));

    rc = internal_cpssDxChPacketAnalyzerStageMatchDataGet(managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          packet analyzer fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      i,numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    GT_U8                                       devNum;
    GT_STATUS                                   rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         *fieldsArr = NULL;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfHitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfSampleFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(sampleFieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,paRuleDbPtr->paKeyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* array allocation */
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /* get all fields for stage */
    rc = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stageId,&numOfFields,fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    if (numOfFields > *numOfSampleFieldsPtr){
        *numOfSampleFieldsPtr = numOfFields;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: for stageId [%d] num of fields [%d]",stageId,numOfFields);
    }

    *numOfSampleFieldsPtr = numOfFields;
    for (i=0; i<numOfFields; i++)
    {
        sampleFieldsValueArr[i].fieldName = fieldsArr[i];
    }

    /* Get packet analyzer rule match attributes for all packet analyzer fields */
    rc = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr);

exit_cleanly_lbl:

    cpssOsFree(fieldsArr);

    return rc;

}

/**
* @internal cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          packet analyzer fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));

    rc = internal_cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal
*           prvCpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] metaData                 - search attribute data
*                                       (Applicable device:Falcon; AC5P)
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS prvCpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    IN    GT_U32                                                metaData,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_PTR    paStageDbPtr = NULL;
    GT_U32                                      i,keyId,numOfSampleFields,numOfFields;
    GT_U32                                      value,muxedBmp,requiredMultInd=0;
    GT_U8                                       devNum;
    GT_STATUS                                   rc = GT_OK;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    *fieldsValueArr = NULL;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC              *fieldsArr = NULL;
    GT_U32                                      numberOfStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E];
    GT_CHAR                                     interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfHitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfSampleFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(sampleFieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for rule */
    rc = prvCpssDxChPacketAnalyzerSamplingEnableCheck(managerId,paRuleDbPtr->paActId);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyId = paRuleDbPtr->paKeyId;
    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /*get logical key stages list */
    rc = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numberOfStages,stagesArr);
    if (rc != GT_OK)
    {
        return rc;
    }

     /*mark valid stages*/
    rc = prvCpssDxChPacketAnalyzerMarkMuxedStagesSet(managerId,devNum,numberOfStages,stagesArr,&muxedBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Check if stage is muxed*/
    for(i=0;i<numberOfStages;i++)
    {
        if(stageId==stagesArr[i])
        {
            if((muxedBmp&(1<<i))==0)
            {
                /*muxed stage = no hit*/
                *numOfHitsPtr=0;
                return GT_OK;
            }
        }
    }

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&paStageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsStrCpy(interfaceId,paStageDbPtr->boundInterface.interfaceId);/* bounded interface */

    /* get number of hits for bounded interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,metaData,interfaceId,GT_FALSE,&value,&requiredMultInd);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    *numOfHitsPtr = value;

    /* get number of fields in interface */
    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceId,&numOfFields);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* allocate fields arrays for interface */
    fieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFields);
    if (fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFields);

    fieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFields);
    if (fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(fieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFields);

    /* fill fields array for interface */
    rc = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceId,&numOfFields,fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* prepare the array */
    for (i=0; i<numOfFields; i++)
    {
        cpssOsStrCpy(fieldsValueArr[i].fieldName,fieldsArr[i].fieldName);
    }

    *numOfSampleFieldsPtr = numOfFields;
    /* get all required interface fields even fields that are mux and not valid */
    rc = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceId,*numOfSampleFieldsPtr,fieldsValueArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    numOfSampleFields=0;
    /* copy only valid fields to array */
    for (i=0; i<*numOfSampleFieldsPtr; i++)
    {
        if (fieldsValueArr[i].isValid == GT_TRUE)
        {
            cpssOsStrCpy(sampleFieldsValueArr[numOfSampleFields].fieldName,fieldsValueArr[i].fieldName);
            cpssOsMemCpy(sampleFieldsValueArr[numOfSampleFields].data,fieldsValueArr[i].data,sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
            cpssOsMemCpy(sampleFieldsValueArr[numOfSampleFields].msk,fieldsValueArr[i].mask,sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
            numOfSampleFields++;
        }
    }

    *numOfSampleFieldsPtr = numOfSampleFields;

exit_cleanly_lbl:

    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);

    return rc;

}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
)
{
    return prvCpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet(managerId,ruleId,groupId,stageId,META_DATA_ALL_CNS,
                                                                numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr);
}

/**
* @internal
*           cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));

    rc = internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet(managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, stageId, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC        *searchAttributePtr,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
)
{
    GT_U32                                      metaData,num = 1;
    GT_U32                                      dummy;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;

    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);

    if(searchAttributePtr->type==CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_PORT_E)
    {
        /*convert*/
        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,searchAttributePtr->portNum,
        &(searchAttributePtr->tile),&(searchAttributePtr->dp),&dummy,NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
            num = 2;
        searchAttributePtr->pipe= searchAttributePtr->dp/(MAX_DP_PER_TILE_CNS/MAX_PIPES_PER_TILE_CNS/num);
        searchAttributePtr->dp%=(MAX_DP_PER_TILE_CNS/MAX_PIPES_PER_TILE_CNS/num);

    }

    rc = prvCpssDxChIdebugEncodeMetaData(devNum,searchAttributePtr,&metaData);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet(managerId,ruleId,groupId,stageId,metaData,
                                                                numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr);
}

/**
* @internal
*           cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributeGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC        *searchAttributePtr,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, ruleId, groupId, stageId, searchAttributePtr, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));

    rc = internal_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet(managerId, ruleId, groupId, stageId, searchAttributePtr, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId, stageId, searchAttributePtr, numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerStageFieldsGet
*           function
* @endinternal
*
* @brief   Get stage fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - (pointer to) stage
*                                       fields list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC              *iDebugFieldsArr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      i,iDebugNumOfFields;
    GT_U32                                      paNumOfFields = 0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         paField;
    GT_STATUS                                   rc;
    GT_U32                                      numOfFields;


    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFields);
    if (rc != GT_OK)
    {
        return rc;
    }
    iDebugNumOfFields = numOfFields;

    /* array allocation */
    iDebugFieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFields);
    if (iDebugFieldsArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* get all interface iDebug fields */
    rc = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stageId,&iDebugNumOfFields,iDebugFieldsArr);
    if (rc != GT_OK)
    {
        cpssOsFree(iDebugFieldsArr);
        return rc;
    }

    /* convert iDebug field to packet analyzer field */
    for (i=0; i<iDebugNumOfFields; i++)
    {
        /* return only existing pa fields */
        paField = idebugFieldToPacketAnalyzerFieldConv(managerId,iDebugFieldsArr[i].fieldName);
        if (paField != CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
        {
             if (paNumOfFields == *numOfFieldsPtr)
             {
                 rc = GT_BAD_SIZE ;
             }

             if (rc == GT_OK)
             {
                fieldsArr[paNumOfFields] = paField;
                paNumOfFields++;
             }
             else
                paNumOfFields++;
        }
    }

    *numOfFieldsPtr = paNumOfFields;
    cpssOsFree(iDebugFieldsArr);
    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerStageFieldsGet function
* @endinternal
*
* @brief   Get stage fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - (pointer to) stage
*                                       fields list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageFieldsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, stageId, numOfFieldsPtr, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerStageFieldsGet(managerId, stageId, numOfFieldsPtr, fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, stageId, numOfFieldsPtr, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerFieldStagesGet
*           function
* @endinternal
*
* @brief   Get field's stage list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - (pointer to)
*                                       field's stage list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerFieldStagesGet
(
    IN    GT_U32                                            managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldName,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[]/*arrSizeVarName=numOfStagesPtr*/
)
{
    GT_U8                                       devNum;
    GT_U32                                      id,i,k;
    GT_U32                                      numOfInterfaces,totalNumOfInterfaces,totalNumOfStages = 0;
    GT_CHAR_PTR                                 *interfacesArr = NULL;
    GT_CHAR                                     idebugField[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;
    GT_CHAR                                     instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_STATUS                                   sizeStatus=GT_OK ,rc = GT_OK;
    GT_BOOL                                     stageExists = GT_FALSE;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;


    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_FIELD_ID_CHECK_MAC(fieldName);
    CPSS_NULL_PTR_CHECK_MAC(numOfStagesPtr);
    CPSS_NULL_PTR_CHECK_MAC(stagesArr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if ((fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
    {
        id = fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",fieldName);

        cpssOsStrCpy(idebugField,paMngDbPtr->udfArr[id].iDebugField);
    }
    else
        cpssOsStrCpy(idebugField,packetAnalyzerFieldToIdebugFieldArr[fieldName]);

    rc = prvCpssDxChIdebugDbNumInterfacesGet(devNum,&totalNumOfInterfaces);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* array allocation */
    interfacesArr  = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*totalNumOfInterfaces);
    if (interfacesArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);;
    }
    cpssOsMemSet(interfacesArr,0,sizeof(GT_CHAR_PTR)*totalNumOfInterfaces);
    for (i=0; i<totalNumOfInterfaces; i++)
    {
        interfacesArr[i] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
        if (interfacesArr[i] == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(interfacesArr[i],0,sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
    }

    numOfInterfaces = totalNumOfInterfaces;
    /* get all applicable interfaces from XML */
    rc = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,idebugField,&numOfInterfaces,interfacesArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (i=0; i<numOfInterfaces; i++)
    {
        rc = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId,devNum,interfacesArr[i],&stageId,instanceId);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        if (stageId == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E)
            continue;

        /* copy without duplication to array */
        stageExists = GT_FALSE;
        for (k=0; k<totalNumOfStages; k++)
        {
            if (stagesArr[k] == stageId)
            {
                /* stage already exists in array */
                stageExists = GT_TRUE;
                break;
            }
        }
        /* add stage to the array */
        if (stageExists == GT_FALSE)
        {
            if (sizeStatus != GT_BAD_SIZE)
            {
                if (*numOfStagesPtr <= totalNumOfStages)
                {
                    CPSS_LOG_ERROR_MAC("Error: num of stages is at least [%d]",totalNumOfStages);
                    sizeStatus = GT_BAD_SIZE;
                }
                else
                {
                    stagesArr[totalNumOfStages] = stageId;
                    totalNumOfStages++;
                }
            }
            else /*count to return the needed size*/
                totalNumOfStages++;
        }
    }
    rc = sizeStatus;
    *numOfStagesPtr = totalNumOfStages;

exit_cleanly_lbl:

    if (interfacesArr)
    {
        for (i=0; i<totalNumOfInterfaces; i++)
            if (interfacesArr[i])
                cpssOsFree(interfacesArr[i]);
        cpssOsFree(interfacesArr);
    }

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerFieldStagesGet function
* @endinternal
*
* @brief   Get field's stage list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - (pointer to)
*                                       field's stage list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerFieldStagesGet
(
    IN    GT_U32                                            managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldName,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[]/*arrSizeVarName=numOfStagesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerFieldStagesGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, fieldName, numOfStagesPtr,stagesArr));

    rc = internal_cpssDxChPacketAnalyzerFieldStagesGet(managerId, fieldName, numOfStagesPtr,stagesArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, fieldName, numOfStagesPtr,stagesArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerMuxStageBind function
* @endinternal
*
* @brief   Set stage that will be valid in group of muxed stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerMuxStageBind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
)
{

    GT_STATUS                                      rc;
    GT_CHAR                                        bus[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_CHAR                                        desc[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                         ii,mux,numOfMuxStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    *muxStagesListArr = NULL ;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  boundInterface;
    GT_U8                                          devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);

    numOfMuxStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    muxStagesListArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numOfMuxStages);
    if (muxStagesListArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated array for device [%d]",devNum);
        goto exit_cleanly_lbl;
    }

    /*check that all muxed stages are unbound*/
    rc = cpssDxChPacketAnalyzerMuxStagesGet(managerId, stageId, &numOfMuxStages, muxStagesListArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }
    for (ii=0;ii<numOfMuxStages;ii++)
    {
        rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,muxStagesListArr[ii],&boundInterface);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
        if(GT_TRUE == boundInterface.isValid)
        {
            CPSS_LOG_ERROR_MAC("Error: stage %s is bounded",paStageStrArr[muxStagesListArr[ii]]);
            rc = GT_BAD_STATE;
            goto exit_cleanly_lbl;
        }
    }

    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterface);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,boundInterface.info.interfaceId);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsStrCpy(bus,boundInterface.info.instanceId);
    mux = boundInterface.info.interfaceIndex;
    cpssOsStrCpy(desc,boundInterface.info.interfaceId);

    /*set SW DB*/
    rc = prvCpssDxChIdebugInerfaceMuxSet(devNum,bus,mux);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    rc = prvCpssDxChPacketAnalyzerStageValiditySet(managerId,devNum,stageId,GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /*set HW*/
    rc = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,desc);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:
    if (muxStagesListArr != NULL)
        cpssOsFree(muxStagesListArr);

    return rc;
}

/**
* @internal cpssDxChPacketAnalyzerMuxStageBind function
* @endinternal
*
* @brief   Set stage that will be valid in group of muxed stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStageBind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerMuxStageBind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, stageId));

    rc = internal_cpssDxChPacketAnalyzerMuxStageBind(managerId, stageId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, stageId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPacketAnalyzerMuxStageUnbind
*           function
* @endinternal
*
* @brief   Set stage that will be invalid in group of muxed
*          stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerMuxStageUnbind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
)
{
    GT_STATUS rc;
    GT_U8 devNum;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC boundInterface;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);

    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterface);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,boundInterface.info.interfaceId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPacketAnalyzerStageValiditySet(managerId,devNum,stageId,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal cpssDxChPacketAnalyzerMuxStageUnbind function
* @endinternal
*
* @brief   Set stage that will be invalid in group of muxed
*          stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStageUnbind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerMuxStageUnbind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, stageId));

    rc = internal_cpssDxChPacketAnalyzerMuxStageUnbind(managerId, stageId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, stageId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerUserDefinedFieldAdd
*           function
* @endinternal
*
* @brief   Add packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[in] fieldNamePtr             - (pointer to) field name
* @param[out] udfIdPtr                - (pointer to)user defined
*                                       field identification.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedFieldAdd
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      id=0,i;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         udf = 0;
    GT_BOOL                                     isBounded = GT_FALSE,found=GT_FALSE;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udfAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldNamePtr);
    CPSS_NULL_PTR_CHECK_MAC(udfIdPtr);
    PRV_CPSS_PACKET_ANALYZER_IDEBUG_FIELD_ID_CHECK_MAC(devNum,fieldNamePtr->fieldNameArr);

    /* check attributes */
    if ((cpssOsStrCmp(udfAttrPtr->udfNameArr,"") == 0) || (cpssOsStrlen(udfAttrPtr->udfNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: udf name not valid");

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check if field is bounded to a PA field */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E; i++)
    {
        udf = i;
        if (cpssOsStrCmp(fieldNamePtr->fieldNameArr,packetAnalyzerFieldToIdebugFieldArr[udf]) == 0)
        {
            isBounded = GT_TRUE;
            break;
        }
    }
    if (isBounded)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: field [%s] is already bounded to PA field [%s]",fieldNamePtr->fieldNameArr,paFieldStrArr[udf]);

    /* check if udf was defined */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        udf = i;
        id = udf - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udfArr[id].name,udfAttrPtr->udfNameArr) == 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: udf [%s] already exist",udfAttrPtr->udfNameArr);
    }

    /* find free udf index */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        udf = i;
        id = udf - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
        {
            found = GT_TRUE;
            break;
        }
    }
    if (found == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "Error: didn't find free index in array for udf [%s]",udfAttrPtr->udfNameArr);


    paMngDbPtr->udfArr[id].valid = GT_TRUE;
    cpssOsStrCpy(paMngDbPtr->udfArr[id].name,udfAttrPtr->udfNameArr);
    cpssOsStrCpy(paMngDbPtr->udfArr[id].iDebugField,fieldNamePtr->fieldNameArr);

    *udfIdPtr = udf;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldAdd function
* @endinternal
*
* @brief   Add packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[in] fieldNamePtr             - (pointer to) field name
* @param[out] udfIdPtr                - (pointer to)user defined
*                                       field identification.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldAdd
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedFieldAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udfAttrPtr, fieldNamePtr, udfIdPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId, udfAttrPtr, fieldNamePtr, udfIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udfAttrPtr, fieldNamePtr, udfIdPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerUserDefinedFieldDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedFieldDelete
(
    IN GT_U32                                           managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      id=0,i;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         udf = 0;
    GT_BOOL                                     found=GT_FALSE;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udfAttrPtr);

    if ((cpssOsStrCmp(udfAttrPtr->udfNameArr,"") == 0) || (cpssOsStrlen(udfAttrPtr->udfNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: udf name not valid");

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find udf id */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        udf = i;
        id = udf - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udfArr[id].name,udfAttrPtr->udfNameArr) == 0)
        {
            found = GT_TRUE;
            break;
        }
    }

    if (found == GT_FALSE)
        return GT_OK;

    cpssOsMemSet(&(paMngDbPtr->udfArr[id]),0,sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldDelete
(
    IN GT_U32                                           managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedFieldDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udfAttrPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId, udfAttrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udfAttrPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerUserDefinedFieldGet
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[out] udfIdPtr                - (pointer to) user
*                                       defined field identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] fieldNamePtr            - (pointer to) field name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedFieldGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      id=0,i;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         udf = 0;
    GT_BOOL                                     found=GT_FALSE;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udfAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldNamePtr);
    CPSS_NULL_PTR_CHECK_MAC(udfIdPtr);

    if ((cpssOsStrCmp(udfAttrPtr->udfNameArr,"") == 0) || (cpssOsStrlen(udfAttrPtr->udfNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: udf name not valid");

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

     /* find udf id */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        udf = i;
        id = udf - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udfArr[id].name,udfAttrPtr->udfNameArr) == 0)
        {
            found = GT_TRUE;
            break;
        }
    }

    if (found == GT_FALSE){
        *validPtr = GT_FALSE ;
        return GT_OK;
    }

    *udfIdPtr = udf;
    *validPtr = paMngDbPtr->udfArr[id].valid;
    cpssOsStrCpy(fieldNamePtr->fieldNameArr, paMngDbPtr->udfArr[id].iDebugField);

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldGet function
* @endinternal
*
* @brief   Get packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[out] udfIdPtr                - (pointer to) user
*                                       defined field identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] fieldNamePtr            - (pointer to) field name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedFieldGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udfAttrPtr, udfIdPtr, validPtr, fieldNamePtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId, udfAttrPtr, udfIdPtr, validPtr, fieldNamePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udfAttrPtr, udfIdPtr, validPtr, fieldNamePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined field information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfId                    - user defined field
*                                       identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] udfAttrPtr              - (pointer to) udf attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] fieldNamePtr            - (pointer to) field name.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             udfId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      id = 0;
    GT_U8                                       devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_UDF_ID_CHECK_MAC(udfId);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(udfAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldNamePtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    id = udfId - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

    if (paMngDbPtr->udfArr[id].valid == GT_TRUE)
    {
        cpssOsStrCpy(fieldNamePtr->fieldNameArr,paMngDbPtr->udfArr[id].iDebugField);
        cpssOsStrCpy(udfAttrPtr->udfNameArr,paMngDbPtr->udfArr[id].name);
    }

    *validPtr = paMngDbPtr->udfArr[id].valid;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined field information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfId                    - user defined field
*                                       identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] udfAttrPtr              - (pointer to) udf attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] fieldNamePtr            - (pointer to) field name.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             udfId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedFieldInfoGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udfId, validPtr, udfAttrPtr, fieldNamePtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedFieldInfoGet(managerId, udfId, validPtr, udfAttrPtr, fieldNamePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udfId, validPtr, udfAttrPtr, fieldNamePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerUserDefinedStageAdd
*           function
* @endinternal
*
* @brief   Add packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[in] interfaceAttrPtr         - (pointer to) interface
*                                       attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedStageAdd
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr,
    IN CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    *interfaceAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stageDbPtr = NULL;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;
    GT_U32                                      i,id = 0;
    GT_BOOL                                     isBounded=GT_FALSE,found=GT_FALSE;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stage = 0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udsAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfaceAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(udsIdPtr);
    PRV_CPSS_PACKET_ANALYZER_INSTANCE_ID_CHECK_MAC(devNum,interfaceAttrPtr->instanceId);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceAttrPtr->interfaceId);

    /* check interface attributes */
    if ((cpssOsStrCmp(interfaceAttrPtr->instanceId,"") == 0) ||
        (cpssOsStrCmp(interfaceAttrPtr->interfaceId,"") == 0))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: wrong attributes for interface [%s]",interfaceAttrPtr->interfaceId);
    if ((cpssOsStrCmp(udsAttrPtr->udsNameArr,"") == 0) ||(cpssOsStrlen(udsAttrPtr->udsNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: uds name not valid");

    /* check if Interface is bounded to a stage */
    rc = prvCpssDxChPacketAnalyzerInterfaceStatusGet(managerId,devNum,interfaceAttrPtr->interfaceId,&isBounded);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isBounded)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: interface [%s] is already bounded",interfaceAttrPtr->interfaceId);


    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check if uds was defined */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stage = i;
        id = stage - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udsArr[id].name,udsAttrPtr->udsNameArr) == 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "Error: uds name [%s] already exist",udsAttrPtr->udsNameArr);
    }

    /* find free uds index */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stage = i;
        id = stage - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
        {
            found = GT_TRUE;
            break;
        }
    }
    if (found == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, "Error: didn't find free index in array for uds [%s]",udsAttrPtr->udsNameArr);


    paMngDbPtr->udsArr[id].valid = GT_TRUE;
    cpssOsMemCpy(&(paMngDbPtr->udsArr[id].uds),interfaceAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
    cpssOsMemCpy(&(paMngDbPtr->udsArr[id].name),udsAttrPtr->udsNameArr,sizeof(CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC));

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stage,&stageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update bounded array with uds info */
    cpssOsMemCpy(&(stageDbPtr->boundInterface),interfaceAttrPtr,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
    /* use 'cpssDxChPacketAnalyzerMuxStageBind' to validate this stage */
    stageDbPtr->isValid = GT_FALSE;

    *udsIdPtr = stage;

    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceAttrPtr->interfaceId,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageAdd function
* @endinternal
*
* @brief   Add packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[in] interfaceAttrPtr         - (pointer to) interface
*                                       attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageAdd
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr,
    IN CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    *interfaceAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedStageAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udsAttrPtr, interfaceAttrPtr, udsIdPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId, udsAttrPtr, interfaceAttrPtr, udsIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udsAttrPtr, interfaceAttrPtr, udsIdPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerUserDefinedStageDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - user defined stage attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedStageDelete
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *paStageDbPtr = NULL;
    GT_U8                                       devNum;
    GT_STATUS                                   rc;
    GT_U32                                      id = 0,i;
    GT_BOOL                                     found = GT_FALSE;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stage = 0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udsAttrPtr);

    if ((cpssOsStrCmp(udsAttrPtr->udsNameArr,"") == 0) || (cpssOsStrlen(udsAttrPtr->udsNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: uds name not valid");

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find uds id */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stage = i;
        id = stage - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udsArr[id].name,udsAttrPtr->udsNameArr) == 0)
        {
            found = GT_TRUE;
            break;
        }
    }

    if (found == GT_FALSE)
        return GT_OK;

    cpssOsMemSet(&(paMngDbPtr->udsArr[id]),0,sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC));

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stage,&paStageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* delete uds from bounded array */
    cpssOsMemSet(&(paStageDbPtr->boundInterface),0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

    return GT_OK;
}

/**
* @internal
*           cpssDxChPacketAnalyzerUserDefinedStageDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - user defined stage attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageDelete
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedStageDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udsAttrPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId, udsAttrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udsAttrPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPacketAnalyzerUserDefinedStageGet
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedStageGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      id = 0,i;
    GT_U8                                       devNum;
    GT_BOOL                                     found=GT_FALSE;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stage = 0;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udsAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(udsIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfaceAttrPtr);

    if ((cpssOsStrCmp(udsAttrPtr->udsNameArr,"") == 0) || (cpssOsStrlen(udsAttrPtr->udsNameArr) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: uds name not valid");

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find uds id */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stage = i;
        id = stage - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (cpssOsStrCmp(paMngDbPtr->udsArr[id].name,udsAttrPtr->udsNameArr) == 0)
        {
            found = GT_TRUE;
            break;
        }
    }

    if (found == GT_FALSE){
        *validPtr = GT_FALSE ;
        return GT_OK;
    }


    *udsIdPtr = stage;
    *validPtr = paMngDbPtr->udsArr[id].valid;

    if (paMngDbPtr->udsArr[id].valid == GT_TRUE)
        cpssOsMemCpy(interfaceAttrPtr,&(paMngDbPtr->udsArr[id].uds),sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageGet function
* @endinternal
*
* @brief   Get packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedStageGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udsAttrPtr, udsIdPtr, validPtr, interfaceAttrPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedStageGet(managerId, udsAttrPtr, udsIdPtr, validPtr, interfaceAttrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udsAttrPtr, udsIdPtr, validPtr, interfaceAttrPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerUserDefinedStageInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzeruser defined stages information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsId                    - user defined stage
*                                       identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] udsAttrPtr              - (pointer to) uds attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerUserDefinedStageInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     udsId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      id = 0;
    GT_U8                                       devNum;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_UDS_ID_CHECK_MAC(udsId);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(udsAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfaceAttrPtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    id = udsId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

    if (paMngDbPtr->udsArr[id].valid == GT_TRUE)
    {
        cpssOsStrCpy(udsAttrPtr->udsNameArr,paMngDbPtr->udsArr[id].name);
        cpssOsMemCpy(interfaceAttrPtr,&(paMngDbPtr->udsArr[id].uds),sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
    }

    *validPtr = paMngDbPtr->udsArr[id].valid;

    return GT_OK;
}

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzeruser defined stages information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsId                    - user defined stage
*                                       identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] udsAttrPtr              - (pointer to) uds attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     udsId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerUserDefinedStageInfoGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, udsId, validPtr, udsAttrPtr, interfaceAttrPtr));

    rc = internal_cpssDxChPacketAnalyzerUserDefinedStageInfoGet(managerId, udsId, validPtr, udsAttrPtr, interfaceAttrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, udsId, validPtr, udsAttrPtr, interfaceAttrPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet function
* @endinternal
*
* @brief   Get packet analyzer rule match attributes for all packet analyzer fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet
(
    IN    GT_U32                                          managerId,
    IN    GT_U32                                          ruleId,
    IN    GT_U32                                          groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                          *numOfHitsPtr,
    INOUT GT_U32                                          *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       sampleFieldsValueArr[]/*arrSizeVarName=numOfSampleFieldsPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U32                                      i,numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    GT_U8                                       devNum;
    GT_STATUS                                   rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         *fieldsArr = NULL;
    GT_U32                                      metaData,num = 1;
    GT_U32                                      dummy;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_VALID_CHECK_MAC(managerId,groupId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(searchAttributePtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfHitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfSampleFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(sampleFieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,paRuleDbPtr->paKeyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that packet analyzer is enabled on specified device */
    if (paMngDbPtr->devsPaEnable == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: PA is not enabled on device [%d]",devNum);

    /* array allocation */
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /* get all fields for stage */
    rc = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stageId,&numOfFields,fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    if (*numOfSampleFieldsPtr < numOfFields){
        *numOfSampleFieldsPtr = numOfFields;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num Of Fields [%d]",numOfFields);
    }
    *numOfSampleFieldsPtr = numOfFields;
    for (i=0; i<numOfFields; i++)
    {
        sampleFieldsValueArr[i].fieldName = fieldsArr[i];
    }

    if(searchAttributePtr->type==CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_PORT_E)
    {
        /*convert*/
        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,searchAttributePtr->portNum,
        &(searchAttributePtr->tile),&(searchAttributePtr->dp),&dummy,NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
            num = 2;
        searchAttributePtr->pipe= searchAttributePtr->dp/(MAX_DP_PER_TILE_CNS/MAX_PIPES_PER_TILE_CNS/num);
        searchAttributePtr->dp%=(MAX_DP_PER_TILE_CNS/MAX_PIPES_PER_TILE_CNS/num);

    }

    rc = prvCpssDxChIdebugEncodeMetaData(devNum,searchAttributePtr,&metaData);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Get packet analyzer rule match attributes for all packet analyzer fields */

    rc = prvCpssDxChPacketAnalyzerStageMatchDataGet(managerId, ruleId, groupId, stageId,
            metaData,numOfHitsPtr, numOfSampleFieldsPtr, sampleFieldsValueArr);

exit_cleanly_lbl:

    cpssOsFree(fieldsArr);

    return rc;

}


/**
* @internal cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet function
* @endinternal
*
* @brief   Get packet analyzer rule match attributes for all packet analyzer fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet
(
    IN    GT_U32                                           managerId,
    IN    GT_U32                                           ruleId,
    IN    GT_U32                                           groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                          *numOfHitsPtr,
    INOUT GT_U32                                          *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC        sampleFieldsValueArr[]/*arrSizeVarName=numOfSampleFieldsPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, ruleId, groupId,
        stageId,searchAttributePtr,numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr));

    rc = internal_cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId, ruleId, groupId,
        stageId,searchAttributePtr,numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, ruleId, groupId,
        stageId,searchAttributePtr,numOfHitsPtr,numOfSampleFieldsPtr,sampleFieldsValueArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer overlapping fields for spesific
*          field in stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in] fieldName                - field name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) fields
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
static GT_STATUS internal_cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                    paMngDbPtr = NULL;
    GT_U8                                                    devNum;
    GT_U32                                                   i,id,numOverlappingFields=0;
    GT_STATUS                                                rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC            interfaceId;
    mxml_node_t                                              *node;
    GT_U32                                                   endBit,startBit,endBitTemp,startBitTemp;
    GT_CHAR                                                  udfName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                                   numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      *interfaceFieldsArr = NULL;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    PRV_CPSS_PACKET_ANALYZER_FIELD_ID_CHECK_MAC(fieldName);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* get bounded interface */
    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&interfaceId);
    if (rc != GT_OK)
    {
        return rc;
    }

    interfaceFieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (interfaceFieldsArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate memory for managerId [%d] stageId [%s]",managerId,paStageStrArr[stageId]);
    }

    numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    rc = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stageId,&numOfFields,interfaceFieldsArr);
    if (rc != GT_OK)
    {
        cpssOsFree(interfaceFieldsArr);
        return rc;
    }

    /* get information for field */
    if ((fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
    {
        id = fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);
        if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",fieldName);

        cpssOsStrCpy(udfName,paMngDbPtr->udfArr[id].iDebugField);

        node = mxmlInterfaceFieldSizeGet(devNum,interfaceId.info.interfaceId,udfName,&startBit,&endBit);
        if(!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",udfName);
    }
    else
    {
        node = mxmlInterfaceFieldSizeGet(devNum,interfaceId.info.interfaceId,packetAnalyzerFieldToIdebugFieldArr[fieldName],&startBit,&endBit);
        if(!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",paFieldStrArr[fieldName]);
    }

    /* check field compared to other fields */
    for (i=0; i<numOfFields; i++)
    {
        if (interfaceFieldsArr[i] == fieldName)
            continue;

        if ((interfaceFieldsArr[i] >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (interfaceFieldsArr[i] <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
        {
            id = interfaceFieldsArr[i] - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);
            if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",interfaceFieldsArr[i]);

            cpssOsStrCpy(udfName,paMngDbPtr->udfArr[id].iDebugField);

            node = mxmlInterfaceFieldSizeGet(devNum,interfaceId.info.interfaceId,udfName,&startBitTemp,&endBitTemp);
            if(!node)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",udfName);
        }
        else
        {
            node = mxmlInterfaceFieldSizeGet(devNum,interfaceId.info.interfaceId,packetAnalyzerFieldToIdebugFieldArr[interfaceFieldsArr[i]],&startBitTemp,&endBitTemp);
            if(!node)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",paFieldStrArr[interfaceFieldsArr[i]]);
        }

        /* check overlapping bits */
        if ((startBit >= startBitTemp && startBit <= endBitTemp) ||
            (endBit >= startBitTemp && endBit <= endBitTemp) ||
            (startBitTemp >= startBit && startBitTemp <= endBit) ||
            (endBitTemp >= startBit && endBitTemp <= endBit))
        {
            /* add overlapping field to array */
            if (numOverlappingFields == *numOfFieldsPtr)
            {
                rc = GT_BAD_SIZE;
            }
            if (rc == GT_OK)
            {
                fieldsArr[numOverlappingFields] = interfaceFieldsArr[i];
                numOverlappingFields++;
            }
            else
                numOverlappingFields++;
        }
    }

    *numOfFieldsPtr = numOverlappingFields;

    return rc;
}

/**
* @internal
*           cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer overlapping fields for spesific
*          field in stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in] fieldName                - field name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) fields
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, managerId, stageId, fieldName, numOfFieldsPtr, fieldsArr));

    rc = internal_cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId, stageId, fieldName,numOfFieldsPtr,fieldsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, managerId, stageId, fieldName, numOfFieldsPtr, fieldsArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
