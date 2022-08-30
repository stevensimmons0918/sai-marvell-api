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
* @file prvCpssDxChiDebug.h
*
* @brief iDebug APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChIdebugh
#define __prvCpssDxChIdebugh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>

#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>


#define PRV_DFX_XSB_TILE_OFFSET_MAC(_devNum,_tileId)                (PRV_CPSS_SIP_6_CHECK_MAC(_devNum)?\
                                                                     prvCpssSip6TileOffsetGet(_devNum,_tileId):0)
#define META_DATA_ALL_CNS 0xFFFFFFFF

#define PRV_CPSS_DXCH_IDEBUG_META_DATA_VALID_OFFSET_CNS      31
#define PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS     30
#define PRV_CPSS_DXCH_IDEBUG_META_DATA_BASE_OFFSET_CNS       12
#define PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS       8
#define PRV_CPSS_DXCH_IDEBUG_META_DATA_PIPE_OFFSET_CNS       4
#define PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS         0
#define PRV_CPSS_DXCH_IDEBUG_META_INVALID_IDX_CNS            0xF

#define PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS                 8

#ifdef ASIC_SIMULATION

#define     PRV_CPSS_ALDRIN_IDEBUG_XML_FILE   "iDebug_aldrin.xml"
#define     PRV_CPSS_ALDRIN2_IDEBUG_XML_FILE  "iDebug_aldrin2.xml"
#define     PRV_CPSS_FALCON_IDEBUG_XML_FILE   "iDebug_falcon.xml"
#define     PRV_CPSS_HAWK_IDEBUG_XML_FILE     "iDebug_ac5p.xml"
#define     PRV_CPSS_PHOENIX_IDEBUG_XML_FILE  "iDebug_ac5x.xml"
#define     PRV_CPSS_HARRIER_IDEBUG_XML_FILE  "iDebug_harrier.xml"
#else
#define     PRV_CPSS_ALDRIN_IDEBUG_XML_FILE   "/usr/bin/iDebug_aldrin.xml"
#define     PRV_CPSS_ALDRIN2_IDEBUG_XML_FILE  "/usr/bin/iDebug_aldrin2.xml"
#define     PRV_CPSS_FALCON_IDEBUG_XML_FILE   "/usr/bin/iDebug_falcon.xml"
#define     PRV_CPSS_HAWK_IDEBUG_XML_FILE     "/usr/bin/iDebug_ac5p.xml"
#define     PRV_CPSS_PHOENIX_IDEBUG_XML_FILE  "/usr/bin/iDebug_ac5x.xml"
#define     PRV_CPSS_HARRIER_IDEBUG_XML_FILE  "/usr/bin/iDebug_harrier.xml"
#endif/*#ifdef ASIC_SIMULATION*/

#define PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(_node)\
     {\
        if(_node->type != MXML_ELEMENT)\
        {\
            _node = _node->next;\
            continue;\
        }\
      }


/* max number of families supported in iDebug DB */
#define PRV_CPSS_DXCH_IDEBUG_MAX_FAMILIES_CNS                   5

#define MAX_MULTIPLICATIONS_NUM 128

/* check that the managerId is in range */
#define PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId)                         \
    if (managerId > PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS || managerId == 0)\
    {                                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                   \
    }

/* check that the interfaceId is in range */
#define PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceId)         \
    if (GT_OK != prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceId))        \
    {                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* check that the iDebugField is in range */
#define PRV_CPSS_PACKET_ANALYZER_IDEBUG_FIELD_ID_CHECK_MAC(devNum,iDebugField)      \
    if (GT_OK != prvCpssDxChIdebugFieldDeviceCheck(devNum, iDebugField))            \
    {                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* check that the instanceId is in range */
#define PRV_CPSS_PACKET_ANALYZER_INSTANCE_ID_CHECK_MAC(devNum,instanceId)           \
    if (GT_OK != prvCpssDxChIdebugInstanceDeviceCheck(devNum, instanceId))    \
    {                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* check that the keyId is in range */
#define PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId)                                \
    if (keyId >  PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS || keyId == 0)   \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/* check that the ruleId is in range */
#define PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId)                              \
    if (ruleId > PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS || ruleId == 0)         \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/* check that the actionId is in range */
#define PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId)                          \
    if (actionId > PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_ACTIONS_NUM_CNS || actionId == 0)  \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/* check that the groupId is in range */
#define PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId)                            \
    if (groupId >  PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS || groupId == 0)    \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/* return first device for managerId */
#define PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId, devNum)                \
    if (GT_OK != prvCpssDxChPacketAnalyzerFirstDevGet(managerId, devNum))               \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
    }

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_ENUM_STC
 *
 *  @brief idebug information to support enumerstions in LUA
 *
*/
typedef struct
{
    GT_CHAR                                     name[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    GT_U32                                      value;

}PRV_CPSS_DXCH_IDEBUG_ENUM_STC;

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC
 *
 *  @brief idebug information to support 'ClearOnRead' operation for SIP5
 *
*/
typedef struct
{
    /*The index of the last multiplication that had hit count greater then zero*/
    GT_U32                                              multInd;
    /* previous value before reset  */
    GT_U32                                              matchCounterValue[MAX_MULTIPLICATIONS_NUM];
    /* For SIP 6 only*/
    GT_U32                                              matchCounterMetaData[MAX_MULTIPLICATIONS_NUM];

}PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC
    *PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR;

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_FIELD_STC
 *
 *  @brief iDebug field name and length
 *
*/
typedef struct{

    /* field name */
    GT_CHAR                                     fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* field start bit */
    GT_U32                                      startBit;

    /* field end bit */
    GT_U32                                      endBit;

}PRV_CPSS_DXCH_IDEBUG_FIELD_STC;

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC
 *
 *  @brief iDebug field name and its content (Data and mask)
 *
*/
typedef struct{

    /* field name */
    GT_CHAR                                     fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* field data */
    GT_U32                                      data[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

    /* field mask */
    GT_U32                                      mask[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

    /* field validity to support muxing of fields */
    GT_BOOL                                     isValid;
}PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC;

/* CB functions */
typedef GT_STATUS (*IDEBUG_FIELD_VALIDITY_LOGIC_UPDATE_CB_FUNC)
(
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName,
    IN  GT_CHAR_PTR                                 fieldName,
    IN  GT_U32                                      numOfFields,
    INOUT  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
);

typedef struct {
    IDEBUG_FIELD_VALIDITY_LOGIC_UPDATE_CB_FUNC       fieldValidityLogicUpdateCbFunc;
} PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC;

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_FAMILY_STC
 *
 *  @brief idebug information for family's pointers
 *
*/
typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT                                  familyId;
    PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC                    *cbFunc;
    CPSS_OS_FILE_TYPE_STC                                    *file;
    mxml_node_t                                              *head;
    GT_U32                                                    **muxDb;

} PRV_CPSS_DXCH_IDEBUG_FAMILY_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_IDEBUG_FAMILY_STC
    *PRV_CPSS_DXCH_IDEBUG_FAMILY_PTR;

/**
 * @struct PRV_CPSS_DXCH_IDEBUG_DB_STC
 *
 *  @brief idebug information for DB
 *
*/
typedef struct
{
    GT_U32                                                    numOfFamilies;
    GT_U32                                                    numOfDevices;
    /* families array*/
    PRV_CPSS_DXCH_IDEBUG_FAMILY_PTR                           idebugFamilyPtrArr[PRV_CPSS_DXCH_IDEBUG_MAX_FAMILIES_CNS];

    /* devices array */
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR                        *idebugInterfacePtrArr;

    /* strings array */
    GT_CHAR_PTR                                               *idebugInterfacesStrPtrArr;
    GT_CHAR_PTR                                               *idebugFieldsStrPtrArr;
    GT_BOOL                                                   *idebugFieldsIsEnumStrPtrArr;
    GT_CHAR_PTR                                               *idebugInstancesStrPtrArr;

    GT_U32                                                    numOfInterfaces;
    GT_U32                                                    numOfFields;
    GT_U32                                                    numOfInstances;

    /* power save mode */
    GT_BOOL                                                   powerSaveEnable;

}PRV_CPSS_DXCH_IDEBUG_DB_STC;

/**
* @internal prvCpssDxChIdebugActionCheck function
* @endinternal
*
* @brief   Check that action parameters are valid.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] actionPtr                - (pointer to) action structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS prvCpssDxChIdebugActionCheck
(
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC     *actionPtr
);

/**
* @internal mxmlFindInterfaceFirstFieldNode function
* @endinternal
*
* @brief   Get pointer to interface's first field.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval mxml_node_t                 - (pointer to) interface's first field.
*/
mxml_node_t * mxmlFindInterfaceFirstFieldNode
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName
);

/**
* @internal mxmlFindInstanceFirstInterfaceNode function
* @endinternal
*
* @brief   Get pointer to instance's first interface (bounded interface).
*
* @param[in] devNum                   - device number
* @param[in] instanceId               - instance identification
* @param[in] noInterfacePtr           - if instance have no interface
*
* @retval mxml_node_t                 - (pointer to) instance's first interface.
*/
mxml_node_t * mxmlFindInstanceFirstInterfaceNode
(
    IN  GT_U8                                     devNum,
    IN  GT_CHAR_PTR                               instanceId,
    OUT GT_BOOL                                   *noInterfacePtr
);

/**
* @internal mxmlInterfaceFieldSizeGet function
* @endinternal
*
* @brief   Get pointer to field in interface and return it's start/end bit.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] fieldName                - field name
* @param[out] startBitPtr             - (pointer to) field's start bit
* @param[out] endBitPtr               - (pointer to) field's end bit
*
* @retval mxml_node_t                 - (pointer to) field in interface.
*/
mxml_node_t * mxmlInterfaceFieldSizeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *startBitPtr,
    OUT GT_U32                                  *endBitPtr
);

/**
* @internal mxmlFieldEnumsGet function
* @endinternal
*
* @brief   Get pointer to field and return it's enumerations.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] numOfEnumsPtr           - (pointer to) field's
*                                       number of enums
* @param[out] enumsArr                - (pointer to) field's
*                                       enums
*
* @retval mxml_node_t                 - (pointer to) field in interface.
*/
mxml_node_t * mxmlFieldEnumsGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *numOfEnumsPtr,
    OUT PRV_CPSS_DXCH_IDEBUG_ENUM_STC           enumsArr[]
);

/**
* @internal mxmlInterfaceSizeGet function
* @endinternal
*
* @brief   Get pointer to interface and return it's size.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] sizePtr                 - (pointer to) interface size. In case of concatenated
*                                        interface this size include all the parts
* @param[out] startBitPtr             - (pointer to) interface start offset.
*                                       Relevant for concatenated interfaces
* @retval mxml_node_t                 - (pointer to) interface.
*/
mxml_node_t * mxmlInterfaceSizeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_U32                                  *sizePtr,
    OUT GT_U32                                  *startBitPtr
);

/**
* @internal mxmlInterfacePolarityGet function
* @endinternal
*
* @brief   Get pointer to interface and return it's polarity.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] polarityPtr             - (pointer to) interface
*                                       polarity
*
* @retval mxml_node_t                 - (pointer to) interface.
*/
mxml_node_t * mxmlInterfacePolarityGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_CHAR                                 *polarityPtr
);

/**
* @internal mxmlInterfaceFieldSizeGet function
* @endinternal
*
* @brief   Get pointer to interface's stage identification.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval mxml_node_t                 - (pointer to) interface's stage id.
*/
mxml_node_t * mxmlInterfaceStageGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *stagePtr
);

/**
* @internal mxmlNumOfInterfacesGet function
* @endinternal
*
* @brief   Get pointer to first interface and return number of interfaces for device.
*
* @param[in] devNum                 - device number
* @param[out] numOfInterfacesPtr    - (pointer to) num of
*                                     interfaces.
*
* @retval mxml_node_t               - (pointer to) first interface.
*/
mxml_node_t * mxmlNumOfInterfacesGet
(
    GT_U8           devNum,
    OUT GT_U32      *numOfInterfacesPtr
);

/**
* @internal mxmlInstanceInfoGet function
* @endinternal
*
* @brief   Get pointer to interface's instance and return instance information regurding interface.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] multiplicationIndex      - in case of multiplied interface this variable holds
*                                       the index of required multiplication.
* @param[out] instanceIdPtr           - (pointer to) instance
*                                       identification.
* @param[out] instanceDfxPipeIdPtr    - (pointer to) instance
*                                       dfx pipe.
* @param[out] instanceDfxBaseAddrPtr  - (pointer to) instance
*                                       dfx base address.
* @param[out] interfaceDfxIndexPtr     - (pointer to) interface
*                                       index in dfx.
*
* @retval mxml_node_t                 - (pointer to)
*                                       interface's instance.
*/
mxml_node_t * mxmlInstanceInfoGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          multiplicationIndex,
    OUT GT_CHAR                                         *instanceIdPtr,
    OUT GT_U32                                          *instanceDfxPipeIdPtr,
    OUT GT_U32                                          *instanceDfxBaseAddrPtr,
    OUT GT_U32                                          *interfaceDfxIndexPtr,
    OUT GT_U32                                          *metaDataPtr
);

/**
* @internal mxmlBaseAddrInstanceGet function
* @endinternal
*
* @brief   Gets instance name by base address and dfx pipe.
*
* @param[in] devNum             - device number
* @param[in] baseAddr           - base address of searched instance .
* @param[out] dfxPipe           - dfx Pipe of searched instance
* @param[out] instanceIdPtr     - (pointer to) instance name.
*/
mxml_node_t * mxmlBaseAddrInstanceGet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        baseAddr,
    IN   GT_U32                                        dfxPipe,
    OUT  GT_CHAR                                       *instanceIdPtr
);

/**
* @internal prvCpssDxChIdebugFieldDeviceCheck function
* @endinternal
*
* @brief   Check if the field is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFieldDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         fieldName
);

/**
* @internal prvCpssDxChIdebugFieldIsEnumCheck function
* @endinternal
*
* @brief   Check if the field is enum in the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] isEnumPtr               - whether field is enum
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFieldIsEnumCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         fieldName,
    OUT GT_BOOL                             *isEnumPtr
);

/**
* @internal prvCpssDxChIdebugInterfaceDeviceCheck function
* @endinternal
*
* @brief   Check if the interface is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugInterfaceDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         interfaceName
);

/**
* @internal prvCpssDxChIdebugInstanceDeviceCheck function
* @endinternal
*
* @brief   Check if the instance is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] instanceName             - instance name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         instanceName
);

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck
*           function
* @endinternal
*
* @brief   Check that sampling on interface is not enabled.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck
(
    IN  GT_U8                                            devNum,
    IN  GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN  GT_CHAR_PTR                                      interfaceName
);

/**
* @internal prvCpssDxChIdebugFileOpen function
* @endinternal
*
* @brief   Open XML file according to device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChIdebugFileOpen
(
    IN  GT_U8                                   devNum
);

/**
* @internal prvCpssDxChIdebugFileClose function
* @endinternal
*
* @brief   Close XML file according to device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChIdebugFileClose
(
    IN  GT_U8                                   devNum
);

/**
* @internal prvCpssDxChIdebugAllInterfacesSamplingDisable
*           function
* @endinternal
*
* @brief   Disable sampling on all interfaces in device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
GT_STATUS prvCpssDxChIdebugAllInterfacesSamplingDisable
(
    IN  GT_U8                                   devNum
);

/**
* @internal prvCpssDxChIdebugDbPowerSaveSet function
* @endinternal
*
* @brief   The function set power save in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum        - device number
* @param[in]  enable        - enable/disable power save mode
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbPowerSaveSet
(
    IN  GT_U8                                           devNum,
    IN  GT_BOOL                                         enable
);

/**
* @internal prvCpssDxChIdebugDbCbGet function
* @endinternal
*
* @brief   The function returns pointer to CB function in iDebug
*          DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum        - device number
* @param[out] idebugCbDbPtr - (pointer to) CB function in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbCbGet
(
    IN  GT_U8                                           devNum,
    OUT PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC           **idebugCbDbPtr
);

/**
* @internal prvCpssDxChIdebugDbHeadGet function
* @endinternal
*
* @brief   The function returns pointer to head of MXML node
*          tree in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum          - device number
* @param[out] idebugHeadDbPtr - (pointer to) head of MXML node
*                               tree in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbHeadGet
(
    IN  GT_U8                                           devNum,
    OUT mxml_node_t                                     **idebugHeadDbPtr
);

/**
* @internal prvCpssDxChIdebugDbInterfaceArrayGet function
* @endinternal
*
* @brief   The function returns pointer to interfaces string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfInterfacesPtr       - num of interfaces in DB
* @param[out] idebugInterfacesPtr      - (pointer to) interfaces
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInterfaceArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInterfacesPtr,
    OUT GT_CHAR_PTR                                     **idebugInterfacesPtr
);

/**
* @internal prvCpssDxChIdebugDbInstanceArrayGet function
* @endinternal
*
* @brief   The function returns pointer to instances string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfInstancesPtr       - num of interfaces in DB
* @param[out] idebugInstancesPtr      - (pointer to) instances
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInstanceArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInstancesPtr,
    OUT GT_CHAR_PTR                                     **idebugInstancesPtr
);

/**
* @internal prvCpssDxChIdebugDbFieldsArrayGet function
* @endinternal
*
* @brief   The function returns pointer to fields string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfFieldssPtr          - num of fields in DB
* @param[out] idebugFieldsPtr          - (pointer to) fields
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbFieldsArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfFieldsPtr,
    OUT GT_CHAR_PTR                                     **idebugFieldsPtr
);

/**
* @internal prvCpssDxChIdebugDbInterfaceGet function
* @endinternal
*
* @brief   The function returns pointer to interface structure
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] idebugInterfaceDbPtr - (pointer to) idebug
*                                   interface structure in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInterfaceGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR              *idebugInterfaceDbPtr
);

/**
* @internal prvCpssDxChIdebugDbNumInterfacesGet function
* @endinternal
*
* @brief   The function returns number of interfaces
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfInterfacesPtr   - (pointer to) number
*                                    of interfaces in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumInterfacesGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInterfacesPtr
);

/**
* @internal prvCpssDxChIdebugDbNumInstancesGet function
* @endinternal
*
* @brief   The function returns number of instances
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfInstancesPtr   - (pointer to) number
*                                    of instances in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumInstancesGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInstancesPtr
);

/**
* @internal prvCpssDxChIdebugDbNumFieldsGet function
* @endinternal
*
* @brief   The function returns number of fields
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfFieldsPtr       - (pointer to) number
*                                    of fields in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumFieldsGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfFieldsPtr
);

/**
* @internal prvCpssDxChIdebugInterfaceNumFieldsGet function
* @endinternal
*
* @brief   Get interface number of fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfFieldsPtr          - (pointer to) number of
*                                       fields for interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugInterfaceNumFieldsGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_U32                                  *numOfFieldsPtr
);

/**
* @internal prvCpssDxChIdebugInterfaceFieldsGet function
* @endinternal
*
* @brief   Get interface fields and sizes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldArr                - interface fields array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugInterfaceFieldsGet
(
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             interfaceName,
    INOUT GT_U32                                  *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC          fieldsArr[]
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSet function
* @endinternal
*
* @brief   Set interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[in] fieldsValueArr           - interface fields values
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
);

/**
* @internal prvCpssDxChIdebugPortGroupSingleInterfaceSet function
* @endinternal
*
* @brief   Set interface fields for single instance address.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] instanceDfxPipeId        - instance Dfx PipeId
* @param[in] instanceDfxBaseAddr      - instance Dfx BaseAddr
* @param[in] interfaceDfxIndex        - interface Dfx Index
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[in] fieldsValueArr           - interface fields values
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupSingleInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  instanceDfxPipeId,
    IN  GT_U32                                  instanceDfxBaseAddr,
    IN  GT_U32                                  interfaceDfxIndex,
    IN  GT_U32                                  numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceGet function
* @endinternal
*
* @brief   Get interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldsValueArr          - interface fields values
*                                       array (Data and Mask)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp,
    IN GT_CHAR_PTR                                  interfaceName,
    INOUT GT_U32                                    *numOfFieldsPtr,
    OUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    fieldsValueArr[]
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceReset function
* @endinternal
*
* @brief   Reset interface in hardware.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceReset
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceResetAll function
* @endinternal
*
* @brief   Reset all interface in hardware.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceResetAll
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceActionSet
*           function
* @endinternal
*
* @brief   Set interface action configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] actionPtr                - (pointer to) interface
*                                       action configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionSet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceActionGet
*           function
* @endinternal
*
* @brief   Get interface action configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[out] actionPtr               - (pointer to) interface
*                                       action configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet
*           function
* @endinternal
*
* @brief   Get number of hits for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] metaData                 - search attribute data
*                                       (Applicable device:Falcon; AC5P)
* @param[in] interfaceName            - interface name
* @param[in] clearOnReadEnable        - enable/disable clear
*                                       on read for counter data
* @param[out] valuePtr                - (pointer to) number of
*                                       counter hits.
* @param[out] multndPtr               - (pointer to) in case of multiplied interface this variable holds
*                                       the index of multiplication that has a hit count greater then 0.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          metaData,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_BOOL                                         clearOnReadEnable,
    OUT GT_U32                                          *valuePtr,
    OUT GT_U32                                          *multndPtr
);

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet
*           function
* @endinternal
*
* @brief   Get sampled data all fields for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldsValueArr          - interface fields values
*                                       array.
*                                       note:
*                                       fieldsValueArr[].mask is
*                                       not applicable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
);

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet
*           function
* @endinternal
*
* @brief   Get sampled data for requested list of fields for
*          interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @param[in] numOfFields              - number of fields
*                                       requested for interface
* @param[in,out] fieldsValueArr       - in: list of requested
*                                       fields for interface
*                                       out: interface requested
*                                       fields values array
*                                       note:
*                                       fieldsValueArr[].mask is
*                                       not applicable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note clearOnRead will clear values on all interface fields.
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName,
    IN    GT_U32                                        numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet
*           function
* @endinternal
*
* @brief   Enable/Disable sampling for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] enable                   - enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_CHAR_PTR                                      interfaceName,
    IN GT_BOOL                                          enable
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet
*           function
* @endinternal
*
* @brief   Get sampling status (enable/disable) for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[out] enablePtr               - (pointer to)
*                                       enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN  GT_CHAR_PTR                                      interfaceName,
    OUT GT_BOOL                                          *enablePtr
);

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSampleDump
*           function
* @endinternal
*
* @brief   Dump sample result for interface
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum              -  device number
* @param[in] metaData            -  info that contain tile/cp/dp (Applicable device:Falcon)
* @param[in] interfaceName       -  interface name
* @param[in] instanceName        -  instance name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleDump
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      metaData,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             instanceName
);


/**
* @internal prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter
*           function
* @endinternal
*
* @brief   Low level function to get number of hits for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                  - device number
* @param[in] instanceDfxBaseAddr     - DFX XSB unit base address
* @param[out] valuePtr               - (pointer to) number of
*                                      counter hits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter
(
    IN    GT_U8       devNum,
    IN    GT_U32      instanceDfxBaseAddr,
    OUT   GT_U32      *valuePtr
);



/**
* @internal prvCpssDxChIdebugInstanceDfxInterfaceSelect function
* @endinternal
*
* @brief   Select  instance DFX index
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] instanceDfxBaseAddr      - instance DFX base
* @param[in] interfaceDfxIndex        - interface DFX index
* @param[in] interfacePolarity        - interface Polarity
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDfxInterfaceSelect
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          instanceDfxBaseAddr,
    IN  GT_U32                                          interfaceDfxIndex,
    IN  GT_CHAR_PTR                                     interfacePolarity
);

/**
* @internal prvCpssDxChIdebugPacketMarkingSet function
* @endinternal
*
* @brief   Enable Packet Marking
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugPacketMarkingSet
(
    IN  GT_U8                                           devNum
);

/**
* @internal prvCpssDxChIdebugInstanceDfxIndexGet function
* @endinternal
*
* @brief   get index of bounded interface
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceId              - interface Id
* @param[out] indexPtr                - (pointer to) interface DFX index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDfxIndexGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceId,
    OUT GT_U32                                          *indexPtr
);

/**
* @internal prvCpssDxChIdebugIsMultipleInterface function
* @endinternal
*
* @brief   The function return the number of multiplications
*          (for example if same descriptor exist per Data path) of descriptor.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfMultiplicationsPtr - (pointer to) the number
*                                       of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugIsMultipleInterface
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *numOfMultiplicationsPtr
);


/**
* @internal prvCpssDxChIdebugMultipleInterfaceByIndexGet
*           function
* @endinternal
*
* @brief   The function return the bus and mux select of specific multiplication index.
*             (used only for indexes greater then 0).
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfMultiplicationsPtr - (pointer to) the number
*                                       of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugMultipleInterfaceByIndexGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          multiplicationIndex,
    OUT GT_CHAR                                         *busPtr,
    OUT GT_U32                                          *muxPtr
);

/**
* @internal prvCpssDxChIdebugInerfaceMuxSet function
* @endinternal
*
* @brief   The function set   interface that will be chosen in case of mux between interfaces.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum             -  device number
* @param[in] currentBus         -  instance
* @param[in]muxIn               -  mux index that should have
*                                  highest priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugInerfaceMuxSet
(
    GT_U8                                       devNum,
    GT_CHAR_PTR                                 currentBus,
    GT_U32                                      muxIn
);

/**
* @internal prvCpssDxChIdebugFailOnUnfoundToken function
* @endinternal
*
* @brief   The function get failure action. The failue is considered not found token in XML.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                - device number
*
* @retval GT_TRUE                  - fail on unfound token
* @retval GT_FALSE                 - do notfail on unfound token
*
*/
GT_BOOL prvCpssDxChIdebugFailOnUnfoundToken
(
    IN  GT_U8                                       devNum
);

/**
* @internal prvCpssDxChIdebugFileNameGet function
* @endinternal
*
* @brief   Get XML file name
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                  - device number
* @param[out] xmlFileName            -(pointer to) XML file name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFileNameGet
(
    IN  GT_U8                                      devNum,
    OUT GT_CHAR_PTR                                *xmlFileName
);

GT_STATUS prvCpssDxChIdebugInstanceDfxPipeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          pipeInd
);

/**
* @internal prvCpssDxChIdebugInterfaceAttributesGet function
* @endinternal
*
* @brief   Get  interface DFX pipes , DFX mux indesx, DFX base adresses and number of multiplications.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] instanceDfxPipeIdPtr    - (pointer to) instance DFX pipe id
* @param[out] instanceDfxBaseAddrPtr  - (pointer to) instance DFX base address
* @param[out] interfaceDfxIndexPtr    - (pointer to) interface DFX index
* @param[out]numOfMultiplicationsPtr    (pointer to) number of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugInterfaceAttributesGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          iDebugOpCode,
    OUT GT_U32                                          *instanceDfxPipeIdPtr,
    OUT GT_U32                                          *instanceDfxBaseAddrPtr,
    OUT GT_U32                                          *interfaceDfxIndexPtr,
    OUT GT_U32                                          *numOfMultiplicationsPtr,
    OUT GT_U32                                          *instanceMetaDataPtr
);

/**
* @internal prvCpssDxChIdebugEncodeMetaData function
* @endinternal
*
* @brief   The function encode search attribute structure into meta data
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2.
*
* @param[in] devNum             -  device number
* @param[in] searchAttributePtr -  (pointer to) search attributes.
* @param[out] metaDataPtr       -  (pointer to) encoded meta data.
*
* @retval GT_OK                 - on success
* @retval GT_OUT_OF_RANGE       - one of the parameters is out of range
*
*/
GT_STATUS prvCpssDxChIdebugEncodeMetaData
(
    IN    GT_U32                                          devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                           *metaDataPtr
);

/**
* @internal prvCpssDxChIdebugFileFpGet function
* @endinternal
*
* @brief  Get pointer to XML file
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
CPSS_OS_FILE_TYPE_STC * prvCpssDxChIdebugFileFpGet
(
    IN  GT_U8                                   devNum
);

/**
* @internal prvCpssDxChIdebugMuxTokenNameGet function
* @endinternal
*
* @brief   The function return the right name token per device
*
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum            - device number
* @param[out]muxTokeName       - (pointer to) mux token Name.
*
* @retval GT_OK                - on success
* @retval GT_NOT_SUPPORTED     - on not applicable device
*
*/
GT_STATUS prvCpssDxChIdebugMuxTokenNameGet
(
    IN  GT_U8                                      devNum,
    OUT GT_CHAR_PTR                                *muxTokeName
);

/**
* @internal prvCpssDxChIdebugInterfaceMxmlSegmentGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
* @param[in] devNum                - device number
* @param[in] interfaceName         - interface name
* @param[out] segmentIdPtr         -(pointer to)index of segment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS prvCpssDxChIdebugInterfaceMxmlSegmentGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *segmentIdPtr
);

/**
* @internal mxmlInterfaceSegmentsArrGet function
* @endinternal
*
* @brief  Get array of interfaces with same mux index as
*         interfaceName in instanceId
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum               - device number
* @param[in] instanceId           - instance Id
* @param[in] interfaceName        - interface name
* @param[out] numOfSegmentsPtr    - (pointer to)num of segments
* @param[out] segmentsArrPtr      - (pointer to)segments array
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
* @retval GT_FAIL                   - on error
*
*/
GT_STATUS mxmlInterfaceSegmentsArrGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     instanceId,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *numOfSegmentsPtr,
    OUT GT_CHAR                                         **segmentsArrPtr
);

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceResetSamplingTriger
*           function
* @endinternal
*
* @brief   clear the sampling triger by reading the first word of it .
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceResetSamplingTriger
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName
);

/**
* @internal prvCpssDxChIdebugPortGroupSingleInterfaceSamplingEnableGet
*           function
* @endinternal
*
* @brief   Get sampling status (enable/disable) for single interface by instance info.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] instanceDfxPipeId        - instance Dfx Pipe Id
* @param[in] instanceDfxBaseAddr      - instance Dfx Base Addr
* @param[out] enablePtr               - (pointer to)
*                                       enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupSingleInterfaceSamplingEnableGet
(
    IN  GT_U8                                            devNum,
    IN  GT_CHAR_PTR                                      interfaceName,
    IN  GT_U32                                           instanceDfxPipeId,
    IN  GT_U32                                           instanceDfxBaseAddr,
    OUT GT_BOOL                                          *enablePtr
);

/**
* @internal prvCpssDxChIdebugClearLongestInterface function
* @endinternal
*
* @brief   set zeros in mask and data of the longest interface that
*          share same instance wite given interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager Id number
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugClearLongestInterface
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName
);

/**
* @internal prvCpssDxChIdebugInterfaceInstancesGet function
* @endinternal
*
* @brief   Get interface instances list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[in,out] numOfInstancesPtr    - in: (pointer to)
*                                       allocated number of
*                                       Instances for
*                                       instance
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       interface
* @param[out] instancesArr            - (pointer to) interface
*                                       instances list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugInterfaceInstancesGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   interfaceName,
    INOUT GT_U32                                        *numOfInstancesPtr,
    OUT   GT_CHAR_PTR                                   *instancesArr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____prvCpssDxChIdebugh */
