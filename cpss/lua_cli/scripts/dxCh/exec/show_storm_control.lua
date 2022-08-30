--
-- ************************************************************************
---
--  show_storm_control
--        @description  show_storm_control - Displays storm-control related configuration
--
--        @return       true on success, otherwise false and error message
--
local function show_storm_control(params)

    local did_error, result, values
    local enablePtr
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization
    header_string = "\n" ..
        "Port    Broadcast    Known    Unknown     Registered   Unregistered    TCP-SYN    Rate(64 bytes/packets)\n" ..
        "                     unicast  unicast     multicast    multicast                                        \n" ..
        "----    ---------    -------  -------     ----------   ------------    -------    ---------------------"
    footer_string = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    for dummy, devNum, portNum in command_data:getPortIterator() do
        did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitGet", {
                                               {"IN", "GT_U8", "devNum", devNum},
                                               {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                                               {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr"}})
        local portGfgPtr = values["portGfgPtr"]
        did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynGet", {
                                               {"IN", "GT_U8", "devNum", devNum},
                                               {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                                               {"OUT", "GT_BOOL", "enablePtr"}})
        local enablePtr = values["enablePtr"]
        command_data["result"] = string.format("%-7s %-12s %-8s %-10s %-13s %-15s %-10s %-9s",
                                               tostring(portNum),
                                               tostring(portGfgPtr.enableBc),
                                               tostring(portGfgPtr.enableUcKnown),
                                               tostring(portGfgPtr.enableMcReg),
                                               tostring(portGfgPtr.enableUcUnk),
                                               tostring(portGfgPtr.enableMc),
                                               tostring(enablePtr),
                                               tostring(portGfgPtr.rateLimit))
        command_data:addResultToResultArray()
        command_data:updatePorts()
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
                                         footer_string, "There is no storm control information to show.\n")

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- ************************************************************************
--
--  show_storm_control_rate_limit_mode
--        @description  Displays storm-control rate limit mode related configuration
--
--        @return       true on success, otherwise false and error message
--
local function show_storm_control_rate_limit_mode(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:clearResultArray()

    local table_info = {
        { head="Device",len=6,path="devNum" },
        { head="Ingress Rate Limit Mode",len=28,align="c",path="rateLimitMode"}
    }

    command_data:initTable(table_info)

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            local entry = {devNum=devNum,rateLimitMode="n/a"}

            -- Get rate limit mode configuration
            if true == command_data["local_status"] then
                result, values = cpssPerDeviceParamGet(
                                    "cpssDxChBrgGenRateLimitGlobalCfgGet", devNum,
                                    "brgRateLimitPtr", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC")
                command_data:handleCpssErrorDevice(result,
                        "get rate limit configuration", devId)
                if 0 == result then
                    entry.rateLimitMode = values["brgRateLimitPtr"].rMode
                end
            end

            command_data:updateStatus()

            command_data:addTableRow(entry)

            command_data:updateEntries()
        end
    end

    -- Resulting table string formatting.
    command_data:showTblResult("There is no rate limit mode to show.")

    return command_data:getCommandExecutionResults()
end


-- exec: show storm_control device %devID
CLI_addCommand("exec", "show storm-control", {
    func = show_storm_control,
    help = "Show storm-control",
    params = {
        { type = "named",
        "#all_interfaces",
        mandatory = { "all_interfaces" }
    }
}
})

-- exec: show storm_control_rate_limit_mode configuration device %devID
CLI_addCommand("exec", "show storm-control rate-limit-mode", {
  func   = show_storm_control_rate_limit_mode,
  help   = "Show rate limit mode configuration ",
  params = {
      { type="named",
          "#all_device",
      }
  }
})

