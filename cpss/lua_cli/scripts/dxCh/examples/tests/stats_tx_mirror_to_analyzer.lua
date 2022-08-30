--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* stats_tx_mirror_to_analyzer.lua
--*
--* DESCRIPTION:
--*       Check transmitting of mirrored packets from analyzer port
--*       according to statistical mirroring.
--*       - enable analyzer ports
--*       - remove analyzers ports from default VLAN
--*       - send traffic to destination ports
--*       - capture traffic from analyzer ports - check analyzer packets and counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-------------------- local declarations --------------------
-- local declaration
local packetsFile  = "dxCh/examples/packets/stats_tx_mirror_to_analyzer.lua"
local configFile   = "dxCh/examples/configurations/stats_tx_mirror_to_analyzer.txt"
local deconfigFile = "dxCh/examples/configurations/stats_tx_mirror_to_analyzer_deconfig.txt"

local devNum = devEnv.dev
local port1  = devEnv.port[1]
local port2  = devEnv.port[2]
local port3  = devEnv.port[3]

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6")

-- next lines will set 'speed' on the ports (because the pre-test may removed speed)
resetPortCounters(devEnv.dev,devEnv.port[1])
resetPortCounters(devEnv.dev,devEnv.port[2])
resetPortCounters(devEnv.dev,devEnv.port[3])


local function callWithErrorHandling(cpssApi, params)
   local rc, values = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
      setFailState()
      print("error of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, values
end

-- currentRatio(0) = disables Tx statistical mirroring on analyzer port.
local function mirrorTxAnalyzerStatisticalMirroring(index, currentRatio)
   -- setup Tx analyzer ratio statistical mirroring.
   callWithErrorHandling(
      "cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioSet",
      {
         {"IN", "GT_U8",       "devNum",          devNum},
         {"IN", "GT_U32",      "index",           index},
         {"IN", "GT_U32",      "ratio",           currentRatio}
      }
   )

end

local index = 2

local function testBody(packets)
    local rc = 0
    local burstCnt = 9

    local scenario = {
        { -- ratio = 0
            transmitInfo = {devNum = devNum, portNum = port2,
                            pktInfo = {fullPacket = packets.tx1}, burstCount = burstCnt},
            egressInfo = {
                {portNum = port3, packetCount = 0}
            }
        },
        { -- ratio = 1
            transmitInfo = {devNum = devNum, portNum = port2,
                            pktInfo = {fullPacket = packets.rx1}, burstCount = burstCnt},
            egressInfo = {
                {portNum = port3, packetCount = burstCount}
            }
        },

        { -- ratio = 3
            transmitInfo = {devNum = devNum, portNum = port2,
                            pktInfo = {fullPacket = packets.rx1},  burstCount = burstCnt},
            egressInfo = {
                {portNum = port3, packetCount = (burstCnt/3)}
            }
        }
    }

   local ratioList = {0, 1, 3}
   local i

    printLog("Use Analyzer Index at index ",index)

    for i, traffic in ipairs(scenario) do
        mirrorTxAnalyzerStatisticalMirroring(index, ratioList[i])
        i = i + 1
        -- send the packet and check captured packets
        rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo,
                                                         traffic.egressInfo)
    end

    ratio = 1;
    mirrorTxAnalyzerStatisticalMirroring(index, ratio)   -- Restoring the default ratio

   if  hwDevNum == 0 then
        -- the logic of 'port moniter' command will not skip index 0,1 since the hwDevNum == 0
        -- so force the logic of 'port moniter' to select index 2
        test_force_specific_analyzer_inerface_index(false)
   end

    return (rc == 0)
end

------------------------------ MAIN PART ---------------------------------------

-- read the packets from the file
local ret, packets = pcall(dofile, packetsFile)
if not ret then
   error("\nError while reading the the packets:"..packets)
end


-- printLog("traffic : " , to_string(traffic))

local rc, hwDevNum = device_to_hardware_format_convert(devNum)
if  hwDevNum == 0 then
    -- the logic of 'port moniter' command will not skip index 0,1 since the hwDevNum == 0
    -- so force the logic of 'port moniter' to select index 2
    test_force_specific_analyzer_inerface_index(true , index)
end

luaTgfSimpleTest(configFile, deconfigFile, testBody, packets);

if  hwDevNum == 0 then
    -- restore the 'no force'
    test_force_specific_analyzer_inerface_index(false)
end
