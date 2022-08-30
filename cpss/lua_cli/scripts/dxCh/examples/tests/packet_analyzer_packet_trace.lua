--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_packet_trace.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer packet track
--*       run once in pipeline mode and once in stage mode
--*       config rule on i-pcl and check for hits on stages that have
--*       packet trace field in them
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_FALCON_E","CPSS_PP_FAMILY_DXCH_AC5X_E","CPSS_PP_FAMILY_DXCH_AC5P_E","CPSS_PP_FAMILY_DXCH_HARRIER_E")
SUPPORTED_FEATURE_DECLARE(devNum,"PACKET_ANALYZER")
local payloads
local devFamily = wrlCpssDeviceFamilyGet(devNum)
local ports     = {devEnv.port[4]}
local trgPort   =  devEnv.port[2]
local ret, status
local expStage = {}
local enterCmd
local numOfLinkUps=0

--default values

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
    if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then -- harrier doesn't have mac stage
        expStage[13] ={ ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E"                , ["hit"]=0x0}
    end


local expfield = {["mac_sa"]                  ={["data"]={0x1144,0,0,0},  ["mask"]={0xff,0xff,0xff,0xff}},
                  ["packet_trace"]            ={["data"]={0x0,0x0,0x0,0}, ["mask"]={0xff,0xff,0xff,0xff}},
                  ["local_dev_src_port"]      ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
                  ["local_dev_trg_phy_port"]  ={["data"]={0,0,0,0},          ["mask"]={0xff,0xff,0xff,0xff}},
                 }

-------------------------------------------------------------------------------------------------------------
--generate ACL redirect test packets
dofile ("dxCh/examples/common/packet_analyzer_utils.lua")

ret, payloads = pcall(dofile, "dxCh/examples/packets/packet_analyze_first_last.lua")
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
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    egressInfoTable = nil

    --set transmitInfo
    transmitInfo2 = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }

    rc,numOfLinkUps = getNumberOfLinkUpPorts(devNum)
    if rc ~= 0 then
        printLog ("getNumberOfLinkUpPorts failed \n")
        setFailState()
    end
    --ignore self port
    numOfLinkUps = numOfLinkUps -1

    expfield["local_dev_src_port"]["data"][1]= sendPort
    expfield["local_dev_trg_phy_port"]["data"][1]= trgPort

    printLog("check for packet-trace at pipeline level  ")
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure pipeline packet-trace
            exit
            clear
            enable pipeline]]
    executeStringCliCommands(enterCmd)

    -- transmit packet 1
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
          [[configure pipeline packet-trace
            match local_dev_src_port ]]..tostring(sendPort).." \n"..
          [[match local_dev_trg_phy_port ]]..tostring(trgPort).." \n"..
          [[match mac_sa 00:00:00:00:11:44
            exit
            clear
            enable pipeline
            exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc ~= 0 then
        printLog ("failed \n")
        setFailState()
    end
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable
            exit]]

    executeStringCliCommands(enterCmd)

        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 0
        expStage[5]["hit"] = 0
        expStage[6]["hit"] = 0
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 0
        expStage[9]["hit"] = 0
        expStage[10]["hit"] = 0
        expStage[11]["hit"] = 0
        expStage[12]["hit"] = 0
        if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then -- harrier doesn't have mac stage
            expStage[13]["hit"] = 0
        end

    --catch packets that match src and trg port and mac da
    printLog("Test 2 rule set expect no hit in i-pcl and all other stages with packet trace field")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device 0
            clear
            enable pipeline
            exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet 2
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo2,egressInfoTable)
    if rc ~= 0 then
        printLog ("failed \n")
        setFailState()
    end
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[disable
            exit]]

    executeStringCliCommands(enterCmd)

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
        if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
            expStage[13]["hit"] = 1
        end

    if (devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E") or (devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E") or
       (devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
            expStage[9]["hit"] = numOfLinkUps
            if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
                expStage[13]["hit"] = numOfLinkUps
            end
    end

    for i=3 , #expStage do
        expfield["packet_trace"]["data"][1] =1
    end

    --catch packets that match src and trg port and mac sa , mac da removed
    printLog("Test 3 rule set expect hit in i-pcl and all other stages with packet trace field")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[no configure pipeline]]

    executeStringCliCommands(enterCmd)

    printLog("check for packet-trace at stage level  ")
    enterCmd = [[

            configure stage i-pcl packet-trace
            match mac_sa 00:00:00:00:11:44
            match local_dev_src_port ]]..tostring(sendPort).." \n"

    executeStringCliCommands(enterCmd)

    printLog("set stages with diffrent packet trace match rule  ")
    enterCmd = [[
        exit
        configure stage bridge
        match packet_trace 1
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit
        configure stage router
        match packet_trace 1
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit
        configure stage header-alt
        match packet_trace 1
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[match local_dev_trg_phy_port ]]..tostring(trgPort).." \n"..
      [[exit
        configure stage e-policer
        match packet_trace 1
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[match local_dev_trg_phy_port ]]..tostring(trgPort).." \n"..
      [[exit
        configure stage e-filter
        match packet_trace 1
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit
        configure stage e-pcl
        match packet_trace 0
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[match local_dev_trg_phy_port ]]..tostring(trgPort).." \n"..
      [[exit
        configure stage equeue
        match packet_trace 0
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit
        configure stage i-policer
        match packet_trace 0
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit
        configure stage tti
        match local_dev_src_port ]]..tostring(sendPort).." \n"
    if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
    enterCmd = enterCmd..[[exit
        configure stage mac
        match packet_trace 1 ]].." \n"
    end
    enterCmd = enterCmd..[[exit
        configure stage i-oam
        match packet_trace 0
        match local_dev_src_port ]]..tostring(sendPort).." \n"..
      [[exit]]

    executeStringCliCommands(enterCmd)

    if (devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E") or (devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E") or
       (devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
        enterCmd = [[
            configure stage tx-queue
            match packet_trace 1
            match local_dev_src_port ]]..tostring(sendPort).." \n"..
          [[exit]]

        executeStringCliCommands(enterCmd)
    end

    enterCmd = [[
        clear
        enable stage
        exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet 1
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo2,egressInfoTable)
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

        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 1
        expStage[3]["hit"] = 1
        expStage[4]["hit"] = 1
        expStage[5]["hit"] = 0
        expStage[6]["hit"] = 0
        expStage[7]["hit"] = 0
        expStage[8]["hit"] = 1
        expStage[9]["hit"] = 0
        expStage[10]["hit"] = 1
        expStage[11]["hit"] = 0
        expStage[12]["hit"] = 1
        if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
            expStage[13]["hit"] = 1
        end

    if (devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E") or (devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E") or
       (devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
            expStage[9]["hit"] = numOfLinkUps
            if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
                expStage[13]["hit"] = numOfLinkUps
            end
    end

    --catch packets that match src and trg port and mac da
    printLog("Test 4 rule set expect hit in i-pcl and where packet trace is match to 1")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[clear
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

    -- transmit packet 1
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

        expStage[1]["hit"] = 1
        expStage[2]["hit"] = 0
        expStage[3]["hit"] = 0
        expStage[4]["hit"] = 0
        expStage[5]["hit"] = 1
        expStage[6]["hit"] = 1
        expStage[7]["hit"] = 1
        expStage[8]["hit"] = 0
        expStage[9]["hit"] = 0
        expStage[10]["hit"] = 0
        expStage[11]["hit"] = 1
        expStage[12]["hit"] = 0
        if devFamily ~= "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
            expStage[13]["hit"] = 0
        end

    --catch packets that match src and trg port and mac da
    printLog("Test 5 rule set expect no hit in i-pcl and hit where packet trace match to 0")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    if ii ~= #ports then
        --clear all rules
        enterCmd = [[
                end
                debug-mode
                no packet-analyzer manager 1
                ]]

        executeStringCliCommands(enterCmd)
    end
end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

