--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* storm_control_rate_limit_mode.lua
--*
--* DESCRIPTION:
--*       Test storm control feature when rate limit mode is set to packet based:
--*        * Use storm-control command to set pps rate limit value and to enable
--*          rate limit to broadcast packets type on specific ingress port.
--*        * Use storm-control rate-limit-mode command to set it to packet based
--*          mode per device.
--*        * Generate ingress wirespeed traffic by command of 'traffic-generator
--*          emulation'
--*
--*       purpose of example:
--*       show rate limit feature works also when rate limit mode is set to packet
--*       based counter
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--ch1,2,3 not support it
SUPPORTED_FEATURE_DECLARE(devNum, "BRIDGE_RATE_LIMIT")

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

-- this test is relevant to ALL tested devices
--##################################
--##################################

-- configure shaper to get 100M rate for AC5/3 to get good results.
local enable_shaper_AC3_5=[[
end
configure
interface ethernet ]]..tostring(devNum)..[[/24
qos traffic shape commited-rate 100000 commited-burst 4096
end
examples
]]
local disable_shaper_AC3_5=[[
end
configure
interface ethernet ]]..tostring(devNum)..[[/24
no qos traffic shape
end
examples
]]


local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

local packet_size = 64
local rate_limit_in_pps = 100000

-- check maximal rate of OUT port 1 to define 100M/1G ports
local max_rate_in_Kbps    = portSpeedKbpsGet(devNum,port1)
if max_rate_in_Kbps < 10000000 then
-- reduce rate to be fine with slow OUT ports
if is_sip_5(devNum) then
rate_limit_in_pps = 30000
else
-- AC3/5 may use either 64k or 128k because of 100M out ports
rate_limit_in_pps = 64000
end
end

local rate_limit_in_kbps = convertPpsToKbps(rate_limit_in_pps,packet_size)
-- Set rate limit of pps as global and string so it can be used while running
-- the command from configuration text file
global_test_data_env.rateLimitInPps = to_string(rate_limit_in_pps)

local rate_limit_diff_in_percent = 15
-- default diff is 2%
local src_port_rate_diff_in_percent = 2

if is_sip_5_20(devNum) then
    -- test use 100G ports. Ingress Storm Control reduces rate on source port.
    src_port_rate_diff_in_percent = 50
end

local rates

if is_sip_5(devNum) then
    rates = {
        rate_limit_in_kbps,
        rate_limit_in_kbps,
        rate_limit_in_kbps,
        committedRateGet(4 , 100 , packet_size, true) -- the wirespeed generating port
    }
else
    rates = {
        rate_limit_in_kbps,
        rate_limit_in_kbps,
        rate_limit_in_kbps,
        100000 -- the wirespeed generating port in AC3/5 use shaper on 100000 kbps
    }
    -- shaper is not precise because of granularity
    src_port_rate_diff_in_percent = 40
end

--##################################
--##################################

local transmitInfo = nil -- packet already doing WS inside the device from the 'cofig' file !!!
local egressInfoTable = {
    -- expected the mac-da to be 'unknown' --> flood the vlan
    {portNum = port1 , rate = rates[1], rateDiffInPercent = rate_limit_diff_in_percent},
    {portNum = port2 , rate = rates[2], rateDiffInPercent = rate_limit_diff_in_percent},
    {portNum = port3 , rate = rates[3], rateDiffInPercent = rate_limit_diff_in_percent},
    {portNum = port4 , rate = rates[4], rateDiffInPercent = src_port_rate_diff_in_percent}
}

-- function to implement the test
local function doTest()
    --set config
    local sectionName = "storm control + Wirespeed from the emulated packet generator"
    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState()
        testAddErrorString(error_string)
    end

    -- print results summary
    testPrintResultSummary(sectionName)
end

local configFileName = "storm_control_rate_limit_mode"

-- run pre-test config
local function preTestConfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName))
end
-- run post-test config
local function postTestConfig()
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName,true))
end

-- run pre-test config
if not is_sip_5(devNum) then
    executeStringCliCommands(enable_shaper_AC3_5)
end
preTestConfig()
-- run the test
doTest()
if not is_sip_5(devNum) then
    executeStringCliCommands(disable_shaper_AC3_5)
end
-- run post-test config
postTestConfig()
