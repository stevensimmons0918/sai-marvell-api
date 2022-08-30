--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_first_last.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer first and last match
--*       runs on ports 16,76 ,send 2 packets with diffrent mac_da
--*       and check if first and last match works
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_ALDRIN_E", "CPSS_PP_FAMILY_DXCH_ALDRIN2_E",
                      "CPSS_PP_FAMILY_DXCH_FALCON_E","CPSS_PP_FAMILY_DXCH_AC5X_E","CPSS_PP_FAMILY_DXCH_AC5P_E","CPSS_PP_FAMILY_DXCH_HARRIER_E")
SUPPORTED_FEATURE_DECLARE(devNum,"PACKET_ANALYZER")

local payloads
local devFamily = wrlCpssDeviceFamilyGet(devNum)
local ports   = {devEnv.port[1]} -- test only 1 port  - see CPSS-9964
local expStage ={}
local enterCmd

local ret, status

if devFamily ~= "CPSS_PP_FAMILY_DXCH_ALDRIN_E" then
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",   ["luaStageName"] = "i-pcl", ["hit"]=0x2}
else
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",["luaStageName"] = "bridge",["hit"]=0x2}
end

local expfield = {
            ["byte_count"]              ={["data"]={94,0,0,0},         ["mask"]={0xff,0xff,0xff,0xff}},
            ["mac_da"]                  ={["data"]={0x3402,0,0,0},     ["mask"]={0xffff,0xff,0xff,0xff}},
            ["local_dev_src_port"]      ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
           }

if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or
   devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E"   or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
    expfield["byte_count"]["data"][1]= expfield["byte_count"]["data"][1] - 4
end

-------------------------------------------------------------------------------------------------------------
--generate ACL redirect test packets
dofile ("dxCh/examples/common/packet_analyzer_utils.lua")

ret, payloads = pcall(dofile, "dxCh/examples/packets/packet_analyze_first_last.lua")

--reset port counters
for ii, sendPort in ipairs(ports) do
    resetPortCounters(devNum, sendPort)
end

for ii, sendPort in ipairs(ports) do
    ------------------------with no rule set-------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[sampling-mode first_match
            exit
            clear
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

    --set transmitInfo 1
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    egressInfoTable = nil

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

    --set transmitInfo 2
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    egressInfoTable = nil

    -- transmit packet 2
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

    local enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable]]

    executeStringCliCommands(enterCmd)

    expfield["mac_da"]["data"][1]= 0x3411
    expfield["local_dev_src_port"]["data"][1]= 63

    --default values
if (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E") and (ii == 1) then
    expfield["local_dev_src_port"]["data"][1]= sendPort -- see CPSS-9964
end

    rc = packetAnalyzerCheckHits("no rule",expStage[1]["stage"],
                                 printPaFieldCpssInt["CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E"],expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)
    rc = packetAnalyzerCheckHits("no rule",expStage[1]["stage"],
                                 printPaFieldCpssInt["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E"],expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)
----------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[sampling-mode last_match
            exit
            clear
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

   --set transmitInfo 1
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    egressInfoTable = nil

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

   --set transmitInfo 2
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    egressInfoTable = nil

    -- transmit packet 2
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable]]

    executeStringCliCommands(enterCmd)

    expfield["mac_da"]["data"][1]= 0x3422
    expfield["local_dev_src_port"]["data"][1]= sendPort

    rc = packetAnalyzerCheckHits("no rule",expStage[1]["stage"],
                                 printPaFieldCpssInt["CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E"],expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)
    rc = packetAnalyzerCheckHits("no rule",expStage[1]["stage"],
                                 printPaFieldCpssInt["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E"],expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ------------------------with rules set---------------------------------------

    expfield["local_dev_src_port"]["data"][1]= sendPort

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[match local_dev_src_port ]]..tostring(sendPort).." \n"..
          [[match byte_count ]]..tostring(expfield["byte_count"]["data"][1]).." \n"..
          [[match mac_da 00:00:00:00:34:02 mask 00:00:00:00:00:00
            sampling-mode first_match
            exit
            clear
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

   --set transmitInfo 1
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    egressInfoTable = nil

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

   --set transmitInfo 2
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    egressInfoTable = nil

    -- transmit packet 2
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable]]

    executeStringCliCommands(enterCmd)

    expfield["mac_da"]["data"][1]= 0x3411
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)
----------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[sampling-mode last_match
            exit
            clear
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

   --set transmitInfo 1
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    egressInfoTable = nil

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

   --set transmitInfo 2
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    egressInfoTable = nil

    -- transmit packet 2
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable]]

    executeStringCliCommands(enterCmd)

    expfield["mac_da"]["data"][1]= 0x3422
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    if ii ~= #ports then
      --clear all rules
        enterCmd = [[
                end
                debug-mode
                packet-analyzer manager 1 device ]]..devNum.." \n"..
              [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
              [[no match
                exit]]

        executeStringCliCommands(enterCmd)
    end
end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

