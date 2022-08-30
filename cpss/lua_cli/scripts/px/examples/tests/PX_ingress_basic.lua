--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_ingress_basic.lua
--*
--* DESCRIPTION:
--*       Ingress-processing configuration for two pairs of ports.
--*       each port in pair sends packet one to another.
--*       Check counters on egress ports. Packet should not be modified on egress.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local devNum   = devEnv.dev
local port1    = devEnv.port[1]
local port2    = devEnv.port[2]
local port3    = devEnv.port[3]
local port4    = devEnv.port[4]

-- Enable/disable packet type key table entry
local function luaWrap_cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, enable)
    local command_data = Command_Data()

    local apiName = "cpssPxIngressPacketTypeKeyEntryEnableSet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN",  "GT_SW_DEV_NUM", "devNum", devNum},
        {"IN",  "GT_U32", "packetType", packetType},
        {"IN",  "GT_BOOL", "enable", enable}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPacketTypeKeyEntryEnableSet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end

local function packetTypesSet(enable)
    for packetType = 0, 31 do
    -- enable/disable all packet types
        if packetType ~= 16 then
            luaWrap_cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, enable)
        end
    end
end

-- disable all packet types before test
packetTypesSet(false)

--run the config file
executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_basic"))

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macDa   = "000000000058"
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end

    return macDa .. macSa ..  partAfterMac .. packetPayload
end

--##################################
--##################################
local ingressPacket = buildPacketUc(nil)
local transmitInfo1 = {portNum = port1 , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable1 = {
    -- expected to egress unmodified
     {portNum = port2, packetCount = 1},
     {portNum = port3, packetCount = 0},
     {portNum = port4, packetCount = 0}
}
local transmitInfo2 = {portNum = port2 , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable2 = {
    -- expected to egress unmodified
     {portNum = port1, packetCount = 1},
     {portNum = port3, packetCount = 0},
     {portNum = port4, packetCount = 0}
}
local transmitInfo3 = {portNum = port3 , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable3 = {
    -- expected to egress unmodified
     {portNum = port1, packetCount = 0},
     {portNum = port2, packetCount = 0},
     {portNum = port4, packetCount = 1}
}
local transmitInfo4 = {portNum = port4 , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable4 = {
    -- expected to egress unmodified
     {portNum = port1, packetCount = 0},
     {portNum = port2, packetCount = 0},
     {portNum = port3, packetCount = 1}
}

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

-- function to implement the test
local function sendTraffic()
    --set config
    local sectionName = "LUA test with traffic "
    local transmitArray = {
            {ingressInfo = transmitInfo1, egressInfo = egressInfoTable1},
            {ingressInfo = transmitInfo2, egressInfo = egressInfoTable2},
            {ingressInfo = transmitInfo3, egressInfo = egressInfoTable3},
            {ingressInfo = transmitInfo4, egressInfo = egressInfoTable4}
        }
--  Skip the traffic operations to get PASSED this test if SMI Interface used
    local devInterface = wrlCpssManagementInterfaceGet(devNum)
    if (devInterface ~= "CPSS_CHANNEL_SMI_E") then
        for index, entry in pairs(transmitArray) do
            -- Send packet to port and check that packet egress port in it's pair
            local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(entry.ingressInfo, entry.egressInfo)

            printLog (delimiter)
            if rc ~= 0 then
                local error_string = "Section " .. sectionName .. " FAILED."
                printLog ("ENDED : " .. error_string .. "\n")
                setFailState()
                --run the deconfig file
                executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_basic_deconfig"))
                return
            end
            delay(100)
        end
    end

    local pass_string = "Section " .. sectionName .. " PASSED."
    printLog ("ENDED : " .. pass_string .. "\n")
end

-- run the test
sendTraffic()

-- enable all packet types after test
packetTypesSet(true)

--run the deconfig file
executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_basic_deconfig"))
