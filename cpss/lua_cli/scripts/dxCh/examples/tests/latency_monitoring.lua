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
--*       Latency monitoring example
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum = devEnv.dev
SUPPORTED_FEATURE_DECLARE(devNum, "SIP6_LATENCY_MONITOR")

lmu_env = {}
local PACKET_COUNT = 10
local status, result

local triggers_configure =
{
    ["IPCL"] = "dxCh/examples/configurations/latency_monitoring_trig_ipcl.txt",
    ["EPCL"] = "dxCh/examples/configurations/latency_monitoring_trig_epcl.txt",
    ["PORT"] = "dxCh/examples/configurations/latency_monitoring_trig_port.txt",
}

local fdb_configure = "dxCh/examples/configurations/latency_monitoring_fdb.txt"

local triggers_deconfigure = "dxCh/examples/configurations/latency_monitoring_trig_deconfigure.txt"

local mac_list = {[33]="00:00:00:00:00:11", [34]="00:00:00:00:00:33", [35]="00:00:00:00:00:44"}

local lmuEgressInfoTable = {}

if is_sip_6_10(devNum) then
    -- SIP 6_10 use Known UC traffic
    lmuEgressInfoTable = {
        {portNum = devEnv.port[2], packetCount = 1 },
        {portNum = devEnv.port[3], packetCount = 0 },
        {portNum = devEnv.port[4], packetCount = 0 }
    }
else
    -- there is flooding
    lmuEgressInfoTable = {
        {portNum = devEnv.port[2], packetCount = 1 },
        {portNum = devEnv.port[3], packetCount = 1 },
        {portNum = devEnv.port[4], packetCount = 1 }
    }
end

local function sendPacket(count, ...)
    local rc
    local mac

    if #arg == 1 then
        mac = string.gsub(arg[1], "%W", "")
    else
        mac = "000000000011"
    end
    local testPacket = genPktL2({
        dstMAC      =  mac,
        srcMAC      =  "000000000022",
        etherType   =  "5555",
        payload     =  "11111111111111111111111111111111"..
                       "11111111111111111111111111111111"..
                       "11111111111111111111111111111111"..
                       "11111111111111111111111111111111"..
                       "11111111111111111111111111111111"
    })
    local transmitInfo = {
        devNum = devNum,
        portNum = devEnv.port[1],
        pktInfo = {fullPacket = testPacket}
    }

    if count == nil then
        count = 1
    end

    for i=1,count do
        rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,lmuEgressInfoTable)
        if rc ~= 0 then
            printLog ("Test failed \n")
            setFailState()
        end
    end
end

local function switchLmTriggering(triggerType, profile, configure, ...)
    local clientInfo
    lmu_env["latencyProfile"] = profile
    if triggerType == "EPCL" then
        lmu_env["ruleId"] = 12
        clientInfo = sharedResourceNamedPclRulesMemoInfoGet("epcl")
        if(clientInfo and clientInfo[1]) then
            lmu_env["ruleId"] = lmu_env["ruleId"] + clientInfo[1].base
        end
    elseif triggerType == "IPCL" then
        lmu_env["ruleId"] = 6
        clientInfo = sharedResourceNamedPclRulesMemoInfoGet("ipcl0")
        if(clientInfo and clientInfo[1]) then
            lmu_env["ruleId"] = lmu_env["ruleId"] + clientInfo[1].base
        end
    elseif triggerType == "PORT" then
        if #arg == 1 then
            lmu_env["port2"] = arg[1]
        else
            lmu_env["port2"] = devEnv.port[2]
        end
    end

    if (triggerType == "EPCL") or (triggerType == "IPCL") then
        if #arg > 0 then
            lmu_env["port2"] = arg[1]
            if #arg == 2 then
                lmu_env["mac"] = arg[2]
            else
                lmu_env["mac"] = "00:00:00:00:00:11"
            end
        else
            lmu_env["port2"] = devEnv.port[2]
        end
    end

    if configure == true then
        executeLocalConfig(triggers_configure[triggerType])
        if is_sip_6_10(devNum) then
            if triggerType == "PORT" then
                -- flooding is used in this case
                lmuEgressInfoTable = {
                    {portNum = devEnv.port[2], packetCount = 1 },
                    {portNum = devEnv.port[3], packetCount = 1 },
                    {portNum = devEnv.port[4], packetCount = 1 }
                }
            else
                -- SIP 6_10 use Known UC traffic. Add FDB entry.
                executeLocalConfig(fdb_configure)

                -- correct expected results
                if lmu_env["port2"] == devEnv.port[2] then
                    lmuEgressInfoTable = {
                        {portNum = devEnv.port[2], packetCount = 1 },
                        {portNum = devEnv.port[3], packetCount = 0 },
                        {portNum = devEnv.port[4], packetCount = 0 }
                    }
                elseif lmu_env["port2"] == devEnv.port[3] then
                    lmuEgressInfoTable = {
                        {portNum = devEnv.port[2], packetCount = 0 },
                        {portNum = devEnv.port[3], packetCount = 1 },
                        {portNum = devEnv.port[4], packetCount = 0 }
                    }
                elseif lmu_env["port2"] == devEnv.port[4] then
                    lmuEgressInfoTable = {
                        {portNum = devEnv.port[2], packetCount = 0 },
                        {portNum = devEnv.port[3], packetCount = 0 },
                        {portNum = devEnv.port[4], packetCount = 1 }
                    }
                end
            end
        end
    else
        executeLocalConfig(triggers_deconfigure)
    end
end

local function configureProfile(profile, parameter, config)
    lmu_env["latencyProfile"]  = profile
    lmu_env["samplThresh"]     = 0x3FFFFFFF
    lmu_env["samplingProfile"] = 0
    lmu_env["notifThresh"]     = 0x3FFFFFFF
    lmu_env["rangeMin"]        = 0
    lmu_env["rangeMax"]        = 0x3FFFFFFF
    lmu_env["samplMode"]       = "deterministic"

    if parameter == "SAMPLING" then
        if config == true then
            if wrlCpssIsAsicSimulation() then
                lmu_env["samplThresh"] = 0x1FFFFFFF
            else -- HW need bigger threshold for even distribution
                lmu_env["samplThresh"] = 0x7FFFFFFF
            end
            lmu_env["samplMode"]       = "random"
            lmu_env["samplingProfile"] = 1
        end
    elseif parameter == "NOTIFICATION" then
        if config == true then
            lmu_env["notifThresh"] = PACKET_COUNT
        end
    elseif parameter == "LATRANGE" then
        if config == true then
            lmu_env["rangeMin"] = 0
            lmu_env["rangeMax"] = 0
        end
    end
    executeLocalConfig("dxCh/examples/configurations/latency_monitoring_type_tests.txt")
end

local function readStats(profileList)
    local lmuBmp
    local i
    local j
    local result, values

    results = {}
    for j,profile in ipairs(profileList) do
        lmuBmp = 0
        for i=0,31 do
            result, values = myGenWrapper("cpssDxChLatencyMonitoringStatGet",{
                { "IN",  "GT_U8",                                   "devNum",           devEnv.dev              },
                { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           bit_shl(1, i)           },
                { "IN",  "GT_U32",                                  "latencyProfile",   profile                 },
                { "OUT", "CPSS_DXCH_LATENCY_MONITORING_STAT_STC",   "statisticsPtr"}
            })
            if 0 == result then
                if values["statisticsPtr"]["minLatency"] ~= 0x3FFFFFFF then
                    lmuBmp = bit_or(lmuBmp, bit_shl(1, i))
                end
            end
        end
        if lmuBmp == 0 then
            results[profile] = {}
            results[profile]["packetsInRange"] = 0
            results[profile]["packetsOutOfRange"] = 0
            results[profile]["minLatency"] = 0
            results[profile]["maxLatency"] = 0
            results[profile]["avgLatency"] = 0
        else
            result, values = myGenWrapper("cpssDxChLatencyMonitoringStatGet",{
                { "IN",  "GT_U8",                                   "devNum",           devEnv.dev              },
                { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           lmuBmp                  },
                { "IN",  "GT_U32",                                  "latencyProfile",   profile                 },
                { "OUT", "CPSS_DXCH_LATENCY_MONITORING_STAT_STC",   "statisticsPtr"}
            })
            if 0 ~= result then
                printLog("cpssDxChLatencyMonitoringStatGet call failed")
                printLog(string.format("Params: %d 0x%08X %d", devEnv.dev, lmuBmp, profile))
                setFailState()
            end
            results[profile] = values["statisticsPtr"]
            results[profile]["packetsInRange"] = tonumber(to_string(results[profile]["packetsInRange"]))
            results[profile]["packetsOutOfRange"] = tonumber(to_string(results[profile]["packetsOutOfRange"]))
        end
    end
    return results
end

local oldStats
local function validateStats(latencyProfile, paramType)
    local diff, newStats
    newStats = readStats({latencyProfile})
    newStats = newStats[latencyProfile]
    diff = {}
    diff["minLatency"] = newStats["minLatency"] - oldStats[latencyProfile]["minLatency"]
    diff["maxLatency"] = newStats["maxLatency"] - oldStats[latencyProfile]["maxLatency"]
    diff["avgLatency"] = newStats["avgLatency"] - oldStats[latencyProfile]["avgLatency"]
    diff["packetsInRange"] = newStats["packetsInRange"] - oldStats[latencyProfile]["packetsInRange"]
    diff["packetsOutOfRange"] = newStats["packetsOutOfRange"] - oldStats[latencyProfile]["packetsOutOfRange"]
    oldStats[latencyProfile] = newStats

    if paramType == "SAMPLING" then
        if (diff["packetsInRange"] < PACKET_COUNT * 0.1) or (diff["packetsInRange"] > PACKET_COUNT * 0.9) then
            printLog(paramType.." test failed\n")
            printLog("Actual packetsInRange "..diff["packetsInRange"].." expected range "..(PACKET_COUNT * 0.1).."-"..(PACKET_COUNT * 0.9))
            setFailState()
        end
        if diff["packetsOutOfRange"] ~= 0 then
            printLog(paramType.." test failed\n")
            printLog(" packetsOutOfRange "..diff["packetsOutOfRange"])
            setFailState()
        end
    elseif paramType == "LATRANGE" then
        if diff["packetsOutOfRange"] ~= PACKET_COUNT then
            printLog(paramType.." test failed\n")
            setFailState()
        end
        if diff["packetsInRange"] ~= 0 then
            printLog(paramType.." test failed\n")
            setFailState()
        end
    elseif paramType == "NOTIFICATION" then
        printLog("NOTIFICATION tests not implemented, failed\n")
        setFailState()
    else
        printLog("Undefined test, failed\n")
        setFailState()
    end
end

local function single_test(trigType, latencyProfile, paramType, port)
    switchLmTriggering(trigType, latencyProfile, true, port, mac_list[latencyProfile])
    configureProfile(latencyProfile, paramType, true)
    sendPacket(PACKET_COUNT, mac_list[latencyProfile])
    configureProfile(latencyProfile, paramType, false)
    switchLmTriggering(trigType, latencyProfile, false, port, mac_list[latencyProfile])
    validateStats(latencyProfile, paramType)

end

local isShortVersion = false -- set to 'true' for debug !

local function clearStats(profileList)
    local j
    local result, values
    results = {}

    for j,profile in ipairs(profileList) do

        result, values = myGenWrapper("cpssDxChLatencyMonitoringStatReset",{
            { "IN",  "GT_U8",                                   "devNum",           devEnv.dev              },
            { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           0Xffffffff              },
            { "IN",  "GT_U32",                                  "latencyProfile",   profile                 }
        })
        if 0 ~= result then
            printLog("cpssDxChLatencyMonitoringStatReset call failed")
            printLog(string.format("Params: %d 0x%08X %d", devEnv.dev, 0Xffffffff, profile))
            setFailState()
        end

        result, values = myGenWrapper("cpssDxChLatencyMonitoringStatGet",{
            { "IN",  "GT_U8",                                   "devNum",           devEnv.dev              },
            { "IN",  "GT_LATENCY_MONITORING_UNIT_BMP",          "lmuBmp",           0Xffffffff              },
            { "IN",  "GT_U32",                                  "latencyProfile",   profile                 },
            { "OUT", "CPSS_DXCH_LATENCY_MONITORING_STAT_STC",   "statisticsPtr"}
        })
        if 0 ~= result then
            printLog("cpssDxChLatencyMonitoringStatGet call failed")
            printLog(string.format("Params: %d 0x%08X %d", devEnv.dev, 0Xffffffff, profile))
            setFailState()
        end

        if ((values["statisticsPtr"]["packetsInRange"]["l"][0] ~= 0) or
            (values["statisticsPtr"]["packetsInRange"]["l"][1] ~= 0) or
            (values["statisticsPtr"]["packetsOutOfRange"]["l"][0] ~= 0) or
            (values["statisticsPtr"]["packetsOutOfRange"]["l"][1] ~= 0) or
            (values["statisticsPtr"]["minLatency"] ~= 0x3FFFFFFF) or
            (values["statisticsPtr"]["maxLatency"] ~= 0) or
            (values["statisticsPtr"]["avgLatency"] ~= 0)) then
            printLog("cpssDxChLatencyMonitoringStatReset call failed")
            setFailState()
        end
    end
    printLog("cpssDxChLatencyMonitoringStatReset sucssesfuly finisehd")
end

local function main_test()
    local trigType
    local paramType
    local latencyProfile
    local i,k,j
    local profileList = {33, 34, 35}
    oldStats = readStats(profileList)

    for i,trigType in ipairs({"IPCL", "EPCL", "PORT"}) do
        for j,latencyProfile in ipairs(profileList) do
            for k,paramType in ipairs({"SAMPLING", "LATRANGE"}) do
                single_test(trigType, latencyProfile, paramType, devEnv.port[1+j])
                if isShortVersion then break end
            end
            if isShortVersion then break end
        end
        if isShortVersion then break end
    end
    local latencyRange = ""
    for j,latencyProfile in ipairs(profileList) do
        latencyRange = latencyRange .. tostring(latencyProfile) .. ","
    end

    printLog("\n\tLatency Monitor Statistic after the Test")
    latencyRange = string.sub(latencyRange, 1, -2)
    executeStringCliCommands("do show latency-monitor statistics device ${dev} latency-profile "..latencyRange)
    clearStats(profileList)
    printLog("\n\tLatency Monitor Statistic after Reseting the stats table")
    executeStringCliCommands("do show latency-monitor statistics device ${dev} latency-profile "..latencyRange)
end


main_test()

lmu_env = nil
