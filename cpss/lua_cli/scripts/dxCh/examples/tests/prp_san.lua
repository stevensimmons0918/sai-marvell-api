--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* prp_san.lua
--*
--* DESCRIPTION:
--*       The test for testing PRP-SAN 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local fromCpuEmbeddedVlanNotOk = true -- currently the GM,WM (bug in IAS) not support it
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation") 


local testName = "PRP-SAN"

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local portCPU = devEnv.portCPU -- cpu port 

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6_30") 

local skip_part_1 = false
local skip_part_2 = false


local ePortBase = 0x3f0 --[[decimal 1008]]

local function hportToEPort(hport)
    return ePortBase + hport
end


-- physical port of PRP port A
global_test_data_env.PRP_port_A = port1
-- physical port of PRP port B
global_test_data_env.PRP_port_B = port2
-- physical port of interlink
global_test_data_env.interlink  = port3

global_test_data_env.CPU = portCPU

-- eport of PRP port A
global_test_data_env.ePort_PRP_port_A = hportToEPort(0)
-- eport of PRP port B
global_test_data_env.ePort_PRP_port_B = hportToEPort(1)
-- eport of Interlink 
global_test_data_env.ePort_interlink = hportToEPort(2) 
-- base eport
global_test_data_env.ePortBase = ePortBase

local remove_rct_bit = 7
local function srcIdForEport(eport,remove_rct)
    local srcId = eport - ePortBase
    if(remove_rct and get_bit(srcId,remove_rct_bit,1) == 0) then
        -- need to remove rct
        srcId = srcId + 2^remove_rct_bit
    end
    
    return srcId
end

-- SRC-ID with indication of 'remove RCT' for ingress from PRP port A
global_test_data_env.remove_rct_src_id_PRP_port_A = srcIdForEport(global_test_data_env.ePort_PRP_port_A,true)
-- SRC-ID with indication of 'remove RCT' for ingress from PRP port B
global_test_data_env.remove_rct_src_id_PRP_port_B = srcIdForEport(global_test_data_env.ePort_PRP_port_B,true)


-- physical port of ring A
local PRP_port_A = global_test_data_env.PRP_port_A
-- physical port of ring B
local PRP_port_B = global_test_data_env.PRP_port_B
-- physical port of interlink
local interlink = global_test_data_env.interlink
-- other port that should not get traffic
local otherPort = port4

local hport_map = {
    [PRP_port_A] = global_test_data_env.ePort_PRP_port_A - ePortBase,
    [PRP_port_B] = global_test_data_env.ePort_PRP_port_B - ePortBase,
    [interlink] = global_test_data_env.ePort_interlink - ePortBase
}


local vidxBase = 1024
local _4G = (4*1024*1024*1024)
function prpTest1VidxToTargetPortsBmp(vidx,wordIndex)
    local relativeVidx = vidx - vidxBase
    local result

    if     relativeVidx == 0 then
        result = 0
    elseif relativeVidx == 1 then
        result =                          2^PRP_port_A
    elseif relativeVidx == 2 then
        result =                2^PRP_port_B
    elseif relativeVidx == 3 then
        result =                2^PRP_port_B + 2^PRP_port_A
    elseif relativeVidx == 4 then
        result = 2^interlink
    elseif relativeVidx == 5 then
        result = 2^interlink +            2^PRP_port_A
    elseif relativeVidx == 6 then
        result = 2^interlink +  2^PRP_port_B
    elseif relativeVidx == 7 then
        result = 2^interlink +  2^PRP_port_B + 2^PRP_port_A
    else
        result = 0
    end
    
    if wordIndex == 1 then
        return math.floor(result / _4G)
    end
    
    if(result >=  _4G) then
        return math.mod(result , _4G)
    end

    return result
end

local command_data = Command_Data()

local macA  = "001122334455"
local macB  = "00FFEEDDBBAA"
local macC  = "00987654321F"
local macA1 = "001122334456"

local macA2 = "001122334457"
local macB1 = "00FFEEDDBBAB"
local macC1 = "00987654321E"


local function makeMacAddrTestFormat (macAddr)
    local str = ""
    local newPart,offset,ii
    
    for ii = 1 , 6 do
        offset = (ii-1)*2
        newPart = string.sub(macAddr, 1 + offset, 2 + offset)
        if(str == "")then
            str = newPart
        else
            str = str .. ":" .. newPart
        end
    end

    return str
end


global_test_data_env.macB1 = makeMacAddrTestFormat(macB1)
global_test_data_env.macA2 = makeMacAddrTestFormat(macA2)
global_test_data_env.macC1 = makeMacAddrTestFormat(macC1)

local transmitInfo =
{
  devNum = devNum,
  pktInfo = {},
  inBetweenSendFunc = nil
}
local egressInfoTable


local payload_short = 
"aaaabbbbccccddddeeeeffff00001111"..
"22223333444455556666777788889999"..
"a1a2a3a4b1b2b3b4c1c2c3c4d1d2d3d4"..
"e1e2e3e4f1f2f3f49192939481828384"


local payload_long = 
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short 

local prpSuffix = "88FB"

local function buildPrpRct(SeqNr,lanId,lsduSize)
    if lanId == nil then lanId = 0 end
    local SeqNrStr          = string.format("%4.4x",SeqNr)      --16 bits
    local lanIdStr      = string.format("%1.1x",lanId)      -- 4 bits
    local lsduSizeStr    = string.format("%3.3x",lsduSize)   --12 bits
    return SeqNrStr .. lanIdStr .. lsduSizeStr .. prpSuffix
end

local function buildVlanTag(vlanId,cfi,vpt)
    local vidStr            = string.format("%3.3x",vlanId)     --12 bits
    local cfiVptStr         = string.format("%1.1x",vpt*2+cfi)  -- 4 bits
    return "8100" .. cfiVptStr .. vidStr
end


--[[
build packet info:
     l2 = {srcMac = , dstMac = }
    ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
    ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
    payload = 
    totalPacketSize = 
    padding = 
]]

local function buildPacket(info)
    local tempPrpTrailer = ""
    local tempVlanTag = ""

    if  info.prpRct and info.prpRct.exists then
        local prpRct = info.prpRct
        tempPrpTrailer = buildPrpRct(prpRct.SeqNr,prpRct.lanId,prpRct.lsduSize)
    end
    
    if  info.vlanTag and info.vlanTag.exists then
        local vlanTag = info.vlanTag
        tempVlanTag = buildVlanTag(vlanTag.vlanId,vlanTag.cfi,vlanTag.vpt)
    end
    
    local totalPacket =  info.l2.dstMac .. info.l2.srcMac .. tempVlanTag .. info.payload .. tempPrpTrailer
    
    if (info.totalPacketSize ~= nil) then
        local currTotalLen   = string.len(totalPacket)  / 2
        local currPayloadLen = string.len(info.payload) / 2
        
        local numBytesRemovedFromPayload = currTotalLen - info.totalPacketSize
        
        local paddingBytes = ""
        if info.padding ~= nil and info.padding ~= 0 then
            for index = 1, info.padding do 
                paddingBytes = paddingBytes .. "00"
            end
        end 

        --update the lsduSize if relevant
        if  info.prpRct and info.prpRct.exists then
            local prpRct = info.prpRct
            local lsduSize = prpRct.lsduSize - numBytesRemovedFromPayload
            if info.padding ~= nil then
                lsduSize = lsduSize + info.padding
            end

            tempPrpTrailer = buildPrpRct(prpRct.SeqNr,prpRct.lanId,lsduSize)
        end

        -- we need tp trim the packet
        totalPacket = info.l2.dstMac .. info.l2.srcMac .. tempVlanTag .. 
            string.sub(info.payload, 1 , (currPayloadLen - numBytesRemovedFromPayload) * 2) .. paddingBytes .. tempPrpTrailer
    end
    
    return totalPacket
end

local function calcLsduSize(payload)
    -- 4 and not 6 because without the 2 bytes of the ethertype
    return 4 + (string.len(payload) / 2)
end

local function clearDDE(index)
    local apiName = "cpssDxChHsrPrpDdeEntryInvalidate"

    printLog("clean DDE index " .. index)

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",    index }
    })
    
end

-- delete the DDE entry for MAC-A with seqNum = 1000 
-- delete before changing the partition mode 
local function clearDDE_20736()
    clearDDE(20736)
end


local function treatWrongLanIdAsRctExistsSet(portNum,enable)
    local apiName = "cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet"
    
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",    "GT_U8",         "devNum"    ,    devNum },
        { "IN",    "GT_PORT_NUM",   "portNum"   ,    portNum },
        { "IN",    "GT_BOOL",       "enable"    ,    enable},
    })
end

local function onPortA_B_treatWrongLanIdAsRctExistsSet(enable)
    treatWrongLanIdAsRctExistsSet(PRP_port_A,enable)
    treatWrongLanIdAsRctExistsSet(PRP_port_B,enable)
end

local function setDsaFromCpuTrgVlanTagged(valid)
    local isValid = valid and "1" or "0"
    --set/unset to egress with vlan tag as indication from the DSA tag and not as 'payload' after the DSA
    executeStringCliCommands("do shell-execute prvTgfTrafficForceVlanTagInfo " .. isValid .. ",0,0,1") 
--[[GT_STATUS prvTgfTrafficForceVlanTagInfo(
    GT_U32         valid,
    GT_U32         vpt,
    GT_U32         cfiBit,
    GT_U32         vid
);]]
end

-- delete the DDE entry for MAC-C1 with seqNum = 5003
-- delete before changing the partition mode 
local function clearDDE_5504()
    clearDDE(5504)
end


--

-- info accordint to :
-- https://sp.marvell.com/sites/EBUSites/Switching/Architecture/Shared%20Documents%20-%20arch%20sharepoint/Projects/Ironman/Verification%20use%20cases/HSR%20and%20PRP%20Use%20Cases.pdf?csf=1

local incomingPacketsDb = 
{
    {packetNum = "1-1" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "New address (SeqNr=0)"}
   ,{packetNum = "1-2" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false , remark = "Existing address (SeqNr=1)"}
   ,{packetNum = "1-3" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "Existing address (SeqNr=2)"}
   ,{packetNum = "1-4" , srcPort = PRP_port_A, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_20736}
   ,{packetNum = "1-5" , srcPort = PRP_port_A, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "Same packet from same port"}
   ,{packetNum = "1-6" , srcPort = PRP_port_B, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "Same packet from other port"}
   ,{packetNum = "1-7" , srcPort = interlink , srcMac = macA1, dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "No ready entry"}
   ,{packetNum = "1-8-A1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2000 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID treat as RCT (first from port A)",
    wrongLanId = true,
    preTestFunc  = onPortA_B_treatWrongLanIdAsRctExistsSet , preTestParams = true  ,
    postTestFunc = onPortA_B_treatWrongLanIdAsRctExistsSet , postTestParams = false}
   ,{packetNum = "1-8-A2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2000 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID treat as RCT (second from port B so dropped)",
    wrongLanId = true,
    preTestFunc  = onPortA_B_treatWrongLanIdAsRctExistsSet , preTestParams = true  ,
    postTestFunc = onPortA_B_treatWrongLanIdAsRctExistsSet , postTestParams = false}
   ,{packetNum = "1-8-B1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2001 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID not treat as RCT (first from port A)",
    wrongLanId = true}
   ,{packetNum = "1-8-B2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2001 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID not treat as RCT (second from port B allowed and not dropped)",
    wrongLanId = true}
   ,{packetNum = "1-9-1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2002 , prpRct = true , vlanTag = true  , remark = "wrong LSDU size not treat as RCT (first from port A)",
    wrongLsduSize = true
    }
   ,{packetNum = "1-9-2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2002 , prpRct = true , vlanTag = true  , remark = "wrong LSDU size not treat as RCT (second from port B allowed and not dropped)",
    wrongLsduSize = true
    }
   ,{packetNum = "1-10-1" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = false , remark = "padding test",
    totalPacketSize = 64 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-2" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 64 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-3" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 65 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-4" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 66 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-5" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 67 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-6" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 68 - 4 --[[CRC]]
    }
    

}
--test that need 'simulation log'
local ssl_test = nil--"5-7"
local sslw = false

local outgoingPacketsDb =
{
    ["1-1"] = {
        {trgPort = PRP_port_A , SeqNr = 0 , prpRct = true , vlanTag = true }
       ,{trgPort = PRP_port_B , SeqNr = 1 , prpRct = true , vlanTag = true }
    }
   ,["1-2"] = {
        {trgPort = PRP_port_A , SeqNr = 2 , prpRct = true , vlanTag = false}
       ,{trgPort = PRP_port_B , SeqNr = 3 , prpRct = true , vlanTag = false}
    }
   ,["1-3"] = {
        {trgPort = PRP_port_A , SeqNr = 4 , prpRct = true , vlanTag = true}
       ,{trgPort = PRP_port_B , SeqNr = 5 , prpRct = true , vlanTag = true}
    }
   ,["1-4"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true}
    }
   ,["1-5"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true}
    }
   ,["1-6"] = {
        {trgPort = interlink  , notEgress = true}
       ,{trgPort = PRP_port_A , notEgress = true}
    }
   ,["1-7"] = {
        {trgPort = PRP_port_A , notEgress = true }
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-A1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-A2"] = {
        {trgPort = interlink  , notEgress = true}
       ,{trgPort = PRP_port_A , notEgress = true}
    }
   ,["1-8-B1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-B2"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_A , notEgress = true }
    }
   ,["1-9-1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-9-2"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_A , notEgress = true }
    }
   ,["1-10-1"] = {
        {trgPort = PRP_port_A , SeqNr = 6 , prpRct = true , vlanTag = false , padding = 0 , totalPacketSize = 64 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-2"] = {
        {trgPort = PRP_port_A , SeqNr = 7 , prpRct = true , vlanTag = true  , padding = 4 , totalPacketSize = 64 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-3"] = {                                                                                               
        {trgPort = PRP_port_A , SeqNr = 8 , prpRct = true , vlanTag = true  , padding = 3 , totalPacketSize = 65 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-4"] = {                                                                                               
        {trgPort = PRP_port_A , SeqNr = 9 , prpRct = true , vlanTag = true  , padding = 2 , totalPacketSize = 66 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-5"] = {                                                                                               
        {trgPort = PRP_port_A , SeqNr = 10, prpRct = true , vlanTag = true  , padding = 1 , totalPacketSize = 67 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-6"] = {                                                                                               
        {trgPort = PRP_port_A , SeqNr = 11, prpRct = true , vlanTag = true  , padding = 0 , totalPacketSize = 68 + 6--[[RCT]] - 4 --[[CRC]]}}
        
   
}


local incomingPacketsDb_toFromCpu = {
    {packetNum = "5-1" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = " From_CPU to PRP network"},
    {packetNum = "5-2" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = " From_CPU to PRP network (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,   
    postTestFunc = setDsaFromCpuTrgVlanTagged , postTestParams = false ,
    hiddenRxBytes = 4    
    },
    {packetNum = "5-3" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , prpRct = true , vlanTag = false  , trgPort = PRP_port_B , remark = " From_CPU to Ring B with PRP RCT"},
    {packetNum = "5-4" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , prpRct = true , vlanTag = false  , trgPort = PRP_port_B , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,   
    postTestFunc = setDsaFromCpuTrgVlanTagged , postTestParams = false ,
    hiddenRxBytes = 4    
    },
   {packetNum = "5-5" , srcPort = PRP_port_A     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, prpRct = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-6" , srcPort = PRP_port_B     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, prpRct = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-7" , srcPort = PRP_port_A    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, prpRct = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504,
    --postTestFunc = showDde , postTestParams = nil ,
   },
   {packetNum = "5-8" , srcPort = PRP_port_B    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, prpRct = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504
   },
}

local outgoingPacketsDb_toFromCpu = {
    ["5-1"] = {
        {trgPort = PRP_port_A  , SeqNr = 0 , prpRct = false , vlanTag = false},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-2"] = {
        {trgPort = PRP_port_A  , SeqNr = 0 , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-3"] = {
        {trgPort = PRP_port_B  , SeqNr = 5000 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-4"] = {
        {trgPort = PRP_port_B  , SeqNr = 5000 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-5"] = {
        {trgPort = portCPU , SeqNr = 5002 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_B   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-6"] = {
        {trgPort = portCPU , SeqNr = 5002 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_A   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-7"] = {
        {trgPort = portCPU , SeqNr = 5003 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_B   , notEgress = true},
        {trgPort = interlink, prpRct = false , vlanTag = false}
    },
    ["5-8"] = {
        {trgPort = portCPU , SeqNr = 5003 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_A   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
}

if fromCpuEmbeddedVlanNotOk then
    outgoingPacketsDb_toFromCpu["5-2"][1].vlanTag =  false
    outgoingPacketsDb_toFromCpu["5-4"][1].vlanTag =  false
end


-- we need to capture the egress packet that loopback from the port to the CPU
-- use iPCL rule to trap it
local function forceCaptureOnTransmitPortSet(force)
    
    if force then 
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            rule-id 0 action trap-to-cpu 
            exit
            
            access-list device ${dev} pcl-ID 11
            rule-id 6 action trap-to-cpu 
            exit

            access-list device ${dev} pcl-ID 12
            rule-id 12 action trap-to-cpu 
            exit

            exit
        ]]
        )
    else
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            delete rule-id 0
            exit
            
            access-list device ${dev} pcl-ID 11
            delete rule-id 6
            exit

            access-list device ${dev} pcl-ID 12
            delete rule-id 12
            exit

            exit
        ]]
        )
    end
    
end

local lanIdMap = {[PRP_port_A] = 0xA , [PRP_port_B] = 0xB}
local functionalityMap = {[PRP_port_A] = "PRP_port_A" , [PRP_port_B] = "PRP_port_B" , [interlink] = "interlink" , ["CPU"] = portCPU }
local packetCounter = 0

local function doFunction (func , params)

    if(not func)then return end
    
    if( type(func) == "table") then
        for _dummy, entry in pairs(func) do
            entry.testFunc(entry.testParams)
        end
    else
        func(params)
    end
end

local function doMainLogic(IN_Packets,OUT_Packets)
    for _dummy1, ingressInfo in pairs(IN_Packets) do
        local caseName = "Send a packet #".. ingressInfo.packetNum .. " , note : ".. ingressInfo.remark

        printLog("========================================")
        printLog(caseName)
        
        ingressLanId = lanIdMap[ingressInfo.srcPort]
        
        if(1 == (packetCounter % 2)) then
            ingressPayload = payload_short
        else
            ingressPayload = payload_long
        end   
        packetCounter = packetCounter + 1
        
        local ingressLsduSize = calcLsduSize(ingressPayload)

        if ingressInfo.wrongLsduSize then
            ingressLsduSize = ingressLsduSize + 0x330
        end 

        if ingressInfo.wrongLanId then
            ingressLanId = ingressLanId + 2 --{0xA --> 0xC} , {0xB --> 0xD}
        end 

        transmitInfo.portNum            = ingressInfo.srcPort

        local forceCaptureOnTransmitPort = false

        if (ingressInfo.srcPort == portCPU) then
            -- we send from the CPU to 'target port'
            transmitInfo.portNum            = ingressInfo.trgPort
            forceCaptureOnTransmitPort      = true
        end



    --[[
    build packet info:
         l2 = {srcMac = , dstMac = }
        ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
        ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
        payload = 
        totalPacketSize = 
    ]]
        local packetInfo = { 
            l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}
           ,prpRct = { exists = ingressInfo.prpRct , SeqNr = ingressInfo.SeqNr , lanId = ingressLanId ,lsduSize = ingressLsduSize}
           ,vlanTag = {exists = ingressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
           ,payload = ingressPayload
           ,totalPacketSize = ingressInfo.totalPacketSize
        }
        
        --printLog("packetInfo=" ,to_string(packetInfo));

        transmitInfo.pktInfo.fullPacket = buildPacket(packetInfo)
        transmitInfo.inBetweenSendFunc  = ingressInfo.inBetweenSendFunc
        if(fromCpuEmbeddedVlanNotOk and (ingressInfo.packetNum == "5-2" or ingressInfo.packetNum == "5-4")) then
            -- there is no vlan tag that is hidden in the DSA
            transmitInfo.hiddenRxBytes      = nil
        else
            transmitInfo.hiddenRxBytes      = ingressInfo.hiddenRxBytes
        end

        --printLog("fullPacket=" ,to_string(transmitInfo.pktInfo.fullPacket));

        if(ingressInfo.preTestFunc)then
            doFunction(ingressInfo.preTestFunc,ingressInfo.preTestParams)
        end

        egressInfoTable = {}
        local index = 1
        for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do
     
            local egressLanId,egressLsduSize
            
            egressLanId = lanIdMap[egressInfo.trgPort] -- egress with lanId according to egress port
            
            if ingressInfo.srcPort == PRP_port_A or  ingressInfo.srcPort == PRP_port_B then
                egressLsduSize = ingressLsduSize
            else
                egressLsduSize = calcLsduSize(ingressPayload)
            end
            
            --[[
            build packet info:
                 l2 = {srcMac = , dstMac = }
                ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
                ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
                payload = 
                totalPacketSize = 
                padding = 
            ]]
            
            local egress_prpRct 

            egressInfoTable[index] = {}
            egressInfoTable[index].portNum = egressInfo.trgPort

            if egressInfo.trgPort == portCPU then
                egressInfoTable[index].portNum = "CPU"
                egressLanId = ingressLanId -- keep the same LanId
            end
            
            if (ingressInfo.srcPort == portCPU and 
                 egressInfo.trgPort == ingressInfo.trgPort) then
                egressInfoTable[index].portNum = "CPU" 
                if egressInfo.prpRct and (ingressInfo.trgPort == PRP_port_A or  ingressInfo.trgPort == PRP_port_B) then
                    egressLanId = ingressLanId -- keep the same LanId
                    egressLsduSize = ingressLsduSize
                end
            end

            local packetInfo = { 
                l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}--[[the L2 kept]]
               ,prpRct = { exists = egressInfo.prpRct , SeqNr = egressInfo.SeqNr , lanId = egressLanId ,lsduSize = egressLsduSize}
               ,vlanTag = {exists = egressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
               ,payload = ingressPayload--[[the payload kept]]
               ,totalPacketSize = egressInfo.totalPacketSize
               ,padding = egressInfo.padding
            }

            if egressInfo.notEgress == true then -- indication of drop
                egressInfoTable[index].pktInfo = nil
                egressInfoTable[index].packetCount = 0 -- check that not getting traffic
            else
                egressInfoTable[index].pktInfo = {fullPacket = buildPacket(packetInfo)}
                egressInfoTable[index].packetCount = nil
            end
            
            index = index + 1
            
           
        end -- end of loop on egress ports
        
        egressInfoTable[index] = {}
        egressInfoTable[index].portNum = otherPort
        egressInfoTable[index].packetCount = 0  -- check that not getting traffic
        egressInfoTable[index].pktInfo = nil

        if ingressInfo.packetNum == ssl_test then
            if sslw == true then
                shell_execute("sslw")
            else
                shell_execute("ssl")
            end
        end

        if forceCaptureOnTransmitPort then
            -- we need to capture the egress packet that loopback from the port to the CPU
            -- use iPCL rule to trap it
            forceCaptureOnTransmitPortSet(true)

        end

        printLog("Ingress from : " .. functionalityMap[transmitInfo.portNum] .. "(port : " .. transmitInfo.portNum ..")")
        for _dummy3, egressInfo in pairs(egressInfoTable) do
            local egressPortString
            if functionalityMap[egressInfo.portNum] then
                egressPortString = "Egress from : " .. functionalityMap[egressInfo.portNum] .. "(port : " .. egressInfo.portNum ..")"
            else
                egressPortString =  "Egress from : port " .. egressInfo.portNum .. "(non-functional port)"
            end

            if(egressInfo.packetCount == 0) then
                egressPortString = "! Not ! " .. egressPortString
            end
            
            printLog(egressPortString)
        end

        --printLog("transmitInfo", to_string(transmitInfo))
        --printLog("egressInfoTable", to_string(egressInfoTable))
 
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           transmitInfo, egressInfoTable);
        local result_string = caseName

        if forceCaptureOnTransmitPort then
            -- restore the default
            forceCaptureOnTransmitPortSet(false)
        end

        if ingressInfo.packetNum == ssl_test then
            shell_execute("nssl")
        end

        if rc ~= 0 then
            result_string = result_string .. " FAILED"
            printLog ("ENDED : " .. result_string .. "\n")
            testAddErrorString(result_string)
        else
            printLog ("ENDED : " .. result_string .. "\n")
            testAddPassString(result_string)
        end
        
        if(ingressInfo.postTestFunc)then
            doFunction(ingressInfo.postTestFunc,ingressInfo.postTestParams)
        end

    end -- loop on incomingPacketsDb[]
end -- end of function doMainLogic

function ddeSameKeyExpirationTimeSet(timeInMilisec)
    local apiName = "cpssDxChHsrPrpDdeTimeFieldSet"
    
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT",    "field",    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E },
        { "IN",     "GT_U32",    "timeInMicroSec",    timeInMilisec * 1000 }-- time in the API is in micro sec
    })    
end

-------------- do the test --------------
executeLocalConfig("dxCh/examples/configurations/prp_san.txt")
if wrlCpssIsAsicSimulation() or isGmUsed() then
    ddeSameKeyExpirationTimeSet(5000) -- allow to run ssl (simulation LOG)
else
    ddeSameKeyExpirationTimeSet(1000)
end
if not skip_part_1 then
    doMainLogic(incomingPacketsDb,outgoingPacketsDb)
end

if not skip_part_2 then
    executeLocalConfig("dxCh/examples/configurations/prp_san_cpu.txt")
    
    doMainLogic(incomingPacketsDb_toFromCpu,outgoingPacketsDb_toFromCpu)
    
    executeLocalConfig("dxCh/examples/configurations/prp_san_cpu_deconfig.txt")
end

ddeSameKeyExpirationTimeSet(2)-- restore the default after init
executeLocalConfig("dxCh/examples/configurations/prp_san_deconfig.txt")
testPrintResultSummary(testName)