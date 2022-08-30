--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* latency_profile_mode.lua
--*
--* DESCRIPTION:
--*       Latency profile mode example
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum = devEnv.dev
SUPPORTED_FEATURE_DECLARE(devNum, "SIP6_10_LATENCY_PROFILE_MODE")

lmu_env = {}
local PACKET_COUNT = 10
local status, result

local port_trig = "dxCh/examples/configurations/latency_monitoring_trig_port.txt"
local fdb_configure = "dxCh/examples/configurations/latency_monitoring_fdb.txt"
local mode_configure = "dxCh/examples/configurations/latency_profile_mode_config.txt"
local queueProfile_configure = "dxCh/examples/configurations/latency_queue_profile_config.txt"
local portProfile_configure = "dxCh/examples/configurations/latency_port_profile_config.txt"
local mode_deconfigure = "dxCh/examples/configurations/latency_profile_mode_deconfig.txt"

local lmuEgressInfoTable = {
        {portNum = devEnv.port[2], packetCount = 1},
        {portNum = devEnv.port[3], packetCount = 1}
}

local function sendPacket(count, ...)

    local rc
    local testPacket = genPktL2({
        dstMAC      =  "000000000033",
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

local function switchLmuTriggering(latencyProfile1, latencyProfile2, configure, ...)

    local result

    if #arg > 0 then
        lmu_env["port2"] = arg[1]
    else
        lmu_env["port2"] = devEnv.port[2]
    end
    lmu_env["mac"] = "00:00:00:00:00:33"

    if configure == true then

        lmu_env["latencyProfile"] = bit_and(latencyProfile1 + latencyProfile2, 0x1FF)
        -- port based LM triggering
        executeLocalConfig(port_trig)

        -- use Known UC traffic. Add FDB entry.
        executeLocalConfig(fdb_configure)

        -- correct expected results
        if lmu_env["port2"] == devEnv.port[2] then
            lmuEgressInfoTable = {
                {portNum = devEnv.port[2], packetCount = 1 },
                {portNum = devEnv.port[3], packetCount = 0 }
           }
        elseif lmu_env["port2"] == devEnv.port[3] then
            lmuEgressInfoTable = {
                {portNum = devEnv.port[2], packetCount = 0 },
                {portNum = devEnv.port[3], packetCount = 1 }
            }
        end
    else
        -- deconfiguration
        executeLocalConfig(mode_deconfigure)
    end
end

local function configureProfile(mode, latencyProfile1, latencyProfile2, config)

    -- mode configuration
    lmu_env["mode"] = mode
    executeLocalConfig(mode_configure)

    lmu_env["latencyProfile"]  = bit_and(latencyProfile1 + latencyProfile2, 0x1FF)
    lmu_env["samplThresh"]     = 0x3FFFFFFF
    lmu_env["samplingProfile"] = 0
    lmu_env["notifThresh"]     = 0x3FFFFFFF
    lmu_env["rangeMin"]        = 0
    lmu_env["rangeMax"]        = 0x3FFFFFFF
    lmu_env["samplMode"]       = "deterministic"

    if mode == "queue" then
        if config == true then
            -- Configure portProfile and queueProfile
            lmu_env["latencyProfile"] = latencyProfile1
            executeLocalConfig(queueProfile_configure)
            lmu_env["latencyProfile"] = latencyProfile2
            executeLocalConfig(portProfile_configure)
        end
    else if mode == "port" then
        if config == true then
            lmu_env["rangeMax"] = 0
        end
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
local function validateStats(mode, latencyProfile1, latencyProfile2)

    local diff, newStats
    local latencyProfile = bit_and(latencyProfile1 + latencyProfile2, 0x1FF)
    newStats = readStats({latencyProfile})
    newStats = newStats[latencyProfile]
    diff = {}
    diff["minLatency"] = newStats["minLatency"] - oldStats[latencyProfile]["minLatency"]
    diff["maxLatency"] = newStats["maxLatency"] - oldStats[latencyProfile]["maxLatency"]
    diff["avgLatency"] = newStats["avgLatency"] - oldStats[latencyProfile]["avgLatency"]
    diff["packetsInRange"] = newStats["packetsInRange"] - oldStats[latencyProfile]["packetsInRange"]
    diff["packetsOutOfRange"] = newStats["packetsOutOfRange"] - oldStats[latencyProfile]["packetsOutOfRange"]
    oldStats[latencyProfile] = newStats

    if mode == "queue" then
        if (diff["packetsInRange"] ~= PACKET_COUNT) then
            printLog(mode.." test failed\n")
            printLog("Actual packetsInRange "..diff["packetsInRange"])
            setFailState()
        end
        if diff["packetsOutOfRange"] ~= 0 then
            printLog(mode.." test failed\n")
            printLog(" packetsOutOfRange "..diff["packetsOutOfRange"])
            setFailState()
        end
    elseif mode == "port" then
        if diff["packetsOutOfRange"] ~= PACKET_COUNT then
            printLog(mode.." test failed\n")
            setFailState()
        end
        if diff["packetsInRange"] ~= 0 then
            printLog(mode.." test failed\n")
            setFailState()
        end
    else
        printLog("Undefined test, failed\n")
        setFailState()
    end
end

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

local function single_test(mode, latencyProfile1, latencyProfile2, port)
    switchLmuTriggering(latencyProfile1, latencyProfile2, true, port)
    configureProfile(mode, latencyProfile1, latencyProfile2, true)
    sendPacket(PACKET_COUNT)
    configureProfile(mode, latencyProfile1, latencyProfile2, false)
    switchLmuTriggering(latencyProfile1, latencyProfile2, false, port)
    validateStats(mode, latencyProfile1, latencyProfile2)
end

local function main_test()
    local i, mode
    local profileList = {67, 23} -- 67:Queue Mode Profile, 23: Port Mode Profile
    oldStats = readStats(profileList)

    for i,mode in ipairs({"port", "queue"}) do
        if mode == "port" then
            single_test(mode, 11, 12, devEnv.port[1+i]) --11: srcProfile, 12:trgProfile
        else
            single_test(mode, 33, 34, devEnv.port[1+i]) --33: portProfile, 34:queueProfile
        end
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
