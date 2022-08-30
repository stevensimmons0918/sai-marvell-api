--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* multi_cpu.lua
--*
--* DESCRIPTION:
--*       The test checks packets to/from CPU port 80,81,82 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--
-- ACL matched and not matched packets
--
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

-- skip test for unsupported devices
SUPPORTED_FEATURE_DECLARE(devNum, "MULTI_CPU_SDMA_PORTS")

local aclNotMatchContent =
    "0000BBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local aclMatchContent =
    "00AABBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local test_scenario =
{
    no_matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclNotMatchContent}
        },
        egressInfoTable =
        {
            {portNum = port1, pktInfo = {fullPacket = aclNotMatchContent}, packetCount = 1},
            {portNum = port2, packetCount = 1},
            {portNum = port4, packetCount = 1},
        }
    },
    matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclMatchContent}
        },
        egressInfoTable =
        {
            {portNum = "CPU", pktInfo = {fullPacket = aclMatchContent, cpuCodeNumber = 195}},
            {portNum = port1, packetCount = 0},
            {portNum = port2, packetCount = 0},
            {portNum = port4, packetCount = 0},
        }
    }
}

-- local declaration
local configFile
local deconfigFile = "dxCh/examples/configurations/cpu_port_deconfig.txt"

local function trafficCheck(trafficScenario)
    local key, scenario;
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        if rc ~= 0 then
            printLog ("Test failed " .. returnCodes[rc] .. "\n")
            setFailState()
        end 
    end
end

------------------------------ MAIN PART ---------------------------------------

local function testQueue(queue)
    local rc = luaTgfTrafficGeneratorDefaultTxSdmaQueueSet(queue);
    if rc ~= 0 then
        printLog ("Test failed : luaTgfTrafficGeneratorDefaultTxSdmaQueueSet: " .. returnCodes[rc] .. "\n")
        setFailState()
    end 

    local cpuIndex = 1 + math.floor(queue/8)
    
    printLog ("\n ***************************** \n" ..
              " CPU SDMA physical port is " .. physicalCpuSdmaPortGet(cpuIndex) ..
              "\n ***************************** \n")

    luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, test_scenario)
end

-- the packet to the CPU may come late .. because the interrupt comes from other MG ... may take time in WM
-- issues with it in VC10 in WM of Falcon 6.4
executeStringCliCommands("do shell-execute tgfTrafficGeneratorRetryRxToCpuAndWaitSet 1")
executeStringCliCommands("do shell-execute tgfTrafficGeneratorCaptureLoopTimeSet 500")

local numOfCpuSdmaPorts =  numOfCpuSdmaPortsGet(devNum)

-- test the second cpu port
configFile   = "dxCh/examples/configurations/second_cpu_port.txt"
testQueue(9) --[[queue 9]]
if numOfCpuSdmaPorts >= 3 then -- test the third cpu port
    configFile   = "dxCh/examples/configurations/third_cpu_port.txt"
    testQueue(17) --[[queue 17]]
end
if numOfCpuSdmaPorts >= 4 then -- test the forth cpu port
    configFile   = "dxCh/examples/configurations/forth_cpu_port.txt"
    testQueue(25) --[[queue 25]]
end

local genericCpuIndex


if numOfCpuSdmaPorts >= 6 then
    -- test the six cpu port
    configFile   = "dxCh/examples/configurations/generic_cpu_port.txt"
    genericCpuIndex = 6
    genericCpuIndexSet(genericCpuIndex)

    testQueue((genericCpuIndex-1)*8 + 5) --[[queue 45]]
end

if numOfCpuSdmaPorts >= 8 then
    -- test the six cpu port
    configFile   = "dxCh/examples/configurations/generic_cpu_port.txt"
    genericCpuIndex = 8
    genericCpuIndexSet(genericCpuIndex)
    -- in the Falcon the 'port delete' WA uses queues 63,62 as 'packet generator'
    testQueue((genericCpuIndex-1)*8 + 5) --[[queue 61]]
end


-- restore TX SDMA Queue to be 7
rc = luaTgfTrafficGeneratorDefaultTxSdmaQueueSet(7);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))

printLog ("\n *************************************** \n" ..
          " (restore) CPU SDMA physical port is " .. physicalCpuSdmaPortGet(1) ..
          "\n *************************************** \n")

-- restore to default value
executeStringCliCommands("do shell-execute tgfTrafficGeneratorRetryRxToCpuAndWaitSet 0")
executeStringCliCommands("do shell-execute tgfTrafficGeneratorCaptureLoopTimeSet 0")
            