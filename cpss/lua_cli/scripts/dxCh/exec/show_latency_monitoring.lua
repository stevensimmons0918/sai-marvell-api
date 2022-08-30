--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_latency_monitoring.lua
--*
--* DESCRIPTION:
--*       Latency monitoring show commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function showLatencyMonitorPortConfigPerPort(command_data, devNum, portNum, params)
    local result, value
    local srcIdx, srcProf, dstIdx, dstProf, portProf
    local enable

    if not is_sip_6(devNum) then
        return
    end

    result, values = myGenWrapper("cpssDxChLatencyMonitoringPortCfgGet",{
        { "IN",  "GT_U8",                "devNum",   devNum                     },
        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portNum                    },
        { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_INGRESS_E" },
        { "OUT", "GT_U32", "indexPtr"   },
        { "OUT", "GT_U32", "profilePtr" }
    })
    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgGet "..returnCodes[result])
    end
    srcIdx = values["indexPtr"]
    srcProf = values["profilePtr"]

    result, values = myGenWrapper("cpssDxChLatencyMonitoringPortCfgGet",{
        { "IN",  "GT_U8",                "devNum",   devNum                     },
        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portNum                    },
        { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_EGRESS_E" },
        { "OUT", "GT_U32", "indexPtr"   },
        { "OUT", "GT_U32", "profilePtr" }
    })
    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgGet "..returnCodes[result])
    end
    dstIdx = values["indexPtr"]
    dstProf = values["profilePtr"]

    result, values = myGenWrapper("cpssDxChLatencyMonitoringPortEnableGet",{
        { "IN",  "GT_U8",                "devNum",   devNum  },
        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portNum },
        { "OUT", "GT_BOOL",              "enablePtr"         }
    })
    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error calling cpssDxChLatencyMonitoringPortEnableGet "..returnCodes[result])
    end

    if values["enablePtr"] == true then
        enable = "yes"
    else
        enable = "no"
    end

    if is_sip_6_10(devNum) then
        result, values = myGenWrapper("cpssDxChLatencyMonitoringPortProfileGet",{
            { "IN",  "GT_U8",                "devNum",   devNum  },
            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portNum },
            { "OUT", "GT_U32",               "portProfilePtr"    }
        })
        if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error calling cpssDxChLatencyMonitoringPortProfileGet "..returnCodes[result])
        end
    end

    portProf = values["portProfilePtr"]

    if is_sip_6_10(devNum) then
        command_data["result"] =
        string.format("%-10s|%-20s|%-12s|%-20s|%-12s|%-9s|%-18s|",
        string.format("%d/%d", devNum, portNum), srcProf, srcIdx, dstProf, dstIdx, enable, portProf)
    else
        command_data["result"] =
        string.format("%-10s|%-20s|%-12s|%-20s|%-12s|%-9s|",
        string.format("%d/%d", devNum, portNum), srcProf, srcIdx, dstProf, dstIdx, enable)
    end
    command_data:addResultToResultArray()
end

local function showLatencyMonitorPortConfig(params)
    local header_string, footer_string

    if is_sip_6_10(devNum) then
        header_string =
        "\n" ..
        "Port      | Src Profile number | Src Index  | Dst Profile number | Dst Index  | Enabled |Queue mode profile|\n" ..
        "----------|--------------------|------------|--------------------|------------|---------|------------------|"
    else
        header_string =
        "\n" ..
        "Port      | Src Profile number | Src Index  | Dst Profile number | Dst Index  | Enabled |\n" ..
        "----------|--------------------|------------|--------------------|------------|---------|"
    end
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string

    -- Perform action --
    return generic_all_ports_show_func(showLatencyMonitorPortConfigPerPort, params)
end

--------------------------------------------------------------------------------
-- command registration: show latency-monitor port-config
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show latency-monitor port-config", {
    func   = showLatencyMonitorPortConfig,
    help   = "Show latency monitoring port configuration",
    params = {{type = "named", "#all_interfaces", mandatory = {"all_interfaces"}}}
})

local function showLatencyMonitorPortTrigger(params)
    local result, value
    local iterator
    local devNum
    local portsList = {}
    local header_string, footer_string
    local i, j
    local command_data = Command_Data()
    local srcProfile
    local dstProfile

    header_string =
    "\n" ..
    "Source Port | Target Port | Profile |\n" ..
    "------------|-------------|---------|"
    footer_string = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    -- Common variables initialization
    i = 1
    command_data:initAllAvaiblesPortIterator(params)
    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()
        portsList[i] = {["dev"]=devNum, ["port"]=portNum}
        i = i + 1
        command_data:updateStatus()
        command_data:updatePorts()
    end

    command_data:clearResultArray()
    for i=1,#portsList do
        for j=1,#portsList do
            if is_sip_6(portsList[i]["dev"]) and (portsList[i]["dev"] == portsList[j]["dev"]) then
                result, values = myGenWrapper("cpssDxChLatencyMonitoringEnableGet",{
                    { "IN",  "GT_U8",                "devNum",      portsList[i]["dev"]  },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "srcPortNum",  portsList[i]["port"] },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "trgPortNum",  portsList[j]["port"] },
                    { "OUT", "GT_BOOL",              "enabledPtr"                }
                })
                if 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error calling cpssDxChLatencyMonitoringEnableGet "..returnCodes[result])
                end

                if values["enabledPtr"] == true then
                    result, values = myGenWrapper("cpssDxChLatencyMonitoringPortCfgGet",{
                        { "IN",  "GT_U8",                "devNum",   portsList[i]["dev"]        },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portsList[i]["port"]       },
                        { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_INGRESS_E" },
                        { "OUT", "GT_U32", "indexPtr"   },
                        { "OUT", "GT_U32", "profilePtr" }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgGet "..returnCodes[result])
                    end
                    srcProfile = values["profilePtr"]

                    result, values = myGenWrapper("cpssDxChLatencyMonitoringPortCfgGet",{
                        { "IN",  "GT_U8",                "devNum",   portsList[i]["dev"]        },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",  portsList[j]["port"]       },
                        { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_EGRESS_E"  },
                        { "OUT", "GT_U32", "indexPtr"   },
                        { "OUT", "GT_U32", "profilePtr" }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgGet "..returnCodes[result])
                    end
                    dstProfile = values["profilePtr"]

                    command_data:addToResultArray(string.format("%-12s|%-13s|%-9s|",
                        string.format("%d/%d", portsList[i]["dev"], portsList[i]["port"]),
                        string.format("%d/%d", portsList[j]["dev"], portsList[j]["port"]),
                        bit_and(srcProfile + dstProfile, 0x1FF)
                    ))
                end
            end
        end
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()

    if #command_data["result_array"] == 0 then
        print("Nothing to show")
    else
        command_data:printAndFreeResultArray()
    end

    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show latency-monitor pair-enabled
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show latency-monitor pair-enabled", {
    func   = showLatencyMonitorPortTrigger,
    help   = "Show latency monitoring enabled port pairs",
    params = {{type = "named", "#all_device"}}
})

local function showLatencyMonitorStatistics(params)
    local result, values
    local j, i
    local command_data = Command_Data()
    local header_string, footer_string
    local lmuBmp

    local devNum = params["device"]

    header_string =
        "\n" ..
        "Profile|       Min      |       Avg      |      Max      |  In range  | Out of Range |\n" ..
        "-------|----------------|----------------|---------------|------------|--------------|"
    footer_string = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:clearResultArray()

    if is_sip_6(devNum) then
        for j = 1, #params["monProfile"] do
            result, values = myGenWrapper("cpssDxChLatencyMonitoringStatGet",{
               { "IN",  "GT_U8",                                   "devNum",           devNum                   },
               { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           0Xffffffff               },
               { "IN",  "GT_U32",                                  "latencyProfile",   params["monProfile"][j]  },
               { "OUT", "CPSS_DXCH_LATENCY_MONITORING_STAT_STC",   "statisticsPtr"}
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringStatGet "..returnCodes[result])
            end
   		    command_data:addToResultArray(string.format("%-7d|%-16d|%-16d|%-15d|%-12d|%-14d|",
                                                   params["monProfile"][j],
                                                   values["statisticsPtr"]["minLatency"],
                                                   values["statisticsPtr"]["avgLatency"],
                                                   values["statisticsPtr"]["maxLatency"],
                                                   tonumber(to_string(values["statisticsPtr"]["packetsInRange"])),
                                                   tonumber(to_string(values["statisticsPtr"]["packetsOutOfRange"]))
                                                   ))
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printAndFreeResultArray()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show latency-monitor statistics
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show latency-monitor statistics", {
    func   = showLatencyMonitorStatistics,
    help   = "Show latency monitoring statistics",
    params = {{type = "named",
                {format="device %devID",  name="device", help = "Device number" },
                {format="latency-profile %latProfRange",  name="monProfile", help = "Latency monitoring profile range" },
                mandatory = {"device", "monProfile"}
             }}
})

local function showLatencyMonitorProfileConfiguration(params)
    local result, values
    local j
    local command_data = Command_Data()
    local header_string, footer_string

    local devNum = params["device"]
    local samplingProfile
    local lmuProfileConfig
    local samplingMode

    header_string =
        "\n" ..
        "Profile | Range min | Range max | Notif. threshold | Sampling Profile | Sampl. threshold  | Sampl. mode  \n" ..
        "--------|-----------|-----------|------------------|------------------|-------------------|---------------"
    footer_string = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    command_data:clearResultArray()

    if is_sip_6(devNum) then
        for j = 1, #params["monProfile"] do
            result, lmuProfileConfig = myGenWrapper("cpssDxChLatencyMonitoringCfgGet",{
                { "IN",  "GT_U8",                                   "devNum",           devNum                  },
                { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           1                       },
                { "IN",  "GT_U32",                                  "latencyProfile",   params["monProfile"][j] },
                { "OUT", "CPSS_DXCH_LATENCY_MONITORING_CFG_STC",    "lmuConfigPtr"}
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringCfgGet "..returnCodes[result])
            end

            result, values = myGenWrapper("cpssDxChLatencyMonitoringSamplingProfileGet",{
                { "IN",  "GT_U8",    "devNum",           devNum                  },
                { "IN",  "GT_U32",   "latencyProfile",   params["monProfile"][j] },
                { "OUT", "GT_U32",   "samplingProfilePtr"}
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringSamplingProfileGet "..returnCodes[result])
            end

            samplingProfile = values["samplingProfilePtr"]

            result, values = myGenWrapper("cpssDxChLatencyMonitoringSamplingConfigurationGet",{
                { "IN",  "GT_U8",    "devNum",            devNum          },
                { "IN",  "GT_U32",   "samplingProfile",   samplingProfile },
                { "OUT", "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT", "samplingModePtr"},
                { "OUT", "GT_U32",   "samplingThresholdPtr"}
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringSamplingConfigurationGet: "..returnCodes[result])
            end

            if values["samplingModePtr"] == "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E" then
                samplingMode = "deterministic"
            elseif values["samplingModePtr"] == "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E" then
                samplingMode = "random"
            end

            command_data:addToResultArray(
                string.format("%-8d|%-11d|%-11d|%-18d|%-18s|%-19s|%-15s",
                params["monProfile"][j],
                lmuProfileConfig["lmuConfigPtr"]["rangeMin"],
                lmuProfileConfig["lmuConfigPtr"]["rangeMax"],
                lmuProfileConfig["lmuConfigPtr"]["notificationThresh"],
                samplingProfile, values["samplingThresholdPtr"], samplingMode))
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printAndFreeResultArray()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show latency-monitor profile-configuration
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show latency-monitor profile-configuration", {
    func   = showLatencyMonitorProfileConfiguration,
    help   = "Show latency profile configuration",
    params = {{type = "named",
                {format="device %devID",  name="device", help = "Device number" },
                {format="profiles %latProfRange",  name="monProfile", help = "Latency monitoring profile range" },
                mandatory = {"device", "monProfile"}
             }}
})

local function showLatencyMonitorProfileMode(params)
    local result, value
    local command_data = Command_Data()
    local devNum = params["device"]

    if not is_sip_6_10(devNum) then
        return
    end

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:clearResultArray()

    result, value = myGenWrapper("cpssDxChLatencyMonitoringProfileModeGet",{
        { "IN",  "GT_U8",                              "devNum",   devNum },
        { "OUT", "CPSS_DXCH_LATENCY_PROFILE_MODE_ENT", "profileModePtr" }
    })
    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error calling cpssDxChLatencyMonitoringProfileModeGet "..returnCodes[result])
    end

    print('profile Mode = ' .. (value["profileModePtr"] == "CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E" and
                               'CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E' or 'CPSS_DXCH_LATENCY_PROFILE_MODE_PORT_E'))

    command_data:analyzeCommandExecution()
    command_data:printAndFreeResultArray()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show latency-profile-mode
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show latency-profile-mode", {
    func   = showLatencyMonitorProfileMode,
    help   = "Show latency monitoring profile mode",
    params = {{type = "named", "#all_device"}}
})

local function showLatencyMonitorQueueProfile(params)
    local result, value, i, queueProf
    local command_data = Command_Data()
    local header_string, footer_string
    local devNum = params["device"]

    if not is_sip_6_10(devNum) then
        return
    end

    header_string =
        "\n" ..
        "|   Traffic Class    | Queue Profile      | \n" ..
        "|--------------------|--------------------|"
    footer_string = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:clearResultArray()

    for i=0, 15 do
        result, value = myGenWrapper("cpssDxChLatencyMonitoringQueueProfileGet",{
            { "IN",  "GT_U8",  "devNum",   devNum },
            { "IN",  "GT_U8",  "tcQueue",   i     },
            { "OUT", "GT_U32", "queueProfilePtr"  }
        })
        if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error calling cpssDxChLatencyMonitoringQueueProfileGet "..returnCodes[result])
        end
        queueProf = value["queueProfilePtr"]

        command_data:addToResultArray(string.format("|%-20s|%-20s|", i, queueProf))
    end

    command_data:analyzeCommandExecution()
    command_data:printAndFreeResultArray()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show queue-profile
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show queue-profile", {
    func   = showLatencyMonitorQueueProfile,
    help   = "Show latency monitoring queue profile",
    params = {{type = "named", "#all_device"}}
})

CLI_addHelp("exec", "show latency-monitor", "Latency monitoring")
