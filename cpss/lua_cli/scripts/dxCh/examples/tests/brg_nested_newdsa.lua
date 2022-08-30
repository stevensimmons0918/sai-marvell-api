--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* brg_nested_newdsa.lua
--*
--* DESCRIPTION:
--*       Force New DSA tag while nested VLANs enabled
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

if not is_sip_5(devNum) then
    setTestStateSkipped()
    return
end


local port1   = devEnv.port[1]
local expected_vid_no_new_dsa = 1
local expected_vid_new_dsa = 7

local sendPacket = genPktL2({
    dstMAC                =  "000000000011",
    srcMAC                =  "000000000022",
    etherType             =  "3333",
    ieee802_1q_Tag        =  "81000001",

    payload = "1111111111111111111111111111111111111111"..
              "1111111111111111111111111111111111111111"..
              "1111111111111111111111111111111111111111"
})


local force_false =
{
    transmitInfo =
    {
        devNum = devNum,
        portNum = port1, pktInfo = {fullPacket = sendPacket}
    },
    egressInfoTable =
    {
        {portNum = "CPU", pktInfo = {fullPacket = sendPacket, cpuCodeNumber = 195}},
    }
}

local force_true =
{
    transmitInfo =
    {
        devNum = devNum,
        portNum = port1, pktInfo = {fullPacket = sendPacket}
    },
    egressInfoTable =
    {
        {portNum = "CPU", pktInfo = {fullPacket = sendPacket, cpuCodeNumber = 195}},
    }
}


-- local declaration
local configFile   = "dxCh/examples/configurations/brg_nested_newdsa.txt"
local deconfigFile = "dxCh/examples/configurations/brg_nested_newdsa_deconfig.txt"

local function trafficCheck(trafficScenario)
    local key, scenario;
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        assert(
            rc==0, string.format("packet transmitting error: %s",
            returnCodes[rc]))
    end
end

local isOk;

local rxQ = registerRx("getPacket",{})
executeStringCliCommands("do cpss-api call cpssDxChBrgVlanForceNewDsaToCpuEnableSet devNum ${dev} enable false")
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, {force_false = force_false})

executeStringCliCommands("do cpss-api call cpssDxChBrgVlanForceNewDsaToCpuEnableSet devNum ${dev} enable true")
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, {force_true = force_true})

--Check DSA eVID
local rxdata_no_new_dsa
local rxdata_new_dsa

isOk, rxdata_no_new_dsa = luaMsgQRecv(rxQ, 2000)
isOk, rxdata_new_dsa    = luaMsgQRecv(rxQ, 2000)

unregisterRx("getPacket",rxQ)

if expected_vid_new_dsa ~= rxdata_new_dsa.vlanId then
    printLog("NEW DSA: VLAN ID is unexpected "..to_string(rxdata_new_dsa.vlanId).."\n")
    setFailState()
end

if expected_vid_no_new_dsa ~= rxdata_no_new_dsa.vlanId then
    printLog("NO NEW DSA: VLAN ID is unexpected "..to_string(rxdata_no_new_dsa.vlanId).."\n")
    setFailState()
end