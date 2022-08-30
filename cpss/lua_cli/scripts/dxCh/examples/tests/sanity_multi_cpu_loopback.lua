--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* sanity_multi_cpu_loopback.lua
--*
--* DESCRIPTION:
--*       check 'from cpu' from 'third' 'CPU SDMA port' (not 63) with target port 
--*       which port 63 , but the CPU code redirect it to 'second' 'CPU SDMA port'
--*       the Test bypass the need for MAC ports ! (in Falcon this is without Raven)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 
local portCPU = devEnv.portCPU -- cpu port

-- skip test for unsupported devices
SUPPORTED_FEATURE_DECLARE(devNum, "MULTI_CPU_SDMA_PORTS")

-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macDa   = "000000000058"
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end
    
    return macDa .. macSa ..  partAfterMac .. packetPayload
end

--##################################
--##################################
local ingressPacket = buildPacketUc(nil)
local transmitInfo = {portNum = "CPU" , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable = {
    -- expected to get to CPU without changes
    {portNum = "CPU", pktInfo = {fullPacket = ingressPacket}}
}

local configFile   = "trap_all_to_generic_cpu_port"

-- run pre-test config
local function preTestConfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFile)) 
end
-- run post-test config
local function postTestConfig()
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName(configFile,true)) 
end 

local numOfTimes = 1
-- function to implement the test
local function doTest(tx_cpuIndex--[[3]] , rx_cpuIndex--[[2]])
    local tx_cpu_port = physicalCpuSdmaPortGet(tx_cpuIndex)
    local rx_cpu_port = physicalCpuSdmaPortGet(rx_cpuIndex)
    local tx_cpu_queue = ((tx_cpuIndex-1) * 8) + 1

    printLog("start iteration : ",numOfTimes)
    -- must be called before 'load the config file'
    genericCpuIndexSet(rx_cpuIndex)
    -- run pre-test config
    preTestConfig()
    -- run the test

    printLog ("\n *************************************** \n" ..
              "    CPU SDMA physical port is " .. tx_cpu_port ..
              "\n *************************************** \n")

          --set config
    local sectionName = "check 'from cpu' from SDMA physical port " .. tx_cpu_port .. "(sdma queue["..tx_cpu_queue.."]) " .. 
        "with target port is 'CPU port' (63). CPU code will redirect it to SDMA physical port " .. rx_cpu_port
        
    local rc = luaTgfTrafficGeneratorDefaultTxSdmaQueueSet(tx_cpu_queue);
    if rc ~= 0 then
        printLog ("Test failed : luaTgfTrafficGeneratorDefaultTxSdmaQueueSet: " .. returnCodes[rc] .. "\n")
        setFailState()
    end 

   
    printLog ("START : " .. sectionName .. "\n")
    -- check that packet egress the needed port(s)
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState() 
    end  

    printLog("ended iteration : ",numOfTimes)
    numOfTimes = numOfTimes + 1
end

local numOfCpuSdmaPorts =  numOfCpuSdmaPortsGet(devNum)

if numOfCpuSdmaPorts >= 3 then
    doTest(3--[['src cpu port']],2--[['trg cpu port']])
    doTest(2--[['src cpu port']],3--[['trg cpu port']])
else -- only 2 CPU SDMA ports 
    doTest(2--[['src cpu port']],1--[['trg cpu port']])
    doTest(1--[['src cpu port']],2--[['trg cpu port']])
end

if numOfCpuSdmaPorts >= 6 then
    doTest(4--[['src cpu port']],6--[['trg cpu port']])
    doTest(5--[['src cpu port']],4--[['trg cpu port']])
    doTest(6--[['src cpu port']],5--[['trg cpu port']])
end

if numOfCpuSdmaPorts >= 8 then
    doTest(1--[['src cpu port']],7--[['trg cpu port']] )
    doTest(7--[['src cpu port']],8--[['trg cpu port']])
    doTest(8--[['src cpu port']],2--[['trg cpu port']])
end

-- run post-test config
postTestConfig()

-- restore TX SDMA Queue to be 7
rc = luaTgfTrafficGeneratorDefaultTxSdmaQueueSet(7);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))

printLog ("\n *************************************** \n" ..
          " (restore) CPU SDMA physical port is " .. physicalCpuSdmaPortGet(1) ..
          "\n *************************************** \n")


