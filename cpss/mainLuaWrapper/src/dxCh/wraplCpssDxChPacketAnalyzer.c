/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChPacketAnalyzer.c
*
* DESCRIPTION:
*       packet analyzer functions wrapper
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <lauxlib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(_manager)\
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.packetAnalyzerSrc.paMngDB[_manager])

#define PRV_CPSS_CHECK_STATUS(status) \
    if (status != GT_OK) \
    { \
        lua_pushinteger(L, (lua_Integer)status); \
        return 1; \
    }

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerKeyAttributesFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper for getting packet analyzer key attributes
*       from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       keyAttrPtr                        - packet analyzer key attributes
*
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerKeyAttributesFromLuaGet
(
    IN  lua_State                                       *L,
    IN  GT_32                                           L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC    *keyAttrPtr
)
{
    size_t strLen;
    const char *name;

    lua_getfield(L, L_index, "name");
    name = lua_tolstring(L, -1, &strLen);
    cpssOsMemCpy(keyAttrPtr->keyNameArr, name, strLen + 1);

    return GT_OK;
}

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerRuleAttributesFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper for getting packet analyzer rule attributes
*       from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       ruleAttrPtr                       - packet analyzer rule attributes
*
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerRuleAttributesFromLuaGet
(
    IN  lua_State                                       *L,
    IN  GT_32                                           L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   *ruleAttrPtr
)
{
    size_t strLen;
    const char *name;

    lua_getfield(L, L_index, "name");
    name = lua_tolstring(L, -1, &strLen);
    cpssOsMemCpy(ruleAttrPtr->ruleNameArr, name, strLen + 1);

    return GT_OK;
}

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerLookupStageFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper for getting the packet analyzer lookup stage
*       from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       stageIdPtr                        - packet analyzer stage
*
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerLookupStageFromLuaGet
(
    IN  lua_State                                       *L,
    IN  GT_32                                           L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *stageIdPtr
)
{
    GT_STATUS                       status          = GT_OK;

    lua_pushvalue(L, L_index);

    *stageIdPtr =
        (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)
            prv_lua_to_c_enum(L, "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT");

    lua_pop(L, 1);

    return status;
}

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerFieldAssignmentModeFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of Enumerator getting for the packet analyzer
*       field assignment mode from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       fieldAssignmentModePtr            - field assignment mode
*
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerFieldAssignmentModeFromLuaGet
(
    IN  lua_State                                               *L,
    IN  GT_32                                                   L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT     *fieldAssignmentModePtr
)
{
    GT_STATUS                       status          = GT_OK;

    lua_pushvalue(L, L_index);

    *fieldAssignmentModePtr =
        (CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT)
            prv_lua_to_c_enum(L, "CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT");

    lua_pop(L, 1);

    return status;
}

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerFieldFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of Enumerator getting for the packet analyzer field
*       from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       fieldPtr                          - packet analyzer field
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerFieldFromLuaGet
(
    IN  lua_State                               *L,
    IN  GT_32                                   L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     *fieldPtr
)
{
    GT_STATUS                       status          = GT_OK;

    lua_pushvalue(L, L_index);

    *fieldPtr =
        (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)
            prv_lua_to_c_enum(L, "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT");

    lua_pop(L, 1);

    return status;
}

/*******************************************************************************
* pvrCpssDxChPacketAnalyzerFieldValueFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper for getting for the packet analyzer field value structure
*       from lua stack.
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*        L                                - lua state
*        L_index                          - entry lua stack index
*
* OUTPUTS:
*       fieldValuePtr                     - Field value structure
*
*       errorMessagePtr                   - error message
*
* RETURNS:
*       operation succed or error code and error message if occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChPacketAnalyzerFieldValueFromLuaGet
(
    IN  lua_State                                   *L,
    IN  GT_32                                       L_index,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   *fieldValuePtr
)
{
    if (lua_type(L, L_index) == LUA_TTABLE)
    {
        cpssOsMemSet(fieldValuePtr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));

        F_ENUM_N(fieldValuePtr, L_index, fieldName, name, CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);

        F_ARRAY_START(fieldValuePtr, L_index, data);
        {
            int idx;
            for (idx = 0; idx <= luaL_getn(L, -1); idx++) {
                F_ARRAY_NUMBER(fieldValuePtr, data, idx, GT_U32);
                /*cpssOsPrintf("Data: %x\n", fieldValuePtr->data[idx]);*/
            }
        }
        F_ARRAY_END(fieldValuePtr, L_index, data);

        F_ARRAY_START(fieldValuePtr, L_index, msk);
        {
            int idx;
            for (idx = 0; idx <= luaL_getn(L, -1); idx++) {
                F_ARRAY_NUMBER(fieldValuePtr, msk, idx, GT_U32);
                /*cpssOsPrintf("Mask: %x\n", fieldValuePtr->msk[idx]);*/
            }
        }
        F_ARRAY_END(fieldValuePtr, L_index, mask);
    }

    return GT_OK;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerLogicalKeyCreate
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function creates logical key for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerLogicalKeyCreate
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageIdArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT fieldId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT fieldAssignmentMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 4)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    /* Get Key Attributes */
    cpssOsMemSet(&keyAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
    rc = pvrCpssDxChPacketAnalyzerKeyAttributesFromLuaGet(L, 3, &keyAttr);
    PRV_CPSS_CHECK_STATUS(rc);

    /* Get stage ID */
    PARAM_ENUM(rc, stageIdArr[0], 4, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId, &keyAttr, 1, stageIdArr, fieldAssignmentMode, 0, &fieldId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerLogicalKeyFieldsAdd
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function adds fields to logical key for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerLogicalKeyFieldsAdd
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 keyId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT fieldId;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, fieldId,     3, CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId, keyId, 1, &fieldId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerLogicalKeyFieldsRemove
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function removes fields from logical key for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerLogicalKeyFieldsRemove
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 keyId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT fieldId;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, fieldId,     3, CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId, keyId, 1, &fieldId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsRemove
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function removes fields from logical key and from group for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsRemove
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 ruleId, groupId, keyId, actionId;
    GT_U32 numOfFields,i,j;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT fieldId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *fieldsValueArr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC  ruleAttr;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 5)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId, 1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, fieldId,     3, CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,    4, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,   5, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    /* get rule data */
    numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    fieldsValueArr = cpssOsMalloc(numOfFields*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
    rc = cpssDxChPacketAnalyzerGroupRuleGet(managerId, ruleId, groupId, &keyId, &ruleAttr, &numOfFields, fieldsValueArr, &actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    /* check that field exist in rule */
    for (i=0; i<numOfFields; i++)
    {
        if (fieldsValueArr[i].fieldName == fieldId)
            break;
    }

    /* field doesn't exist in rule; nothing to do */
    if (i == numOfFields)
    {
        cpssOsFree(fieldsValueArr);
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    /* delete rule from group */
    rc = cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);
    PRV_CPSS_CHECK_STATUS(rc);

    /* remove field from key */
    rc = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId, keyId, 1, &fieldId);
    PRV_CPSS_CHECK_STATUS(rc);

    numOfFields--;

    /* copy all the remaining fields */
    for (j=i; j<numOfFields; j++)
    {
         fieldsValueArr[j].fieldName = fieldsValueArr[j+1].fieldName;
         cpssOsMemCpy(fieldsValueArr[j].data, fieldsValueArr[j+1].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
         cpssOsMemCpy(fieldsValueArr[j].msk, fieldsValueArr[j+1].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
    }

    /* add rule to group */
    rc = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, numOfFields, fieldsValueArr, actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    cpssOsFree(fieldsValueArr);
    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}


/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function checks if fields has values in group for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 ruleId, groupId, keyId, actionId;
    GT_U32 numOfFields,i;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT fieldId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *fieldsValueArr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC  ruleAttr;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 5)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId, 1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, fieldId,     3, CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,    4, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,   5, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    /* get rule data */
    numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    fieldsValueArr = cpssOsMalloc(numOfFields*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
    rc = cpssDxChPacketAnalyzerGroupRuleGet(managerId, ruleId, groupId, &keyId, &ruleAttr, &numOfFields, fieldsValueArr, &actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    /* check that field exist in rule */
    for (i=0; i<numOfFields; i++)
    {
        if (fieldsValueArr[i].fieldName == fieldId)
            break;
    }

    /* field doesn't exist in rule*/
    if (i == numOfFields)
    {
        cpssOsFree(fieldsValueArr);
        lua_pushinteger(L, (lua_Integer)GT_NOT_FOUND);
        return 1;
    }

    /* field exist in rule; check if it's mask is not '0'*/
    if (fieldsValueArr[i].msk[0] != 0)
    {
        cpssOsFree(fieldsValueArr);
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)fieldsValueArr[i].msk[0]);
        return 2;
    }

    cpssOsFree(fieldsValueArr);
    lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerActionInverseSet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function update inverse status in action for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerActionInverseSet
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 action;
    GT_BOOL inverseEnable;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC  actionId;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    PARAM_NUMBER(rc, action,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    PARAM_BOOL(rc, inverseEnable, 3);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerActionGet(managerId, action, &actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    actionId.inverseEnable = inverseEnable;
    rc = cpssDxChPacketAnalyzerActionUpdate(managerId, action, &actionId);
    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerActionSamplingModeSet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function update sampling mode in action for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerActionSamplingModeSet
(
    IN lua_State *L
)
{
    GT_U32 action,managerId;
    CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT samplingMode;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC  actionId;


    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    PARAM_NUMBER(rc, action,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    PARAM_ENUM(rc, samplingMode, 3, CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerActionGet(managerId, action, &actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    actionId.samplingMode = samplingMode;

    rc = cpssDxChPacketAnalyzerActionUpdate(managerId, action, &actionId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerLogicalKeyDelete
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function deletes logical key for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerLogicalKeyDelete
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 keyId;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId, keyId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupRuleAdd
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function creates rule for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupRuleAdd
(
    IN lua_State *L
)
{
    GT_U32 groupId,managerId;
    GT_U32 keyId;
    GT_U32 ruleId;
    GT_U32 actionId;
    GT_STATUS rc = GT_OK;
    GT_U32 length;
    GT_U32 i;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC  ruleAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *fieldValue;

    if (lua_gettop(L) != 8)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, keyId,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,  3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,   4, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = pvrCpssDxChPacketAnalyzerRuleAttributesFromLuaGet(L, 5, &ruleAttr);
    PRV_CPSS_CHECK_STATUS(rc);

    PARAM_NUMBER(rc, length,   6, GT_U32);
    fieldValue = cpssOsMalloc((length+1)*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));

    for (i = 0; i < length; i++)
    {
        int top;
        lua_rawgeti(L, 7, i);
        top = lua_gettop(L);
        rc = pvrCpssDxChPacketAnalyzerFieldValueFromLuaGet(L, top, &fieldValue[i]);
        if (rc != GT_OK)
        {
            cpssOsFree(fieldValue);
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }
    }

    PARAM_NUMBER(rc, actionId,   8, GT_U32);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldValue);
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, length, fieldValue, actionId);

    cpssOsFree(fieldValue);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupRuleFieldsAdd
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function update rule for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupRuleFieldsAdd
(
    IN lua_State *L
)
{
    GT_U32 groupId,managerId;
    GT_U32 ruleId,keyId;
    GT_U32 actionId;
    GT_STATUS rc = GT_OK,st;
    GT_U32 length;
    GT_BOOL found = GT_FALSE;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *fieldValue;
    GT_U32 numOfFields,i,j,numOfFieldsTemp;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *fieldsValueArr,*fieldsValueArrTemp;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC  ruleAttr;

    if (lua_gettop(L) != 6)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,  3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, length,   4, GT_U32);
    fieldValue = cpssOsMalloc(length*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));

    /* new params to add */
    for (i = 0; i < length; i++)
    {
        int top;
        lua_rawgeti(L, 5, i);
        top = lua_gettop(L);
        rc = pvrCpssDxChPacketAnalyzerFieldValueFromLuaGet(L, top, &fieldValue[i]);
        if (rc != GT_OK)
        {
            cpssOsFree(fieldValue);
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }
    }

    PARAM_NUMBER(rc, actionId,    6, GT_U32);

    /* get rule data */
    numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    fieldsValueArr = cpssOsMalloc(numOfFields*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
    fieldsValueArrTemp = cpssOsMalloc(numOfFields*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
    rc = cpssDxChPacketAnalyzerGroupRuleGet(managerId, ruleId, groupId, &keyId, &ruleAttr, &numOfFields, fieldsValueArr, &actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    /* check if field exist in rule */
    for (i=0; i<numOfFields; i++)
    {
        /* all fields to update */
        for (j=0; j<length; j++)
        {
            if (fieldsValueArr[i].fieldName == fieldValue[j].fieldName)
            {
                found = GT_TRUE;
                /*cpssOsPrintf("\n cpssDxChPacketAnalyzerGroupRuleUpdate ruleId %d groupId %d fieldName %s fileData %d",ruleId,groupId,paFieldStrArr[fieldValue[j].fieldName],fieldValue[j].data[0]);*/
                rc = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId, ruleId, groupId, 1, &(fieldValue[j]), actionId);
                if (rc != GT_OK)
                {
                    cpssOsFree(fieldsValueArr);
                    cpssOsFree(fieldValue);
                    lua_pushinteger(L, (lua_Integer)rc);
                    return 1;
                }
            }
        }
    }

    /* updated the fields */
    if (found == GT_TRUE)
    {
        cpssOsFree(fieldsValueArr);
        cpssOsFree(fieldValue);
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    /* field doesn't exist in rule; add those fields */
    for (i=0; i<numOfFields; i++)
    {
        fieldsValueArrTemp[i].fieldName = fieldsValueArr[i].fieldName;
    }
    numOfFieldsTemp = numOfFields;

    /* add additional fields */
    for (i=0;i<length; i++)
    {
        fieldsValueArr[numOfFields].fieldName = fieldValue[i].fieldName;
        cpssOsMemCpy(fieldsValueArr[numOfFields].data, fieldValue[i].data, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);
        cpssOsMemCpy(fieldsValueArr[numOfFields].msk, fieldValue[i].msk, sizeof(GT_U32)*CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS);

        numOfFields++;
    }

    /* check that no overlapping fields in fieldsValueArr[] */
    rc = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
    if (rc != GT_OK)
    {
        /* delete rule from group */
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);
        PRV_CPSS_CHECK_STATUS(st);

        /* remove fields from key */
        for (i=0;i<length;i++)
        {
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,1,&fieldValue[i].fieldName);
            PRV_CPSS_CHECK_STATUS(st);
        }

        /* add rule to group */
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, numOfFieldsTemp, fieldsValueArrTemp, actionId);
        PRV_CPSS_CHECK_STATUS(st);
    }
    PRV_CPSS_CHECK_STATUS(rc);

    /* delete rule from group */
    rc = cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);
    PRV_CPSS_CHECK_STATUS(rc);

    /* add rule to group */
    /*cpssOsPrintf("\n cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d groupId %d numOfFields %d fieldName %s fileData %d",ruleId,groupId,numOfFields,paFieldStrArr[fieldValue[0].fieldName],fieldValue[0].data[0]);*/
    rc = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, numOfFields, fieldsValueArr, actionId);
    PRV_CPSS_CHECK_STATUS(rc);

    cpssOsFree(fieldsValueArr);
    cpssOsFree(fieldsValueArrTemp);
    cpssOsFree(fieldValue);
    lua_pushinteger(L, (lua_Integer)rc);

    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupRuleDelete
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function delete rule for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupRuleDelete
(
    IN lua_State *L
)
{
    GT_U32 ruleId,managerId;
    GT_U32 groupId;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,    3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);

    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}


/*******************************************************************************
* wrlCpssDxChPacketAnalyzerRuleMatchDataNumOfHitsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return num of hits for rule in group for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerRuleMatchHitsOnStagesGet
(
    IN lua_State *L
)
{

    GT_U32 ruleId,groupId,managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId;
    GT_U32 numOfMatchedStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    GT_U32 numOfSampledFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT *matchedStagesArr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *sampleFieldsValueArr=NULL;
    GT_STATUS rc = GT_OK;
    GT_U32 i,numOfHits = 0;

    if (lua_gettop(L) != 4)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,     3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, stageId,     4, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
    PRV_CPSS_CHECK_STATUS(rc);


    matchedStagesArr = cpssOsMalloc(numOfMatchedStages*sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT));
    sampleFieldsValueArr = cpssOsMalloc(numOfSampledFields*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));

    rc = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId, ruleId, groupId, &numOfMatchedStages, matchedStagesArr);
    PRV_CPSS_CHECK_STATUS(rc);

    for (i=0; i<numOfMatchedStages; i++)
    {
        if (matchedStagesArr[i] == (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)stageId)
        {
            rc = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,
                groupId,stageId,&numOfHits, &numOfSampledFields, sampleFieldsValueArr);

            lua_pushinteger(L, (lua_Integer)rc);
            lua_pushinteger(L, (lua_Integer)numOfHits);

            cpssOsFree(matchedStagesArr);
            cpssOsFree(sampleFieldsValueArr);

            return 2;
        }
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)numOfHits);

    cpssOsFree(matchedStagesArr);
    cpssOsFree(sampleFieldsValueArr);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return num of hits for all stages in group for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet
(
    IN lua_State *L
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR      paRuleDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stagePtr = NULL;
    GT_U32 ruleId,groupId,keyId,managerId;
    GT_CHAR interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId;
    GT_STATUS rc = GT_OK;
    GT_U32 numOfHits = 0, requiredMultInd = 0 ;
    GT_U8 devNum;

    if (lua_gettop(L) != 4)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, ruleId,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,     3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, stageId,     4, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    cpssOsStrCpy(interfaceId,"");

    /* get pointer to rule in packet analyzer {group,rule} DB */
    rc = prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDbPtr);
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

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stagePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsStrCpy(interfaceId,stagePtr->boundInterface.interfaceId); /* bounded interface */

    /* get number of hits for bounded interface */
    if (stagePtr->isValid == GT_TRUE)
    {
        rc = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceId,GT_FALSE,&numOfHits,&requiredMultInd);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)numOfHits);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerSamplingAllInterfacesSet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function enable/disable sampling on all bounded interfaces for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerSamplingAllInterfacesSet
(
    IN lua_State *L
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  boundInterface;
    GT_U32 i,stageId,managerId;
    GT_U8 devNum;
    GT_BOOL enable;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, devNum,    2, GT_U8);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_BOOL(rc, enable, 3);
    PRV_CPSS_CHECK_STATUS(rc);

    for (i=0; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stageId = i;

        rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterface);
        if (rc != GT_OK)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }

        rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,boundInterface.info.interfaceId,enable);
        if (rc != GT_OK)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}

/*******************************************************************************
* wrlPacketAnalyzerXmlPresent
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return 0 if xml is present
*                           1 if xml in not present
*
* COMMENTS: same code as prvTgfPacketAnalyzerXmlPresent
*
*******************************************************************************/
int wrlPacketAnalyzerXmlPresent
(
    IN  lua_State               *L
)
{
    CPSS_OS_FILE_TYPE_STC     file;
    GT_STATUS                 rc;
    GT_CHAR_PTR               xmlFileName;
    GT_U8                     devNum;

    PARAM_NUMBER(rc, devNum,    1, GT_U8);

       /* open CIDER XML */
    rc= prvCpssDxChIdebugFileNameGet(devNum,&xmlFileName);
    if(rc!=GT_OK)
    {
         return GT_FALSE;
    }

    file.fd =cpssOsFopen(xmlFileName, "r",&file);
    if(file.fd == CPSS_OS_FILE_INVALID)
    {
        return 1;
    }

    cpssOsFclose(&file);
    return 0;
}


/*******************************************************************************
* wrlCpssDxChPacketAnalyzerManagerNumDevicesGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of muxed stages for specific stage
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerManagerNumDevicesGet
(
    IN lua_State *L
)
{

    GT_U32 managerId;
    GT_BOOL deviceState;
    GT_U32 numOfDevices = PRV_CPSS_MAX_PP_DEVICES_CNS;
    GT_U8 *devicesArr;
    GT_STATUS rc = GT_OK;

    if (lua_gettop(L) != 1)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    devicesArr = cpssOsMalloc(PRV_CPSS_MAX_PP_DEVICES_CNS*sizeof(GT_U8));
    rc = cpssDxChPacketAnalyzerManagerDevicesGet(managerId, &deviceState, &numOfDevices, devicesArr);

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)numOfDevices);

    cpssOsFree(devicesArr);
    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerNumOfInterfacesGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of interfaces for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerNumOfInterfacesGet
(
    IN lua_State *L
)
{

    GT_U32 i,managerId,numOfInterfaces,freeNumOfInterfaces=0;
    GT_CHAR_PTR *interfacesArr = NULL;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    GT_BOOL isBounded= GT_FALSE,found = GT_FALSE;

    if (lua_gettop(L) != 1)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChIdebugDbNumInterfacesGet(devNum,&numOfInterfaces);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChPacketAnalyzerInterfacesGet(managerId,devNum,&numOfInterfaces,&interfacesArr);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    for (i=0; i<numOfInterfaces; i++)
    {

        found = GT_FALSE;

        /* check if Interface is bounded to a stage - then don't add it */
        rc = prvCpssDxChPacketAnalyzerInterfaceStatusGet(managerId,devNum,interfacesArr[i],&isBounded);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (isBounded)
            found =  GT_TRUE;

        if (found == GT_FALSE)
            freeNumOfInterfaces++;

    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)freeNumOfInterfaces);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerInterfaceGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single interface for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerInterfaceGet
(
    IN lua_State *L
)
{

    GT_U32 i,managerId,numOfInterfaces,freeNumOfInterfaces=0,interfaceIndex;
    GT_CHAR_PTR *interfacesArr = NULL;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    GT_BOOL isBounded= GT_FALSE,found = GT_FALSE;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, interfaceIndex,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChIdebugDbNumInterfacesGet(devNum,&numOfInterfaces);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChPacketAnalyzerInterfacesGet(managerId,devNum,&numOfInterfaces,&interfacesArr);

    if (interfaceIndex > numOfInterfaces)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    for (i=0; i<numOfInterfaces; i++)
    {

        found = GT_FALSE;

        /* check if Interface is bounded to a stage - then don't add it */
        rc = prvCpssDxChPacketAnalyzerInterfaceStatusGet(managerId,devNum,interfacesArr[i],&isBounded);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (isBounded)
            found =  GT_TRUE;

        if (found == GT_FALSE)
            freeNumOfInterfaces++;

        if (freeNumOfInterfaces == interfaceIndex+1)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            lua_pushstring(L, interfacesArr[i]);

            return 2;
        }

    }

    lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerInstanceForInterfaceGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single instance for interface for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerInstanceAllInterfacesGet
(
    IN lua_State *L
)
{
    GT_U32 managerId,numOfInstances=5000,numOfInterfaces;
    GT_U32 i,j,counter=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    const GT_CHAR *interfaceId;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC interfacesArrTemp[MAX_MULTIPLICATIONS_NUM];
    GT_CHAR interfacesArr[MAX_MULTIPLICATIONS_NUM][CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_CHAR **instancesArr;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    if (lua_isstring(L, 2))
    {
        interfaceId = lua_tostring(L, 2);
    }
    else
    {
        rc = GT_BAD_VALUE;
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChPacketAnalyzerInstancesGet(managerId,devNum,&numOfInstances,&instancesArr);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    for (i=0;i<numOfInstances;i++)
    {
        numOfInterfaces = MAX_MULTIPLICATIONS_NUM;
        rc = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instancesArr[i],&numOfInterfaces,interfacesArrTemp);
        if (rc != GT_OK)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }
        for (j=0; j<numOfInterfaces; j++)
        {
            if (cpssOsStrCmp(interfacesArrTemp[j].interfaceId,interfaceId) == 0)
            {
                cpssOsStrCpy(interfacesArr[counter],interfacesArrTemp[j].instanceId);
                counter++;
            }
        }
    }

    if (counter == 0)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)counter);
    for (i=0; i<counter; i++)
    {
        lua_pushstring(L, interfacesArr[i]);
    }

    return counter+2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerNumOfFieldsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of fields for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerNumOfFieldsGet
(
    IN lua_State *L
)
{

    GT_U32 i,j,id,managerId,numOfFields,freeNumOfFields = 0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    GT_CHAR_PTR *fieldsArr = NULL;
    GT_BOOL found = GT_FALSE;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR   paMngDbPtr = NULL;

    if (lua_gettop(L) != 1)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChIdebugDbFieldsArrayGet(devNum,&numOfFields,&fieldsArr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    for (i=0; i<numOfFields; i++)
    {
        found = GT_FALSE;

        /* check if field is bounded to a PA field - then don't add it */
        for (j=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_FIRST_E; j<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E; j++)
        {
            if (cpssOsStrCmp(fieldsArr[i],packetAnalyzerFieldToIdebugFieldArr[j]) == 0)
            {
                found =  GT_TRUE;
                break;
            }
        }

        /* check if udf was defined */
        for (j=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; j<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; j++)
        {
            id = j - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

            if (cpssOsStrCmp(paMngDbPtr->udfArr[id].iDebugField,fieldsArr[i]) == 0)
            {
                found =  GT_TRUE;
                break;
            }

        }

        if (found == GT_FALSE)
            freeNumOfFields++;
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)freeNumOfFields);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerFieldGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single field for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerFieldGet
(
    IN lua_State *L
)
{

    GT_U32 i,j,id,managerId,numOfFields,freeNumOfFields=0;
    GT_U8 devNum;
    GT_CHAR_PTR *fieldsArr = NULL;
    GT_STATUS rc = GT_OK;
    GT_U32 fieldIndex;
    GT_BOOL found = GT_FALSE;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR   paMngDbPtr = NULL;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, fieldIndex,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChIdebugDbFieldsArrayGet(devNum,&numOfFields,&fieldsArr);

    if (fieldIndex > numOfFields)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    for (i=0; i<numOfFields; i++)
    {
        found = GT_FALSE;

        /* check if field is bounded to a PA field - then don't add it */
        for (j=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_FIRST_E; j<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E; j++)
        {
            if (cpssOsStrCmp(fieldsArr[i],packetAnalyzerFieldToIdebugFieldArr[j]) == 0)
            {
                found =  GT_TRUE;
                break;
            }
        }

        /* check if udf was defined */
        for (j=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; j<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; j++)
        {
            id = j - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

            if (cpssOsStrCmp(paMngDbPtr->udfArr[id].iDebugField,fieldsArr[i]) == 0)
            {
                found =  GT_TRUE;
                break;
            }

        }

        if (found == GT_FALSE)
        {
            freeNumOfFields++;
        }
        if (freeNumOfFields == fieldIndex+1)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            lua_pushstring(L, fieldsArr[i]);

            return 2;
        }
    }

    lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerNumOfUdfsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of user defined fields for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerNumOfUdfsGet
(
    IN lua_State *L
)
{

    GT_U32 i,id,managerId,numOfFields=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR  paMngDbPtr = NULL;

    if (lua_gettop(L) != 1)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find all valid udfs */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        id = i - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (paMngDbPtr->udfArr[id].valid == GT_TRUE)
        {
            numOfFields++;
        }
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)numOfFields);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerUdfGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single user defined field for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerUdfGet
(
    IN lua_State *L
)
{

    GT_U32 i,id,managerId,numOfFields=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    GT_U32 fieldIndex;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR   paMngDbPtr = NULL;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, fieldIndex,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find valid spesific valid udf */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E; i++)
    {
        id = i - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);

        if (paMngDbPtr->udfArr[id].valid == GT_TRUE)
        {
            numOfFields++;
        }
        if (numOfFields == fieldIndex+1)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            lua_pushstring(L, paMngDbPtr->udfArr[id].name);

            return 2;
        }
    }

    lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return first and last user defined field index for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet
(
    IN lua_State *L
)
{

    lua_pushinteger(L, (lua_Integer)GT_OK);
    lua_pushinteger(L, (lua_Integer)PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E);
    lua_pushinteger(L, (lua_Integer)PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E);

    return 3;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return first and last user defined stage index for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet
(
    IN lua_State *L
)
{

    lua_pushinteger(L, (lua_Integer)GT_OK);
    lua_pushinteger(L, (lua_Integer)PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E);
    lua_pushinteger(L, (lua_Integer)PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E);

    return 3;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerNumOfUdssGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of user defined stages for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerNumOfUdssGet
(
    IN lua_State *L
)
{

    GT_U32 i,id,managerId,numOfStages=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR  paMngDbPtr = NULL;

    if (lua_gettop(L) != 1)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find all valid udss */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        id = i - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (paMngDbPtr->udsArr[id].valid == GT_TRUE)
        {
            numOfStages++;
        }
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)numOfStages);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerUdsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single user defined stage for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerUdsGet
(
    IN lua_State *L
)
{

    GT_U32 i,id,managerId,numOfStages=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    GT_U32 stageIndex;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR   paMngDbPtr = NULL;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, stageIndex,     2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* find valid spesific valid udf */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        id = i - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (paMngDbPtr->udsArr[id].valid == GT_TRUE)
        {
            numOfStages++;
        }
        if (numOfStages == stageIndex+1)
        {
            lua_pushinteger(L, (lua_Integer)rc);
            lua_pushstring(L, paMngDbPtr->udsArr[id].name);

            return 2;
        }
    }

    lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
    return 1;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerGroupActivateEnableGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function get activated group for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerGroupActivateEnableGet
(
    IN lua_State *L
)
{
    GT_U32 managerId,groupId;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR       paGrpDbPtr = NULL;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, groupId,    2, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
    if (paGrpDbPtr == NULL)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    if (paGrpDbPtr->activateEnable == GT_TRUE)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)GT_TRUE);
    }
    else
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)GT_FALSE);
    }

    return 2;

}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerStageValidityGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function get stage validity in SW for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerStageValidityGet
(
    IN lua_State *L
)
{
    GT_U8 devNum;
    GT_U32 managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC        *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR           paDevDbPtr = NULL;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, devNum,    2, GT_U8);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, stageId,     3, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
    if (stageDbPtr == NULL)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    if (stageDbPtr->isValid == GT_TRUE)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)GT_TRUE);
    }
    else
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)GT_FALSE);
    }

    return 2;

}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerPreDefinedStageInfoGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function get packet analyzer pre defined stages information
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerPreDefinedStageInfoGet
(
    IN lua_State *L
)
{
    GT_U8 devNum;
    GT_U32 managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC        *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR           paDevDbPtr = NULL;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_NUMBER(rc, devNum,    2, GT_U8);
    PRV_CPSS_CHECK_STATUS(rc);
    PARAM_ENUM(rc, stageId,     3, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
    PRV_CPSS_CHECK_STATUS(rc);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
    if (stageDbPtr == NULL)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }


    if (stageDbPtr->isValid == GT_TRUE)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushstring(L, stageDbPtr->boundInterface.interfaceId);
        lua_pushstring(L, stageDbPtr->boundInterface.instanceId);
        lua_pushinteger(L, (lua_Integer)stageDbPtr->boundInterface.interfaceIndex);
        lua_pushinteger(L, (lua_Integer)GT_TRUE);

        return 5;
    }
    else
    {
        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushstring(L, stageDbPtr->boundInterface.interfaceId);
        lua_pushstring(L, stageDbPtr->boundInterface.instanceId);
        lua_pushinteger(L, (lua_Integer)stageDbPtr->boundInterface.interfaceIndex);
        lua_pushinteger(L, (lua_Integer)GT_FALSE);

        return 5;
    }
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of enums for field for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet
(
    IN lua_State *L
)
{

    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    mxml_node_t *node;
    const GT_CHAR *fieldId;
    GT_CHAR fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 numOfEnums;
    PRV_CPSS_DXCH_IDEBUG_ENUM_STC enumsArr[MAX_MULTIPLICATIONS_NUM];
    GT_BOOL isEnum;

    if (lua_gettop(L) != 2)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    PRV_CPSS_CHECK_STATUS(rc);
    if (lua_isstring(L, 2))
    {
        fieldId = lua_tostring(L, 2);
    }
    else
    {
        rc = GT_BAD_VALUE;
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    cpssOsStrCpy(fieldName,fieldId);

    rc = prvCpssDxChIdebugFieldIsEnumCheck(devNum,fieldName,&isEnum);
    PRV_CPSS_CHECK_STATUS(rc);

    if (isEnum == GT_TRUE)
    {
        node = mxmlFieldEnumsGet(devNum,fieldName,&numOfEnums,enumsArr);
        if(!node)
        {
            rc = GT_BAD_VALUE;
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }

        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushinteger(L, (lua_Integer)numOfEnums);

        return 2;
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, 0);

    return 2;
}


/*******************************************************************************
* wrlCpssDxChPacketAnalyzerFieldEnumGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return single enum for field for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerFieldEnumGet
(
    IN lua_State *L
)
{

    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    mxml_node_t *node;
    const GT_CHAR *fieldId;
    GT_CHAR fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 numOfEnums,enumIndex;
    PRV_CPSS_DXCH_IDEBUG_ENUM_STC enumsArr[MAX_MULTIPLICATIONS_NUM];
    GT_BOOL isEnum;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    PRV_CPSS_CHECK_STATUS(rc);
    if (lua_isstring(L, 2))
    {
        fieldId = lua_tostring(L, 2);
    }
    else
    {
        rc = GT_BAD_VALUE;
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }
    PARAM_NUMBER(rc, enumIndex,     3, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);

    cpssOsStrCpy(fieldName,fieldId);

    rc = prvCpssDxChIdebugFieldIsEnumCheck(devNum,fieldName,&isEnum);
    PRV_CPSS_CHECK_STATUS(rc);

    if (isEnum == GT_TRUE)
    {
        node = mxmlFieldEnumsGet(devNum,fieldName,&numOfEnums,enumsArr);
        if(!node)
        {
            rc = GT_BAD_STATE;
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }

        if (numOfEnums == 0)
        {
            rc = GT_BAD_STATE;
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }

        if (enumIndex > numOfEnums)
        {
            lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
            return 1;
        }

        lua_pushinteger(L, (lua_Integer)rc);
        lua_pushstring(L, enumsArr[enumIndex].name);
        lua_pushinteger(L, enumsArr[enumIndex].value);

        return 3;
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, 0);

    return 2;
}

/*******************************************************************************
* wrlCpssDxChPacketAnalyzerIndexesForInterfaceAndInstanceGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       The function return number of indexes for {instance,interface} for packet analyzer
*
* APPLICABLE DEVICES:
*        Aldrin; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        xCat; xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       error code and error message, if error occurs
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPacketAnalyzerIndexesForInterfaceAndInstanceGet
(
    IN lua_State *L
)
{
    GT_U32 managerId;
    GT_U32 i,counter=0;
    GT_U8 devNum;
    GT_STATUS rc = GT_OK;
    const GT_CHAR *interfaceId,*instanceId;
    GT_CHAR instanceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 indexesArr[PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS];
    mxml_node_t *node;
    const GT_CHAR *attr;
    GT_CHAR_PTR  muxTokeName;
    GT_BOOL noInterface = GT_FALSE;

    if (lua_gettop(L) != 3)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    PARAM_NUMBER(rc, managerId,    1, GT_U32);
    PRV_CPSS_CHECK_STATUS(rc);
    if (lua_isstring(L, 2))
    {
        interfaceId = lua_tostring(L, 2);
    }
    else
    {
        rc = GT_BAD_VALUE;
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }
    if (lua_isstring(L, 3))
    {
        instanceId = lua_tostring(L, 3);
    }
    else
    {
        rc = GT_BAD_VALUE;
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    rc = prvCpssDxChPacketAnalyzerFirstDevGet(managerId, &devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)rc);
        return 1;
    }

    cpssOsStrCpy(instanceName,instanceId);

    /* first {instanceId,interfaceId} */
    node = mxmlFindInstanceFirstInterfaceNode(devNum,instanceName,&noInterface);
    if (!node)
    {
        /*from sip6_10 instance can be without interfaces and return null */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if (noInterface == GT_FALSE)
            {
                rc = GT_BAD_VALUE;
                lua_pushinteger(L, (lua_Integer)rc);
                return 1;
            }
        }
        else
        {
            rc = GT_BAD_VALUE;
            lua_pushinteger(L, (lua_Integer)rc);
            return 1;
        }
    }

    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"internalName");
        if (cpssOsStrCmp(attr,interfaceId)==0)
        {
            rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokeName);
            if (rc != GT_OK)
            {
                rc = GT_FAIL;
                lua_pushinteger(L, (lua_Integer)rc);
            }
            attr = prvCpssMxmlElementGetAttr(node,muxTokeName);
            indexesArr[counter] = cpssOsStrTo32(attr);
            counter++;
        }
        node = node->next;
    }

    lua_pushinteger(L, (lua_Integer)rc);
    lua_pushinteger(L, (lua_Integer)counter);
    for (i=0; i<counter; i++)
    {
        lua_pushinteger(L, (lua_Integer)indexesArr[i]);
    }
    return counter+2;
}

