/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file luaCLIEngine_genWrapper.c
*
* @brief a generic wrapper to CPSS APIs
*
* @version   1
********************************************************************************
*/

#define CPSS_GENWRAPPER_ENGINE
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <extUtils/luaCLI/luaCLIEngine.h>
#include <cpssCommon/private/mxml/prvCpssMxml.h>

#define MAX_PARAMS 30
#define MAX_TYPE_LENGTH 30

#define GET_ENUM_VALUE      0
#define GET_ENUM_STRING     1
#define ENUM_ARRAY_MAZ_SIZE 100

/* initialized in function: prvLuaCpssAPI_register */
mxml_node_t *cpssapi_enums = NULL;      /* pointer to xml enum section */
mxml_node_t *cpssapi_structures = NULL; /* pointer to xml structures section */

typedef enum {FALSE = 0, TRUE = !FALSE} bool;

typedef enum
{
    PARAM_DIRECTION_IN_E,
    PARAM_DIRECTION_OUT_E,
    PARAM_DIRECTION_INOUT_E,
    PARAM_DIRECTION_WRONG_E
} PARAM_DIRECTION_ENT;
typedef enum {
    PARAM_TYPE_VAL_U32_E, /* IN GT_U32 */
    PARAM_TYPE_VAL_U16_E, /* IN GT_U16 */
    PARAM_TYPE_VAL_U8_E,  /* IN GT_U8 */
    PARAM_TYPE_VAL_U64_E, /* IN GT_U64 */
    PARAM_TYPE_PTR_U32_E, /* OUT GT_U32* */
    PARAM_TYPE_PTR_U16_E, /* OUT GT_U16* */
    PARAM_TYPE_PTR_U8_E,  /* OUT GT_U8* */
    PARAM_TYPE_PTR_U64_E,  /* OUT GT_U64* */
    PARAM_TYPE_PTR_32_E,  /* OUT GT_32* */
    PARAM_TYPE_PTR_16_E,  /* OUT GT_16* */
    PARAM_TYPE_PTR_8_E,   /* OUT GT_8* */
    PARAM_TYPE_PTR_PTR_E, /* OUT GT_PTR* */
    PARAM_TYPE_PTR_E,     /* _STC*  ARR*  */
    PARAM_TYPE_ARR_NUM_E, /* Array of numbers */
    PARAM_TYPE_ARR_PTR_E, /* Array of pointers */
    PARAM_TYPE_UNKNOWN_E
} PARAM_TYPE_ENT;

/*************************************************************************
* mgmType_to_c_int
* mgmType_to_c_GT_U8
* mgmType_to_c_GT_U16
* mgmType_to_c_GT_U32
*
* Description:
*       Convert to integer/GT_U8/GT_U16/GT_U32
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_c_int(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushnumber(L, 0);
    else if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}


/*************************************************************************
* mgmType_to_lua_int
* mgmType_to_lua_GT_U8
* mgmType_to_lua_GT_U16
* mgmType_to_lua_GT_U32
*
* Description:
*       Convert integer/GT_U8/GT_U16/GT_U32 to number
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_lua_int(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_c_bool
* mgmType_to_c_GT_BOOL
*
* Description:
*       Convert to integer(GT_BOOL)
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_c_bool(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushnumber(L, 0);
    else if (lua_isboolean(L, 1))
        lua_pushnumber(L, lua_toboolean(L, 1));
    else if (lua_isnumber(L, 1))
        lua_pushnumber(L, lua_tointeger(L, 1) == 0 ? 0 : 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_bool
* mgmType_to_lua_GT_BOOL
*
* Description:
*       Convert integer(GT_BOOL) to boolean
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_lua_bool(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushboolean(L, lua_tointeger(L, 1) == 0 ? 0 : 1);
    return 1;
}

/*************************************************************************
* string_equal_to
*
* Description:
*       Compare string at the top of Lua's stack with constant string
*
* Parameters:
*       string at the top of
*
* Returns:
*  Nonzero if strings equal
*  zero if not equal
*
*************************************************************************/
static int string_equal_to(lua_State *L, const char *literal)
{
    int result;
    lua_pushstring(L, literal);
    result = lua_equal(L, -1, -2);
    lua_pop(L, 1);
    return result;
}

/*************************************************************************
* prvInterpretParameterDirection
*
* Description:
*       Convert parameter direction ("IN", "OUT", "INOUT") to enum
*
* Parameters:
*       string at the top of stack
*
* Returns:
*  PARAM_DIRECTION_WRONG_E if error
*  enum value if successful
*
*************************************************************************/
static PARAM_DIRECTION_ENT prvInterpretParameterDirection(lua_State *L)
{
    PARAM_DIRECTION_ENT ret = PARAM_DIRECTION_WRONG_E;
    if (string_equal_to(L, "IN"))
        ret = PARAM_DIRECTION_IN_E;
    if (string_equal_to(L, "OUT"))
        ret = PARAM_DIRECTION_OUT_E;
    if (string_equal_to(L, "INOUT"))
        ret = PARAM_DIRECTION_INOUT_E;
    lua_pop(L, 1);
    return ret;
}

/*************************************************************************
* prvCheckArraySuffix
*
* Description:
*       Check if type has array suffix [ddd] where ddd is number in decimal
*       remove suffix
*
* Parameters:
*       string at the top of stack
*
* Returns:
*  -1 if no suffix
*  -2 if error
*  number
*
*************************************************************************/
static int prvCheckArraySuffix(lua_State *L)
{
    GT_CHAR *p, *d;
    const char *s = lua_tostring(L, -1);
    int n = 0;
    p = cpssOsStrChr((const GT_CHAR*)s, '[');
    if (p == NULL)
        return -1;
    if (p[1] == ']')
        return -2;

    for (d = p+1; *d && *d != ']'; d++)
    {
        if (*d < '0' || *d > '9')
            return -2;
        n = n * 10 + (*d - '0');
    }
    if (d[0] != ']' || d[1] != 0)
        return -2;
    lua_pushlstring(L, s, p-s);
    lua_remove(L, -2);
    return n;
}


/* one parameter description for I/O */
typedef struct PRV_PARAM_DATA_STCT {
    GT_UINTPTR          param;
    GT_UINTPTR          value;
    PARAM_DIRECTION_ENT typeIO;
    PARAM_TYPE_ENT      pType;
    int                 elemSize;
    int                 arraySize;
} PRV_PARAM_DATA_STC;

/*************************************************************************
* prvGetTypeCvtFunc
*
* Description:
*
*
* Parameters:
*       typename string at the top of stack
*       p - parameter data (C value, type, etc)
*
* Returns:
*    1 if suceeded, function on the top of stack
*    0 if error
*  number
*
*************************************************************************/
static int prvGetTypeCvtFunc(
    lua_State               *L,
    LUA_CLI_GETFUNCBYNAME_T getFunctionByName,
    PRV_PARAM_DATA_STC     *p,
    int                     param,
    int                     isLuaToC
)
{
    GT_U32 str_len;

    lua_CFunction cvtFunc;
    if (!lua_isstring(L, -1))
    {
        lua_pushfstring(L, "paramter type for parameter %d is not string", param);
        return 0;
    }

    /* stk: typename; params[paramNo] */
    if (isLuaToC)
        lua_pushliteral(L, "mgmType_to_c_");
    else
        lua_pushliteral(L, "mgmType_to_lua_");

    /* stk: "mgmType_to_XXX_" ; typename; params[paramNo] */
    lua_insert(L, -2);

    /* stk: typename; "mgmType_to_XXX_"; params[paramNo] */
    lua_concat(L, 2);

    /* stk: "mgmType_to_XXX_typename"; params[paramNo] */
    lua_getglobal(L, lua_tostring(L, -1));
    if (lua_isfunction(L, -1))
        return 1;
    lua_pop(L, 1);

    str_len = lua_objlen(L,-1);

    /* check if the typename is IN/INOUT & union structure:
       last 3 chars of substring should be "UNT" */
    if ( (p->typeIO != PARAM_DIRECTION_OUT_E) &&
         (cpssOsStrNCmp(lua_tostring(L, -1) + ((GT_U32)str_len - 3), "UNT", 3 ) == 0) )
    {
        /* stk: "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        /* params table is in the stack at index '-2' */
        lua_rawgeti(L, -2, 4); /* push only data onto stack ( params[param][4] ) */
        if (!lua_istable(L, -1))
        {
            lua_pushfstring(L, "params.param[4] is not a table" );
            return 1;
        }

        /* stk: params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        lua_pushnil(L);

        /* stk: nil ;params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        lua_next(L, -2);

        /* stk: value; keyName; params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        lua_pop(L, 1); /* 'pop value' */


        /* stk: keyName; params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        if (lua_isnumber(L, -1)){ /* in case of number insead of keyName - break from sequence */
            lua_pop(L, 2); /* pop 2 values from stack */
            goto TypeCvtFuncLabel;
        }

        /* stk: keyName; params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; params[paramNo] */
        lua_insert(L, -2);

        /* stk: params[paramNo][4] ; "mgmType_to_XXX_typename(UNT)"; keyName; params[paramNo] */
        lua_pop(L, 1);

        /* stk: "mgmType_to_XXX_typename(UNT)"; keyName; params[paramNo] */
        lua_pushliteral(L, "_");

        /* stk: "_"; "mgmType_to_XXX_typename(UNT)"; keyName; params[paramNo] */
        lua_insert(L, -2);

        /* stk: "mgmType_to_XXX_typename(UNT)"; "_"; keyName; params[paramNo] */
        lua_concat(L, 3);

        /* stk: "mgmType_to_XXX_typename(UNT)_keyName"; params[paramNo] */
    }

TypeCvtFuncLabel:
    /* stk: "mgmType_to_XXX_typename"; params[paramNo] */
    cvtFunc = (lua_CFunction) getFunctionByName(lua_tostring(L, -1));
    if (cvtFunc)
    {
        lua_pushcfunction(L, cvtFunc);
        return 1;
    }
    lua_pushfstring(L, "function '%s' is not defined", lua_tostring(L, -1));
    return 0;
}

/*************************************************************************
* prvParseParam
*
* Description:
*
*
* Input:
*   paramNo     - parameter number (1..)
*
* Output:
*   p           - parameter data (C value, type, etc)
*
* Returns:
*    1 if suceeded, function on the top of stack
*    0 if error
*  number
*
*************************************************************************/
static int prvParseParam(
    lua_State              *L,
    LUA_CLI_GETFUNCBYNAME_T getFunctionByName,
    int                     paramNo,
    PRV_PARAM_DATA_STC     *p
)
{
    int i;
    p->arraySize = -1;
    p->elemSize = 0;
    lua_rawgeti(L, 2, paramNo); /* params[param] */
    if (!lua_istable(L, -1))
    {
        lua_pushfstring(L, "params[%d] is not a table", paramNo);
        return 1;
    }
    lua_rawgeti(L, -1, 1); /* params[paramNo][1] */
    /* stk: params[paramNo][1]; params[param] */
    p->typeIO = prvInterpretParameterDirection(L);
    if (p->typeIO == PARAM_DIRECTION_WRONG_E)
    {
        lua_pushfstring(L, "bad parameter type for parameter %d, must be IN|OUT|INOUT", paramNo);
        return 1;
    }
    lua_rawgeti(L, -1, 2); /* params[paramNo][2] */
    /* stk: params[paramNo][2]; params[param] */
    if (!lua_isstring(L, -1))
    {
        lua_pushfstring(L, "no type specified for parameter %d", paramNo);
        return 1;
    }
    p->arraySize = prvCheckArraySuffix(L);
    if (p->arraySize == -2)
    {
        lua_pushfstring(L, "wrong type of parameter %d", paramNo);
        return 1;
    }
    /* stk: typename; params[paramNo] */
    /* TODO: handle arraySize==0 */
    if (p->typeIO == PARAM_DIRECTION_IN_E &&
            (string_equal_to(L, "string") || string_equal_to(L, "GT_CHAR")))
    {
        /* special case: `IN string' will be passed as `const char*' */
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 4);
        /* stk: value; params[paramNo] */
        if (!lua_isstring(L, -1))
        {
            lua_pushfstring(L, "parameter %d is string but the value doesn't", paramNo);
            return 1;
        }
        lua_remove(L, -2); /* params[paramNo] */
        /* stk: value */
        p->pType = PARAM_TYPE_PTR_E;
        p->value = (GT_UINTPTR)lua_tostring(L, -1);
        return 0;
    }
    /* stk: typename; params[paramNo] */
    if (p->arraySize > 0)
    {
        /* Array */
        p->pType = PARAM_TYPE_PTR_E;
        p->elemSize = -1;
        if (string_equal_to(L, "GT_U16"))
        {
            p->elemSize = 2;
            p->pType = PARAM_TYPE_ARR_NUM_E;
        }
        if (string_equal_to(L, "GT_U8"))
        {
            p->elemSize = 1;
            p->pType = PARAM_TYPE_ARR_NUM_E;
        }

        if (p->elemSize < 0)
        {
            if (!prvGetTypeCvtFunc(L, getFunctionByName, p, paramNo, 1))
                return 1;
            /* stk: cvt_func; typename; params[paramNo] */
            /* first call to detect element size */
            /* stk: cvt_func; typename; params[paramNo] */
            lua_pushvalue(L, -1); /* duplicate function */
            lua_pushnil(L);
            if (lua_pcall(L, 1, 1, 0))
            {
                lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
                lua_insert(L, -2);
                lua_concat(L, 2);
                return 1;
            }
            /* stk: retVal; cvt_func; typename; params[paramNo] */
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                p->elemSize = sizeof(GT_U32);
                p->pType = PARAM_TYPE_ARR_NUM_E;
            }
            else if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
            {
                p->elemSize = sizeof(GT_UINTPTR);
                p->pType = PARAM_TYPE_ARR_PTR_E;
            }
            else if (lua_type(L, -1) == LUA_TUSERDATA)
            {
                p->elemSize = lua_objlen(L, -1);
            }
            else
            {
                lua_pushfstring(L, "p1: not supported parameter %d", paramNo);
                return 1;
            }
            lua_pop(L, 1);
            /* stk: cvt_func; typename; params[paramNo] */
        }
        else
        {
            lua_pushnil(L);
            /* stk: nil; typename; params[paramNo] */
        }
        lua_newuserdata(L, p->arraySize * p->elemSize);
        cpssOsMemSet(lua_touserdata(L, -1), 0, lua_objlen(L, -1));
        /* stk: userdata; nil/cvt_func; typename; params[paramNo] */
        lua_insert(L, -4);
        /* stk: nil/cvt_func; typename; params[paramNo]; userdata */
        p->value = (GT_UINTPTR)lua_touserdata(L, -4);
        if (p->typeIO == PARAM_DIRECTION_OUT_E)
        {
            lua_pop(L, 3);
            /* stk: userdata */
            return 0;
        }
        /* typeIO = IN || INOUT */
        /* stk: nil/cvt_func; typename; params[paramNo]; userdata */
        lua_rawgeti(L, -3, 4);
        if (!lua_istable(L, -1))
        {
            lua_pushfstring(L, "parameter %d value must be a table",paramNo);
            return 1;
        }
        /* stk: table; nil/cvt_func; typename; params[paramNo]; userdata */
        for (i = 0; i < p->arraySize; i++)
        {
            GT_UINTPTR elemPtr = p->value + p->elemSize * i;
            if (p->elemSize == 1 || p->elemSize == 2)
            {
                /* GT_U8 || GT_U16 */
                lua_rawgeti(L, -1, i);
                if (lua_type(L, -1) == LUA_TNUMBER)
                {
                    if (p->elemSize == 1)
                        *((GT_U8*)elemPtr) = (GT_U8)lua_tonumber(L, -1);
                    else /* p->elemSize == 2 */
                        *((GT_U16*)elemPtr) = (GT_U16)lua_tonumber(L, -1);
                }
                if (lua_type(L, -1) != LUA_TNUMBER && lua_type(L, -1) != LUA_TNIL)
                {
                    lua_pushfstring(L, "parameter %d[%d] expected to be number", paramNo, i);
                    return 1;
                }
                lua_pop(L, 1);
                continue;
            }
            lua_pushvalue(L, -2); /* duplicate function */
            /* stk: cvt_func; table; cvt_func; typename; params[paramNo]; userdata */
            lua_rawgeti(L, -2, i);
            /* stk: table[i]; cvt_func; table; cvt_func; typename; params[paramNo]; userdata */
            if (lua_pcall(L, 1, 1, 0))
            {
                lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -3));
                lua_insert(L, -2);
                lua_concat(L, 2);
                return 1;
            }
            /* stk: table[i](converted); table; cvt_func; typename; params[paramNo]; userdata */
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                *((GT_U32*)elemPtr) = (GT_U32)lua_tonumber(L, -1);
            }
            else if (lua_type(L, -1) == LUA_TUSERDATA)
            {
                cpssOsMemCpy((void*)elemPtr, lua_touserdata(L, -1), p->elemSize);
            }
            lua_pop(L, 1);
            /* stk: table; cvt_func; typename; params[paramNo]; userdata */
        }
        /* stk: table; nil/cvt_func; typename; params[paramNo]; userdata */
        lua_pop(L, 4);
        return 0;
    } /* array end */
    /* stk: typename; params[paramNo] */
    if (string_equal_to(L, "GT_U16") || string_equal_to(L, "GT_U8") || string_equal_to(L, "GT_U64_BIT") )
    {
        p->value = 0;
        if (string_equal_to(L, "GT_U16"))
            p->pType = PARAM_TYPE_VAL_U16_E;
        else if (string_equal_to(L, "GT_U8"))
            p->pType = PARAM_TYPE_VAL_U8_E;
        else
           p->pType = PARAM_TYPE_VAL_U64_E;

        if (p->typeIO != PARAM_DIRECTION_OUT_E)
        {
            lua_rawgeti(L, -2, 4);

            if ((lua_type(L, -1) == LUA_TNUMBER) ||
                ((lua_type(L, -1) == LUA_TSTRING) && (p->pType = PARAM_TYPE_VAL_U64_E)))
            {

                if (p->pType == PARAM_TYPE_VAL_U16_E)
                    p->value = (GT_U16)lua_tonumber(L, -1);
                else if ( p->pType == PARAM_TYPE_VAL_U8_E )
                    p->value = (GT_U8)lua_tonumber(L, -1);
                else
                    p->value = (GT_U64_BIT)strtoul(lua_tostring(L, -1),NULL,16);

            }

            if (( lua_type(L, -1) != LUA_TNUMBER && lua_type(L, -1) != LUA_TNIL ) &&
                ((lua_type(L, -1) != LUA_TSTRING) && (p->pType = PARAM_TYPE_VAL_U64_E)))
            {
                lua_pushfstring(L, "parameter %d[4] expected to be number", paramNo);
                return 1;
            }
            lua_pop(L, 1);
        }
        if (p->typeIO != PARAM_DIRECTION_IN_E)
        {
            if (p->pType == PARAM_TYPE_VAL_U64_E)
                p->pType = PARAM_TYPE_PTR_U64_E;
            else if (p->pType == PARAM_TYPE_VAL_U16_E)
                p->pType = PARAM_TYPE_PTR_U16_E;
            else
                p->pType = PARAM_TYPE_PTR_U8_E;
        }
        lua_pop(L, 2);
        return 0;
    } /* GT_U16 || GT_U8 */

    /* set parameter type */
    if (p->typeIO == PARAM_DIRECTION_IN_E)
        p->pType = PARAM_TYPE_VAL_U32_E;
    else
    {
        /* check if signed number */
        if (string_equal_to(L, "GT_32"))
            p->pType = PARAM_TYPE_PTR_32_E;
        else if (string_equal_to(L, "GT_16"))
            p->pType = PARAM_TYPE_PTR_16_E;
        else if (string_equal_to(L, "GT_8"))
            p->pType = PARAM_TYPE_PTR_8_E;
        else
            p->pType = PARAM_TYPE_PTR_U32_E;
    }

    /* stk: typename; params[paramNo] */
    if (!prvGetTypeCvtFunc(L, getFunctionByName, p, paramNo, 1))
        return 1;
    /* stk: cvt_func; typename; params[paramNo] */
    if (p->typeIO != PARAM_DIRECTION_OUT_E)
        lua_rawgeti(L, -3, 4);
    else
        lua_pushnil(L);
    /* stk: value/nil; cvt_func; typename; params[paramNo] */
    if (lua_pcall(L, 1, 1, 0))
    {
        lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
        lua_insert(L, -2);
        lua_concat(L, 2);
        return 1;
    }
    /* stk: value(converted); typename; params[paramNo] */
    if (lua_type(L, -1) == LUA_TNUMBER)
    {
        if ( p->pType == PARAM_TYPE_VAL_U32_E )
            p->value = (GT_UINTPTR)(GT_U32)(lua_tonumber(L, -1));
        else
            p->value = (GT_UINTPTR)(lua_tointeger(L, -1));

        lua_pop(L, 3);
        return 0;
    }
    /* stk: value(converted); typename; params[paramNo] */
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
    {
        p->value = (GT_UINTPTR)lua_touserdata(L, -1);
        p->pType = (p->typeIO == PARAM_DIRECTION_IN_E)
            ? PARAM_TYPE_PTR_E
            : PARAM_TYPE_PTR_PTR_E;
        lua_pop(L, 2);
        return 0;
    }
    if (lua_type(L, -1) != LUA_TUSERDATA)
    {
        lua_pushfstring(L, "parameter %d: wrong value/converter", paramNo);
        return 1;
    }
    /* userdata */
    /* stk: value(converted); typename; params[paramNo] */
    lua_insert(L, -3);
    lua_pop(L, 2);
    /* stk: value(converted) */
    p->value = (GT_UINTPTR)lua_touserdata(L, -1);
    p->pType = PARAM_TYPE_PTR_E;
    p->elemSize = lua_objlen(L, -1);
    return 0;
}

/*************************************************************************
* cpssGenWrapper
*
* Description:
*       Call CPSS API function
*
* Parameters:
*  cpssAPIname     - Fx: "prvCpssDrvHwPpReadRegister"
*  params          - table:
*                    each row is table of 3 elements:
*                    1. direction (IN | OUT | INOUT)
*                    2. parameter type
*                       the valid types:
*                       "int"
*                       "bool"
*                       "GT_U8",
*                       "GT_U16",
*                       "GT_U32",
*                       "string"
*                       "custom_type_name" with a set of functions:
*                          mgmType_to_c_${custom_type_name}(param)
*                                  - convert lua variable/table to
*                                    userdata object
*                          mgmType_to_lua_${custom_type_name}(param)
*                                  - convert userdata object to lua
*                    3. parameter name
*                    4. parameter value(optional, for IN and INOUT only)
*  noLock          - if true then don't protect call with mutex
*
*
* Returns:
*  GT_STATUS, array of output values if successful
*  string if error
*
*
* Example
* status, values = cpssGenWrapper(
*                      "prvCpssDrvHwPpReadRegister", {
*                           { "IN", "GT_U8", "devNum", 0 },
*                           { "IN", "GT_U32","regAddr", 0x200000 },
*                           { "OUT","GT_U32","regData" }
*                      })
* regData = values.regData
*************************************************************************/
static int cpssGenWrapper(
    lua_State *L
)
{
    PRV_PARAM_DATA_STC      params[MAX_PARAMS];
    GT_STATUS               retval;
    typedef GT_STATUS (*CPSS_API_FUNC_PTR)(GT_UINTPTR, ...);
    CPSS_API_FUNC_PTR funcPtr;
    int nparams;
    int k,values;

    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;

    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
    {
        lua_pushliteral(L, "_getFunctionByName not defined");
        return 1;
    }
    if (lua_gettop(L) < 2)
    {
        lua_pushliteral(L, "no enough parameters");
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushliteral(L, "bad parameter 1");
        return 1;
    }
    if (!lua_istable(L, 2))
    {
        lua_pushliteral(L, "bad parameter 2");
        return 1;
    }

    lua_settop(L, 2); /* remove extra parameters */

    funcPtr = (CPSS_API_FUNC_PTR) getFunctionByName(lua_tostring(L, 1));
    if (!funcPtr)
    {
        lua_pushliteral(L, "function '");
        lua_pushvalue(L, 1);
        lua_pushliteral(L, "' not found");
        lua_concat(L, 3);
        return 1;
    }

    cpssOsMemSet(params, 0, sizeof(params));
    /* prepare parameters */
    nparams = (int)lua_objlen(L, 2);
    for (k = 0; k < nparams; k++)
    {
        PRV_PARAM_DATA_STC *p;
        void *valPtr;
        p = &(params[k]);
        valPtr = (void*)(&(p->value));
        if (prvParseParam(L, getFunctionByName, k+1, p) != 0)
        {
            return 1;
        }

        switch (p->pType)
        {
            case PARAM_TYPE_VAL_U32_E: /* IN GT_U32 */
            case PARAM_TYPE_VAL_U16_E: /* IN GT_U16 */
            case PARAM_TYPE_VAL_U8_E:  /* IN GT_U8 */
            case PARAM_TYPE_VAL_U64_E:  /* IN GT_U8 */
                p->param = p->value;
                break;
            case PARAM_TYPE_PTR_U32_E: /* OUT GT_U32* */
                *((GT_U32*)valPtr) = (GT_U32)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_U16_E: /* OUT GT_U16* */
                *((GT_U16*)valPtr) = (GT_U16)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_U8_E:  /* OUT GT_U8* */
                *((GT_U8*)valPtr) = (GT_U8)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_U64_E:  /* OUT GT_U64* */
                *((GT_U64_BIT*)valPtr) = (GT_U64_BIT)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_32_E: /* OUT GT_32* */
                *((GT_32*)valPtr) = (GT_32)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_16_E: /* OUT GT_16* */
                *((GT_16*)valPtr) = (GT_16)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_8_E:  /* OUT GT_8* */
                *((GT_8*)valPtr) = (GT_8)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_PTR_E: /* OUT GT_PTR* */
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_E:     /* _STC*  ARR*  */
            case PARAM_TYPE_ARR_NUM_E: /* Array of numbers */
            case PARAM_TYPE_ARR_PTR_E: /* Array of pointers */
                p->param = p->value;
                break;
            case PARAM_TYPE_UNKNOWN_E:
                lua_pushfstring(L, "unknown parameter class for param %d", k+1);
                return 1;
        }
    }

    /* call function */
    retval = funcPtr(
        params[0].param,
        params[1].param,
        params[2].param,
        params[3].param,
        params[4].param,
        params[5].param,
        params[6].param,
        params[7].param,
        params[8].param,
        params[9].param,
        params[10].param,
        params[11].param,
        params[12].param,
        params[13].param,
        params[14].param,
        params[15].param,
        params[16].param,
        params[17].param,
        params[18].param,
        params[19].param,
        params[20].param,
        params[21].param,
        params[22].param,
        params[23].param,
        params[24].param,
        params[25].param,
        params[26].param,
        params[27].param,
        params[28].param,
        params[29].param);

    /* push results */
    lua_pushnumber(L, retval);
    lua_newtable(L);
    values = lua_gettop(L);

    for (k = 0; k < nparams; k++)
    {
        PRV_PARAM_DATA_STC *p;
        int paramDescIdx;
        void* valPtr;
        p = &(params[k]);
        valPtr = (void*)(&(p->value));
        if (p->typeIO == PARAM_DIRECTION_IN_E)
            continue;

        lua_rawgeti(L, 2, k+1); /* params[k+1] */
        paramDescIdx = lua_gettop(L); /* param = params[k+1] */
        /* stk: params[k+1] */

        lua_rawgeti(L, paramDescIdx, 2);
        prvCheckArraySuffix(L); /* strip "[ddd]" suffix */
        /* stk: typename; params[k+1] */

        if (p->arraySize > 0)
        {
            int i;
            /* Array */
            if (p->pType == PARAM_TYPE_VAL_U16_E || p->elemSize == 2)
            {
                lua_pushnil(L);
            }
            else
            {
                if (!prvGetTypeCvtFunc(L, getFunctionByName, p, k+1, 0))
                    return 1;
            }
            /* stk: nil/cvt_func; typename; params[k+1] */
            lua_newtable(L);
            /* stk: ret_tbl; nil/cvt_func; typename; params[k+1] */
            for (i = 0; i < p->arraySize; i++)
            {
                GT_UINTPTR elemPtr = p->value + p->elemSize * i;
                if (p->elemSize == 1 || p->elemSize == 2)
                {
                    /* GT_U8 || GT_U16 */
                    if (p->elemSize == 1)
                        lua_pushnumber(L, *((GT_U8*)elemPtr));
                    else /* p->elemSize == 2 */
                        lua_pushnumber(L, *((GT_U16*)elemPtr));
                    lua_rawseti(L, -2, i);
                    continue;
                }
                lua_pushvalue(L, -2);
                /* stk: cvt_func; ret_tbl; cvt_func; typename; params[k+1] */
                if (p->pType == PARAM_TYPE_ARR_NUM_E)
                {
                    /* array elem is number */
                    lua_pushnumber(L, *((GT_U32*)elemPtr));
                }
                else if (p->pType == PARAM_TYPE_ARR_PTR_E)
                {
                    /* array elem is pointer */
                    lua_pushlightuserdata(L, (void *)(*((GT_UINTPTR *)elemPtr)));
                }
                else
                {
                    lua_newuserdata(L, p->elemSize);
                    cpssOsMemCpy(lua_touserdata(L, -1), (void*)elemPtr, p->elemSize);
                }
                /* stk: value; cvt_func; ret_tbl; cvt_func; typename; params[k+1] */
                /* convert return values to lua */
                if (lua_pcall(L, 1, 1, 0))
                {
                    lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -4));
                    lua_insert(L, -2);
                    lua_concat(L, 2);
                    return 1;
                }
                /* stk: lua_value; ret_tbl; cvt_func; typename; params[k+1] */
                lua_rawseti(L, -2, i);
                /* stk: ret_tbl; cvt_func; typename; params[k+1] */
            }
            lua_rawgeti(L, paramDescIdx, 3);
            /* TODO: check name is string */
            /* stk: param_name; ret_tbl; cvt_func; typename; params[k+1] */
            lua_insert(L, -2);
            /* stk: ret_tbl; param_name; cvt_func; typename; params[k+1] */
            lua_setfield(L, values, lua_tostring(L, -2));
            /* stk: param_name; cvt_func; typename; params[k+1] */
            lua_pop(L, 4);
            continue;
        }

        /* Not array */
        if (!prvGetTypeCvtFunc(L, getFunctionByName, p, k+1, 0))
            return 1;
        /* stk: cvt_func; typename; params[k+1] */

        switch (p->pType)
        {
            case PARAM_TYPE_PTR_U32_E: /* OUT GT_U32* */
                lua_pushnumber(L, *((GT_U32*)valPtr));
                break;
            case PARAM_TYPE_PTR_U16_E: /* OUT GT_U16* */
                lua_pushnumber(L, *((GT_U16*)valPtr));
                break;
            case PARAM_TYPE_PTR_U8_E:  /* OUT GT_U8* */
                lua_pushnumber(L, *((GT_U8*)valPtr));
                break;
            case PARAM_TYPE_PTR_U64_E:  /* OUT GT_U64* */
                lua_pushnumber(L, *((GT_U64_BIT*)valPtr));
                break;
            case PARAM_TYPE_PTR_32_E: /* OUT GT_32* */
                lua_pushnumber(L, *((GT_32*)valPtr));
                break;
            case PARAM_TYPE_PTR_16_E: /* OUT GT_16* */
                lua_pushnumber(L, *((GT_16*)valPtr));
                break;
            case PARAM_TYPE_PTR_8_E:  /* OUT GT_8* */
                lua_pushnumber(L, *((GT_8*)valPtr));
                break;
            case PARAM_TYPE_PTR_PTR_E: /* OUT GT_PTR* */
#if 0
                lua_pushlightuserdata(L, (void*)p->value);
                break;
#else
                lua_pop(L, 2);
                /* stk: params[k+1] */
                lua_rawgeti(L, -1, 3);
                /* TODO: check name is string */
                lua_pushlightuserdata(L, (void*)p->value);
                lua_setfield(L, values, lua_tostring(L, -2));
                /* stk: params[k+1][3]; params[k+1] */
                lua_pop(L, 2);
                continue;
#endif
            case PARAM_TYPE_PTR_E:     /* _STC* */
                lua_newuserdata(L, p->elemSize);
                cpssOsMemCpy(lua_touserdata(L, -1), (void*)p->param, p->elemSize);
                break;
            default:
                lua_pushnil(L);
                break;/* should never happen */
        }
        /* stk: value; cvt_func; typename; params[k+1] */
        /* convert return values to lua */
        if (lua_pcall(L, 1, 1, 0))
        {
            lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
            lua_insert(L, -2);
            lua_concat(L, 2);
            return 1;
        }
        /* stk: lua_value; typename; params[k+1] */
        lua_rawgeti(L, paramDescIdx, 3);
        /* TODO: check name is string */
        /* stk: param_name; lua_value; typename; params[k+1] */
        lua_insert(L, -2);
        /* stk: lua_value; param_name; typename; params[k+1] */
        lua_setfield(L, values, lua_tostring(L, -2));
        /* stk: param_name; typename; params[k+1] */
        lua_pop(L, 3);
    }
    return 2;
}

/*************************************************************************
* cpssGenWrapperCheckParam
*
* Description:
*       mgmType_to_lua_${custom_type_name}(mgmType_to_c_${custom_type_name}(param))
*       This can be used for
*       1. Convert
*
* Parameters:
*       custom_type_name - Fx: "CPSS_MAC_ENTRY_EXT_STC"
*       param            - A value to check
*
*
* Returns:
*       output value if successful
*       string if error
*
*
* Example
*       value = cpssGenWrapperCheckParam("CPSS_MAC_ENTRY_EXT_STC", nil)
*       value.isStatic = true
*       value = cpssGenWrapperCheckParam("CPSS_MAC_ENTRY_EXT_STC", value)
*
*************************************************************************/
static int cpssGenWrapperCheckParam(
    lua_State *L
)
{
    int i;
    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;

    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
    {
        lua_pushliteral(L, "_getFunctionByName not defined");
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushliteral(L, "wrong type name");
        return 1;
    }
    lua_settop(L, 2);
    /*
     * push "mgmType_to_lua_${custom_type_name}"
     * push function mgmType_to_lua_${custom_type_name}
     * push "mgmType_to_c_${custom_type_name}"
     * push function mgmType_to_c_${custom_type_name}
     */
    for (i = 0; i < 2; i++)
    {
        /* convert lua value to C */
        lua_pushstring(L, i == 0 ? "mgmType_to_lua_" : "mgmType_to_c_");
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
        lua_getglobal(L, lua_tostring(L, -1));
        if (!lua_isfunction(L, -1))
        {
            /* try to lookup C function with the same name */
            lua_CFunction cvtFunc;
            cvtFunc = (lua_CFunction) getFunctionByName(lua_tostring(L, -2));
            if (cvtFunc)
            {
                lua_pop(L, 1);
                lua_pushcfunction(L, cvtFunc);
            }
        }
        if (!lua_isfunction(L, -1))
        {
            lua_pushfstring(L, "function '%s' is not defined", lua_tostring(L, -2));
            return 1;
        }
    }

    lua_pushvalue(L, 2);

    /*
     * call function mgmType_to_c_${custom_type_name} (param)
     * call function mgmType_to_lua_${custom_type_name}
     */
    for (i = 0; i < 2; i++)
    {
        if (lua_pcall(L, 1, 1, 0))
        {
            lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
            lua_insert(L, -2);
            lua_concat(L, 2);
            return 1;
        }
        /* remove function name */
        lua_remove(L, -2);
    }
    return 1;
}

/* This function convert integer to string */
char* itoa_func(int inNumbr, char* outString, int base)
{
    int i, j;
    bool isNeg = FALSE;
    int mod;
    char tmp;

    strcpy(outString, "0");

    if (inNumbr == 0)
        return outString;

    /* negative number */
    if (inNumbr < 0 && base == 10)
    {
        isNeg = TRUE;
        inNumbr = -inNumbr;
    }

    /* loop all digits and concatanate string */
    for( i = 0 ; inNumbr != 0 ; inNumbr = inNumbr/base )
    {
        mod = inNumbr % base;
        outString[i++] = (mod >= 10) ? (mod - 10) + 'a' : mod + '0';
    }

    /* add negative sign */
    if (isNeg)
        outString[i++] = '-';

    outString[i] = '\0'; /* add NULL character */

    for (j = 0 ; j < (i - 1) ; j++, i--) {
        tmp = outString[i - 1];
        outString[i - 1] = outString[j];
        outString[j] = tmp;
    }

    return outString;
}

/*************************************************************************
* get_cpssAPI_xml_enum
*
* Description:
*       Convert Enum value to string or vice versa, based on Enum typedef string
*
* Parameters:
*       enumString  (for example: "CPSS_PCL_LOOKUP_NUMBER_ENT") - can't be NULL
*       enumName    (for enumName="CPSS_PCL_LOOKUP_1_E" -> enumValue will return 1 )
*                   can't be NULL while enumGetKey=="GET_ENUM_VALUE"
*
*       enumValue   (for enumValue=1 -> enumName will return "CPSS_PCL_LOOKUP_1_E" )
*                   can't be NULL while enumGetKey=="GET_ENUM_STRING"
*
*       enumGetKey  ( GET_ENUM_VALUE or GET_ENUM_STRING )
*
* Returns:
*       integer:
*                   -1 - Failure
*                   0  - Success
*
*************************************************************************/
static int get_cpssAPI_xml_enum(
    const char      *enumString,
    char            *enumName,
    int             *enumValue,
    int              maxSize,
    int              enumGetKey
)
{
    mxml_node_t *node;
    char arr[ENUM_ARRAY_MAZ_SIZE]; /* for example: "CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_LAST_E" */
    char *search_attr = (char*)arr;
    const char *attr, *enum_name, *search_string;

    if ( ( enumName == NULL || enumValue == NULL ) || enumString == NULL ){
        cpssOsPrintf("\nget_cpssAPI_xml_enum Error - bad input parameters\n");
        return -1;
    }

    node = cpssapi_enums;

    while(TRUE){ /* lookup for enumartion in XML */
        /* get enum node from cpssAPI.xml */
        node = prvCpssMxmlFindElement(node, cpssapi_enums, "Enum", NULL, NULL, MXML_DESCEND);
        if (!node){
            cpssOsPrintf("\nget_cpssAPI_xml_enum Error - cpssAPI.xml is not initialized\n");
            return -1;
        }
        enum_name = prvCpssMxmlElementGetAttr(node,"name");
        if (cpssOsStrCmp(enum_name, enumString) != 0) { /* check if enum entered from CLI is valid */
            node = node->next;
            /* get next enum element node from cpssAPI.xml */
            continue;
        }
        break;
    };

    if ( enumGetKey == GET_ENUM_VALUE ){ /* search for enum value by string */
        search_attr = (void*) enumName;
        search_string = "str";
    }
    else{ /* GET_ENUM_STRING - search for enum string by enumarion */
        itoa_func(*enumValue, search_attr, 10);
        search_string = "num";
    }

    node = prvCpssMxmlFindElement(node, node, NULL, search_string , (const char*) search_attr, MXML_DESCEND);
    if (!node)
        goto xml_enum_error;

    if ( enumGetKey == GET_ENUM_VALUE )
        search_string = "num";

    else /* GET_ENUM_STRING */
        search_string = "str";

    /* get enum string/value node from cpssAPI.xml */
    attr = prvCpssMxmlElementGetAttr(node,search_string);
    if ( ( attr == NULL ) || ( strlen(attr) > (unsigned int)maxSize ) )
        goto xml_enum_error;
    else if ( enumGetKey == GET_ENUM_VALUE )
        *enumValue = cpssOsStrTo32(attr);
    else /* (enumName==GET_ENUM_STRING) */
        strncpy( enumName, attr, ENUM_ARRAY_MAZ_SIZE - 2 );
    return 0;

xml_enum_error:
    cpssOsPrintf("\nget_cpssAPI_xml_enum failed - Enum: %s\n"
                 "Searched for %s: %s\n", enumString, search_string, search_attr);
    return -1;
}

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
)
{
    int enumValue = 0; /* default enumartaion */

    if (lua_isnumber(L, -1))
        return (int)lua_tointeger(L, -1);
    if (!enumString)
        return 0;
    if (lua_isstring(L, -1))
    {
        int delta = 0;

        GT_CHAR *plu = cpssOsStrChr(lua_tostring(L, -1), '+');

        if (plu != NULL)
        {
            char *d = plu + 1;
            while (*d != 0 && *d >= '0' && *d <= '9')
            {
                delta = delta * 10 + ((*d)-'0');
                d++;
            }
            if (*d == 0)
            {
                *plu = 0;
            }
            else
            {
                delta = 0;
                plu = NULL;
            }
        }

        if (get_cpssAPI_xml_enum( enumString, (char *)lua_tostring(L, -1), &enumValue, ENUM_ARRAY_MAZ_SIZE, GET_ENUM_VALUE ) != 0 )
            return 0;

        if (plu != NULL)
            *plu = '+';
        return enumValue + delta;

        if (plu != NULL)
            *plu = '+';
    }
    return 0;
}

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
)
{
    char enum_name[ENUM_ARRAY_MAZ_SIZE]; /* for example: "CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_LAST_E" */

    enum_name[ENUM_ARRAY_MAZ_SIZE - 1] = '\0';
    if (enumString)
    {
        if (get_cpssAPI_xml_enum( enumString, enum_name, &enumValue, ENUM_ARRAY_MAZ_SIZE, GET_ENUM_STRING ) != 0 ){

            /* couldn't find given enum value */
            lua_pushinteger(L, enumValue);
            return;
        }

        lua_pushstring(L, enum_name);
        return;
    }
    lua_pushnil(L);
}

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
)
{
    if (get_cpssAPI_xml_enum(enumName, enumString, &enumValue, size, GET_ENUM_STRING) != 0)
        return NULL;

    return enumString;
}
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
)
{
    int enumValue;

    if (lua_type(L, 1) != LUA_TSTRING)
    {
        lua_pushboolean(L, 0);
        return 0;
    }

    if (get_cpssAPI_xml_enum( enumString, (char *)lua_tostring(L, 1), &enumValue, ENUM_ARRAY_MAZ_SIZE, GET_ENUM_VALUE ) != 0 ){
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;
}

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
    const char *enumString
)
{
    int         index = 0;
    mxml_node_t *node;
    const char  *str, *attr, *enum_name;
    size_t      len;

    if (lua_type(L, 1) != LUA_TSTRING)
        return 0;

    str = lua_tolstring(L, 1, &len);
    lua_newtable(L);

    node = cpssapi_enums;

    if (enumString)
    {
        do{ /* lookup for enumartion in XML */
            /* get enum node from cpssAPI.xml */
            node = prvCpssMxmlFindElement(node, cpssapi_enums, "Enum", NULL, NULL, MXML_DESCEND);
            if (!node)
                return 0;
            enum_name = prvCpssMxmlElementGetAttr(node,"name");
            if (cpssOsStrCmp(enum_name, enumString) != 0) { /* check if enum entered from CLI is valid */
                node = node->next;
                /* get next enum element node from cpssAPI.xml */
                continue;
            }
            break;

        }while(node != NULL);

        node = prvCpssMxmlFindElement(node, cpssapi_enums, "Value", NULL, NULL, MXML_DESCEND);
        while (node != NULL)
        {
            attr = prvCpssMxmlElementGetAttr(node,"str");
            if ( attr == NULL ) /* if not exist value of enum - set to 0 */
                return 0;

            /* print all string enumartions starts with input str */
            if (cpssOsStrNCmp(str, attr, (GT_U32)len) == 0){
                lua_pushstring(L, attr);
                lua_rawseti(L, -2, ++index);
            }
            node = prvCpssMxmlFindElement(node, cpssapi_enums, "Value", NULL, NULL, MXML_NO_DESCEND);
        }
    }

    return 1;
}

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
    const char *unionString
)
{
    int         index = 0;
    mxml_node_t *node;
    const char  *str, *attr, *union_name;
    size_t      len;

    if (lua_type(L, 1) != LUA_TSTRING)
        return 0;

    str = lua_tolstring(L, 1, &len);
    lua_newtable(L);

    node = cpssapi_structures;

    if (unionString)
    {
        do{ /* lookup for union structure in XML */
            /* get structure node from cpssAPI.xml */
            node = prvCpssMxmlFindElement(node, cpssapi_structures, "Structure", NULL, NULL, MXML_DESCEND);
            if (!node)
                return 0;
            union_name = prvCpssMxmlElementGetAttr(node,"name");
            if (cpssOsStrCmp(union_name, unionString) != 0) { /* check if enum entered from CLI is valid */
                node = node->next;
                /* get next enum element node from cpssAPI.xml */
                continue;
            }
            break;

        }while(node != NULL);

        node = prvCpssMxmlFindElement(node, cpssapi_structures, "Member", NULL, NULL, MXML_DESCEND);
        while (node != NULL)
        {
            attr = prvCpssMxmlElementGetAttr(node,"type");
            if ( attr == NULL ) /* if not exist value of enum - set to 0 */
                return 0;

            if (cpssOsStrNCmp(str, attr, (GT_U32)len) == 0){
                lua_pushstring(L, attr);
                lua_rawseti(L, -2, ++index);
            }
            node = prvCpssMxmlFindElement(node, cpssapi_structures, "Member", NULL, NULL, MXML_NO_DESCEND);
        }
    }

    return 1;
}

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
)
{
    GT_U32 k;
    GT_U8  data;

    for (k = 0; k < dataLen / 2; k++)
    {
        data = input_data[k];
        output_data[k] = input_data[dataLen - k - 1];
        output_data[dataLen - k - 1] = data;
    }

    return GT_OK;
}


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
)
{
    GT_U32 k;
    for (k = 0; k < dataLen; k++)
    {
        char mask = (char) ~(((const char*)appiedMask)[k]);
        ((char*)output_data)[k] = (char) (((const char*)input_data)[k] & mask);
    }
    return GT_OK;
}


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
)
{
    GT_U32 k;
    for (k = 0; k < dataLen; k++)
    {
        char mask = (char) ~(((const char*)ignoreMask)[k]);
        if (    ( ((const char*)data1)[k] & mask ) !=
                ( ((const char*)data2)[k] & mask ) )
        {
            return 0;
        }
    }
    return 1;
}


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
void prv_lua_to_c_GT_U64(
    lua_State *L,
    GT_U64  *val
)
{
    F_ARRAY_START(val, -1, l);
        F_ARRAY_NUMBER(val, l, 0, GT_U32);
        F_ARRAY_NUMBER(val, l, 1, GT_U32);
    F_ARRAY_END(val, -1, l);
}

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
void prv_c_to_lua_GT_U64(
    lua_State *L,
    GT_U64 *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, l);
        FO_ARRAY_NUMBER(val, l, 0, GT_U32);
        FO_ARRAY_NUMBER(val, l, 1, GT_U32);
    FO_ARRAY_END(val, t, l);
}

add_mgm_type(GT_U64);
#endif /* !defined(LUACLI_NOCPSS) */


/*************************************************************************
* mgmType_to_c_GT_PTR
*
* Description:
*       Convert to GT_PTR
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_c_GT_PTR(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushlightuserdata(L, NULL);
    else if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_GT_PTR
*
* Description:
*       Convert GT_PTR to Lua lightuserdata
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_lua_GT_PTR(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_c_uintptr
* mgmType_to_c_GT_UINTPTR
*
* Description:
*       Convert to GT_PTR
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_c_uintptr(
    lua_State *L
)
{
    if (lua_isnumber(L, 1))
        lua_pushlightuserdata(L, (void*)((GT_UINTPTR)lua_tonumber(L, 1)));
    else if (lua_islightuserdata(L, 1))
        lua_pushvalue(L, 1);
    else
        lua_pushlightuserdata(L, NULL);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_uintptr
* mgmType_to_lua_GT_UINTPTR
*
* Description:
*       Convert GT_PTR to Lua lightuserdata
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_lua_uintptr(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushlightuserdata(L, (lua_touserdata(L, 1)));
    return 1;
}



/**
* @internal luaEngineGenWrapper_init function
* @endinternal
*
* @brief   initializing of CPSS contexted lua tipes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - no returned rarameters added no lua stack
*/
int luaEngineGenWrapper_init(lua_State *L)
{
    lua_register(L, "cpssGenWrapper", cpssGenWrapper);
    lua_register(L, "cpssGenWrapperCheckParam", cpssGenWrapperCheckParam);
    lua_register(L, "mgmType_to_c_int", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U8", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U64_BIT", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U16", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_TRUNK_ID",             mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_32",                   mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U32", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_UOPT", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_PORT_NUM",             mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_PHYSICAL_PORT_NUM",    mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_CPSS_PX_PTP_TYPE_INDEX",    mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID",    mgmType_to_c_int);
    lua_register(L, "mgmType_to_lua_int", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U8", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U64_BIT", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U16", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_32",                 mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U32", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_UOPT", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_PORT_NUM",           mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_PHYSICAL_PORT_NUM",  mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_CPSS_PX_PTP_TYPE_INDEX", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID",  mgmType_to_lua_int);
    lua_register(L, "mgmType_to_c_bool", mgmType_to_c_bool);
    lua_register(L, "mgmType_to_c_GT_BOOL", mgmType_to_c_bool);
    lua_register(L, "mgmType_to_lua_bool", mgmType_to_lua_bool);
    lua_register(L, "mgmType_to_lua_GT_BOOL", mgmType_to_lua_bool);
#ifndef LUACLI_NOCPSS
    lua_register(L, "mgmType_to_c_GT_U64", mgmType_to_c_GT_U64);
    lua_register(L, "mgmType_to_lua_GT_U64", mgmType_to_lua_GT_U64);
#endif /* !defined(LUACLI_NOCPSS) */
    lua_register(L, "mgmType_to_c_GT_PTR", mgmType_to_c_GT_PTR);
    lua_register(L, "mgmType_to_lua_GT_PTR", mgmType_to_lua_GT_PTR);
    lua_register(L, "mgmType_to_c_uintptr", mgmType_to_c_uintptr);
    lua_register(L, "mgmType_to_c_GT_UINTPTR", mgmType_to_c_uintptr);
    lua_register(L, "mgmType_to_lua_uintptr", mgmType_to_lua_uintptr);
    lua_register(L, "mgmType_to_lua_GT_UINTPTR", mgmType_to_lua_uintptr);
    return 0;
}

