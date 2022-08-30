/*************************************************************************
* genwrapper_API.h
*
* DESCRIPTION:
*       a generic wrapper to CPSS APIs
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 28 $
**************************************************************************/

#ifndef __genwrapper_API_h

#include <lua.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/prvCpssGlobalMutex.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef CPSS_GENWRAPPER_ENGINE
#ifndef LUACLI_NOCPSS
#include <cmdShell/common/cmdExtServices.h>
#endif /* !defined(LUACLI_NOCPSS) */
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/************************************************************************
* A helper macroses/functions for Lua <=> c structures convertation
************************************************************************/

#define ENUM_ENTRY(a) { a, #a }

/*************************************************************************
* prv_lua_to_c_enum
*
* Description:
*       Convert lua value to enum
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_to_c_enum(
    lua_State   *L,
    const char  *enumString
);

/*************************************************************************
* prv_c_to_lua_enum
*
* Description:
*       Convert enum to lua
*
* Parameters:
*       cmd
*
* Returns:
*       string at the top of stack
*
*************************************************************************/
void prv_c_to_lua_enum(
    lua_State   *L,
    const char  *enumString,
    int         enumValue
);


#ifndef LUACLI_NOCPSS
/*******************************************************************************
* prvCpssGetEnumString
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Returns enum member string according to its enumerated value or NULL
*       when enumerated value is wrong.
*
* APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       enumValue             - enum enumerated value
*       enumName              - enum typedef name string
*       size                  - size of buffer enumString
*
* OUTPUTS:
*       enumString            - buffer to stopre enum member string
*
* RETURNS:
*       enum member string or NULL when entry enumerated value is wrong
*
* COMMENTS:
*
*******************************************************************************/
const GT_CHAR* prvCpssGetEnumString
(
    IN  int                     enumValue,
    IN  const char              *enumName,
    OUT char                    *enumString,
    IN  int                     size
);
#endif /* !defined(LUACLI_NOCPSS) */


/*************************************************************************
* prv_lua_check_enum
*
* Description:
*       Return true if L => 1st is a member of enum
*
* Parameters:
*       L => 1st    - string to check
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_check_enum(
    lua_State   *L,
    const char  *enumString
);

/*************************************************************************
* prv_lua_complete_enum
*
* Description:
*       Return the list of alternatives for autocomplete matching L => 1st
*
* Parameters:
*       L => 1st    - string to complete
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_complete_enum(
    lua_State   *L,
    const char  *enumString
);

/*************************************************************************
* prv_lua_complete_union
*
* Description:
*       Return the list of alternatives for autocomplete matching L => 1st
*
* Parameters:
*       L => 1st    - string to complete
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_complete_union(
    lua_State   *L,
    const char  *unionString
);


#ifndef LUACLI_NOCPSS
/**
* @internal prv_swap_byte_array function
* @endinternal
*
* @brief   Swaps array in reversiveorder
*
* @param[in] input_data               - input byte array
* @param[in] dataLen                  - swapping bytes counmt
*
* @param[out] output_data              - output byte array
*                                       GT_OK
*/
GT_STATUS prv_swap_byte_array
(
    IN  const GT_U8_PTR     input_data,
    IN  GT_U32              dataLen,
    OUT GT_U8_PTR           output_data
);


/**
* @internal prv_lua_apply_mask function
* @endinternal
*
* @brief   Compare two structures using ignore mask
*
* @param[in] input_data               - Input data
* @param[in] output_data              - Output data
* @param[in] appiedMask               - data mask: the bits which will be reseted
*                                      are set to 1
* @param[in] dataLen                  - Data length (the same for data1, data2, ignoreMask)
*                                       GT_OK
*/
GT_STATUS prv_lua_apply_mask(
    IN  const void* input_data,
    IN  const void* output_data,
    IN  const void* appiedMask,
    IN  GT_U32      dataLen
);


/**
* @internal prv_lua_compare_mask function
* @endinternal
*
* @brief   Compare two structures using ignore mask
*
* @param[in] data1                    - Input data
* @param[in] data2                    - Input data
* @param[in] ignoreMask               - data mask: the bits which will be ignored
*                                      are set to 1
* @param[in] dataLen                  - Data length (the same for data1, data2, ignoreMask)
*                                       1 if matched
*                                       0 if not matched
*/
int prv_lua_compare_mask(
    IN  const void* data1,
    IN  const void* data2,
    IN  const void* ignoreMask,
    IN  GT_U32      dataLen
);

/*************************************************************************
* prv_lua_to_c_GT_U64
*
* Description:
*       Convert to "c" GT_U64
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_U64
(
    lua_State   *L,
    GT_U64      *val
);

/*************************************************************************
* prv_c_to_lua_GT_U64
*
* Description:
*       Convert GT_U64 to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_U64
(
    lua_State   *L,
    GT_U64      *val
);
#endif /* !defined(LUACLI_NOCPSS) */

/* add_mgm_enum
 *    mgmType_to_c_     ## enumName()
 *    mgmType_to_lua_   ## enumName()
 *    mgmType_check_    ## enumName()
 *    mgmType_complete_ ## enumName()
 */
#define add_mgm_enum(typeName) \
int mgmType_to_c_ ## typeName( \
    lua_State *L \
) \
{ \
    if (lua_isnil(L, 1)) \
    { \
        lua_pushnumber(L, 0); \
        return 1; \
    } \
    lua_settop(L, 1); \
    lua_pushnumber(L, prv_lua_to_c_enum(L, #typeName)); \
    return 1; \
} \
int mgmType_to_lua_ ## typeName( \
    lua_State *L \
) \
{ \
    if (lua_isnil(L, 1)) \
        return 1; \
 \
    if (!lua_isnumber(L, 1)) \
    { \
        lua_pushnil(L); \
    } \
    prv_c_to_lua_enum(L, #typeName, (int)(lua_tointeger(L, 1))); \
 \
    return 1; \
} \
int mgmType_check_ ## typeName( \
    lua_State *L \
) \
{ \
    return prv_lua_check_enum(L, # typeName); \
} \
int mgmType_complete_ ## typeName( \
    lua_State *L \
) \
{ \
    return prv_lua_complete_enum(L, # typeName); \
}

#define add_mgm_union(typeName) \
int mgmType_complete_ ## typeName( \
    lua_State *L \
) \
{ \
    return prv_lua_complete_union(L, # typeName); \
}

/************************************************************************
*
* The usual case is a structure as member of another structure
* For this pursose we need two more functions for Lua <=> c conevrtation
*
* These functions doesn't allocate memory and don't use the whole
* userdata object.
*
* For these purpose a pair of functions win common API must be created:
*
* void prv_lua_to_c_XXX(lua_State *L, XXX *val);
* void prv_c_to_lua_XXX(lua_State *L, XXX *val);
*
* The function prv_lua_to_c_XXX convert Lua object at the top of stack
* to C structure of type XXX, writing data to val
*
* The function prv_c_to_lua_XXX convert structure XXX pointed by val
* to Lua object. This object keept at the top of stack
*
************************************************************************/

#define use_prv_struct(typeName) \
    extern void prv_lua_to_c_ ## typeName(lua_State *L, typeName *val); \
    extern void prv_c_to_lua_ ## typeName(lua_State *L, typeName *val);

#define use_prv_array_type(typeName) \
    extern void prv_lua_to_c_ ## typeName(lua_State *L, typeName val); \
    extern void prv_c_to_lua_ ## typeName(lua_State *L, typeName val);

#define use_prv_mask_struct(typeName) \
    extern void prv_lua_to_c_mask_ ## typeName(lua_State *L, typeName *val); \

#ifndef LUACLI_NOCPSS
#define use_prv_print_struct(typeName)                                       \
    extern void prv_lua_c_print_ ## typeName(const GT_CHAR_PTR name,         \
                                             const GT_CHAR_PTR nameExtention,\
                                             const typeName *val);

#define use_prv_print_struct_array(typeName)                                 \
    extern void prv_lua_c_array_print_ ##                                    \
                                    typeName(const GT_CHAR_PTR name,         \
                                             const GT_CHAR_PTR nameExtension,\
                                             const GT_U32  array_items_count,\
                                             const typeName      *array);

#define use_prv_print_union(keyName, typeName)                               \
    extern void prv_lua_c_union_print_ ## typeName(const GT_CHAR_PTR name,   \
                                             const GT_CHAR_PTR nameExtention,\
                                             const keyName key,              \
                                             const typeName *val);
#endif /* !defined(LUACLI_NOCPSS) */

/* support macroses: Lua => C */
#define PARAM_BOOL(status,var,Lindex) \
    if (lua_isboolean(L, Lindex)) \
    { \
        (var) = lua_toboolean(L, Lindex) ? GT_TRUE : GT_FALSE; \
    } else { \
            (var) = GT_FALSE; \
        status = GT_BAD_PARAM; \
    }

#define PARAM_NUMBER(status,var,Lindex,vartype) \
    if (lua_isnumber(L, Lindex)) \
    { \
        (var) = (vartype)lua_tonumber(L, Lindex); \
    } else { \
            (var) = (vartype)0; \
        status = GT_BAD_PARAM; \
    }

#define PARAM_ENUM(status,var,Lindex,vartype) \
    if (lua_type(L, Lindex) == LUA_TNUMBER || lua_type(L, Lindex) == LUA_TSTRING) \
    { \
        lua_pushvalue(L, Lindex); \
        (var) = (vartype)prv_lua_to_c_enum(L, # vartype); \
        lua_pop(L, 1); \
    } else { \
            (var) = (vartype)0; \
        status = GT_BAD_PARAM; \
    }

#define PARAM_STRUCT(status,structPtr,Lindex,structtype) \
    if (lua_type(L, Lindex) == LUA_TTABLE) \
    { \
        lua_pushvalue(L, Lindex); \
        prv_lua_to_c_ ## structtype(L, (structPtr)); \
        lua_pop(L, 1); \
    } else { \
            cpssOsMemSet(structPtr,0,sizeof(*structPtr)); \
        status = GT_BAD_PARAM; \
    }

#define PARAM_ARRAY_N(status,arrtPtr,numElements,Lindex,structtype) \
    if (lua_type(L, Lindex) == LUA_TTABLE && numElements != 0) \
    { \
        GT_U32 i; \
        for (i = 0; i < numElements; i++) \
        { \
            lua_rawgeti(L, Lindex, i+1); \
            prv_lua_to_c_ ## structtype(L, &(arrtPtr[i])); \
            lua_pop(L, 1); \
        } \
    } else { \
            cpssOsMemSet(arrtPtr,0,sizeof(*arrtPtr)); \
            status = GT_BAD_PARAM; \
    }


#define F_BOOL_N(structPtr, Lindex, field, fieldname) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TBOOLEAN) \
    { \
        (structPtr)->field = lua_toboolean(L, -1) ? GT_TRUE : GT_FALSE; \
    } \
    else \
    { \
        (structPtr)->field = GT_FALSE; \
    } \
    lua_pop(L, 1);

#define F_BOOL(structPtr, Lindex, field) \
    F_BOOL_N(structPtr, Lindex, field, field)

#define F_ARRAY_BOOL(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TBOOLEAN) \
    { \
        (structPtr)->field[index] = lua_toboolean(L, -1) ? GT_TRUE : GT_FALSE; \
    } \
    else \
    { \
        (structPtr)->field[index] = GT_FALSE; \
    } \
    lua_pop(L, 1);

#define F_UNION_BOOL(structPtr, Lindex, field, fieldname, fieldtype) \
    if (lua_type(L, -1) == LUA_TBOOLEAN) \
    { \
        (structPtr)->field =  lua_toboolean(L, -1) ? GT_TRUE : GT_FALSE; \
    } \
    lua_pop(L, 1);

#define F_NUMBER_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (strcmp("GT_U64_BIT", #fieldtype) == 0) \
    { \
        (structPtr)->field = (fieldtype)(strtoul(lua_tostring(L, -1),NULL,16)); \
    } \
    else if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field = (fieldtype)lua_tonumber(L, -1); \
    } \
    else \
    { \
        (structPtr)->field = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define F_NUMBER(structPtr, Lindex, field, fieldtype) \
    F_NUMBER_N(structPtr, Lindex, field, field, fieldtype)

#define F_UNION_NUMBER_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field = (fieldtype)lua_tonumber(L, -1); \
    } \
    lua_pop(L, 1);

#define F_UNION_NUMBER(structPtr, Lindex, field, fieldtype) \
    F_UNION_NUMBER_N(structPtr, Lindex, field, field, fieldtype)

#define F_FLOAT_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field = (fieldtype)lua_tonumber(L, -1); \
    } \
    else \
    { \
        (structPtr)->field = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define F_FLOAT(structPtr, Lindex, field, fieldtype) \
    F_FLOAT_N(structPtr, Lindex, field, field, fieldtype)

#define F_ENUM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    (structPtr)->field = (fieldtype)prv_lua_to_c_enum(L, # fieldtype); \
    lua_pop(L, 1);

#define F_ENUM(structPtr, Lindex, field, fieldtype) \
    F_ENUM_N(structPtr, Lindex, field, field, fieldtype)

#define F_STRUCT_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, &((structPtr)->field)); \
    } \
    lua_pop(L, 1);

#define F_STRUCT(structPtr, Lindex, field, fieldtype) \
    F_STRUCT_N(structPtr, Lindex, field, field, fieldtype)

#define F_STRUCT_PTR(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_pop(L, 1); \
    lua_newuserdata(L, sizeof(fieldtype)); \
    structPtr->field = (fieldtype*)lua_touserdata(L, -1); \
    lua_pushvalue(L, 1); \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, ((structPtr)->field)); \
    } \
    lua_pop(L, 1); \

#define F_STRUCT_CUSTOM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    prv_lua_to_c_ ## fieldtype(L, &((structPtr)->field)); \
    lua_pop(L, 1);

#define F_STRUCT_CUSTOM(structPtr, Lindex, field, fieldtype) \
    F_STRUCT_CUSTOM_N(structPtr, Lindex, field, field, fieldtype)

#define F_ARRAY_TYPE_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, ((structPtr)->field)); \
    } \
    lua_pop(L, 1);

#define F_ARRAY_TYPE(structPtr, Lindex, field, fieldtype) \
    F_ARRAY_TYPE_N(structPtr, Lindex, field, field, fieldtype)

#define F_ARRAY_TYPE_CUSTOM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    prv_lua_to_c_ ## fieldtype(L, ((structPtr)->field)); \
    lua_pop(L, 1);

#define F_ARRAY_TYPE_CUSTOM(structPtr, Lindex, field, fieldtype) \
    F_ARRAY_TYPE_CUSTOM_N(structPtr, Lindex, field, field, fieldtype)

#define F_PTR(structPtr, Lindex, field, fieldtype) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA) \
    { \
        (structPtr)->field = (fieldtype)lua_touserdata(L, -1); \
    } \
    else \
    { \
        (structPtr)->field = (fieldtype)NULL; \
    } \
    lua_pop(L, 1);

#define F_ARRAY_START(structPtr, Lindex, field) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    {

#define F_ARRAY_END(structPtr, Lindex, field) \
    } \
    lua_pop(L, 1);

#define F_ARRAY_NUMBER(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field[index] = (fieldtype)lua_tonumber(L, -1); \
    } \
    else \
    { \
        (structPtr)->field[index] = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define F_ARRAY_FLOAT(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field[index] = (fieldtype)lua_tonumber(L, -1); \
    } \
    else \
    { \
        (structPtr)->field[index] = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define F_ARRAY_STRUCT(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, &((structPtr)->field[index])); \
    } \
    lua_pop(L, 1);

#define F_ARRAY_UNION(structPtr, field, index, fieldtype) \
        F_ARRAY_STRUCT(structPtr, field, index, fieldtype)

#define F_ARRAY_ENUM(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    (structPtr)->field[index] = (fieldtype)prv_lua_to_c_enum(L, # fieldtype); \
    lua_pop(L, 1);


/* support macroses: C => Lua */
#define FO_BOOL_N(structPtr, Lindex, field, fieldname) \
    lua_pushboolean(L, ((structPtr)->field == GT_TRUE) ? 1 : 0); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_BOOL(structPtr, Lindex, field) \
    FO_BOOL_N(structPtr, Lindex, field, field)

#define FO_NUMBER_N(structPtr, Lindex, field, fieldname, fieldtype) \
    if (strcmp(#fieldtype,"GT_U64_BIT") == 0) { \
        char buffer[9]; \
        sprintf (buffer, "0x%llx", (structPtr)->field); \
        lua_pushstring(L, buffer); \
    } \
    else \
        lua_pushnumber(L, (lua_Number)((structPtr)->field)); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_NUMBER(structPtr, Lindex, field, fieldtype) \
    FO_NUMBER_N(structPtr, Lindex, field, field, fieldtype)

#define FO_FLOAT_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_pushnumber(L, (structPtr)->field); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_FLOAT(structPtr, Lindex, field, fieldtype) \
    FO_FLOAT_N(structPtr, Lindex, field, field, fieldtype)

#define FO_ENUM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    prv_c_to_lua_enum(L, # fieldtype, (structPtr)->field); \
    if (lua_isnil(L, -1)) \
        lua_pop(L, 1); \
    else \
        lua_setfield(L, Lindex, #fieldname);

#define FO_ENUM(structPtr, Lindex, field, fieldtype) \
    FO_ENUM_N(structPtr, Lindex, field, field, fieldtype)

#define FO_STRUCT_N(structPtr, Lindex, field, fieldname, fieldtype) \
    prv_c_to_lua_ ## fieldtype(L, &((structPtr)->field)); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_STRUCT(structPtr, Lindex, field, fieldtype) \
    FO_STRUCT_N(structPtr, Lindex, field, field, fieldtype)

#define FO_STRUCT_PTR(structPtr, Lindex, field, fieldname, fieldtype) \
    if (structPtr->field ==NULL ) { \
        lua_newuserdata(L, sizeof(fieldtype)); \
        structPtr->field = (fieldtype *)lua_touserdata(L, -1); \
    } \
    prv_c_to_lua_ ## fieldtype(L, ((structPtr)->field)); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_ARRAY_TYPE_N(structPtr, Lindex, field, fieldname, fieldtype) \
    prv_c_to_lua_ ## fieldtype(L, ((structPtr)->field)); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_ARRAY_TYPE(structPtr, Lindex, field, fieldtype) \
    FO_ARRAY_TYPE_N(structPtr, Lindex, field, field, fieldtype)

#define FO_PTR(structPtr, Lindex, field, fieldtype) \
    lua_pushlightuserdata(L, (structPtr)->field); \
    lua_setfield(L, Lindex, #field);

#define FO_ARRAY_START(structPtr, Lindex, field) \
    lua_newtable(L);

#define FO_ARRAY_END(structPtr, Lindex, field) \
    lua_setfield(L, Lindex, #field);

#define FO_ARRAY_NUMBER(structPtr, field, index, fieldtype) \
    lua_pushnumber(L, (structPtr)->field[index]); \
    lua_rawseti(L, -2, index);

#define FO_ARRAY_FLOAT(structPtr, field, index, fieldtype) \
    lua_pushnumber(L, (structPtr)->field[index]); \
    lua_rawseti(L, -2, index);

#define FO_ARRAY_BOOL(structPtr, field, index, fieldtype) \
    lua_pushboolean(L, ((structPtr)->field[index] == GT_TRUE) ? 1 : 0); \
    lua_rawseti(L, -2, index);

#define FO_ARRAY_STRUCT(structPtr, field, index, fieldtype) \
    prv_c_to_lua_ ## fieldtype(L, &((structPtr)->field[index])); \
    lua_rawseti(L, -2, index);

#define FO_ARRAY_UNION(structPtr, field, index, fieldtype) \
        FO_ARRAY_STRUCT(structPtr, field, index, fieldtype)


#define FO_ARRAY_ENUM(structPtr, field, index, fieldtype) \
    prv_c_to_lua_enum(L, # fieldtype, (structPtr)->field[index]); \
    if (lua_isnil(L, -1)) \
        lua_pop(L, 1); \
    else \
        lua_rawseti(L, -2, index);


/* support macroses: Lua => C mask */
#define FM_BOOL_N(structPtr, Lindex, field, fieldname) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TBOOLEAN) \
    { \
        (structPtr)->field = (GT_BOOL)0; \
    } \
    lua_pop(L, 1);

#define FM_BOOL(structPtr, Lindex, field) \
    FM_BOOL_N(structPtr, Lindex, field, field)

#define FM_NUMBER_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define FM_NUMBER(structPtr, Lindex, field, fieldtype) \
    FM_NUMBER_N(structPtr, Lindex, field, field, fieldtype)

#define FM_ENUM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) \
    { \
        (structPtr)->field = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define FM_ENUM(structPtr, Lindex, field, fieldtype) \
    FM_ENUM_N(structPtr, Lindex, field, field, fieldtype)

#define FM_STRUCT_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_mask_ ## fieldtype(L, &((structPtr)->field)); \
    } \
    lua_pop(L, 1);

#define FM_STRUCT(structPtr, Lindex, field, fieldtype) \
    FM_STRUCT_N(structPtr, Lindex, field, field, fieldtype)

#define FM_STRUCT_CUSTOM_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    prv_lua_to_c_mask_ ## fieldtype(L, &((structPtr)->field)); \
    lua_pop(L, 1);

#define FM_STRUCT_CUSTOM(structPtr, Lindex, field, fieldtype) \
    FM_STRUCT_CUSTOM_N(structPtr, Lindex, field, field, fieldtype)

#define FM_PTR(structPtr, Lindex, field, fieldtype) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA) \
    { \
        (structPtr)->field = (fieldtype)NULL; \
    } \
    lua_pop(L, 1);

#define FM_ARRAY_START(structPtr, Lindex, field) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    {

#define FM_ARRAY_END(structPtr, Lindex, field) \
    } \
    lua_pop(L, 1);

#define FM_ARRAY_NUMBER(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field[index] = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define FM_ARRAY_STRUCT(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_mask_ ## fieldtype(L, &((structPtr)->field[index])); \
    } \
    lua_pop(L, 1);

#define FM_ARRAY_ENUM(structPtr, field, index, fieldtype) \
    lua_rawgeti(L, -1, index); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        (structPtr)->field[index] = (fieldtype)0; \
    } \
    lua_pop(L, 1);


/***** UNION MACROSES *****/
#define F_UNION_N(structPtr, Lindex, field, fieldname, fieldtype) \
    lua_getfield(L, Lindex, #fieldname); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, &((structPtr)->field)); \
    } \
    lua_pop(L, 1);

#define F_UNION(structPtr, Lindex, field, fieldtype) \
        F_UNION_N(structPtr, Lindex, field, field, fieldtype)

#define FO_UNION_N(structPtr, Lindex, field, fieldname, fieldtype) \
    prv_c_to_lua_ ## fieldtype(L, &((structPtr)->field)); \
    lua_setfield(L, Lindex, #fieldname);

#define FO_UNION(structPtr, Lindex, field, fieldtype) \
    FO_UNION_N(structPtr, Lindex, field, field, fieldtype)

#define F_UNION_MEMBER_NUMBER(unionObject,Lindex,field,fieldtype) \
    F_UNION_NUMBER(unionObject,Lindex,field,fieldtype)

#define F_UNION_MEMBER_ARRAY(unionObject,Lindex,field,fieldtype) \
	F_STRUCT_N(unionObject, Lindex, field, field, fieldtype)

#define F_UNION_MEMBER_BOOL(unionObject,Lindex,field,fieldtype) \
    F_UNION_BOOL(unionObject,Lindex,field,field,fieldtype)

#define F_UNION_MEMBER_STRUCT(unionObject,Lindex,field,fieldtype) \
    F_STRUCT(unionObject,Lindex,field,fieldtype)

#define FO_UNION_MEMBER_NUMBER(unionObject,Lindex,field,fieldtype) \
    FO_NUMBER(&unionObject,Lindex,field,fieldtype)

#define FO_UNION_MEMBER_STRUCT(unionObject,Lindex,field,fieldtype) \
    FO_STRUCT(&unionObject,Lindex,field,fieldtype)

#define PRV_UNION_LUA_TO_C_START(uniontype,member) \
void prv_lua_to_c_ ## uniontype ## _ ## member( \
    lua_State         *L, \
    uniontype         *val \
) \
{
#define PRV_UNION_LUA_TO_C_END() \
}

#define PRV_UNION_C_TO_LUA_START(uniontype,member) \
void prv_c_to_lua_ ## uniontype ## _ ## member( \
    lua_State         *L, \
    uniontype         *val \
) \
{ \
    int t; \
    lua_newtable(L); \
    t = lua_gettop(L);
#define PRV_UNION_C_TO_LUA_END() \
}


#define add_mgm_type_union(typeName,member) \
int mgmType_to_c_ ## typeName ## _ ## member( \
    lua_State *L \
) \
{ \
    typeName *val = NULL; \
    lua_newuserdata(L, sizeof(typeName)); \
    if (lua_isnil(L, 1)) \
    { \
        return 1; \
    } \
 \
    val = (typeName*)lua_touserdata(L, -1); \
    cmdOsMemSet(val, 0, sizeof(*val)); \
    lua_pushvalue(L, 1); \
    prv_lua_to_c_ ## typeName ## _ ## member(L, val); \
    while (lua_gettop(L) != 2) \
        lua_pop(L, 1); \
    return 1; \
} \
 \
int mgmType_to_lua_ ## typeName ## _ ## member( \
    lua_State *L \
) \
{ \
    typeName *val; \
    if (lua_isnil(L, 1)) \
        return 1; \
    if (!lua_isuserdata(L, 1)) \
    { \
        lua_pushnil(L); \
        return 1; \
    } \
 \
    val = (typeName*)lua_touserdata(L, 1); \
    prv_c_to_lua_ ## typeName ## _ ## member(L, val); \
 \
    return 1; \
}

#define UNION_MEMBER_STRUCT(typename,member,membertype) \
use_prv_struct(membertype) \
PRV_UNION_LUA_TO_C_START(typename,member) \
    F_STRUCT(val, -1, member, membertype); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_STRUCT(val, t, member, membertype); \
PRV_UNION_C_TO_LUA_END() \
 \
add_mgm_type_union(typename,member);

/* UNION MEMBER NUMBER */
#define UNION_MEMBER_NUMBER(typename,member,membertype) \
PRV_UNION_LUA_TO_C_START(typename, member) \
    F_NUMBER(val, -1, member, membertype); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_NUMBER(val, t, member, membertype); \
PRV_UNION_C_TO_LUA_END() \
 \
add_mgm_type_union(typename,member);


/* UNION MEMBER ARRAY */
#define UNION_MEMBER_ARRAY(typename,member,membertype,params) \
PRV_UNION_LUA_TO_C_START(typename, member) \
    F_ARRAY_START(val, -1, member); \
    { \
        int idx; \
        for (idx = 0; idx < params; idx++) { \
            F_ARRAY_NUMBER(val, member, idx, membertype); \
        } \
    } \
    F_ARRAY_END(val, -1, temp); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_ARRAY_START(val, t, member); \
    { \
        int idx; \
        for (idx = 0; idx < params; idx++) { \
            FO_ARRAY_NUMBER(val, member, idx, membertype); \
        } \
    } \
    FO_ARRAY_END(val, t, temp); \
	PRV_UNION_C_TO_LUA_END() \
 \
add_mgm_type_union(typename,member);

/* UNION MEMBER BOOLEAN */
#define UNION_MEMBER_BOOL(typename,member,membertype) \
PRV_UNION_LUA_TO_C_START(typename, member) \
    F_BOOL(val, -1, member); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_BOOL(val, t, member); \
PRV_UNION_C_TO_LUA_END() \
 \
add_mgm_type_union(typename,member);

/* support macroses: private Lua wrappers data printing */
#define P_MESSAGE(message)                                                  \
    cpssOsPrintf(message "\n");

#define P_MESSAGE1(message, arg1)                                           \
    cpssOsPrintf(message "\n", arg1);

#define P_MESSAGE4(message, arg1, arg2, arg3, arg4)                         \
    cpssOsPrintf(message "\n", arg1, arg2, arg3, arg4);

#define P_MESSAGE6(message, arg1, arg2, arg3, arg4, arg5, arg6)             \
    cpssOsPrintf(message "\n", arg1, arg2, arg3, arg4, arg5, arg6);

#define P_CONDITIONAL_MESSAGE(condition, message)                           \
    if (condition) { cpssOsPrintf(message "\n"); }

#define P_BOOL_N(structPtr, structName, field, fieldname)                   \
    cpssOsPrintf("%s." #fieldname " = %s\n", structName,                    \
                 GT_TRUE == (structPtr)->field? "GT_TRUE":"GT_FALSE")

#define P_BOOL(structPtr, structName, field)                                \
    P_BOOL_N(structPtr, structName, field, field)

#define P_NUMBER_N(structPtr, structName, field, fieldname, fieldtype)      \
    cpssOsPrintf("%s." #fieldname " = %d\n", structName, (structPtr)->field)

#define P_NUMBER(structPtr, structName, field, fieldtype)                   \
    P_NUMBER_N(structPtr, structName, field, field, fieldtype)

#define P_ENUM_N(structPtr, structName, field, fieldname, fieldtype)        \
    {                                                                       \
        const GT_CHAR* string_ ## fieldtype;                                \
        char temp[100];                                                     \
            string_ ## fieldtype = prvCpssGetEnumString(                    \
                    (GT_U32) ((structPtr)->field), # fieldtype, temp, 100); \
        if (NULL != string_ ## fieldtype)                                   \
        {                                                                   \
            cpssOsPrintf("%s." #fieldname " = %s\n", structName,            \
                         string_ ## fieldtype);                             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            cpssOsPrintf("%s." #fieldname " = %u\n", structName,            \
                         (unsigned int) ((structPtr)->field));              \
        }                                                                   \
    }

#define P_ENUM(structPtr, structName, field, fieldtype)                     \
    P_ENUM_N(structPtr, structName, field, field, fieldtype)

#define P_STRUCT_N(structPtr, structName, field, fieldname, fieldtype)      \
    prv_lua_c_print_ ## fieldtype(structName, #fieldname, &((structPtr)->field))

#define P_STRUCT(structPtr, structName, field, fieldtype)                   \
    P_STRUCT_N(structPtr, structName, field, field, fieldtype)

#define P_ARRAY_START(structPtr, structName, field)

#define P_ARRAY_END(structPtr, structName, field)

#define P_ARRAY_NUMBER(structPtr, structName, field, index, fieldtype)      \
    cpssOsPrintf("%s." #field "[%d] = %d\n", structName, index,             \
                 (structPtr)->field[index])

#define P_STRUCT_ARRAY_START(structPtr, structName)

#define P_STRUCT_ARRAY_END(structPtr, structName)

#define P_STRUCT_ARRAY_ITEM(item, name, fieldtype)                          \
    prv_lua_c_print_ ## fieldtype(name, NULL, (item));


#define P_ARRAY_STRUCT(structPtr, structName, field, index, fieldtype)      \
    prv_lua_c_print_ ## fieldtype(structName.field, &((structPtr)->field[index]))

#define P_ARRAY_ENUM(structPtr, structName, field, index, fieldtype)        \
    cpssOsPrintf("%s." #field " = %d\n", structName, (structPtr)->field[index])

#define P_NAME_COMPLEX_CONVERSIONS(target_name, original_name, separator,   \
                                                           name_extension)  \
    GT_CHAR_PTR     target_name   = original_name;                          \
    GT_CHAR         target_name ## _ ## name_extension ##                   \
                            _buffer[LUA_DEBUG_PRINT_NAME_BUFFER_MAX_SIZE];  \
                                                                            \
    if ((NULL != name_extension) && (0 < cpssOsStrlen(name_extension)))     \
    {                                                                       \
        if (LUA_DEBUG_PRINT_NAME_BUFFER_MAX_SIZE - 1 >                      \
                cpssOsStrlen(original_name) +                               \
                cpssOsStrlen(separator) +                                   \
                cpssOsStrlen(name_extension))                               \
        {                                                                   \
            cpssOsStrCpy(target_name ## _ ##  name_extension ## _buffer,    \
                                                          original_name);   \
            cpssOsStrCat(target_name ## _ ##  name_extension ## _buffer,    \
                                                              separator);   \
            cpssOsStrCat(target_name ## _ ##  name_extension ## _buffer,    \
                                                         name_extension);   \
            target_name = target_name ## _ ##  name_extension ## _buffer;   \
        }                                                                   \
        else                                                                \
        {                                                                   \
            target_name = name_extension;                                   \
        }                                                                   \
    }

#define P_NAME_CONVERSIONS(target_name, original_name, name_extension)      \
    P_NAME_COMPLEX_CONVERSIONS(target_name, original_name,                  \
                               LUA_DEBUG_PRINT_NAME_SEPARATOR,              \
                               name_extension)

#define P_NAME_CONVERSIONS_BEGIN(target_name, original_name, name_extension) \
    {                                                                       \
        GT_CHAR target_name ## _extension[] = name_extension;               \
        P_NAME_COMPLEX_CONVERSIONS(target_name, original_name,              \
                                   LUA_DEBUG_PRINT_NAME_SEPARATOR,          \
                           target_name ## _extension)

#define P_NAME_CONVERSIONS_END(target_name, original_name, name_extension)  \
    }

#define P_ARRAY_ITEM_NAME_CONVERSIONS_BEGIN(target_name, original_name,     \
                                                            item_number)    \
    {                                                                       \
        GT_CHAR target_name ## _extension[16];                              \
        cpssOsSprintf(target_name ## _extension, "[%u]",                    \
                (unsigned int) (item_number));                              \
        {                                                                   \
            P_NAME_COMPLEX_CONVERSIONS(target_name, original_name,          \
                                       LUA_DEBUG_PRINT_NAME_EMPTY_SEPARATOR,\
                                       target_name ## _extension)


#define P_ARRAY_ITEM_NAME_CONVERSIONS_END(target_name, original_name,       \
                                                          item_number)      \
        }                                                                   \
    }


/* add
 *    mgmType_to_c_ ## typeName()
 *    mgmType_to_lua_ ## typeName()
 * functions assuming
 *    prv_c_to_lua_ ## typeName() and
 *    prv_lua_to_c_ ## typeName()
 * already created
 */
#define __add_mgm_type_to_c(typeName) \
int mgmType_to_c_ ## typeName( \
    lua_State *L \
) \
{ \
    typeName *val = NULL; \
    int num_elem = 1; \
    int arr = 0; \
    if (lua_isnumber(L, 2)) \
    { \
        num_elem = (int)lua_tonumber(L, 2); \
        if (num_elem < 1) \
            return 0; \
        arr = 1; \
        lua_pop(L, 1); \
    } \
    lua_newuserdata(L, sizeof(typeName)*num_elem); \
    \
    val = (typeName*)lua_touserdata(L, -1); \
    cpssOsMemSet(val, 0, sizeof(*val)*num_elem); \
    if (lua_isnil(L, 1)) \
    { \
        return 1; \
    } \
    if (!arr) \
    { \
        lua_pushvalue(L, 1); \
        prv_lua_to_c_ ## typeName(L, val); \
        while (lua_gettop(L) != 2) \
            lua_pop(L, 1);\
        return 1; \
    } \
    if (lua_type(L, 1) != LUA_TTABLE) \
        return 1; \
    for (arr = 0; arr < num_elem; arr++) \
    { \
        lua_rawgeti(L, 1, arr); \
        if (!lua_isnil(L, -1)) \
        { \
            prv_lua_to_c_ ## typeName(L, val+arr); \
        } \
        lua_pop(L, 1); \
    } \
    return 1; \
}

#define __add_mgm_type_to_c_gc(typeName) \
static int mgmType_gc_ ## typeName(lua_State *L) \
{ \
    typeName * val; \
    if (lua_type(L,1) != LUA_TUSERDATA) \
        return 0; \
    val = (typeName*)lua_touserdata(L, 1); \
    if (val != NULL) \
        prv_gc_ ## typeName(val); \
    return 0; \
} \
int mgmType_to_c_ ## typeName( \
    lua_State *L \
) \
{ \
    typeName *val = NULL; \
    lua_newuserdata(L, sizeof(typeName)); \
    \
    val = (typeName*)lua_touserdata(L, -1); \
    cpssOsMemSet(val, 0, sizeof(*val)); \
    if (lua_isnil(L, 1)) \
    { \
        return 1; \
    } \
    lua_newtable(L); \
    lua_pushcfunction(L, mgmType_gc_ ## typeName); \
    lua_setfield(L, -2, "__gc"); \
    lua_setmetatable(L, -2); \
    \
    lua_pushvalue(L, 1); \
    prv_lua_to_c_ ## typeName(L, val); \
    lua_pop(L, 1); \
    return 1; \
}

#define __add_mgm_type_to_lua(typeName) \
int mgmType_to_lua_ ## typeName( \
    lua_State *L \
) \
{ \
    typeName *val; \
    int num_elem = 1; \
    int arr = 0; \
    if (lua_isnumber(L, 2)) \
    { \
        num_elem = (int)lua_tonumber(L, 2); \
        if (num_elem < 1) \
            return 0; \
        arr = 1; \
        lua_pop(L, 1); \
    } \
    if (lua_isnil(L, 1)) \
        return 1; \
    if (!lua_isuserdata(L, 1)) \
    { \
        lua_pushnil(L); \
        return 1; \
    } \
 \
    val = (typeName*)lua_touserdata(L, 1); \
    if (!arr) \
    { \
        prv_c_to_lua_ ## typeName(L, val); \
        return 1; \
    } \
    lua_newtable(L); \
    for (arr = 0; arr < num_elem; arr++) \
    { \
        prv_c_to_lua_ ## typeName(L, val+arr); \
        lua_rawseti(L, -2, arr); \
    } \
 \
    return 1; \
}

#define add_mgm_type(typeName) \
    __add_mgm_type_to_c(typeName) \
    __add_mgm_type_to_lua(typeName)

#define add_mgm_type_gc(typeName) \
    __add_mgm_type_to_c_gc(typeName) \
    __add_mgm_type_to_lua(typeName)

#define add_mgm_array_type(typeName) \
struct __structEncl_ ## typeName { typeName val; }; \
int mgmType_to_c_ ## typeName( \
    lua_State *L \
) \
{ \
    struct __structEncl_ ## typeName *val; \
    lua_newuserdata(L, sizeof(typeName)); \
    val = (struct __structEncl_ ## typeName *)lua_touserdata(L, -1); \
    \
    cpssOsMemSet(val, 0, sizeof(*val)); \
    if (lua_isnil(L, 1)) \
    { \
        return 1; \
    } \
    lua_pushvalue(L, 1); \
    prv_lua_to_c_ ## typeName(L, val->val); \
    lua_pop(L, 1); \
    return 1; \
} \
int mgmType_to_lua_ ## typeName( \
    lua_State *L \
) \
{ \
    struct __structEncl_ ## typeName *val; \
    if (lua_isnil(L, 1)) \
        return 1; \
    if (!lua_isuserdata(L, 1)) \
    { \
        lua_pushnil(L); \
        return 1; \
    } \
 \
    val = (struct __structEncl_ ## typeName *)lua_touserdata(L, 1); \
    prv_c_to_lua_ ## typeName(L, val->val); \
 \
    return 1; \
}

#endif /* __genwrapper_API_h */

