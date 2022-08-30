--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policer_flow.lua
--*
--* DESCRIPTION:
--*       The test for policer flow feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]

local ret, status

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

-- function return burst size of conformance level : called from test:
-- policy-id 1 device ${dev} police 3 burst ${@@policerBurstSize("red")} exceed-action drop

function policerBurstSize(coformanceLevel)
    local burstSize
    if is_sip_6_10(devNum) then
        -- MRU should be
        burstSize = {["red"] = 0,["yellow"]=0,["green"]=3000}
    else
        burstSize = {["red"] = 30--[[smaller than MRU 10K]],["yellow"]=30--[[smaller than MRU 10K]],["green"]=13000--[[more than MRU 10K]]}
    end

    printLog("use value [" .. tostring(burstSize[coformanceLevel]) .. "] for conformance level [" .. coformanceLevel .."]")

    return burstSize[coformanceLevel]
end
--generate mac pdu test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/vlan10_20.lua")

--------------------------------------------------------------------------------
--load configuration for 1st part of test
executeLocalConfig("dxCh/examples/configurations/policer_flow.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)


printLog("Testing counters in port"..port1..", port"..port2..", port"..port3)
printLog("Sending traffic from port"..port1)

ret, status = sendPacket({[devNum]={port1}}, payloads["vlanNo"])
delay(100)
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                        {[devNum] = { [port1] = {Rx=1, Tx = 1},
                                 [port2] = {Rx=0, Tx = 0},
                                 [port3] = {Rx=0, Tx = 0} }})


printLog("Sending traffic from port"..port2)
ret, status = sendPacket({[devNum]={port2}}, payloads["vlanNo"])
delay(100)
printLog("Check counters ..")

checkExpectedCounters(debug.getinfo(1).currentline,
                        {[devNum] = { [port1] = {Rx=0, Tx = 0},
                                 [port2] = {Rx=0, Tx = 1},
                                 [port3] = {Rx=0, Tx = 0}        }})


printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/policer_flow_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")


