--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_basic_check.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer basic check
--*       runs on ports 16,76 and check hit\no hit
--*       for fields byte_count mac_da and mac_sa
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local devNum  = devEnv.dev
SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_ALDRIN_E","CPSS_PP_FAMILY_DXCH_ALDRIN2_E",
                      "CPSS_PP_FAMILY_DXCH_FALCON_E","CPSS_PP_FAMILY_DXCH_AC5X_E","CPSS_PP_FAMILY_DXCH_AC5P_E","CPSS_PP_FAMILY_DXCH_HARRIER_E")
SUPPORTED_FEATURE_DECLARE(devNum,"PACKET_ANALYZER")

local payloads
local devFamily = wrlCpssDeviceFamilyGet(devNum)
local ports     = {devEnv.port[1],devEnv.port[4]}
local trgPort   =  devEnv.port[2]
local ret, status
local expStage = {}
local enterCmd
local numOfLinkUps = 0
--default values
if devFamily ~= "CPSS_PP_FAMILY_DXCH_ALDRIN_E" then
    expStage[1] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E"               , ["hit"]=0x0}
    expStage[2] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E"               , ["hit"]=0x0}
    expStage[3] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E"            , ["hit"]=0x0}
    expStage[4] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E"            , ["hit"]=0x0}
    expStage[5] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E"               , ["hit"]=0x0}
    expStage[6] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E"           , ["hit"]=0x0}
    expStage[7] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E"           , ["hit"]=0x0}
    expStage[8] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E"             , ["hit"]=0x0}
    expStage[9] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E"     , ["hit"]=0x0}
    expStage[10] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E"  , ["hit"]=0x0}
    expStage[11] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E"                , ["hit"]=0x0}
    expStage[12] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E"            , ["hit"]=0x0}
else -- no i-pcl stage
    expStage[1] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E"               , ["hit"]=0x0}
    expStage[2] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E"            , ["hit"]=0x0}
    expStage[3] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E"            , ["hit"]=0x0}
    expStage[4] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E"               , ["hit"]=0x0}
    expStage[5] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E"           , ["hit"]=0x0}
    expStage[6] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E"           , ["hit"]=0x0}
    expStage[7] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E"             , ["hit"]=0x0}
    expStage[8] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E"     , ["hit"]=0x0}
    expStage[9] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E"  , ["hit"]=0x0}
    expStage[10] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E"                , ["hit"]=0x0}
    expStage[11] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E"            , ["hit"]=0x0}
end

if devFamily== "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or
   devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E"  or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E"  then
    expStage[13] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E"        , ["hit"]=0x0}
end
local expfield = {["byte_count"]              ={["data"]={94,0,0,0},         ["mask"]={0xff,0xff,0xff,0xff}},
                  ["mac_da"]                  ={["data"]={0x3402,0,0,0},     ["mask"]={0xff,0xff,0xff,0xff}},
                  ["mac_sa"]                  ={["data"]={0x11,0x11,0x11,0}, ["mask"]={0xff,0xff,0xff,0xff}},
                  ["local_dev_src_port"]      ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
                  ["local_dev_trg_phy_port"]  ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
                 }

if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or
   devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E"   or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
    expfield["byte_count"]["data"][1]= expfield["byte_count"]["data"][1] - 4
end

-------------------------------------------------------------------------------------------------------------
--generate ACL redirect test packets
dofile ("dxCh/examples/common/packet_analyzer_utils.lua")

ret, payloads = pcall(dofile, "dxCh/examples/packets/packet_analyze_basic_check.lua")
--reset port counters
for ii, sendPort in ipairs(ports) do
    resetPortCounters(devNum, sendPort)
end
-------------------------------------------------------------------------------------------------------------
for ii, sendPort in ipairs(ports) do
    --set transmitInfo
    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket"]}
    }
    egressInfoTable = nil
    expfield["local_dev_src_port"]["data"][1]= sendPort
    expfield["local_dev_trg_phy_port"]["data"][1]= trgPort

    rc,numOfLinkUps = getNumberOfLinkUpPorts(devNum)
    --ignore self port
    numOfLinkUps = numOfLinkUps -1

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[clear
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
            exit]]

    executeStringCliCommands(enterCmd)

    --check all pipeline for traffic pass
    printLog("Test 1 no rule set")
    rc = packetAnalyzerCheckHits("no rule",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure pipeline
            match local_dev_src_port ]]..tostring(sendPort).." \n"..
          [[match local_dev_trg_phy_port ]]..tostring(trgPort).." \n"..
          [[match mac_sa 00:01:00:00:34:00
            exit
            clear
            enable pipeline
            exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("failed \n")
        setFailState()
    end

    local enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable
            exit]]

    executeStringCliCommands(enterCmd)

    if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E" then
        expStage[1]["hit"] = 2 -- should be 1 - CPSS-9753
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 0
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 1
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 0
    elseif devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 0
        expStage[5]["hit"] = 0
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 1
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
    elseif devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 0
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 1
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 0
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
        expStage[13]["hit"] = 1
    else
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 0
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 1
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = numOfLinkUps
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
        expStage[13]["hit"] = 1

    end

    --catch packets that match src and trg port and mac sa , mac da removed
    printLog("Test 2 rule set")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure pipeline
            match byte_count 13
            exit
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
            exit]]

    executeStringCliCommands(enterCmd)

    for k=1,#expStage do
        expStage[k]["hit"] = 0
    end

    if devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" or
       devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
        expStage[9]["hit"] = numOfLinkUps
    end

    --catch packets that match src and trg port and wrong byte count - no hit expected
    printLog("Test 3 rule set - 0 hit ")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure pipeline
            no match mac_sa
            match byte_count ]]..tostring(expfield["byte_count"]["data"][1]).." \n"..
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
            exit]]

    executeStringCliCommands(enterCmd)

    if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E" then
        expStage[1]["hit"] = 2 -- should be 1 - CPSS-9753
        expStage[2]["hit"] = 1
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 0
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 1
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 0
    elseif devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 1
        expStage[3]["hit"] = 1
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 0
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 1
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
    elseif devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 1
        expStage[3]["hit"] = 1
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 1
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 0
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
        expStage[13]["hit"] = 1
    else
        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 1
        expStage[3]["hit"] = 1
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 1
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = numOfLinkUps
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 1
        expStage[13]["hit"] = 1
    end

    --catch packets that match src and trg port and byte count - mac sa removed
    printLog("Test 4 rule set")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ---------------------------------------------------------------------

    if ii ~= #ports then
        --clear all rules
        enterCmd = [[
                end
                debug-mode
                packet-analyzer manager 1 device ]]..devNum.." \n"..
              [[configure pipeline
                no match
                exit]]

        executeStringCliCommands(enterCmd)
    end
end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

