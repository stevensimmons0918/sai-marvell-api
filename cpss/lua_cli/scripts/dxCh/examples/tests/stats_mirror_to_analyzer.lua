--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* stats_mirror_to_analyzer.lua
--*
--* DESCRIPTION:
--*       Check transmitting of mirrored packets from analyzer port
--*       according to statistical mirroring.
--*       - enable analyzer ports
--*       - remove analyzers ports from default VLAN
--*       - send traffic to destination ports
--*       - capture traffic from analyzer ports - chack analyzer packets and counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declaration
local packetsFile  = "dxCh/examples/packets/stats_mirror_to_analyzer.lua"

local configFile   = "dxCh/examples/configurations/stats_mirror_to_analyzer.txt"
local deconfigFile = "dxCh/examples/configurations/stats_mirror_to_analyzer_deconfig.txt"


local devNum = devEnv.dev

local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]


SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6")

local function callWithErrorHandling(cpssApi, params)
   local rc, values = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
      setFailState()
      print("error of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, values
end

local currentRatio = 10

-- enables(isEnable = true) or disables(isEnable = false) rx statistical mirroring on analyzer port.
local function mirrorRxAnalyzerStatisticalMirroring(isEnable)
   local index = 2;
   local currentRatio2 = currentRatio/2;
   if not isEnable then
      currentRatio = 1 -- all packets are mirrored
      currentRatio2 = 1
   end

   -- setup rx analyzer port[3] statistical mirroring.
   callWithErrorHandling(
      "cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet",
      {
         {"IN", "GT_U8",       "devNum",          devNum},
         {"IN", "GT_U32",      "index",           index},
         {"IN", "GT_BOOL",     "enable",          isEnable},
         {"IN", "GT_U32",      "ratio",           currentRatio}
      }
   )

   -- setup rx analyzer port[4] statistical mirroring.
   callWithErrorHandling(
      "cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet",
      {
         {"IN", "GT_U8",       "devNum",          devNum},
         {"IN", "GT_U32",      "index",           index+1},
         {"IN", "GT_BOOL",     "enable",          isEnable},
         {"IN", "GT_U32",      "ratio",           currentRatio2}
      }
   )

end

-- return false if an error occcured and true overwise.
local function testBody(trafficScenario)
   local isOk = true
   local rc = 1 -- not GT_OK
   local key, traffic;

    mirrorRxAnalyzerStatisticalMirroring(true)

    print("Clean before test")
    -- send the packet and check captured packets
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(trafficScenario.pre_test_clean.transmitInfo, trafficScenario.pre_test_clean.egressInfo)
    if rc~=0 then
       setFailState()
       print("packet transmitting error: ".. rc)
    end

    print("Perform test")
    -- send the packet and check captured packets
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(trafficScenario.test.transmitInfo, trafficScenario.test.egressInfo)
    if rc~=0 then
       setFailState()
       print("packet transmitting error: ".. rc)
    end

    mirrorRxAnalyzerStatisticalMirroring(false)

    print("Clean after test")
    -- new value of statistical mirroring became active by first packet. Send one packet to make sure that following tests
    -- will get "disable" statistical mirroring.
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(trafficScenario.after_test_clean.transmitInfo, trafficScenario.after_test_clean.egressInfo)
    if rc~=0 then
       setFailState()
       print("packet transmitting error: ".. rc)
    end

   return (rc==0)
end

------------------------------ MAIN PART ---------------------------------------

-- read the packets from the file
local ret, values = pcall(dofile, packetsFile)
if not ret then
   error("\nError while reading the the packets:"..values)
end

local burstCount = 40
local preTestBurst = 10
local afterTestBurst = 1

local traffic = {
   pre_test_clean = {
         transmitInfo = {
            devNum = devNum,
            portNum = port1, pktInfo = {fullPacket = values.rx1},
      	  burstCount = preTestBurst},
         egressInfo = {
            {portNum = port2, packetCount = preTestBurst, pktInfo = {fullPacket = values.tx2} , captureMode = "pcl"},
         }
   },
   test = {
         transmitInfo = {
            devNum = devNum,
            portNum = port1, pktInfo = {fullPacket = values.rx1},
      	  burstCount = burstCount},
         egressInfo = {
            {portNum = port2, packetCount = burstCount, pktInfo = {fullPacket = values.tx2} , captureMode = "pcl"},
            {portNum = port3, packetCount = burstCount / currentRatio, captureMode = "pcl"},
            {portNum = port4, packetCount = burstCount / (currentRatio/2), captureMode = "pcl"}
         }
   },
   after_test_clean = {
         transmitInfo = {
            devNum = devNum,
            portNum = port1, pktInfo = {fullPacket = values.rx1},
           burstCount = afterTestBurst},
         egressInfo = {
            {portNum = port2, packetCount = afterTestBurst, pktInfo = {fullPacket = values.tx2} , captureMode = "pcl"},
         }
   },
}

-- printLog("traffic : " , to_string(traffic))

local isOk = true;

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

return isOk
