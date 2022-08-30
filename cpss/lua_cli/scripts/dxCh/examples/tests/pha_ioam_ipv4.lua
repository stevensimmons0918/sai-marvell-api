--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pha_ioam_ipv4.lua
--*
--* DESCRIPTION:
--*       The test for 'ingress switch' IOAM over VXLAN-GPE.
--*       packet ingress as 'ethernet' packet and egress with the IOAM over VXLAN-GPE 
--*       encapsulation
--*
--*       The test for 'transit switch' IOAM over VXLAN-GPE.
--*       packet ingress as IOAM over VXLAN-GPE (over udp-ipv4) packet and egress with 
--*       additional IOAM fields
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
-- the PHa supported on sip6 devices
SUPPORTED_FEATURE_DECLARE(devNum, "PHA_IOAM")


local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]
local port6   = devEnv.port[6]

local pkt
local pktInfo
local payloads
local status



-- fill global table used by the test's config-files
if test_env == nil then
    test_env = {}
end
local rc, hwDevNum = device_to_hardware_format_convert(devEnv.dev)
if (rc ~= 0) then
    setFailState()
    printLog(hwDevNum)      -- hwDevNum will store error line if rc ~= 0
    return
end
local srcHwDevNum = hwDevNum
local trgHwDevNum = hwDevNum
test_env.srcHwDev = srcHwDevNum
test_env.trgHwDev = trgHwDevNum

--generate pha_ioam_ipv4 test packets
local function buildPackets()
    status, payloads = pcall(dofile, "dxCh/examples/packets/pha_ioam_ipv4.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

-- ignore bytes that are 'unpredictable'
local my_maskedBytesTable_port3 = {
     {startByte = 18, endByte = 19 ,reason = "2 bytes 'identification field'"}
    ,{startByte = 24, endByte = 25 ,reason = "2 bytes 'ipv4 checksum'"}
    ,{startByte = 34, endByte = 35 ,reason = "2 bytes 'srcUdpPort' hash based"}
    ,{startByte = 62, endByte = 62+8-1 ,reason = "8 bytes time stamp (sec + nano)"}
}

local my_maskedBytesTable_port4 = {
     {startByte = 18, endByte = 19 ,reason = "2 bytes 'identification field'"}
    ,{startByte = 24, endByte = 25 ,reason = "2 bytes 'ipv4 checksum'"}
    ,{startByte = 34, endByte = 35 ,reason = "2 bytes 'srcUdpPort' hash based"}
    ,{startByte = 62, endByte = 62+4-1 ,reason = "4 bytes time stamp (nano)"}
    ,{startByte = 94, endByte = 94+4-1 ,reason = "4 bytes lmCounter/time stamp (nano) in the e2e header"}
}

local my_maskedBytesTable_port6 = {
     {startByte = 18, endByte = 19 ,reason = "2 bytes 'identification field'"}
    ,{startByte = 24, endByte = 25 ,reason = "2 bytes 'ipv4 checksum'"}
    ,{startByte = 34, endByte = 35 ,reason = "2 bytes 'srcUdpPort' hash based"}
    ,{startByte = 62, endByte = 62+8-1 ,reason = "8 bytes time stamp (sec + nano)"}
}

local function runTest_uni_to_all()
    -- from port 1 flooding to other ports 
    local packetsInfo = payloads.port1_flood
    
    local transmitInfo = {devNum = devNum, portNum = port1 ,
        pktInfo = {fullPacket = packetsInfo.sender} }
    local egressInfoTable = {
        -- expected egress port (apply also mask on needed fields)
        {portNum = port2  ,
            pktInfo = {fullPacket = packetsInfo.port2}}
        ,{portNum = port3 ,
            pktInfo = {fullPacket = packetsInfo.port3 ,
            maskedBytesTable = my_maskedBytesTable_port3}}
        ,{portNum = port4 ,
            pktInfo = {fullPacket = packetsInfo.port4 ,
            maskedBytesTable = my_maskedBytesTable_port4}}
        ,{portNum = port5 , packetCount = 0 }
        ,{portNum = port6 , packetCount = 0 }
    }
    -- transmit packet and check that egress as expected
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("uni_to_all Test passed \n")
    else
        printLog ("uni_to_all Test failed \n")
        setFailState()
    end
end

local function runTest_transit()
    -- from port 5 to port 6
    local packetsInfo = payloads.port5_transit
    
    --printLog("payloads.port5_transit",to_string(payloads.port5_transit))
    
    --local printable_port6 = packetAsStringToPrintableFormat(packetsInfo.port6)
    --printLog(printable_port6)
    
    local transmitInfo = {devNum = devNum, portNum = port5 ,
        pktInfo = {fullPacket = packetsInfo.sender} }
    local egressInfoTable = {
        -- expected egress port (apply also mask on needed fields)
        {portNum = port6  ,
            pktInfo = {fullPacket = packetsInfo.port6,
            maskedBytesTable = my_maskedBytesTable_port6}}
        ,{portNum = port1 , packetCount = 0 }
        ,{portNum = port2 , packetCount = 0 }
        ,{portNum = port3 , packetCount = 0 }
        ,{portNum = port4 , packetCount = 0 }
    }
    -- transmit packet and check that egress as expected
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("transit Test passed \n")
    else
        printLog ("transit Test failed \n")
        setFailState()
    end
end

--load configuration 
executeLocalConfig("dxCh/examples/configurations/vxlan_gre_ioam.txt")
-- load packet format (must be after the 'configuration' because using values set by it)
buildPackets()

-- 'ingress device' test
runTest_uni_to_all()
-- 'transit device' test
runTest_transit()

--load de-configuration 
executeLocalConfig("dxCh/examples/configurations/vxlan_gre_ioam_deconfig.txt")
