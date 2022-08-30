--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_address_table_hash.lua
--*
--* DESCRIPTION:
--*       display FDB Hash mode and and the length of hash chain for address
--*       lookups in the Forwarding Database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants






-- ************************************************************************
---
--  Command_Data:show_mac_address_table_hash_func
--        @description  show's of FDB Hash mode and Hash chain length for
--                      all devices in the unit
--
--        @param params - params["all_device"]: all or given devices
--
--        @return       true on success, otherwise false and error message
--
local function show_mac_address_table_hash_func(params)
    -- Common variables declaration
    local result, values, devNum
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:clearResultArray()

    local table_info = {
        { head="Device"      ,len=6  ,align="c" ,path="devNum"},
        { head="Mode"        ,len=9  ,align="c" ,path="mode"},
        { head=" Chain Length",len=14 ,align="c" ,path="length"}
    }

    command_data:initTable(table_info)

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            local entry = {devNum=devNum,mode="n/a",length="n/a"}
            -- Get Hash mode
            if true == command_data["local_status"] then
                result, values = cpssPerDeviceParamGet("cpssDxChBrgFdbHashModeGet",
                                                        devNum,
                                                       "mode",
                                                       "CPSS_MAC_HASH_FUNC_MODE_ENT")
                command_data:handleCpssErrorDevice(result, "get FDB Hash mode", devId)

                if 0 == result then
                    -- Fill entry with FDB Hash mode
                    entry.mode = fdbHashModeStrGet(values["mode"])
                end
            end

            -- Get Hash chain length
            if true == command_data["local_status"] then
                result, values = cpssPerDeviceParamGet( "cpssDxChBrgFdbMaxLookupLenGet",
                                                         devNum,
                                                         "length",
                                                         "GT_U32")
                command_data:handleCpssErrorDevice(result, "get Hash chain length", devId)

                if 0 == result then
                    -- Fill entry with Hash chain length
                    entry.length = values.length
                end
            end

            command_data:updateStatus()

            command_data:addTableRow(entry)

            command_data:updateEntries()
        end
    end

    -- Resulting table string formatting.
    command_data:showTblResult("There is no FDB Hash to show.")

    return command_data:getCommandExecutionResults()
end


-----------------------------------------------------------
-- command registration: show mac address-table hash
-----------------------------------------------------------
CLI_addCommand("exec", "show mac address-table hash", {
  func   = show_mac_address_table_hash_func,
  help   = "Show FDB Hash mode and the length of hash chain " ..
           "for address lookups in the FDB table",
  params = {
      { type="named",
          "#all_device",
      }
  }
})
