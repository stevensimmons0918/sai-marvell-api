--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_uds.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer uds
--*       runs on 2 ports and uds configuration and rule set
--*       stage uds_0
--*       field mac sa
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
local ret, status
local expStage = {}
local udsConfig = {}
local enterCmd


udsConfig["CPSS_PP_FAMILY_DXCH_ALDRIN_E"]  ={["instance"]="ipcl_tcam_or_debug_buses"                       , ["interface"]="L2I_2_MT_NA_BUS"   , ["leg"]="4" }
udsConfig["CPSS_PP_FAMILY_DXCH_ALDRIN2_E"] ={["instance"]="l2i_ipvx_desc_or_debug_buses_0_8"               , ["interface"]="l2i2mt_na_bus"     , ["leg"]="2" }
udsConfig["CPSS_PP_FAMILY_DXCH_FALCON_E"]  ={["instance"]="tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm"   , ["interface"]="L2I_2_MT_LEARN"    , ["leg"]="1" }
udsConfig["CPSS_PP_FAMILY_DXCH_AC5X_E"]    ={["instance"]="pnx_l2i_ipvx_macro_mt_or_shm"                   , ["interface"]="L2I_2_MT_LEARN"    , ["leg"]="0" }
udsConfig["CPSS_PP_FAMILY_DXCH_AC5P_E"]    ={["instance"]="tile0_l2i_ipvx_macro_mt_or_shm"                 , ["interface"]="L2I_2_MT_LEARN"    , ["leg"]="0" }
udsConfig["CPSS_PP_FAMILY_DXCH_HARRIER_E"] ={["instance"]="har_l2i_ipvx_macro_mt_or_shm"                   , ["interface"]="L2I_2_MT_LEARN"    , ["leg"]="0" }

--default values
expStage[1] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E"               , ["hit"]=0x0}

local expfield = {
                    ["mac_sa"]={["data"]={0x1144,0x0,0x0,0}, ["mask"]={0xffff,0xffff,0xffff,0xffff}},
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
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    egressInfoTable = nil
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[user-defined stage s1 interface ]]..udsConfig[devFamily]["interface"].." instance "..udsConfig[devFamily]["instance"].." leg "..udsConfig[devFamily]["leg"]

    executeStringCliCommands(enterCmd)

    if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
        enterCmd = [[
                no configure stage router]]
        executeStringCliCommands(enterCmd)
    end

    enterCmd =  [[
            configure stage s1
            match mac_sa 00:00:00:00:11:44
            exit
            enable stage
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

    expStage[1]["hit"] = 1

    --catch packets that match  mac sa
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    transmitInfo = {
        devNum = devNum,
        portNum = sendPort,
        pktInfo = {fullPacket = payloads["unkownUcPacket1"]}
    }
    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[clear
            enable stage
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

    --catch packets that doesnt match mac sa  - no hit expected
    printLog("rule set - 0 hit ")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    if ii ~= #ports then
        --clear all rules
        enterCmd = [[
                end
                debug-mode
                packet-analyzer manager 1 device ]]..devNum.." \n"..
              [[no user-defined stage s1]]

        executeStringCliCommands(enterCmd)

        if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
        enterCmd = [[
                configure stage router
                exit]]
        executeStringCliCommands(enterCmd)
        end
    end
end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

