--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_udf.lua
--*
--* DESCRIPTION:
--*       The test for packet_analyzer udf
--*       runs on 2 ports and check udf configuration and match
--*       udf field : ttl ,is_bc
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

--default values
expStage[1] = { ["stage"] ="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E"               , ["hit"]=0x0}

local expfield = {
                    ["mac_da"]={["data"]={0xffff,0xffff,0xffff,0xffff}, ["mask"]={0xffff,0xffff,0xffff,0xffff}},
                    ["is_bc"]={["data"]={0x1,0x0,0x0,0x0}, ["mask"]={0xffff,0xffff,0xffff,0xffff}},
                    ["ttl"]={["data"]={0x40,0x0,0x0,0x0}, ["mask"]={0xffff,0xffff,0xffff,0xffff}},
                    ["local_dev_src_port"]={["data"]={0,0,0,0}, ["mask"]={0xff,0xff,0xff,0xff}},
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
        pktInfo = {fullPacket = payloads["unkownUcPacket3"]}
    }
    egressInfoTable = nil

    expfield["local_dev_src_port"]["data"][1]= sendPort
    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[user-defined field f1 field-id is_bc
            user-defined field f2 field-id ttl
            configure stage i-pcl
            match local_dev_src_port ]]..tostring(sendPort).." \n"..
          [[match mac_da ff:ff:ff:ff:ff:ff
            match f1 1
            match f2 64
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
        pktInfo = {fullPacket = payloads["unkownUcPacket2"]}
    }
    ---------------------------------------------------------------------

    enterCmd = [[
            end
            debug-mode
            packet-analyzer manager 1 device ]]..devNum.." \n"..
          [[configure stage i-pcl
            match mac_da 00:00:00:00:34:22
            match f1 0
            exit
            enable stage
            exit]]

    executeStringCliCommands(enterCmd)

    expfield["mac_da"][1]=0x3422
    expfield["mac_da"][2]=0x0
    expfield["mac_da"][3]=0x0
    expfield["mac_da"][4]=0x0
    expfield["is_bc"][1]=0x0

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

    printLog("rule set - 1 hit ")
    rc = packetAnalyzerCheckHits("rule set",nil,nil,expStage,expfield)
    packetAnalyzerCheckReturnStatus(rc)

    if ii ~= #ports then
        --clear all rules
        enterCmd = [[
                end
                debug-mode
                packet-analyzer manager 1 device ]]..devNum.." \n"..
              [[no user-defined field f1
                no user-defined field f2
                configure stage i-pcl
                no match]]

        executeStringCliCommands(enterCmd)
    end
end
---------------------------------------------------------------------

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/packet_analyzer_basic_check_deconfig.txt")

