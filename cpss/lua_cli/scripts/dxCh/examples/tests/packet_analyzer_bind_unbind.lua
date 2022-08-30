--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_bind_unbind.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer binding function
--*       runs on ports 16,76 ,send packet
--*       and check if 2 muxed stages for hits
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_DXCH_ALDRIN_E","CPSS_PP_FAMILY_DXCH_ALDRIN2_E",
                      "CPSS_PP_FAMILY_DXCH_FALCON_E" ,"CPSS_PP_FAMILY_DXCH_AC5X_E","CPSS_PP_FAMILY_DXCH_AC5P_E","CPSS_PP_FAMILY_DXCH_HARRIER_E" )
SUPPORTED_FEATURE_DECLARE(devNum,"PACKET_ANALYZER")

local payloads
local devFamily = wrlCpssDeviceFamilyGet(devNum)
local ports   = {devEnv.port[4],devEnv.port[1]}
local expStage = {}
local ret, status
local enterCmd

--default values

local expfield = {
            ["byte_count"]              ={["data"]={94,0,0,0},         ["mask"]={0xff,0xff,0xff,0xff}},
            ["local_dev_src_port"]      ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
           }

if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E", ["luaStageName"] = "router" ,["hit"]=0x0}
    expStage[2] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",    ["luaStageName"] = "i-oam"  ,["hit"]=0x0}
elseif devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",["luaStageName"] = "e-policer" ,["hit"]=0x0}
    expStage[2] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",    ["luaStageName"] = "pha"       ,["hit"]=0x0}
    expfield["byte_count"]["data"][1]= 110
elseif (devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E") or (devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E")    then
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",["luaStageName"] = "e-oam" ,["hit"]=0x0}
    expStage[2] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",["luaStageName"] = "pha"   ,["hit"]=0x0}
    expfield["byte_count"]["data"][1]= 110
elseif devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E" then
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E", ["luaStageName"] = "bridge" ,["hit"]=0x0}
    expStage[2] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E", ["luaStageName"] = "router" ,["hit"]=0x0}
else --harrier
    expStage[1] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E", ["luaStageName"] = "e-pcl" ,["hit"]=0x0}
    expStage[2] = { ["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E", ["luaStageName"] = "e-oam" ,["hit"]=0x0}
    expfield["byte_count"]["data"][1]= 110
end


-------------------------------------------------------------------------------------------------------------
--generate ACL redirect test packets
dofile ("dxCh/examples/common/packet_analyzer_utils.lua")

ret, payloads = pcall(dofile, "dxCh/examples/packets/packet_analyze_basic_check.lua")

--reset port counters
for ii, sendPort in ipairs(ports) do
    resetPortCounters(devNum, sendPort)
end


for ii, sendPort in ipairs(ports) do
    ------------------------with no rule set-------------------------------------
    expfield["local_dev_src_port"]["data"][1]= sendPort

    --in oreder not to get more the 1 hit in egress stages we match the cpu port
    if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E"  or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or
       devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E"    or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
        expfield["local_dev_src_port"]["data"][1] = 63
        --test only 1 time
        if ii > 1 then
            break
        end
    end

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[exit
            configure pipeline
            match local_dev_src_port ]]..tostring(expfield["local_dev_src_port"]["data"][1]).." \n"..
          [[match byte_count ]]..tostring(expfield["byte_count"]["data"][1]).." \n"..
          [[sampling-mode last_match
            exit
            clear
            enable pipeline
            exit]]

    executeStringCliCommands(enterCmd)

   --set transmitInfo
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket"]}
    }
    egressInfoTable = nil

    -- transmit packet
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable
            ]]

    executeStringCliCommands(enterCmd)

    expStage[1]["hit"] = 1
    expStage[2]["hit"] = 0

    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)
    ----------------------------------------------------
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[no configure stage ]]..expStage[1]["luaStageName"].." \n"..
          [[configure stage ]]..expStage[2]["luaStageName"].." \n"..
          [[exit
            clear
            enable pipeline
            exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("luaTgfTransmitPacketsWithExpectedEgressInfo failed \n")
        setFailState()
    end

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable
            ]]

    executeStringCliCommands(enterCmd)

    expStage[1]["hit"] = 0
    expStage[2]["hit"] = 1

    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    if ii ~= #ports then
        --clear all rules
        enterCmd = [[
                end
                debug-mode
                packet-analyzer manager 1 device ]]..devNum.." \n"..
              [[no configure stage ]]..expStage[2]["luaStageName"].." \n"..
              [[configure pipeline
                no match
                exit]]

        executeStringCliCommands(enterCmd)
    end

end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

