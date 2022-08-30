/*******************************************************************************
*              (c), Copyright 2011, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* luaGenericCpssAPI_mxml.c
*
* DESCRIPTION:
*       Lua <=> C interface for CPSS API function descriptions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
**************************************************************************/
#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngine.h>
#include <cmdShell/FS/cmdFS.h>
#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>

static mxml_node_t *cpssapi_root = NULL;
static mxml_node_t *cpssapi_functions = NULL;
extern mxml_node_t *cpssapi_structures;
extern mxml_node_t *cpssapi_enums;

static mxml_node_t *cpssapi_ext_root = NULL;
static mxml_node_t *cpssapi_ext_functions = NULL;
static mxml_node_t *cpssapi_ext_structures = NULL;
static mxml_node_t *cpssapi_ext_enums = NULL;


mxml_node_t* prvLuaMxmlLookupNode(mxml_node_t *node, const char *path);
int prvLuaXml_lookup_tag_with_attr_common(
    lua_State *L,
    mxml_node_t *parent,
    const char  *tagname,
    const char  *attrname,
    const char  *value,
    int         ignorecase
);
int prvLuaXml_compl_by_tag_with_attr_common(
    lua_State *L,
    mxml_node_t *parent,
    const char  *tagname,
    const char  *attrname,
    const char  *prefix,
    size_t      prefix_len,
    int         ignorecase
);

/*******************************************************************************
* prvLuaXml_merge_sub_tree
*
* DESCRIPTION:
*       Merge XML subtree of same data to parent tree
*
* INPUTS:
*       parent              - parent node
*       parent_of_subtree   - parent node of subtree that will be merged to parent
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
static void prvLuaXml_merge_sub_tree(
    mxml_node_t *parent,
    mxml_node_t *parent_of_subtree
)
{
    mxml_node_t* node;

    /* find last node of main tree */
    for (node = parent->child; node->next; node = node->next);

    /* add subtree to last node of main tree */
    node->next = parent_of_subtree->child;

    /* disconnect moved tree from parent of subtree */
    parent_of_subtree->child = NULL;

    return;
}


/*******************************************************************************
* prvLuaCpssAPI_load_mxml
*
* DESCRIPTION:
*       Load cpss API xml
*
* INPUTS:
*       L                   - lua state
*           filename        - filename
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_load_mxml(lua_State *L)
{
    GEN_FS_STAT_STC st;
    CPSS_OS_FILE_TYPE_STC  *fp;
    genFS_PTR fs;

    fp = cpssOsMalloc(sizeof(CPSS_OS_FILE_TYPE_STC));
    fp->type = CPSS_OS_FILE_RAMFS;
    fp->param = (GT_VOID *) &cmdFS;
    fs = (genFS_PTR)luaCLIEngineGetData(L, "_genFS", 0);

    if (cpssapi_root != NULL)
    {
        lua_pushboolean(L, 1);
        return 1;
    }

    if (!lua_isstring(L, 1) || fs == NULL)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (fs->stat(lua_tostring(L, 1), &st))
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (cpssapi_root != NULL)
    {
        prvCpssMxmlDelete(cpssapi_root);
        cpssapi_root = NULL;
        cpssapi_functions = NULL;
        cpssapi_structures = NULL;
        cpssapi_enums = NULL;

        if (cpssapi_ext_root != NULL)
        {
            prvCpssMxmlDelete(cpssapi_ext_root);
            cpssapi_ext_functions = NULL;
            cpssapi_ext_structures = NULL;
            cpssapi_ext_enums = NULL;
            cpssapi_ext_root = NULL;
        }
    }

    fp->fd = fs->open(lua_tostring(L, 1), GEN_FS_O_RDONLY);
    if ((int)fp->fd < 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    cpssapi_root = prvCpssMxmlLoadFd(NULL, fp, fs, MXML_NO_CALLBACK);
    fs->close(fp->fd);

    fp->fd = fs->open("cpssAPI_C005.xml", GEN_FS_O_RDONLY);
    if ((int)fp->fd >= 0)
    {
        cpssOsPrintf(" Load C005 XML \n");
        cpssapi_ext_root = prvCpssMxmlLoadFd(NULL, fp, fs, MXML_NO_CALLBACK);
        fs->close(fp->fd);
    }

    cpssOsFree(fp);

    if (!cpssapi_root)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    cpssapi_functions = prvLuaMxmlLookupNode(cpssapi_root, "CpssFunctionPrototypes.Functions");
    cpssapi_structures = prvLuaMxmlLookupNode(cpssapi_root, "CpssFunctionPrototypes.Structures");
    cpssapi_enums = prvLuaMxmlLookupNode(cpssapi_root, "CpssFunctionPrototypes.Enums");
    if (cpssapi_functions == NULL || cpssapi_structures == NULL || cpssapi_enums == NULL)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (cpssapi_ext_root)
    {
        /* get subtree of extended functions and merge it to main tree of functions */
        cpssapi_ext_functions = prvLuaMxmlLookupNode(cpssapi_ext_root, "CpssFunctionPrototypes.Functions");

        /* get subtree of extended structures and merge it to main tree of structures */
        cpssapi_ext_structures = prvLuaMxmlLookupNode(cpssapi_ext_root, "CpssFunctionPrototypes.Structures");

        /* get subtree of extended enums and merge it to main tree of enums */
        cpssapi_ext_enums = prvLuaMxmlLookupNode(cpssapi_ext_root, "CpssFunctionPrototypes.Enums");
        if (cpssapi_ext_functions == NULL || cpssapi_ext_structures == NULL || cpssapi_ext_enums == NULL)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        prvLuaXml_merge_sub_tree(cpssapi_functions, cpssapi_ext_functions);
        prvLuaXml_merge_sub_tree(cpssapi_structures, cpssapi_ext_structures);
        prvLuaXml_merge_sub_tree(cpssapi_enums, cpssapi_ext_enums);
    }

    lua_pushboolean(L, 1);
    return 1;
}

/*******************************************************************************
* luaCpssAPI_reset_mxml
*
* DESCRIPTION:
*       Reset mxml state (used with cliMemLib reset)
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
void luaCpssAPI_reset_mxml(void)
{
    cpssapi_root = NULL;
    cpssapi_functions = NULL;
    cpssapi_structures = NULL;
    cpssapi_enums = NULL;
}

/*******************************************************************************
* prvLuaCpssAPI_GetList
*
* DESCRIPTION:
*       return cpss API root node
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_GetList(lua_State *L)
{
    if (cpssapi_functions)
        lua_pushlightuserdata(L, cpssapi_functions);
    else
        lua_pushnil(L);
    return 1;
}

/*******************************************************************************
* prvLuaCpssAPI_lookup
*
* DESCRIPTION:
*       return cpss API function node node
*
* INPUTS:
*       L                   - lua state
*           funcname        - function name
*           ignorecase      - ignore case in function name (optional)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_lookup(lua_State *L)
{
    int ignorecase = 0;
    if (!cpssapi_functions)
    {
        lua_pushnil(L);
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushnil(L);
        return 1;
    }
    if (lua_isboolean(L, 2))
    {
        ignorecase = lua_toboolean(L, 2);
    }
    return prvLuaXml_lookup_tag_with_attr_common(L,
            cpssapi_functions,
            "Function", "name", lua_tostring(L, 1), ignorecase);
}

/*******************************************************************************
* prvLuaCpssAPI_list
*
* DESCRIPTION:
*       List all functions which match prefix
*
* INPUTS:
*       L                   - lua state
*           prefix          - prefix or empty string
*           ignorecase      - ignore case in function name (optional)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       compl, help
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_list(lua_State *L)
{
    const char *prefix;
    size_t prefix_len;
    int ignorecase = 0;
    if (lua_isboolean(L, 2))
    {
        ignorecase = lua_toboolean(L, 2);
    }
    if (!cpssapi_functions)
    {
        return 2;
    }
    if (!lua_isstring(L, 1))
    {
        return 2;
    }
    prefix = lua_tolstring(L, 1, &prefix_len);
    return prvLuaXml_compl_by_tag_with_attr_common(L,
            cpssapi_functions,
            "Function", "name",
            prefix, prefix_len,
            ignorecase);
}

/*******************************************************************************
* prvLuaCpssAPI_enums
*
* DESCRIPTION:
*       return cpss API enums root node
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_enums(lua_State *L)
{
    if (cpssapi_enums)
        lua_pushlightuserdata(L, cpssapi_enums);
    else
        lua_pushnil(L);
    return 1;
}


/*******************************************************************************
* prvLuaCpssAPI_structures
*
* DESCRIPTION:
*       return cpss API structures root node
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_structures(lua_State *L)
{
    if (cpssapi_structures)
        lua_pushlightuserdata(L, cpssapi_structures);
    else
        lua_pushnil(L);
    return 1;
}

/*******************************************************************************
* prvLuaCpssAPI_lookup_enum
*
* DESCRIPTION:
*       return cpss API enum node
*
* INPUTS:
*       L                   - lua state
*           structname        - function name
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_lookup_enum(lua_State *L)
{
    if (!cpssapi_enums)
    {
        lua_pushnil(L);
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushnil(L);
        return 1;
    }
    return prvLuaXml_lookup_tag_with_attr_common(L,
            cpssapi_enums,
            "Enum", "name", lua_tostring(L, 1), 0);
}


/*******************************************************************************
* prvLuaCpssAPI_lookup_struct
*
* DESCRIPTION:
*       return cpss API structure node
*
* INPUTS:
*       L                   - lua state
*           structname        - function name
*
* OUTPUTS:
*       None
*
* RETURNS:
*       xml node
*
* COMMENTS:
*
*******************************************************************************/
static int prvLuaCpssAPI_lookup_struct(lua_State *L)
{
    if (!cpssapi_structures)
    {
        lua_pushnil(L);
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushnil(L);
        return 1;
    }
    return prvLuaXml_lookup_tag_with_attr_common(L,
            cpssapi_structures,
            "Structure", "name", lua_tostring(L, 1), 0);
}


int prvLuaMxmlParser_register(lua_State *L);
/*******************************************************************************
* prvLuaCpssAPI_register
*
* DESCRIPTION:
*       register cider functions
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
int prvLuaCpssAPI_register(lua_State *L)
{
    prvLuaMxmlParser_register(L);
    lua_register(L, "cpssAPI_load", prvLuaCpssAPI_load_mxml);
    lua_register(L, "cpssAPI_getlist", prvLuaCpssAPI_GetList);
    lua_register(L, "cpssAPI_lookup", prvLuaCpssAPI_lookup);
    lua_register(L, "cpssAPI_list", prvLuaCpssAPI_list);
    lua_register(L, "cpssAPI_structures", prvLuaCpssAPI_structures);
    lua_register(L, "cpssAPI_enums", prvLuaCpssAPI_enums);
    lua_register(L, "cpssAPI_getstruct", prvLuaCpssAPI_lookup_struct);
    lua_register(L, "cpssAPI_getenum", prvLuaCpssAPI_lookup_enum);
    return 0;
}

/*
 * Short reference
 * ===============
 *
 * cpssAPI_load(filename)
 *      Load CPSS API descriptions xml
 *
 *      Params:
 *          filename        - filename
 *
 *      Returns:
 *          bool status
 *
 *
 * cpssAPI_getlist() -- unit,subunit, ...
 *      Return CPSS API root node (XML node)
 *
 *
 * cpssAPI_lookup(funcname)
 *      Search for function description
 *
 *      Params:
 *          funcname        - function name
 *          ignorecase      - ignore case in function name (optional)
 *
 *      Returns:
 *          XML node
 *
 *
 * cpssAPI_list(prefix)
 *      List all functions which match prefix
 *
 *      Params:
 *          prefix          - prefix or empty string
 *          ignorecase      - ignore case in function name (optional)
 *
 *      Returns:
 *          compl, help
 *
 *
 * cpssAPI_structures() --
 *      Return CPSS API structures root node (XML node)
 *
 *
 * cpssAPI_getstruct(structname)
 *      Search for structure description
 *
 *      Params:
 *          structname       - structure name
 *
 *      Returns:
 *          XML node
 *
 */
