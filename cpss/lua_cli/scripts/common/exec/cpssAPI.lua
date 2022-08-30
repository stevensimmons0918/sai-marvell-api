--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpssAPI.lua
--*
--* DESCRIPTION:
--*
--* FILE REVISION NUMBER:
--*       $Revision: 26 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- cpssAPI_desc        - Cpss API name and input parameters
--
cmdLuaCLI_callCfunction("prvLuaCpssAPI_register")

do

-- default value of cpss-api call output mode is silent
local cpssVerboseMode = false

local cpssAPI_initialized = false
local register_cpssAPI_commands
local cpssAPI_xml = "cpssAPI.xml"

local function lookup_cpssAPI_xml()
    if fs.stat(cpssAPI_xml) ~= nil then
        return true
    end
    return false
end

local function cpssAPI_initialize()
    if cpssAPI_initialized then
        -- already registered
        print("Already loaded")
        return true
    end
    if not lookup_cpssAPI_xml() then
        print("Please upload cpssAPI.xml first")
        return false
    end
    if not cpssAPI_load(cpssAPI_xml) then
        print("Failed to load register definitions from "..cpssAPI_xml)
        return false
    end

    register_cpssAPI_commands()
    cpssAPI_initialized = true
end


local cpssAPI_name = nil
local cpssAPI_type = nil
local cpssAPI_tag = nil
local cpssAPI_params = {}
local cpssAPI_callP = {}
local cpssAPI_current_param = 0
local cpssAPI_current_param_s = 0
local cpssAPI_ignorecase = true
local function cpssAPI_rst()
    cpssAPI_name = nil
    cpssAPI_type = nil
    cpssAPI_tag = nil
    cpssAPI_params = {}
    cpssAPI_callP = {}
    cpssAPI_current_param = 0
    cpssAPI_current_param_s = 0
end

-- ************************************************************************
---
--  cpssAPI_next_param
--        @description  Skip to first/next parameter IN/INOUT
--                      copy OUT parameters to cpssAPI_callP
--
--        @return       true    - no more parameters (all parameters parsed)
--                      false   - parameter available
--
local function cpssAPI_next_param()
    while cpssAPI_current_param < #cpssAPI_params
    do
        cpssAPI_current_param = cpssAPI_current_param + 1
        local p = cpssAPI_params[cpssAPI_current_param]
        local arraySize = nil
        local arrayName = nil

        if string.sub(p.cls,-1) == "]" then -- in case of array
            arrayPos = string.find(p.cls, '%[') -- p.cls=int[5] arrayPos=4
            arraySize = string.sub(p.cls,arrayPos+1,-2) -- example: p.cls=int[5] arraySize=5
            if arraySize ~= "" then
                arrayName = arraySize
            elseif p.array ~= nil then
                arrayName = p.array
            else
                arrayName = ''
            end
            p.type = p.type.."[".. arrayName .."]"
        end

        if p.dir == "OUT" then
            table.insert(cpssAPI_callP, { p.dir, p.type, p.name })
            -- handle last parameter
            if cpssAPI_current_param == #cpssAPI_params then
                cpssAPI_current_param = cpssAPI_current_param + 1
                cpssAPI_current_param_s = 0
            end
        elseif (p.cls == "struct") or (p.dir == "IN" and string.sub(p.cls,-1) == "]") then -- in case of array or struct
            --TODO handle structures with special=="string"
            table.insert(cpssAPI_callP, { "s"..p.dir, p.type, p.name})
        elseif p.cls == "union" then
            table.insert(cpssAPI_callP, { "s"..p.dir, p.type, p.name})
            cpssAPI_current_param_s = 0
            return false
        else
            cpssAPI_current_param_s = 0
            return false
        end
    end
    return true
end

cpssConst = {
    help = "Cpss constant values",
    enum = {
        ["CPSS_PORT_GROUP_UNAWARE_MODE_CNS"] = { value=0xffffffff },
        ["CPSS_PARAM_NOT_USED_CNS"] = { value=0xffffffff },
        ["CPSS_MAX_PORT_GROUPS_CNS"] = { value=8 },
        ["CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS"] = { value=0 },
        ["CPSS_CPU_PORT_NUM_CNS"] = { value=63 },
        ["CPSS_MAX_PORTS_NUM_CNS"] = { value=256 },
        ["CPSS_NULL_PORT_NUM_CNS"] = { value=62 }
    }
}

local function xml_dequote(s)
    s = string.gsub(s, "&lt;", "<")
    s = string.gsub(s, "&gt;", ">")
    s = string.gsub(s, "&amp;", "&")
    return s
end

local function prefix_match_i(prefix,str)
    if cpssAPI_ignorecase ~= true then
        return prefix_match(prefix,str)
    end
    if string.lower(string.sub(str, 1, string.len(prefix))) == string.lower(prefix) then
        return true
    end
    return false
end
local function xprefix(prefix,str)
    return prefix .. string.sub(str, string.len(prefix)+1)
end

-- ************************************************************************
---
--  check_param_cpssAPI_params
--        @description  Check parameter that it is in the correct form,
--                      CPSS API name and parameters
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--        @param varray         - previous values array
--
--        @return       check_status, value, sequence_flag
--                      Here:
--                          check_status  - true if parameter valid
--                          value         - parsed value (any type)
--                          sequence_flag - for arrays (@type_name):
--                              nil     - nothing
--                              true    - no more words will follow
--                              false   - more words are required
--
local function check_param_cpssAPI_params(param, name, desc, varray)
    local p, s, v
    if varray == nil or (type(varray) == "table" and #varray == 0) then
        cpssAPI_rst()
    end
    if cpssAPI_name == nil then
        -- first item: CPSS API name
        cpssAPI_tag = cpssAPI_lookup(param, cpssAPI_ignorecase)
        if cpssAPI_tag == nil then
            return false, "No description found for function "..param
        end
        cpssAPI_name = xml_get_attr(cpssAPI_tag, "name")
        cpssAPI_type = xml_get_attr(cpssAPI_tag, "type")
        --TODO: get description ??
        -- parse params section
        local params = xml_lookup(cpssAPI_tag, "Params")
        if params == nil then
            -- No params section, assume cpssAPI_name(void)
            return true, param, true
        end
        local iterator = nil
        repeat
            iterator = xml_getchild(params, iterator)
            if iterator == nil then
                break
            end
            if xml_tagname(iterator) == "Param" then
                p = {}
                p.name = xml_get_attr(iterator, "name")
                p.dir  = xml_get_attr(iterator, "dir")
                p.type = xml_get_attr(iterator, "type")
                p.cls  = xml_get_attr(iterator, "class")
                p.array  = xml_get_attr(iterator, "array")
                p.special = xml_get_attr(iterator, "special")
                -- add description
                p.desc = xml_inlinetext(xml_lookup(iterator,"Description"))
                if type(p.desc) == "string" then
                    p.desc = xml_dequote(p.desc)
                end
                table.insert(cpssAPI_params, p)
            end
        until iterator == nil
        return true, param, cpssAPI_next_param()
    end
    -- parameter
    if cpssAPI_current_param > #cpssAPI_params then
        return false, "too many parameters for "..cpssAPI_name
    end
    p = cpssAPI_params[cpssAPI_current_param]
    if cpssAPI_current_param_s == 0 then
        -- check parameter name
        if param ~= p.name then
            return false, "Bad parameter name ("..param.."), "..p.name.." required"
        end
        cpssAPI_current_param_s = 1
        -- parameter value will follow
        return true, param, false
    end
    -- check value
    if p.cls == "bool" then
        local d = CLI_type_dict["bool"]
        s,v = d.checker(param, p.name, d)
        if not s then
            return s, v
        end
        table.insert(cpssAPI_callP, { p.dir, p.type, p.name, v })
    elseif p.cls == "enum" then
        v = cmdLuaCLI_callCfunction("mgmType_check_"..p.type, param)
        if not v then
            return false, param
        end
        table.insert(cpssAPI_callP, { p.dir, p.type, p.name, param })
    elseif p.cls == "union" then
        table.remove(cpssAPI_callP)
        table.insert(cpssAPI_callP, { "s"..p.dir, p.type, p.name, param })
        return true, param, cpssAPI_next_param()
    elseif p.cls == "int" then
        if prefix_match("CPSS", param) then
            s, v = CLI_check_param_enum(param, p.name, cpssConst)
            if not s then
                return s, v
            end
        else
            if (p.type == "GT_U64_BIT") then
                v = tostring(param)
            else
                v = tonumber(param)
            end
            if v == nil then
                return false, p.name .. " not a number"
            end
        end
        table.insert(cpssAPI_callP, { p.dir, p.type, p.name, v })
    else
        return false, "bad parameter class: "..p.cls.." must be one of bool,int,enum"
    end

    -- ok
    return true, param, cpssAPI_next_param()
end


-- ************************************************************************
---
--  complete_value_cpssAPI_name
--        @description  autocompletes CPSS API name
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--
--        @return        cpss API params autocomplete
--
local function complete_value_cpssAPI_name(param,name,desc)
    local compl = {}
    local help = {}
    local i
    compl, help = cpssAPI_list(param, cpssAPI_ignorecase)
    help.def = "CPSS API function name"
    -- special handling for too long list
    if #compl > 20 then
        local c = duplicate(compl)
        table.sort(c)
        local ncompl = {}
        local short, short_prev
        local o
        short_prev = nil
        for i = 1, #c do
            if i == 1 then
                o = c[i]
            else
                if string.len(c[i]) < string.len(o) then
                    o = string.sub(o, 1, string.len(c[i]))
                end
                while o ~= "" do
                    if not prefix_match_i(o, c[i]) then
                        o = string.sub(o, 1, string.len(o)-1)
                    else
                        break
                    end
                end
            end
            local apref = string.sub(c[i],1,string.len(param))
            short = string.match(c[i], apref .. "%u[%l%d]*")
            if short == nil then
                short = string.match(c[i], apref .. "%d+")
            end
            if short == nil then
                local t
                for t = 1, #compl do
                    compl[t] = xprefix(param, compl[t])
                end
                return compl, help
            end
            if short ~= c[i] then
                short = short .. "*"
            end
            if short ~= short_prev then
                table.insert(ncompl, short)
                short_prev = short
            end
        end

        if string.len(o) == string.len(param) then
            for i = 1, #ncompl do
                ncompl[i] = xprefix(param, ncompl[i])
            end
            return ncompl, { def = "CPSS API function name" }
        end
    end
    for i = 1, #compl do
        compl[i] = xprefix(param, compl[i])
    end
    return compl, help
end

-- ************************************************************************
---
--  complete_value_cpssAPI_params
--        @description  autocompletes CPSS API name and parameters
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--        @param varray             - previous values array
--
--        @return        cpss API params autocomplete
--
local function complete_value_cpssAPI_params(param,name,desc,varray)
    if varray == nil or (type(varray) == "table" and #varray == 0) then
        cpssAPI_rst()
    end
    local compl = {}
    local help = {}
    if cpssAPI_name == nil then
        return complete_value_cpssAPI_name(param,name,desc)
    end
    -- parameter
    if cpssAPI_current_param > #cpssAPI_params then
        return compl, help
    end
    local p = cpssAPI_params[cpssAPI_current_param]
    if cpssAPI_current_param_s == 0 then
        if not prefix_match(param, p.name) then
            return compl, help
        end
        compl[1] = p.name
        help[1] = p.desc
        return compl, help
    end
    -- complete value
    help.def = p.desc
    if p.cls == "bool" then
        local d = CLI_type_dict["bool"]
        return d.complete(param, p.name, d)
    elseif p.cls == "enum" or p.cls == "union" then
        compl = cmdLuaCLI_callCfunction("mgmType_complete_"..p.type, param)
        return compl, help
    elseif p.cls == "int" then
        if prefix_match("CPSS", param) then
            return CLI_complete_param_enum(param, p.name, cpssConst)
        end
        return compl, help
    end
    return compl, help
end

CLI_type_dict["cpssAPI_params"] = {
    checker = check_param_cpssAPI_params,
    complete = complete_value_cpssAPI_params,
    help = "Cpss function name and parameters"
}

-- ************************************************************************
---
--  check_param_cpssAPI_name
--        @description  Check parameter that it is in the correct form,
--                      CPSS API name
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       check_status, value, sequence_flag
--                      Here:
--                          check_status  - true if parameter valid
--                          value         - parsed value (any type)
--
local function check_param_cpssAPI_name(param, name, desc)
    cpssAPI_tag = cpssAPI_lookup(param, cpssAPI_ignorecase)
    if cpssAPI_tag == nil then
        return false, "No description found for function "..param
    end
    cpssAPI_name = xml_get_attr(cpssAPI_tag, "name")
    return true, cpssAPI_name
end

CLI_type_dict["cpssAPI_name"] = {
    checker = check_param_cpssAPI_name,
    complete = complete_value_cpssAPI_name,
    help = "Cpss function name"
}



-- ************************************************************************
---
--  get_member
--        @description  get table member by path
--                      where path is "aaa" or "aaa.bbb.ccc"
--        @param        tbl
--        @param        path
--
--        @return       table member or nil
--
local function get_member(tbl, path)
    local p = string.find(path, ".",1,true)
    if p == nil then
        return tbl[path]
    end
    local t = tbl[string.sub(path,1,p-1)]
    if type(t) ~= "table" then
        return nil
    end
    return get_member(t, string.sub(path,p+1))
end

-- ************************************************************************
---
--  set_member
--        @description  set table member by path
--                      where path is "aaa" or "aaa.bbb.ccc"
--        @param        tbl
--        @param        path
--
--        @return       table member or nil
--
local function set_member(tbl, path,val)
    local p = string.find(path, ".",1,true)
    if p == nil then
        tbl[path] = val
        return
    end
    local t = tbl[string.sub(path,1,p-1)]
    if type(t) ~= "table" then
        tbl[string.sub(path,1,p-1)]={}
        t=tbl[string.sub(path,1,p-1)]
    end
    return set_member(t, string.sub(path,p+1),val)
end


local m_compl_name
local m_compl_type
local m_compl_class
-- ************************************************************************
---
--  complete_value_cpssAPI_struct_member
--        @description  autocompletes CPSS API name and parameters
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--        @param varray             - previous values array
--
--        @return        cpss API params autocomplete
--
local function complete_value_cpssAPI_struct_member(param, name, desc, varray)
    local compl = {}
    local help = {}
    help.def = "("..m_compl_type..")"
    if m_compl_class == "bool" then
        local d = CLI_type_dict["bool"]
        compl, help = d.complete(param, m_compl_name, d)
    elseif m_compl_class == "enum" or m_compl_class == "union" then
        compl = cmdLuaCLI_callCfunction("mgmType_complete_"..m_compl_type, param)
        for i = 1, #compl do
            help[i] = "("..m_compl_type..")"
        end
    elseif m_compl_class == "int" then
        if prefix_match("CPSS", param) then
            compl, help = CLI_complete_param_enum(param, m_compl_name, cpssConst)
        end
    end
    return compl, help
end
CLI_type_dict["cpssAPI_struct_member"] = {
    complete = complete_value_cpssAPI_struct_member,
    help = "Cpss structure member"
}

-- ************************************************************************
---
--  searchUnionAttrParams
--        @description  This function search for exact name and type of union structure from cpssAPI.xml
--
--        @param unionName              - union name ("CPSS_MAC_ENTRY_EXT_KEY_UNT")
--        @param structName             - structure name ("CPSS_MAC_ENTRY_EXT_KEY_IPV4_UNICAST_STC")
--        @param param                  - parameter in the xml of the structName, if not specified - return name parameter
--
--        @return                       paramName
--
-- examples:
-- function call:   searchUnionAttrParams("CPSS_MAC_ENTRY_EXT_KEY_UNT","CPSS_MAC_ENTRY_EXT_KEY_IPV4_UNICAST_STC")
-- return:          ipv4Unicast
-- function call:   searchUnionAttrParams("CPSS_MAC_ENTRY_EXT_KEY_UNT","CPSS_MAC_ENTRY_EXT_KEY_IPV4_UNICAST_STC","class")
-- return:          struct
local function searchUnionAttrParams(unionName, structName, param)
    local struct = cpssAPI_getstruct(unionName)
    local tagname = nil
    local paramMember = nil
    local paramType = nil
    local paramArray = nil

    if struct == nil then
        print("pname: no description for type "..unionName)
        return val
    end

    repeat
        paramMember = xml_getchild(struct, paramMember)
        if paramMember == nil then
            break
        end
        tagname = xml_tagname(paramMember)

        if tagname == "Member" then
            paramType = xml_get_attr(paramMember,"type")
            if paramType == structName then
                paramName = xml_get_attr(paramMember,"name")
                paramArray = xml_get_attr(paramMember,"array")
                if paramArray ~= nil then
                     paramName =paramName.."[]"
                end
                break
            end
        end
    until paramMember == nil

    if param == nil then
        return paramName
    else
        return xml_get_attr(paramMember,param)
    end

end

-- ************************************************************************
---
--  readStructMember
--        @description  read structure member
--
--        @param pname              - parameter name
--        @param mname              - member name
--        @param mtype              - member type
--        @param mclass             - member calss (int|bool|enum|struct|string)
--        @param val                - previous value
--
--        @return        new value
--
readStructMember = function (pname,mname,mtype,mclass,val)
--print("pname = " .. pname.. ", mname = "..mname..", mtype = "..mtype..", mclass = "..mclass..", val = "..to_string(val))
    local prompt = pname .. "." .. mname
    if mclass == "struct" then
        return readStructParam(mtype,prompt,val)
    elseif mclass == "string" then
        s = cmdLuaCLI_readLine(prompt.."("..tostring(val)..")>",false)
        if s ~= "" then
            return s
        end
        return val
    else -- mclass == "int", "bool", "enum", "union"
        m_compl_name = mname
        m_compl_type = mtype
        m_compl_class = mclass
        if mclass == "union" then
            val = mtype
        end
        s = cmdLuaCLI_readLine_compl(
                    { format="%cpssAPI_struct_member", name="structMember", help=""},
                    prompt.."()>",
                    false)
        if s == "*" then
            return nil
        end
        if s ~= "" then
            if mclass == "bool" then
                local d = CLI_type_dict["bool"]
                local st,v = d.checker(s, mname, d)
                if st then
                    return v
                else
                    print("Warning: bad value, must be true/false, fallback to false")
                end
            elseif mclass == "int" then
                if type(tonumber(s)) == "number" then
                    return tonumber(s)
                end
                if prefix_match("CPSS", s) then
                    local st, v
                    st, v = CLI_check_param_enum(s, mname, cpssConst)
                    if st then
                        return v
                    else
                        print("Warning: bad value, must be from enum "..mname..", fallback to 0")
                    end
                end
            elseif mclass == "union" and string.sub(val,-3) == "UNT" then -- in case of 'union' inside struct
                union_mname = searchUnionAttrParams(val, s)
                if union_mname ~= nil then
                    prompt = prompt .. "." .. union_mname
                    return {[union_mname] = readStructParam(s,prompt,nil)}
                end
            end
            return s
        end
        return val
    end
end

-- ************************************************************************
---
--  readStructParam
--        @description  read parameter which is a structure
--
--        @param ptype              - parameter type(structure name)
--        @param pname              - parameter name
--        @param val                - previous value
--
--        @return        new value
--
-- example: readStructParam("CPSS_MAC_ENTRY_EXT_KEY_STC",".key")
readStructParam = function(ptype,pname,val)

    local prompt, special, s
    local union_param_array = false

    if string.sub(pname,-1) == "]" then
        union_param_array = true
        pname=pname:sub(1,-3) -- remove "[]" from the union_mname, which we added to identify param is an array
    end

    if val == nil or val == 0 then
        val={}
    end
    local struct = cpssAPI_getstruct(ptype)
    -- scalar case in union
    if struct == nil then
        -- take care as a number
        if val == "int" then
            local s = cmdLuaCLI_readLine(pname.."("..val..")>",false)
            if (ptype == "GT_U64_BIT") then
                return tostring(s)
            else
                return tonumber(s)
            end
        end
        -- take care as a boolean
        if val == "bool" then
            local s = cmdLuaCLI_readLine(pname.."("..tostring(val)..")>",false)
            if tostring(s)=="true" then
                return true
            end
            return false
        end
    end
    -- special structures
    local special = xml_get_attr(struct, "special")
    if special == "unused" then
        return val
    end
    if special == "string" then
        local prompt
        if type(val) == "table" then
            prompt = pname.."()>"
        else
            prompt = pname.."("..val..")>"
        end
        local s = cmdLuaCLI_readLine(prompt,false)
        if s == "" then
            return val
        end
        return s
    end
    local member = nil
--print("p0: val="..to_string(val))
--print("p0: special="..to_string(special))
    repeat
        local tagname
        repeat
            member = xml_getchild(struct, member)
            if member == nil then
                tagname = nil
                break
            end
            tagname = xml_tagname(member)
        until tagname == "Member"
        if member == nil then
            break
        end
        local mname = xml_get_attr(member,"name")
        local mtype = xml_get_attr(member,"type")
        local mclass = xml_get_attr(member,"class")
        local marr= tonumber(xml_get_attr(member,"array"))
        v = {}
        if type(marr) == "number" then
            -- array
            local i
            for i = 0, marr-1 do
                v[i] = readStructMember(pname,string.format("%s[%d]",mname,i),mtype,mclass,0)
                if v[i] == nil then -- "*", skip array
                    break
                end
            end
        else
            v = readStructMember(pname, mname,mtype,mclass,v)
            if v == nil then -- "*", skip this
                break
            end
        end

        set_member(val, mname, v)
--print("p1: val="..to_string(val))
    until member == nil
    return val
end

-- ************************************************************************
---
--  call_cpss_API
--        @description  call CPSS function using generic wrapper
--
--        @param params     - parameters, unused because all required data
--                            collected in cpssAPI_name, cpssAPI_callP
--
--        @return true
--
local function call_cpss_API(params)
    -- read structure parameters
    local has_struct_param = false
    local i, p, info
    for i=1,#cpssAPI_callP do
        p = cpssAPI_callP[i]
        info = cpssAPI_params[i]
        local union_mname = nil
        local union_mtype = nil
        local paramName = nil
        local structName = p[2]
        local memberName = p[3]
        local arraySize = nil

        if info == nil then
            print("too many arguments for cpss-api call: "..i)
            return false
        end
        if string.sub(structName,-1) == "]" then -- in case of array parameter (for all direction cases: IN/OUT/INOUT)
            arrayPos = string.find(structName, '%[') -- structName=struct[]
            arrayName = string.sub(structName,arrayPos+1,-2) -- example: structName=GT_U32[arrLen] arrayName=arrLen

            if arrayName == "" then -- in case array size unknown - get from CLI
                local prompt = memberName.."[arraySize]>"
                s = cmdLuaCLI_readLine(prompt,false)
                arraySize = tonumber(s)
                if s == "" or type(arraySize) ~= "number" then
                    print("error - arraySize must be a valid number: [" .. s .. "]")
                    return false
                end
                info.cls = string.sub(info.cls,1,arrayPos-1) -- info.cls=struct
                arrayName = string.sub(structName,1,arrayPos-1) -- example: structName=struct[] arrayName=struct
                p[2] = arrayName .. "[" .. to_string(arraySize) .. "]"
            else -- in case array size known from xml
                for j = 1, #cpssAPI_callP do
                    if cpssAPI_callP[j][3] == arrayName then
                        arraySize = cpssAPI_callP[j][4]
                    end
                end
                if arraySize ~= nil then
                    p[2] = string.gsub(structName, arrayName , to_string(arraySize))
                end
            end
        end
        if p[1] == "sIN" or p[1] == "sINOUT" then
            p[1] = string.sub(p[1],2)
            if string.sub(structName,-3) == "UNT" then -- in case of 'union' input to cpss call
                union_mname = searchUnionAttrParams(structName, p[4])
                union_mtype = p[4]
            end
            if union_mname == nil then
                if string.sub(structName,-1) == "]" then -- in case of input array parameter
                    if type(arraySize) == "number" then -- arraySize not a number
                        local a
                        local v = {}
                        typePos = (string.find(p[2], '%['))
                        mtype = string.sub(p[2],1,typePos-1)

                        arrayPos = (string.find(info.cls, '%[')) -- info.cls=struct[]
                        info.cls = string.sub(info.cls,1,arrayPos-1) -- info.cls=struct
                        if info.cls == struct then
                            local val={}
                            val[mname]={}
                            local v = val[mname]
                        end
                        for a = 0, arraySize-1 do
                            v[a] = readStructMember(info.name,string.format("%s[%d]",p[2],a),mtype,info.cls,0)
                            if v[a] == nil then -- "*", skip array
                                break
                            end
                            if info.cls == struct then
                                set_member(val, info.name, v[a])
                            end
                        end
                        if info.cls ~= struct then
                            p[4] = v
                        end
                    end
                else
                    p[4] = readStructParam(structName, memberName)
                end
            elseif union_mname ~= nil then
                local class = searchUnionAttrParams(structName, union_mtype,"class")
                --case array in a union
                if string.sub(union_mname,-1) == "]" then
                    union_mname=union_mname:sub(1,-3) -- remove "[]" from the union_mname, which we added to identify param is an array
                    local v = {}
                    local num_params = searchUnionAttrParams(structName, union_mtype,"array")

                    for i = 0, num_params-1 do
                        v[i] = readStructMember(memberName,string.format("%s[%d]",union_mname,i),union_mtype,class,0)
                    end
                    p[4] = {}
                    p[4][union_mname]=v
                else
                    local v
                    if (class == "int") or (class == "bool") then
                         v = readStructParam(union_mtype, memberName.."."..union_mname,class)
                    else
                         v = readStructParam(union_mtype, memberName.."."..union_mname)
                    end
                    p[4] = {}
                    p[4][union_mname] = v
                 end
            end
            has_struct_param = true
        end
    end
    if cpssVerboseMode then
      print("Call "..cpssAPI_name.."() with the following parameters:")
      print(to_string(cpssAPI_callP))
--[[
    else
      io.write("Call "..cpssAPI_name.."()  ")
      local i
      for i = 1,#cpssAPI_callP do
        if (cpssAPI_callP[i][3] ~= nil) and (cpssAPI_callP[i][4] ~= nil) then
          io.write(tostring(cpssAPI_callP[i][3]), "=", tostring(cpssAPI_callP[i][4]), ",  ")
        end
      end
      io.write("\n")
]]--
    end
    local result, values = myGenWrapper(cpssAPI_name, cpssAPI_callP)
    if cpssAPI_type == "GT_STATUS" then
        print("result="..returnCodes[result])
    end
    print("values="..to_string(values))
    if has_struct_param then
        -- add to CLI_running_Cfg_Tbl manually
        CLI_dont_add_to_running_config = true
        table.insert(CLI_running_Cfg_Tbl, "callCpssAPI "..cpssAPI_name.."\n<<<PARAMS\n"..to_string(cpssAPI_callP).."\n>>>")
    end
    return true
end



-- functions that fail to be in the XML
local manualFunctionForSerach = {
    "cpssDxChTrunkTableEntryGet"
}


local function do_search_cpss_API(params)
    local l = cpssAPI_list("")
    local pat = string.lower(params.string)
    local found = false
    local i
    for i = 1, #l do
        if string.find(string.lower(l[i]), pat) ~= nil then
            print(l[i])
            found = true
        end
    end

    l = manualFunctionForSerach
    for i = 1, #l do
        if string.find(string.lower(l[i]), pat) ~= nil then
            print(l[i])
            found = true
        end
    end


    if not found then
        print("No matching CPSS APIs found")
    end
end

local function man_cpss_API(params)
    local str
    str = xml_inlinetext(xml_lookup(cpssAPI_tag, "Comment"))
    if type(str) == "string" then
        print(str)
        return true
    end
    str = xml_inlinetext(xml_lookup(cpssAPI_tag, "Description"))
    if type(str) == "string" then
        print("Description: "..str)
        return true
    end
    print("No manual/description found for "..cpssAPI_name)
    return false
end

-- ************************************************************************
---
--  setCpssOutputMode
--        @description  set output mode for tests log
--
--        @param - mode name (silent or verbose)
--
local function setCpssOutputMode(param)
    if param["mode"] == nil then
      print("Error: undefined mode")
      return false
    end

    if param["mode"] == "laconic" then
      cpssVerboseMode = false
    elseif param["mode"] == "verbose" then
      cpssVerboseMode = true
    else
      print("Error: wrong mode")
      return false
    end

    return true
end


local function search_cpss_API(params)
    return do_command_with_print_nice(
            "do_search_cpss_API()",
            do_search_cpss_API,
            params)
end

function register_cpssAPI_commands()
    -- call cpss api @cpssAPI_params
    CLI_addCommand("debug", "cpss-api call", {
        help="Call CPSS function",
        func=call_cpss_API,
        params={
            { type="values",
              { format="@cpssAPI_params", allowduplicate=true, help="" }
            }
            ,{ type="named" }
        }
    })
    CLI_addCommand("debug", "cpss-api search", {
        help="Call CPSS function",
        func=search_cpss_API,
        params={
            { type="values",
              { format="%string", name="string", help="CPSS API name substring to match" }
            }
        }
    })
    -- The same for exec mode

    -- call cpss api @cpssAPI_params
    CLI_addCommand("exec", "cpss-api call", {
        help="Call CPSS function",
        func=call_cpss_API,
        params={
            { type="values",
              { format="@cpssAPI_params", allowduplicate=true, help="" }
            }
            ,{ type="named" }
        }
    })
    -- print comment block for cpss api
    CLI_addCommand("exec", "cpss-api man", {
        help="Show CPSS API function info",
        func=man_cpss_API,
        params={
            { type="values",
              { format="%cpssAPI_name", help="" }
            }
        }
    })
    CLI_addCommand("exec", "cpss-api search", {
        help="Call CPSS function",
        func=search_cpss_API,
        params={
            { type="values",
              { format="%string", name="string", help="CPSS API name substring to match" }
            }
        }
    })

-- set mode for cpss-api call output
--
CLI_addCommand({"exec", "debug"}, "cpss-api mode", {
    func = setCpssOutputMode,
    help = "Set mode for cpss-api call output",
    params={
        { type="named",
            {format="laconic",  help = "set laconic mode for output"},
            {format="verbose",  help = "set verbose mode for output"},
            alt = { mode = {"laconic", "verbose"}},
            mandatory={"mode"}
        }
    }
})


if dxCh_family == true then
    require_safe_dx("exec/cpssAPI")
end

if px_family == true then
    require_safe_px("exec/cpssAPI")
end


end


-- load at startup if cpssAPI.xml present
if lookup_cpssAPI_xml() then
    cpssAPI_initialize()
end

if not cpssAPI_initialized then
    CLI_addCommand("debug", "cpssAPI_load", {
        help = "Load cpssAPI.xml, initialize 'call cpss api ...'",
        func = cpssAPI_initialize
    })
end



end
