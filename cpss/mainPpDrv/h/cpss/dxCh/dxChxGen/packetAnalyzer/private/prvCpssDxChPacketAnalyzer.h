/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssDxChPacketAnalyzer.h
*
* @brief Private Packet Analyzer APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPacketAnalyzerh
#define __prvCpssDxChPacketAnalyzerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

    /* max number of logical keys */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS            1024

/* max number of PA rules */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS                   _2K

/* max number of PA managers */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS               10

/* max number of PA groups */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS                 1024

/* max number of PA actions */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_ACTIONS_NUM_CNS                128

/* max number of bytes for PA regular rule */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_BYTES_FOR_REGULAR_RULE_NUM_CNS 64

/* max number of bytes for PA extended rule */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_BYTES_FOR_EXT_RULE_NUM_CNS     256

/* max number of User defined fields */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_UDF_NUM_CNS                    64

/* number of stages in Aldrin2 */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN2_CNS          16

/* number of stages in Aldrin */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN_CNS           16

/* number of stages in Falcon */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_FALCON_CNS           18

/* number of stages in Hawk */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HAWK_CNS             18

/* number of stages in phoenix */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_PHOENIX_CNS          18

/* number of stages in harrier */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HARRIER_CNS          16

/* defines for internal usage of enumerations */
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_FIRST_E  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_LAST_E   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_LAST_E    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_FIRST_E                     CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E                      CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E                         CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E                          CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_63_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E                 CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E
#define PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E                  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E

/********************************************* DB **********************************/
/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC
 *
 *  @brief stage interface attributes
 *
*/
typedef struct
{
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC                       info;

    /* if interface is bounded to instance */
    GT_BOOL                                                             isValid;

}PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC
 *
 *  @brief packet analyzer user defined field information for DB
 *
*/
typedef struct
{
    /* udf is not used */
    GT_BOOL                                 valid;

    /* idebug field bounded to udf */
    GT_CHAR                                 iDebugField[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* udf name */
    GT_CHAR                                 name[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

}PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC
 *
 *  @brief packet analyzer user defined stage information for DB
 *
*/
typedef struct
{
    /* uds is not used */
    GT_BOOL                                         valid;

    /* idebug stage bounded to uds */
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   uds;

    /* uds name */
    GT_CHAR                                         name[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

}PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC
 *
 *  @brief packet analyzer user defined stage information for DB
 *
*/
typedef struct
{
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageName;
    /* if stage is bounded to instance */
    GT_BOOL                                             isValid;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC       boundInterface;
}PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_PTR;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC
 *
 *  @brief packet analyzer rule information for DB
 *
*/
typedef struct
{
    GT_U32                                              paRuleId;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC       paRuleAttr;
    GT_U32                                              paKeyId;
    GT_U32                                              paActId;
    GT_U32                                              numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC           *fieldsValueArr;
    GT_BOOL                                             *isFieldValueSetArr;

}PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC
 *
 *  @brief packet analyzer group information for DB
 *
*/
typedef struct
{
    GT_U32                                               paGrpId;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC       paGrpAttr;
    GT_U32                                               numOfRules;
    GT_BOOL                                              activateEnable;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          *stagesArr;

    /* rules array*/
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR               paRulePtrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS+1];

}PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC
 *
 *  @brief packet analyzer key information for DB
 *
*/
typedef struct
{
    GT_U32                                               paKeyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         paKeyAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          *stagesArr;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  *fieldsArr;
    /* only for 'partial fields' mode */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  **perStagesFieldsArr;
    GT_U32                                               *perStageNumOfFields;

}PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR;

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_STC
 *
 *  @brief packet analyzer action information for DB
 *
*/
typedef struct
{
    GT_U32                                               paActId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                 action;
    GT_U32                                               numOfGrps;

    /* groups array*/
    GT_U32                                               *paGrpPtrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS + 1];

} PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR;

/* bitmap of keys used by PA Manarer */
typedef GT_U32 PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_KEYS_BITMAP[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS/32 + 1];

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC
 *
 *  @brief packet analyzer device information for DB
 *
*/
typedef struct
{
    GT_U32                                                   paDevId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_PTR                 stagesArr;
    GT_BOOL                                                  samplingDisable;
    GT_BOOL                                                  packetMarkNeeded;

} PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR;

/* bitmap of devices used by PA Manarer */
typedef GT_U32 PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_DEVS_BITMAP[PRV_CPSS_MAX_PP_DEVICES_CNS/32 + 1];

/**
 * @struct PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC
 *
 *  @brief packet analyzer manager information for DB
 *
*/
typedef struct
{
    GT_U32                                                   paMngId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_DEVS_BITMAP            devsBitmap;
    CPSS_PP_FAMILY_TYPE_ENT                                  devFamily;
    GT_U32                                                   numOfDevices;
    GT_BOOL                                                  devsPaEnable;
    PRV_CPSS_DXCH_PACKET_ANALYZER_UDF_STC                    *udfArr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_UDS_STC                    *udsArr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_KEYS_BITMAP            keysBitmap;
    GT_U32                                                   numOfKeys;

    /* devices array*/
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR                    paDevPtrArr[PRV_CPSS_MAX_PP_DEVICES_CNS + 1];

    /* group array */
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR                    paGrpPtrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS + 1];

    /* action array */
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR                    paActPtrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_ACTIONS_NUM_CNS + 1];

    /* keys array*/
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyPtrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS + 1];

} PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_STC
    *PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR;


/* macro PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_SET_MAC
    to set a device in bitmap of devices in the PA manager database

  devsBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_DEVS_BITMAP device bitmap
  devNum - the device num to set in the device bitmap
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)/32] |= 1 << ((devNum) & 0xff)

/* macro PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_CLEAR_MAC
    to clear a device from the bitmap of devices in the PA manager database

  devsBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_DEVS_BITMAP device bitmap
  devNum - the device num to set in the device bitmap
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_DEV_BMP_CLEAR_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)/32] &= ~(1 << ((devNum) & 0xff))

/* macro PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC
    to check if device is set in the bitmap of devices in the PA manager database

  devsBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_DEVS_BITMAP device bitmap
  devNum - the device num to set in the device bitmap

  return 0 -- device not set in bmp
  return 1 -- device set in bmp
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (((devsBitmap)[(devNum)/32] & (1 << ((devNum) & 0xff))) ? 1 : 0)

/* macro PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_SET_MAC
    to set key id in bitmap of keys in the PA manager database

  keysBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_KEYS_BITMAP keys bitmap
  keyId - the key id to set in the keys bitmap
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_SET_MAC(keysBitmap, keyId)   \
    (keysBitmap)[(keyId)/32] |= 1 << ((keyId) & 0x7ff)

/* macro PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_CLEAR_MAC
    to clear key id from the bitmap of keys in the PA manager database

  keysBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_KEYS_BITMAP keys bitmap
  keyId - the key id to clear in the keys bitmap
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_KEY_BMP_CLEAR_MAC(keysBitmap, keyId)   \
    (keysBitmap)[(keyId)/32] &= ~(1 << ((keyId) & 0x7ff))

/* macro PRV_CPSS_PACKET_ANALYZER_MNG_IS_KEY_BMP_SET_MAC
    to check if key id is set in the bitmap of keys in the PA manager database

  keysBitmap - of type PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_KEYS_BITMAP keys bitmap
  keyId - the key id to check in the keys bitmap

  return 0 -- key not set in bmp
  return 1 -- key set in bmp
*/
#define PRV_CPSS_PACKET_ANALYZER_MNG_IS_KEY_BMP_SET_MAC(keysBitmap, keyId)   \
    (((keysBitmap)[(keyId)/32] & (1 << ((keyId) & 0x7ff))) ? 1 : 0)

/* static array to convert from PA field to iDebug field */
extern const GT_CHAR_PTR packetAnalyzerFieldToIdebugFieldArr[PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1];

extern const char* paStageStrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1];
extern const char* paFieldStrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1];

/* check that the udfId is in range */
#define PRV_CPSS_PACKET_ANALYZER_UDF_ID_CHECK_MAC(udfId)                    \
    if ( (udfId < PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) ||           \
         (udfId > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E ))             \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);      \
    }

/* check that the udsId is in range */
#define PRV_CPSS_PACKET_ANALYZER_UDS_ID_CHECK_MAC(udsId)                    \
    if ( (udsId < PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) ||   \
         (udsId > PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))     \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);      \
    }

/* check that the stageId is in range */
#define PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId)                            \
    if (stageId > PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)               \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/* check that the stageId is valid for uds */
#define PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId)                         \
    if ((stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) && (stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E )) \
    {                                                                                   \
        if (prvCpssDxChPacketAnalyzerUserDefinedValidCheck(managerId,stageId) != GT_OK) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);              \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,stageId) == GT_FALSE) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);              \
    }


/* check that the device support DFX server */
#define PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum)     \
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE) {   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);     \
    }

/* check that DFX is not supported in simulation */
#define PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC()                        \
    do                                                                              \
    {                                                                               \
        if(paIsSimulation()){                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);      \
        }                                                                           \
    }while(0);

/* check that stage is supported in device ,skip if not */
#define PRV_CPSS_PACKET_ANALYZER_IS_STAGE_SUPPORTED_MAC(managerId,devNum,stageId)   \
    if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(managerId,devNum,stageId) == GT_FALSE) \
        continue;

/* Prevent warning - return GT_TRUE in case of simulation run */
GT_BOOL paIsSimulation
(
    GT_VOID
);

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerIsStageApplicableInDev function
 * @endinternal
 *
 * @brief  check if stage is applicable in device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] devNum               - device number
 * @param[in] stageId              - stageId
 *
 * @retval GT_TRUE                 - in case stage is applicable in device
 * @retval GT_FALSE                - in case stage is not applicable in device
 */
GT_BOOL prvCpssDxChPacketAnalyzerIsStageApplicableInDev
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId
);

/**
* @internal prvCpssDxChPacketAnalyzerDbDeviceGet function
* @endinternal
*
* @brief   The function returns pointer to devNum in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[out] paDevDbPtr    - (pointer to) PA device structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbDeviceGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       *paDevDbPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbManagerGet function
* @endinternal
*
* @brief   The function returns pointer to manager in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[out] paMngDbPtr    - (pointer to) PA manager structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbManagerGet
(
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       *paMngDbPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbActionGet function
* @endinternal
*
* @brief   The function returns pointer to action in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in] actionId       - rule action identification
*                             (APPLICABLE RANGES:1..128)
* @param[out] paActDbPtr    - (pointer to) PA action structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       *paActDbPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbKeyGet function
* @endinternal
*
* @brief   The function returns pointer to key in packet analyzer manger DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[in]  keyId         - logical key identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] paKeyDbPtr    - (pointer to) PA key in DB
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbKeyGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          keyId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR           *paKeyDbPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbRuleGet function
* @endinternal
*
* @brief   The function returns pointer to rule in packet analyzer group DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId       - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  ruleId         - rule identification
*                             (APPLICABLE RANGES: 1..0xFFFFFFFF)
* @param[in]  groupId        - group identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] paRuleDbPtr    - (pointer to) PA rule in DB
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbRuleGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          ruleId,
    IN  GT_U32                                          groupId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR          *paRuleDbPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbStageGet function
* @endinternal
*
* @brief   The function returns pointer to stage in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  managerId     - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[in]  stageId       - stage identification
* @param[out] stagePtr      - (pointer to) PA stage
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbStageGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC          **stagePtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbDevGet function
* @endinternal
*
* @brief   The function returns pointer to device number
*          attached to key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId       - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  keyId          - logical key identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] devNumPtr      - (pointer to) device number
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_NOT_FOUND      - on not found parameters
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbDevGet
(
    IN  GT_U32                               managerId,
    IN  GT_U32                               keyId,
    OUT  GT_U8                               *devNumPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerDbManagerValidCheck function
* @endinternal
*
* @brief   The function checks if manager id was initialized.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
*
* @retval status            - status of manager id
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbManagerValidCheck
(
    IN  GT_U32                               managerId
);

/**
* @internal prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet
*           function
* @endinternal
*
* @brief   Enable/disable DFX power save mode for all connected
*          XSB units.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable DFX power
*                                       save mode
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet
(
    IN  GT_U8                                       devNum,
    IN  GT_BOOL                                     enable
);

/**
* @internal prvCpssDxChPacketAnalyzerStageInterfacesSet function
* @endinternal
*
* @brief   Set stage idebug interfaces attributes list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[in] boundInterface           - bounded interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesSet
(
    IN GT_U32                                           managerId,
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC boundInterface
);

/**
* @internal prvCpssDxChPacketAnalyzerStageInterfacesGet function
* @endinternal
*
* @brief   Get stage idebug interfaces attributes list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[out] boundInterfacePtr       - (pointer to) bounded
*                                       interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  *boundInterfacePtr
);

/**** DUMP APIs for packet analyzer *******/

/**
* @internal prvCpssDxChPacketAnalyzerStageFieldsGet function
* @endinternal
*
* @brief   Get stage fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for stage
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       stage
* @param[out] fieldsArr               - (pointer to) stage
*                                       fields list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC                    fieldsArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerInstanceInterfacesGet function
* @endinternal
*
* @brief   Get instance interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] instanceId               - instance identification
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces for
*                                       instance
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       instance
* @param[out] interfacesArr           - (pointer to) instance
*                                       interface list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInstanceInterfacesGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   instanceId,
    INOUT GT_U32                                        *numOfInterfacesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC interfacesArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerFieldInterfacesGet function
* @endinternal
*
* @brief   Get field interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] fieldName                - field identification
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces for
*                                       field
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       field
* @param[out] interfaceArr            - (pointer to) field
*                                       interface list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldInterfacesGet
(
    IN    GT_U32                                  managerId,
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             fieldName,
    INOUT GT_U32                                  *numOfInterfacesPtr,
    OUT   GT_CHAR                                 *interfacesArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerInterfacesGet function
* @endinternal
*
* @brief   Get interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces
*                                       out: (pointer to) actual
*                                       number of interfaces
* @param[out] interfaceArr            - (pointer to) interface
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfacesGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfInterfacesPtr,
    OUT   GT_CHAR                                   **interfacesArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerInstancesGet function
* @endinternal
*
* @brief   Get instances list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfInstancesPtr    - in: (pointer to)
*                                       allocated number of
*                                       instances
*                                       out: (pointer to) actual
*                                       number of instances
* @param[out] instancesArr            - (pointer to) instance
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInstancesGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfInstancesPtr,
    OUT   GT_CHAR                                   **instancesArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerFieldsGet function
* @endinternal
*
* @brief   Get fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - (pointer to) fields
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldsGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfFieldsPtr,
    OUT   GT_CHAR                                   **fieldsArr[]
);

/**
* @internal prvCpssDxChPacketAnalyzerInterfaceInfoGet function
* @endinternal
*
* @brief   Get interface stage and instance.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[out] stageIdPtr              - (pointer to) stage
*                                       identification
* @param[out] instanceIdPtr           - (pointer to) instance
*                                       identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfaceInfoGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   interfaceName,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   *stageIdPtr,
    OUT   GT_CHAR                                       *instanceIdPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerFieldSizeGet function
* @endinternal
*
* @brief   Get packet analyzer field's size.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] lengthPtr               - (pointer to) fields
*                                       length in bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *lengthPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerUserDefinedValidCheck
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined stage validity.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsId                    - uds identification
*
* @retval GT_OK                    - on valid
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on non valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS prvCpssDxChPacketAnalyzerUserDefinedValidCheck
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      udsId
);

GT_STATUS prvCpssDxChPacketAnalyzerMarkMuxedStagesSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT *stagesArr,
    OUT GT_U32                                      *muxValidBmpPtr
);

/**
* @internal prvCpssDxChIdebugSelectInerfaceByPriority function
* @endinternal
*
* @brief   Select specific mux based on defined priority
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] currentBus               - instance
* @param[in] priority                 - muxed interface priority
* @param[out] muxOutPtr               - (pointer to) muxed interface index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugSelectInerfaceByPriority
(
    GT_U8                                       devNum,
    GT_CHAR_PTR                                 currentBus,
    GT_U32                                      priority,
    GT_U32                                      *muxOutPtr
);

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerStageValiditySet function
 * @endinternal
 *
 * @brief   The function set stage validity in case of mux
 *          between interfaces.
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] managerId                - manager identification
 *                                       (APPLICABLE RANGES: 1..10)
 * @param[in] devNum                   - device number
 * @param[in] stageId                  - stage identification
 * @param[in] isValid                  - interface validity
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE             - on invalid parameter
 */
GT_STATUS prvCpssDxChPacketAnalyzerStageValiditySet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId,
    IN  GT_BOOL                                         isValid
);

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet
 *           function
 * @endinternal
 *
 * @brief   Get bounded interfaces of specific device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] devNum                    - device number
 * @param[out] boundedInterfacesArrPtr  - (pointer to) bounded
 *                                        interfaces of specific
 *                                        device
 * @param[out] sizePtr                  - (pointer to) size of
 *                                        array
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE             - on invalid parameter
 */
GT_STATUS prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet
(
    IN   GT_U8                                              devNum,
    OUT  PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  *boundedInterfacesArrPtr,
    OUT  GT_U32                                             *sizePtr
);

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerStageOrderGet function
 * @endinternal
 *
 * @brief  Get stage by order index for specific device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] managerId             - manager identification
 *                                   (APPLICABLE RANGES: 1..10)
 * @param[in]  devNum               - device number
 * @param[in]  index                - index of wanted stage
 * @param[in]  offset               - offset to reduce
 * @param[out] stageIdPtr           - (pointer to)stage match to index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_SUPPORTED         - on not supported parameter
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssDxChPacketAnalyzerStageOrderGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    IN  GT_U32                                          offset,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *stageIdPtr
);

/**
* @internal prvCpssDxChPacketAnalyzerFirstDevGet function
* @endinternal
*
* @brief   The function returns pointer to first device number
*          in packet analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[out] devNumPtr     - (pointer to) device number
*
* @retval GT_OK             - on success
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
* @retval GT_FAIL           - on error
*/
GT_STATUS prvCpssDxChPacketAnalyzerFirstDevGet
(
    IN  GT_U32                               managerId,
    OUT  GT_U8                               *devNumPtr
);

/**
* @internal
*           prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet
*           function
* @endinternal
*
* @brief   The function set default choosed interfaces.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] boundedInterfacesArr     - (pointer to) bounded
*                                       interfaces array
* @param[in]size                      - size of bounded
*                                       interfaces array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet
(
    IN  GT_U32                                              managerId,
    IN  GT_U8                                               devNum,
    IN  PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *boundedInterfacesArr,
    IN  GT_U32                                              size
);

/**
* @internal prvCpssDxChPacketAnalyzerInterfaceStatusGet function
* @endinternal
*
* @brief   Check if Interface is bounded to a stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[out] isBoundedPtr            - (pointer to) whether
*                                       interface is bounded to
*                                       a stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfaceStatusGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName,
    OUT GT_BOOL                                     *isBoundedPtr
);

GT_STATUS prvCpssDxChPacketAnalyzerDumpSwCounters
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stage
);

/**
* @internal
*           prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable
*           function
* @endinternal
*
* @brief   Disable sampling on all used interfaces in device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
GT_STATUS prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
);

/**
* @internal
*           prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields
*           function
* @endinternal
*
* @brief   Check no overlapping fields in rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
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
GT_STATUS prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[]
);

/**
 * @internal prvCpssDxChPacketAnalyzerIsXmlPresent function
 * @endinternal
 *
 * @brief  check if xml is present for device
 *
 * @param[in] devNum               - device number
 *
 * @retval GT_TRUE                 - in case xml is present
 * @retval GT_FALSE                - in case xml is not present
 */
GT_BOOL prvCpssDxChPacketAnalyzerIsXmlPresent
(
    IN GT_U8       devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPacketAnalyzerh */
