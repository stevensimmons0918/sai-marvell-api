--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv6_mc_route.lua
--*
--* DESCRIPTION:
--*       creating/destroing of IPV6 MC Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--constants


local function ltt_entry_for_mc_next_hop(next_hop_index)
    return {
        routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
        numOfPaths = 1,
        routeEntryBaseIndex = next_hop_index,
        ucRPFCheckEnable = false,
        sipSaCheckMismatchEnable = false,
        ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",  -- IPV6 only
        priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E"
    };
end

-- ************************************************************************
---
--  ipv6_mc_route_func
--        @description  creates IPv6 Mc Routes
--
--        @param params - params["group-ipv6-prefix"]: prefix of group ipv6 address;
--                        params["group-ipv6-prefix"]["length"]: - prefix length
--                        params["nexthop"]: next-hop entry index
--                        params["source"]: prefix of source ipv6 address;
--                        params["source"]["length"]: - prefix length
--                        params["vrf-id"]: Virtual Router ID
--
--        @return       true on success, otherwise false and error message
--
local function ipv6_mc_route_func(params)
    -- print(to_string(params));
    -- Common variables declaration
    local result, values;
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local group_ipv6_address, group_mask_length, next_hop_enrty_index, vrf_id;
    local src_ipv6_address, src_mask_length;
    local apiName, is_failed, result, OUT_values;

    -- Command specific variables initialization.
    group_ipv6_address        = params["group-ipv6-prefix"];
    group_mask_length         = group_ipv6_address["length"]
    next_hop_enrty_index      = params["nexthop"]
    src_ipv6_address          = params["source"]
        or {string="0:0:0:0:0:0:0:0/0", length=0, "0","0","0","0", "0","0","0","0"};
    src_mask_length         = src_ipv6_address["length"];
    vrf_id                  = params["vrf-id"] or 0 -- set 0 if nil

    -- Ipv6 prefix adding.
    apiName = "cpssDxChIpLpmIpv6McEntryAdd";
    is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            { "IN",     "GT_IPV6ADDR", "ipGroup",              group_ipv6_address},
            { "IN",     "GT_U32",      "ipGroupPrefixLen",     group_mask_length  },
            { "IN",     "GT_IPV6ADDR", "ipSrc",                src_ipv6_address},
            { "IN",     "GT_U32",      "ipSrcPrefixLen",       src_mask_length},
            { "IN",     "CPSS_DXCH_IP_LTT_ENTRY_STC",     "mcRouteLttEntryPtr",
                        ltt_entry_for_mc_next_hop(next_hop_enrty_index)},
            { "IN",     "GT_BOOL",     "override",    true },
            { "IN",     "GT_BOOL",     "defragmentationEnable",    true }
        });

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_ipv6_mc_route_func
--        @description  destroes IPv6 Mc Routes
--
--                        params["group-ipv6-prefix"]["length"]: - prefix length
--                        params["nexthop"]: next-hop entry index
--                        params["source"]: prefix of source ipv6 address;
--                        params["source"]["length"]: - prefix length
--                        params["vrf-id"]: Virtual Router ID
--
--        @return       true on success, otherwise false and error message
--
local function no_ipv6_mc_route_func(params)
    -- print(to_string(params));
    -- Common variables declaration
    local result, values;
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local group_ipv6_address, group_mask_length, vrf_id;
    local src_ipv6_address, src_mask_length;
    local apiName, is_failed, result, OUT_values;

    -- Command specific variables initialization.
    group_ipv6_address        = params["group-ipv6-prefix"];
    group_mask_length         = group_ipv6_address["length"];
    src_ipv6_address          = params["source"]
        or {string="0:0:0:0:0:0:0:0/0", length=0, "0","0","0","0","0","0","0","0"} ;
    src_mask_length         = src_ipv6_address["length"];
    vrf_id                  = params["vrf-id"] or 0 -- set 0 if nil

    -- Ipv6 prefix adding.
    apiName = "cpssDxChIpLpmIpv6McEntryDel";
    is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            { "IN",     "GT_IPV6ADDR", "ipGroup",              group_ipv6_address},
            { "IN",     "GT_U32",      "ipGroupPrefixLen",     group_mask_length  },
            { "IN",     "GT_IPV6ADDR", "ipSrc",                src_ipv6_address},
            { "IN",     "GT_U32",      "ipSrcPrefixLen",       src_mask_length}
        });

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------------------------


--------------------------------------------------------------------------------
-- command registration: ipv6 mc-route
--------------------------------------------------------------------------------
-- ipv6 mc-route <ipv6-addr /prefix-len>
--     nexthop <next_hop_index>
--     [source <ipv6-addr/prefix-len>]
--     [vrf-id = <vrf-id>]
CLI_addCommand("config", "ipv6 mc-route", {
func   = ipv6_mc_route_func,
help   = "Creating of MC IPV6 Routes",
  params = {
      { type = "values",
          {format = "%ipv6_mc_prefix", name = "group-ipv6-prefix",
              help = "Ipv6 group address prefix"}
      },
      { type="named",
          {format = "nexthop %next-hop-id",
              help = "Apply to given next-hop id",
              requirements = {"group-ipv6-prefix"}},
          -- optional parameters
          {format="source %ipv6_uc_prefix",
              help = "Source Ipv6 address prefix",
              requirements = {"nexthop"}},
          {format="vrf-id %vrf_id",
              help = "a Virtual Router Id. The default is 0",
              requirements = {"nexthop"}},
          mandatory = { "nexthop" },
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ipv6 mc-route
--------------------------------------------------------------------------------
-- no ipv6 mc-route <ipv6-addr/prefix-len>
--     [source <ipv6-addr/prefix-len>]
--     [vrf-id = <vrf-id>]
CLI_addCommand("config", "no ipv6 mc-route", {
func   = no_ipv6_mc_route_func,
help   = "Removing of MC IPV6 Routes",
    params = {
        { type = "values",
            {format = "%ipv6_mc_prefix", name = "group-ipv6-prefix",
                help = "Ipv6 group address prefix"},
        },
        { type="named",
            -- optional parameters
            {format="source %ipv6_uc_prefix",
                help = "Source Ipv6 pddress prefix",
                requirements = {"group-ipv6-prefix"}},
            {format="vrf-id %vrf_id",
                help = "a Virtual Router Id. The default is 0",
                requirements = {"group-ipv6-prefix"}},
            mandatory = { "group-ipv6-prefix" },
        }
    }
})

-- show command ---------------------------------------------------------------
-- "Search" should be called before "GetNext" to check that VRID initialized and
-- Prefix presents. "GetNext" must be called with Prefix presents in memory.

-- initializes entry_inout_data used as iterator
local function init_get_next_mc_route_inout_data()
    return {
        group_ipv6_address  = "ff00::",
        group_mask_length   = 8,
        src_ipv6_address    = "::",
        src_mask_length     = 0,
        };
end

-- returns (not is_failed), CPSS_DXCH_IP_LTT_ENTRY_STC of current entry
local function search_mc_route_entry(command_data, vrf_id, entry_inout_data)
    local apiName, is_failed, result, OUT_values, values;

    apiName = "cpssDxChIpLpmIpv6McEntrySearch"
    result, OUT_values = myGenWrapper(
        apiName,
        {
            { "IN",  "GT_U32",                      "lpmDBId",               0},
            { "IN",  "GT_U32",                      "vrId",                  vrf_id},
            { "IN",  "GT_IPV6ADDR",                 "ipGroupPtr",            entry_inout_data.group_ipv6_address},
            { "IN",  "GT_U32",                      "ipGroupPrefixLen",      entry_inout_data.group_mask_length},
            { "IN",  "GT_IPV6ADDR",                 "ipSrcPtr",              entry_inout_data.src_ipv6_address},
            { "IN",  "GT_U32",                      "ipSrcPrefixLen",        entry_inout_data.src_mask_length},
            { "OUT", "CPSS_DXCH_IP_LTT_ENTRY_STC",  "mcRouteLttEntryPtr"},
            { "OUT", "GT_U32",                      "tcamGroupRowIndexPtr"},
            { "OUT", "GT_U32",                      "tcamSrcRowIndexPtr"}
        });

    -- print("OUT_values " .. to_string(OUT_values));
    is_failed = false;
    if result ~= 0 then
        is_failed = true;
        if (result ~= 0x0B) and (result ~= 0x12) then
            -- GT_NOT_FOUND             (0x0B) /* Item not found                        */
            -- GT_NOT_INITIALIZED       (0x12) /* The item is not initialized           */
            command_data:addErrorAndPrint(apiName .. " failed, rc = " .. tonumber(result));
        end
    end

    values = nil;
    if not is_failed then
        values = OUT_values.mcRouteLttEntryPtr;
    end

    return (not is_failed), values;
end

-- updates entry_inout_data used as iterator
-- returns (not is_failed), CPSS_DXCH_IP_LTT_ENTRY_STC of current entry
local function get_next_mc_route_entry(command_data, vrf_id, entry_inout_data)
    local apiName, is_failed, result, OUT_values, values;
    apiName = "cpssDxChIpLpmIpv6McEntryGetNext";
    result, OUT_values = myGenWrapper(
        apiName,
        {
            {  "IN",    "GT_U32",                      "lpmDBId",               0      },
            {  "IN",    "GT_U32",                      "vrId",                  vrf_id },
            {  "INOUT", "GT_IPV6ADDR",                 "ipGroupPtr",            entry_inout_data.group_ipv6_address},
            {  "INOUT", "GT_U32",                      "ipGroupPrefixLenPtr",   entry_inout_data.group_mask_length},
            {  "INOUT", "GT_IPV6ADDR",                 "ipSrcPtr",              entry_inout_data.src_ipv6_address},
            {  "INOUT", "GT_U32",                      "ipSrcPrefixLenPtr",     entry_inout_data.src_mask_length},
            {  "OUT",   "CPSS_DXCH_IP_LTT_ENTRY_STC",  "mcRouteLttEntryPtr"},
            {  "OUT",   "GT_U32",                      "tcamGroupRowIndexPtr"},
            {  "OUT",   "GT_U32",                      "tcamSrcRowIndexPtr"},
        });

    -- print("OUT_values " .. to_string(OUT_values));
    is_failed = false;
    if result ~= 0 then
        is_failed = true;
        if (result ~= 0x0B) and (result ~= 0x12) then
            -- GT_NOT_FOUND             (0x0B) /* Item not found                        */
            -- GT_NOT_INITIALIZED       (0x12) /* The item is not initialized           */
            command_data:addErrorAndPrint(apiName .. " failed, rc = " .. tonumber(result));
        end
    end
    values = nil;
    if not is_failed then
        -- update iterator
        entry_inout_data.group_ipv6_address  = OUT_values.ipGroupPtr;
        entry_inout_data.group_mask_length   = OUT_values.ipGroupPrefixLenPtr;
        entry_inout_data.src_ipv6_address    = OUT_values.ipSrcPtr;
        entry_inout_data.src_mask_length     = OUT_values.ipSrcPrefixLenPtr;
        values = OUT_values.mcRouteLttEntryPtr;
    end
    return (not is_failed), values;
end

local mcPrefixCol = {
    {{1}, "Group Ipv6 Address / prefix length         ",  "s"},
    {{2}, "Source Ipv6 Address / prefix length        ",  "s"},
    {{3}, "Next Hop",     "d"}
};

local function show_ipv6_mc_route_func(params)
    local command_data = Command_Data()
    local vrf_id = params["vrf-id"];
    if not vrf_id then
        vrf_id = 0;
    end
    local entry_inout_data = init_get_next_mc_route_inout_data();
    local res_ok, ipv6_ltt_entry, line;
    print("MC LPM entries:")
    print(string_for_table_break(nil, mcPrefixCol));
    print(string_for_table_title(nil, mcPrefixCol));
    print(string_for_table_break(nil, mcPrefixCol));

    res_ok, ipv6_ltt_entry = search_mc_route_entry(
        command_data, vrf_id, entry_inout_data);

    while res_ok do
        print(string.format("%39s/%-3d %39s/%-3d %d", entry_inout_data.group_ipv6_address,
                entry_inout_data.group_mask_length, entry_inout_data.src_ipv6_address,
                entry_inout_data.src_mask_length, ipv6_ltt_entry.routeEntryBaseIndex))

        res_ok, ipv6_ltt_entry = get_next_mc_route_entry(
            command_data, vrf_id, entry_inout_data)
        if not res_ok then
            break;
        end
    end
    print(string_for_table_break(nil, mcPrefixCol));
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show ipv6 mc-route
--------------------------------------------------------------------------------
-- show ipv6 mc-route
CLI_addCommand("exec", "show ipv6 mc-route", {
    func   = show_ipv6_mc_route_func,
    help   = "Show all MC IPV6 Routes",
    params = {
        -- parameter vrf-id is optional
        { type="named",
            {format="vrf-id %vrf_id",
                help = "a Virtual Router Id. The default is 0"},
        }
    }
})

