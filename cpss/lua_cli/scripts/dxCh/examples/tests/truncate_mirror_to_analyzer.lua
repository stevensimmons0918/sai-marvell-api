--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* truncate_mirror_to_analyzer.lua
--*
--* DESCRIPTION:
--*       Check transmitting of mirrored packets from analyzer port
--*       according to truncate field.
--*       - enable analyzer port
--*       - remove analyzer port from default VLAN
--*       - send traffic to destination port
--*       - capture traffic from analyzer port - check analyzer packets are truncated and counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declaration
local packetsFile  = "dxCh/examples/packets/truncate_mirror_to_analyzer.lua"

local configFile   = "dxCh/examples/configurations/truncate_mirror_to_analyzer.txt"
local deconfigFile = "dxCh/examples/configurations/truncate_mirror_to_analyzer_deconfig.txt"


local devNum = devEnv.dev

local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]


SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6")

local function callWithErrorHandling(cpssApi, params)
   local rc, values = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
      setFailState()
      print("error of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, values
end

-- return false if an error occcured and true overwise.
local function testBody(...)
   local isOk = true
   local rc = 1 -- not GT_OK

    for _, traffic in ipairs({...}) do
      -- send the packet and check captured packets
      rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo, traffic.egressInfo)
      if rc~=0 then
         setFailState()
         print("packet transmitting error: ".. rc)
         break
      end
    end
    
   return (rc==0)
end

------------------------------ MAIN PART ---------------------------------------

-- read the packets from the file
local ret, values = pcall(dofile, packetsFile)
if not ret then
   error("\nError while reading the the packets:"..values)
end

local burstCount = 1

local traffic = {
   transmitInfo = {
      devNum = devNum,
      portNum = port1, pktInfo = {fullPacket = values.rx1},
	  burstCount = burstCount},
   egressInfo = {
      {portNum = port2, packetCount = burstCount, pktInfo = {fullPacket = values.tx2} , captureMode = "pcl"},
      {portNum = port3, packetCount = burstCount, pktInfo = {fullPacket = values.tx3} , captureMode = "pcl"}
}}

-- printLog("traffic : " , to_string(traffic))

local isOk

local rc, hwDevNum = device_to_hardware_format_convert(devNum)
if  hwDevNum == 0 then
    -- the logic of 'port moniter' command will not skip index 0,1 since the hwDevNum == 0
    -- so force the logic of 'port moniter' to select index 2
    test_force_specific_analyzer_inerface_index(true , 2)
end

isOk = luaTgfSimpleTest(configFile, deconfigFile, testBody, traffic)

if  hwDevNum == 0 then
    -- restore the 'no force'
    test_force_specific_analyzer_inerface_index(false)
end

--flush the FDB
executeLocalConfig(luaTgfBuildConfigFileName("flush_fdb_only_dynamic")) 

return isOk
