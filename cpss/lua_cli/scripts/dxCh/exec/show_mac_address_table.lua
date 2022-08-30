--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_address_table.lua
--*
--* DESCRIPTION:
--*       showing of entries in the bridge-forwarding database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

--includes

--constants

--[[ ************************************************************************
---  is_same_table_instance
--      @description compares 2 instance of tested table(Mac key)
--      @param t1 = first instance to comapre
--             t2 = second instance to comapre
--             ignore_mt = true  - Ignore table metadata comparision
--                         false - Comapre table metadta
--
--      @return       true - both the instance matches, otherwise false
]]
function is_same_table_instance(t1,t2,ignore_mt)
    local ty1 = type(t1)
    local ty2 = type(t2)
    if ty1 ~= ty2 then return false end
    -- non-table types can be directly compared
    if ty1 ~= 'table' and ty2 ~= 'table' then return t1 == t2 end
    -- as well as tables which have the metamethod __eq
    local mt = getmetatable(t1)
    if not ignore_mt and mt and mt.__eq then return t1 == t2 end
    for k1,v1 in pairs(t1) do
       local v2 = t2[k1]
       if v2 == nil or not is_same_table_instance(v1,v2) then return false end
    end
    for k2,v2 in pairs(t2) do
       local v1 = t1[k2]
       if v1 == nil or not is_same_table_instance(v1,v2) then return false end
    end
    return true
end

--[[ ************************************************************************
---  is_entry_exist
--      @description Verifies the entry key exist in table.
--      @param key   = mac key need to be serach
--             table = table which holds are entries
--
--      @return       true - entry found in table, otherwise false
]]
function is_entry_exist(key, table)
    for _,v in pairs(table) do
        if(is_same_table_instance(v, key, false)) then
            return true
        end
    end
    return false
end

-- ************************************************************************
---
--  show_interfaces_status
--        @description  show's entries in the bridge-forwarding database
--
--        @param params         - params["static_dynamic"]: static
--                                property, could be irrelevant;
--                                params["static_dynamic_all"]: static
--                                entry property,could be irrelevant;
--                                params["vlan"]: vlan Id, could be
--                                irrelevant;
--                                params["ethernet"]: interface range,
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be
--                                irrelevant;
--                                params["mac-address"]: mac-address, could
--                                be irrelevant;
--                                params["skipped"]: skipping property,
--                                could be irrelevant;
--                                params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--
--        @return       true on success, otherwise false and error message
--
local function show_mac_address_table_func(params)
    -- Common variables declaration
    local result, values
    local devNum, entry_index
    local command_data = Command_Data()
    local entryTable   = {}
    local duplicateEntry

    -- if wrlCpssIsGmUsed() then -- TODO:
    --     print("The 'show mac address-table' command is disable in GM for perfomance reasons")
    --     return true
    -- end

    local function mac_table_cmd_view(str)
        if not prefix_match("CPSS_MAC_TABLE_",str) then
            return str
        end
        str = prefix_del("CPSS_MAC_TABLE_", str)
        if string.sub(str,-2) == "_E" then
            str = string.sub(str,1,-3)
        end
        return string.lower(str)
    end
    local table_info = {
        { head="Index",len=5,align="c",path="mac_entry_index",type="number" },
        { head="Address",len=22,align="c",path="addr_s"},
        { head="Vlan",len=5,align="c",path="mac_entry.key.key.macVlan.vlanId" },
        { head="VID1",len=5,align="c",path="mac_entry.key.vid1" },
        { head="Skip",len=4,align="c",path="skip_valid_aged.skip",type="boolYN" },
        { head="Interface",len=14,path="mac_entry.dstInterface",type=interfaceInfoStrGet },
        { head="Static",len=7,path="mac_entry.isStatic",type="bool" },
        { head="DA Route",len=8,path="mac_entry.daRoute",type="boolYN" },
        { head="saCommand",len=9,path="mac_entry.saCommand",type=mac_table_cmd_view },
        { head="daCommand",len=9,path="mac_entry.daCommand",type=mac_table_cmd_view },
        { head="sp",len=7,path="mac_entry.spUnknown",type="boolYN" }
    }


    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceMacEntryIterator(
        params, { ["dstInterface"] =
                    {["type"]    = { "CPSS_INTERFACE_PORT_E",
                                     "CPSS_INTERFACE_TRUNK_E",
                                     "CPSS_INTERFACE_VIDX_E",
                                     "CPSS_INTERFACE_VID_E"    }}})


    command_data:initTable(table_info)
    -- Main entries handling cycle
    if true == command_data["status"] then
        local iterator

        --display specified entire
        local entryExist = false
        for iterator, devNum, entry_index in command_data:getValidMacEntryIterator() do
            duplicateEntry = false
            if ((params["all_device"] == "all") and (true == isMultiDeviceConfigured())) then
                -- print("Triggered New logic of discarding duplicate for multi device enviroment")
                if(is_entry_exist(command_data.mac_entry.key.key, entryTable)) then
                    duplicateEntry = true
                else
                    table.insert(entryTable, command_data.mac_entry.key.key)
                end
            end

            if (command_data.mac_entry.key.key.macVlan ~= nil) and (duplicateEntry == false)then
                entryExist = true
                command_data:clearEntryStatus()

                -- Mac address entry getting.
                command_data:clearLocalStatus()

                local second_address_string = ""
                if (true == command_data["local_status"]) then
                    command_data.mac_entry_index = entry_index

                    command_data.addr_s = macOrIpAddressFirstStrGet(
                                command_data.mac_entry.key.entryType,
                                command_data.mac_entry.key.key.macVlan.macAddr,
                                ((command_data.mac_entry.key["key"] or {})["ipMcast"] or {})["sip"],
                                ((command_data.mac_entry.key["key"] or {})["ipMcast"] or {})["dip"])

                    second_address_string = macOrIpAddressSecondStrGet(
                                command_data.mac_entry.key.entryType,
                                command_data.mac_entry.key.key.macVlan.macAddr,
                                ((command_data.mac_entry.key["key"] or {})["ipMcast"] or {})["sip"],
                                ((command_data.mac_entry.key["key"] or {})["ipMcast"] or {})["dip"])
                end
                -- display all entries
                if params.entry == nil then
                    command_data:addTableRow(command_data)
                    if second_address_string ~= "" then
                        command_data:addTableRow({addr_s=second_address_string})
                    end
                else
                    -- display a specific entire
                    if entry_index == tonumber(params.entry) then
                        command_data:printTableRow(command_data)
                        if second_address_string ~= "" then
                            command_data:printTableRow({addr_s=second_address_string})
                        end
                        entryExist = true
                        break
                    end
                end

                command_data:updateEntries()
            end
        end
        if params.entry == nil then
            command_data:showTblResult("There is no mac address information to show.")
        end
        if params.entry ~= nil and entryExist == false then
            print("Invalid entry index: "..to_string(params.entry))
        end
    end

	return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show mac address-table
--------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- show mac address-table
-- @class table
-- @name show mac address-table
-- @description             show's entries in the bridge-forwarding database
-- @field command_path      exec/show_mac_address_table
-- @field static            show only static entries
-- @field dynamic           show only dynamic entries
CLI_addHelp("exec", "show mac", "MAC configuration")
CLI_addCommand("exec", "show mac address-table", {
  func   = show_mac_address_table_func,
  help   = "Show MAC address table configuration",
  params = {
      { type = "named",
          "#static_dynamic_all",
          "#all_device",
          "#vlan",
          "#interface_port_channel",
          "#mac-address",
          "#skipped",
          { format="entry %num", name = "entry", help="To display a certain row" },
          {format = "vid1 %unchecked-vlan"},
      }
  }
})


--##################################
--##################################
-- global variable that hold indication that the 'fdb shadow is used'
--[[local]] fdb_use_shadow = false

-- implement 'per device' : 'debug' --> "fdb-use-shadow" command
--[[local]] function per_device__debug_fdb_use_shadow(command_data,devNum,params)
    local apiName = "prvTgfBrgFdbInit"
    local devNum_param         = {"IN","GT_U8"  ,"devNum"        ,devNum}
    local allowAnyDevice_param = {"IN","GT_BOOL","allowAnyDevice",true  }
    
    local api_params = {devNum_param,allowAnyDevice_param}
    
    if command_data == nil then
        local ret = myGenWrapper(apiName,api_params)
        if ret ~= 0 then
            print(apiName .. " : failed"..to_string(returnCodes[ret]))
        end
    else
        genericCpssApiWithErrorHandler(command_data,
            apiName, api_params)
    end
    
    fdb_use_shadow = true
end

-- implement : 'debug' --> "fdb-use-shadow" command
local function debug_fdb_use_shadow(params)
    return generic_all_device_func(per_device__debug_fdb_use_shadow,params)
end 

local help_debug_fdb_use_shadow = "start to use FDB shadow. used with 'show fdb' and 'clear fdb' "
CLI_addHelp("debug", "fdb-use-shadow", help_debug_fdb_use_shadow) 
CLI_addCommand("debug", "fdb-use-shadow",
    { func = debug_fdb_use_shadow,
      help = help_debug_fdb_use_shadow,
      params =
      {
            { type="named",
            "#all_device",-- specific device or on all devices
            }
      }
    });
--##################################
--##################################
