--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* latency_monitoring.lua
--*
--* DESCRIPTION:
--*       Latency monitoring
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function latencyAssignSampling(params)
    local result
    local devlist
    local i, j
    local value
    local command_data = Command_Data()

    if params["all_device"] ~= "all" then
        devlist = {params["all_device"]}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i=1,#devlist do
        if is_sip_6(devlist[i]) then
            for j = 1, #params["monProfile"] do
                result = myGenWrapper("cpssDxChLatencyMonitoringSamplingProfileSet",{
                    { "IN", "GT_U8",    "devNum",           devlist[i]              },
                    { "IN", "GT_U32",   "latencyProfile",   params["monProfile"][j] },
                    { "IN", "GT_U32",   "samplingProfile",  params["smpProfile"]    }
                })
                if 0 ~= result then
                    command_data:setFailLocalStatus()
                    command_data:addError("Error calling cpssDxChLatencyMonitoringSamplingProfileSet "..returnCodes[result])
                end
            end
            result, value = myGenWrapper("cpssDxChLatencyMonitoringSamplingConfigurationGet",{
                { "IN", "GT_U8",    "devNum",            devlist[i]           },
                { "IN", "GT_U32",   "samplingProfile",   params["smpProfile"] },
                { "OUT", "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT",  "samplingModePtr" },
                { "OUT", "GT_U32",  "samplingThresholdPtr" }
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringSamplingConfigurationGet: "..returnCodes[result])
            end

            if params["threshold"] then
                value["samplingThresholdPtr"] =  params["threshold"]
            end

            if params["smpMode"] then
                value["samplingModePtr"] =  params["smpMode"]
            end

            result = myGenWrapper("cpssDxChLatencyMonitoringSamplingConfigurationSet",{
                { "IN", "GT_U8",    "devNum",            devlist[i]                     },
                { "IN", "GT_U32",   "samplingProfile",   params["smpProfile"]           },
                { "IN", "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT",   "samplingMode", value["samplingModePtr"]},
                { "IN", "GT_U32",   "samplingThreshold", value["samplingThresholdPtr"]  }
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringSamplingConfigurationSet: "..returnCodes[result])
            end
        end
    end
    command_data:analyzeCommandExecution()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor profile-sampling-assignment
--------------------------------------------------------------------------------
CLI_addCommand("config", "latency-monitor profile-sampling-assignment", {
    func   = latencyAssignSampling,
    help   = "Assign sampling profile to monitoring profile and set threshold",
    params = {
        {type = "named",
            "#all_device",
            {format="latency-profile %latProfRange", name="monProfile", help = "Latency monitoring profile range" },
            {format="sampling-profile %smpProf", name="smpProfile", help = "Sampling profile" },
            {format="mode %smpMode",             name="smpMode",    help = "Sampling mode" },
            {format="threshold %GT_U32",         name="threshold",  help = "Sampling threshold" },
            mandatory = {"monProfile", "smpProfile"}
        }
    }
})

local function latencyProfileConfiguration(params)
    local result, values
    local devlist
    local i, j
    local command_data = Command_Data()

    if params["all_device"] ~= "all" then
        devlist = {params["all_device"]}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i=1,#devlist do
        if is_sip_6(devlist[i]) then
            for j = 1, #params["monProfile"] do
                if params["flagNo"] == true then
                    values = {["lmuConfigPtr"] = {}}
                    values["lmuConfigPtr"]["rangeMin"] = 0
                    values["lmuConfigPtr"]["rangeMax"] = 0x3FFFFFFF
                    values["lmuConfigPtr"]["notificationThresh"] = 0x3FFFFFFF
                else
                    result, values = myGenWrapper("cpssDxChLatencyMonitoringCfgGet",{
                        { "IN", "GT_U8",                                "devNum",           devlist[i]              },
                        { "IN", "GT_LATENCY_MONITORING_UNIT_BMP",       "lmuBmp",           0Xffffffff              },
                        { "IN", "GT_U32",                               "latencyProfile",   params["monProfile"][j] },
                        { "OUT", "CPSS_DXCH_LATENCY_MONITORING_CFG_STC","lmuConfigPtr"                              },
                    })
                    if 0 ~= result then
                        command_data:setFailLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringCfgGet: "..returnCodes[result])
                    end

                    if params["range_min"] ~= nil then
                        values["lmuConfigPtr"]["rangeMin"] = params["range_min"]
                    end

                    if params["range_max"] ~= nil then
                        values["lmuConfigPtr"]["rangeMax"] = params["range_max"]
                    end

                    if params["threshold"] ~= nil then
                        values["lmuConfigPtr"]["notificationThresh"] = params["threshold"]
                    end
                end

                result = myGenWrapper("cpssDxChLatencyMonitoringCfgSet",{
                    { "IN",  "GT_U8",                                "devNum",           devlist[i]                 },
                    { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",       "lmuBmp",           0Xffffffff                 },
                    { "IN",  "GT_U32",                               "latencyProfile",   params["monProfile"][j]    },
                    { "IN",  "CPSS_DXCH_LATENCY_MONITORING_CFG_STC", "lmuConfigPtr",     values["lmuConfigPtr"]     }
                })
                if 0 ~= result then
                    command_data:setFailLocalStatus()
                    command_data:addError("Error calling cpssDxChLatencyMonitoringCfgSet: "..returnCodes[result])
                end
            end
        end
    end
    command_data:analyzeCommandExecution()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor profile-configuration
--------------------------------------------------------------------------------
CLI_addCommand("config", "latency-monitor profile-configuration", {
    func   = latencyProfileConfiguration,
    help   = "Configure latency profile",
    params = {
        {type = "named",
            "#all_device",
            {format="latency-profile %latProfRange",  name="monProfile", help = "Latency monitoring profile range" },
            {format="range-min %latency",             name="range_min",  help = "Range lower limit" },
            {format="range-max %latency",             name="range_max",  help = "Range upper limit" },
            {format="threshold %latency",             name="threshold",  help = "Threshold packet count" },
            mandatory = {"monProfile"}
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no latency-monitor profile-configuration
--------------------------------------------------------------------------------
CLI_addCommand("config", "no latency-monitor profile-configuration", {
    func   = function(params)
                params["flagNo"] = true
                latencyProfileConfiguration(params)
             end,
    help   = "Restore defaults for latency profile",
    params = {
        {type = "named",
            "#all_device",
            {format="latency-profile %latProfRange",  name="monProfile", help = "Latency monitoring profile range" },
            mandatory = {"monProfile"}
        }
    }
})

local function latencyMonitoringPortEnable(params)
    local result, values
    local command_data = Command_Data()
    local all_ports
    local portNum, portIndex, portNums
    local enable

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)
    all_ports = getGlobal("ifRange")

    if params["flagNo"] == true then
        enable = false
    else
        enable = true
    end

    for devNum, portNums in pairs(all_ports) do
        if is_sip_6(devNum) and (not is_sip_6_10(devNum)) then
            for portIndex = 1, #portNums do
                portNum = portNums[portIndex]

                result = myGenWrapper("cpssDxChLatencyMonitoringPortEnableSet",{
                    { "IN", "GT_U8",                "devNum",  devNum  },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "IN", "GT_BOOL",              "enable",  enable  }
                })
                if 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error calling cpssDxChLatencyMonitoringPortEnableSet "..returnCodes[result])
                end
            end
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor enable
--------------------------------------------------------------------------------
CLI_addCommand("interface", "latency-monitor enable", {
    func   = latencyMonitoringPortEnable,
    help   = "Enable latency monitoring for given port",
})

--------------------------------------------------------------------------------
-- command registration: no latency-monitor enable
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no latency-monitor enable", {
    func   = function(params)
                params["flagNo"] = true
                latencyMonitoringPortEnable(params)
            end,
    help   = "Disable latency monitoring for given port"
})

local function latencyMonitoringPortConfig(params)
    local result, values
    local command_data = Command_Data()
    local all_ports
    local portNum, portIndex, portNums

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)
    all_ports = getGlobal("ifRange")

    for devNum, portNums in pairs(all_ports) do
        if is_sip_6(devNum) then
            for portIndex = 1, #portNums do
                portNum = portNums[portIndex]
                if params["flagNo"] == true then
                    result = myGenWrapper("cpssDxChLatencyMonitoringEnableSet",{
                        { "IN", "GT_U8",                "devNum",       devNum               },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "srcPortNum",   portNum              },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "trgPortNum",   params["egressPort"] },
                        { "IN", "GT_BOOL",              "enabled",      false                }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringEnableSet "..returnCodes[result])
                    end

                    result = myGenWrapper("cpssDxChLatencyMonitoringPortCfgSet",{
                        { "IN", "GT_U8",                "devNum",     devNum                    },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",    portNum                   },
                        { "IN", "CPSS_DIRECTION_ENT",   "portType",   "CPSS_DIRECTION_INGRESS_E"},
                        { "IN", "GT_U32",               "indexPtr",   0                         },
                        { "IN", "GT_U32",               "profilePtr", 0                         }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgSet "..returnCodes[result])
                    end

                    result = myGenWrapper("cpssDxChLatencyMonitoringPortCfgSet",{
                        { "IN", "GT_U8",                "devNum",     devNum                    },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",    portNum                   },
                        { "IN", "CPSS_DIRECTION_ENT",   "portType",   "CPSS_DIRECTION_EGRESS_E" },
                        { "IN", "GT_U32",               "indexPtr",   0                         },
                        { "IN", "GT_U32",               "profilePtr", 0                         }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgSet "..returnCodes[result])
                    end
                else
                    result = myGenWrapper("cpssDxChLatencyMonitoringPortCfgSet",{
                        { "IN", "GT_U8",                "devNum",     devNum                            },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",    portNum                           },
                        { "IN", "CPSS_DIRECTION_ENT",   "portType",   "CPSS_DIRECTION_INGRESS_E"        },
                        { "IN", "GT_U32",               "indexPtr",   portNum%255+1                     },
                        { "IN", "GT_U32",               "profilePtr", bit_and(params["monProfile"], 0xF)}
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgSet "..returnCodes[result])
                    end

                    result = myGenWrapper("cpssDxChLatencyMonitoringPortCfgSet",{
                        { "IN", "GT_U8",                "devNum",     devNum                                },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",    params["egressPort"]                               },
                        { "IN", "CPSS_DIRECTION_ENT",   "portType",   "CPSS_DIRECTION_EGRESS_E"             },
                        { "IN", "GT_U32",               "indexPtr",   params["egressPort"] % 255+1          },
                        { "IN", "GT_U32",               "profilePtr", bit_and(params["monProfile"], 0x1F0)  }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringPortCfgSet "..returnCodes[result])
                    end

                    result = myGenWrapper("cpssDxChLatencyMonitoringEnableSet",{
                        { "IN", "GT_U8",                "devNum",       devNum               },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "srcPortNum",   portNum              },
                        { "IN", "GT_PHYSICAL_PORT_NUM", "trgPortNum",   params["egressPort"] },
                        { "IN", "GT_BOOL",              "enabled",      true                 }
                    })
                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error calling cpssDxChLatencyMonitoringEnableSet "..returnCodes[result])
                    end

                    if not is_sip_6_10(devNum) then
                        result = myGenWrapper("cpssDxChLatencyMonitoringPortEnableSet",{
                            { "IN", "GT_U8",                "devNum",  devNum               },
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", params["egressPort"] },
                            { "IN", "GT_BOOL",              "enable",  true                 }
                        })
                        if 0 ~= result then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error calling cpssDxChLatencyMonitoringPortEnableSet "..returnCodes[result])
                        end
                    end
                end
            end
        end
    end

    command_data:analyzeCommandExecution()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor config
--------------------------------------------------------------------------------
CLI_addCommand("interface", "latency-monitor config", {
    func   = latencyMonitoringPortConfig,
    help   = "Configure port-pair for latency monitoring",
    params = {
        {type = "named",
            {format="egress-port %portNum",     name="egressPort", help = "Egress port" },
            {format="latency-profile %latProf", name="monProfile", help = "Latency monitoring profile" },
            mandatory = {"monProfile", "egressPort"}
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no latency-monitor config
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no latency-monitor config", {
    func   = function(params)
                params["flagNo"] = true
                latencyMonitoringPortConfig(params)
            end,
    help   = "Restore default configurations for the port pair",
    params = {
        {type = "named",
            {format="egress-port %portNum",     name="egressPort", help = "Egress port" },
            mandatory = {"egressPort"}
        }
    }
})

local function latencyMonitoringProfileModeSet(params)
    local result
    local command_data = Command_Data()
    local devlist
    local i

    if params["all_device"] ~= "all" then
        devlist = {params["all_device"]}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i=1,#devlist do
        if is_sip_6_10(devlist[i]) then
            result = myGenWrapper("cpssDxChLatencyMonitoringProfileModeSet",{
                { "IN", "GT_U8",    "devNum",           devlist[i]              },
                { "IN", "CPSS_DXCH_LATENCY_PROFILE_MODE_ENT",   "profileMode",   params["prfMode"] }
            })
            if 0 ~= result then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringProfileModeSet "..returnCodes[result])
            end
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor profile-mode
--------------------------------------------------------------------------------
CLI_addCommand("config", "latency-monitor profile-mode", {
    func   = latencyMonitoringProfileModeSet,
    help   = "select latency profile mode",
    params = {
        {type = "named",
            "#all_device",
            {format="mode %prfMode", name="prfMode", help = "Latency Profile Mode" },
            mandatory = {"prfMode"}
        }
    }
})

local function latencyMonitoringPortProfileSet(params)
    local result
    local command_data = Command_Data()
    local devlist
    local i

    if params["all_device"] ~= "all" then
        devlist = {params["all_device"]}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i=1,#devlist do
        if is_sip_6_10(devlist[i]) then
            result = myGenWrapper("cpssDxChLatencyMonitoringPortProfileSet",{
                    { "IN", "GT_U8",                "devNum",  devlist[i]           },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", params["egressPort"] },
                    { "IN", "GT_U32",               "portProfile",  params["portProfile"]}
                    })
            if 0 ~= result then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error calling cpssDxChLatencyMonitoringPortProfileSet "..returnCodes[result])
            end
        end
    end
    command_data:analyzeCommandExecution()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor-profile
--------------------------------------------------------------------------------
CLI_addCommand("interface", "latency-monitor-profile", {
    func   = latencyMonitoringPortProfileSet,
    help   = "Configure port profile",
    params = {
        {type = "named",
                "#all_device",
                {format="egress-port %portNum",     name="egressPort", help = "Egress port" },
                {format="latency-profile %latProf", name="portProfile", help = "port profile" },
                mandatory = {"portProfile", "egressPort"}
        }
    }
})


local function latencyMonitoringQueueProfileSet(params)
    local result
    local command_data = Command_Data()
    local tcQueue, i, j
    local devlist

    if params["all_device"] ~= "all" then
        devlist = {params["all_device"]}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i=1,#devlist do
        if is_sip_6_10(devlist[i]) then
            -- Loop over Traffic Class range
            for j=1, #params.tcRange do
                -- Get TC number from the list
                tcQueue = params.tcRange[j]
                result = myGenWrapper("cpssDxChLatencyMonitoringQueueProfileSet",{
                    { "IN", "GT_U8",  "devNum",  devlist[i]   },
                    { "IN", "GT_U8",  "tcQueue", tcQueue      },
                    { "IN", "GT_U32", "queueProfile",  params["queueProfile"]}
                    })
                if 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error calling cpssDxChLatencyMonitoringQueueProfileSet "..returnCodes[result])
                end
            end
        end
    end
    command_data:analyzeCommandExecution()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: latency-monitor queue-profile
--------------------------------------------------------------------------------
CLI_addCommand("config", "latency-monitor queue-profile", {
    func   = latencyMonitoringQueueProfileSet,
    help   = "Configure queue profile",
    params = {
        {type = "named",
                "#all_device",
                {format="tc %tc_ranges", name="tcRange", help="traffic class" },
                {format="latency-profile %latProf", name="queueProfile", help = "queue profile" },
                mandatory = {"queueProfile", "tcRange"}
        }
    }
})

CLI_addHelp("interface", "latency-monitor", "Latency monitoring")
CLI_addHelp("interface", "no latency-monitor", "Latency monitoring")
CLI_addHelp("config", "latency-monitor", "Latency monitoring")
CLI_addHelp("config", "no latency-monitor", "Latency monitoring")


