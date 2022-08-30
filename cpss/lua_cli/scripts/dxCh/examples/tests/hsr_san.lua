--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hsr_san.lua
--*
--* DESCRIPTION:
--*       The test for testing HSR-SAN 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local clearDdeNotOk = false -- isGmUsed() - fixed in GM
local fromCpuEmbeddedVlanNotOk = true -- currently the GM,WM (bug in IAS) not support it


cmdLuaCLI_registerCfunction("wrlDxChDsaToString") 
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation") 

local testName = "HSR-SAN"

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


-- physical port of ring A
global_test_data_env.ringA      = port1
-- physical port of ring B
global_test_data_env.ringB      = port2
-- physical port of interlink
global_test_data_env.interlink  = port3

global_test_data_env.CPU = portCPU

-- eport of ring port A
global_test_data_env.ePort_ringA = hportToEPort(0)
-- eport of ring port B
global_test_data_env.ePort_ringB = hportToEPort(1)
-- eport of Interlink 
global_test_data_env.ePort_interlink = hportToEPort(2) 
-- base eport
global_test_data_env.ePortBase = ePortBase

-- physical port of ring A
local ringA = global_test_data_env.ringA
-- physical port of ring B
local ringB = global_test_data_env.ringB
-- physical port of interlink
local interlink = global_test_data_env.interlink
-- other port that should not get traffic
local otherPort = port4

local hport_map = {
    [ringA] = global_test_data_env.ePort_ringA - ePortBase,
    [ringB] = global_test_data_env.ePort_ringB - ePortBase,
    [interlink] = global_test_data_env.ePort_interlink - ePortBase
}


local vidxBase = 1024
local _4G = (4*1024*1024*1024)
function hsrTest1VidxToTargetPortsBmp(vidx,wordIndex)
    local relativeVidx = vidx - vidxBase
    local result
    local result2 = 0
    
    if relativeVidx >= 8 and relativeVidx <= 15 then
        relativeVidx = relativeVidx - 8
        result2      = 2^portCPU --bit 63
    end

    if     relativeVidx == 0 then
        result = 0
    elseif relativeVidx == 1 then
        result =                          2^ringA
    elseif relativeVidx == 2 then
        result =                2^ringB
    elseif relativeVidx == 3 then
        result =                2^ringB + 2^ringA
    elseif relativeVidx == 4 then
        result = 2^interlink
    elseif relativeVidx == 5 then
        result = 2^interlink +            2^ringA
    elseif relativeVidx == 6 then
        result = 2^interlink +  2^ringB
    elseif relativeVidx == 7 then
        result = 2^interlink +  2^ringB + 2^ringA
    else
        result = 0
    end

    result = result + result2
    
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

local macK1 = "00AA22334457"
local macK2 = "00AA22334458"

local macA2 = "001122334457"
local macB1 = "00FFEEDDBBAB"
local macC1 = "00987654321E"

local MC1   = "010033445566" --multicast mac

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
global_test_data_env.MC1   = makeMacAddrTestFormat(MC1)


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

local HsrEtherTypeStr = "892F" -- ethertype 16 bits

local function buildHsrTag(SeqNr,lanId,lsduSize)
    local SeqNrStr          = string.format("%4.4x",SeqNr)      --16 bits
    local HsrPathIdStr      = string.format("%1.1x",lanId)      -- 4 bits
    local HsrLsduSizeStr    = string.format("%3.3x",lsduSize)   --12 bits
    return HsrEtherTypeStr .. HsrPathIdStr .. HsrLsduSizeStr .. SeqNrStr
end

local function buildVlanTag(vlanId,cfi,vpt)
    local vidStr            = string.format("%3.3x",vlanId)     --12 bits
    local cfiVptStr         = string.format("%1.1x",vpt*2+cfi)  -- 4 bits
    return "8100" .. cfiVptStr .. vidStr
end

local function buildDsaFrwToVidx(vidx,vlanTag)
    local hwDevNum = 0x321 --[[801 decimal]] --[[ other device in the 'stack' ]] 
    local edsaStc = {
      dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                     vpt    = (vlanTag and vlanTag.exists) and vlanTag.vpt    or 0,
                     cfiBit = (vlanTag and vlanTag.exists) and vlanTag.cfi    or 0,
                     vid    = (vlanTag and vlanTag.exists) and vlanTag.vlanId or 1
                     },
      dsaInfo = {
        forward = {
          tag0TpidIndex        = 0, 
          srcHwDev             = hwDevNum,
          srcIsTagged          = (vlanTag and vlanTag.exists) and "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E" or "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          srcIsTrunk           = false,
          isTrgPhyPortValid    = false,
          dstInterface         ={ type="CPSS_INTERFACE_VIDX_E", vidx=vidx },
          phySrcMcFilterEnable = true,
          skipFdbSaLookup      = true,

          source = {portNum = portCPU},-- src eport
          origSrcPhy = {portNum = portCPU}   -- 12 bits in eDSA because muxed with origTrunkId
        }
      }
    } 

    local rc, edsaBytes = wrlDxChDsaToString(devNum, edsaStc)
    if (rc ~=0) then
        setFailState()
        printLog(edsaBytes .. ". Error code is " .. rc)
        return ""
    end
    
    return edsaBytes
end

--[[
build packet info:
     l2 = {srcMac = , dstMac = }
    ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
    ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
    ,payload = 
    ,dsaTag = {vidx = }
]]

local function buildPacket(info)
    local tempHsrTag = ""
    local tempVlanTag = ""
    local tempDsaTag = ""

    if  info.hsrTag and info.hsrTag.exists then
        local hsrTag = info.hsrTag
        tempHsrTag = buildHsrTag(hsrTag.SeqNr,hsrTag.lanId,hsrTag.lsduSize)
    end
    
    if info.dsaTag then
        if info.dsaTag.vidx then
            -- the CPU send DSA tag , that need to build 
            tempDsaTag = buildDsaFrwToVidx(info.dsaTag.vidx,info.vlanTag)
        elseif info.dsaTag.note == "CPU get FRW" then
            -- the CPU get DSA FRW tag , that was removed by the CPSS
            -- we not build DSA and not vlan tag that was embedded in the FRW DSA
        end
    else
        -- if we build DSA tag as FORWARD , we indicate in the DSA that it came with vlan tag (and embedded in the DSA)
        if  info.vlanTag and info.vlanTag.exists then
            local vlanTag = info.vlanTag
            tempVlanTag = buildVlanTag(vlanTag.vlanId,vlanTag.cfi,vlanTag.vpt)
        end
    end
    
    
    return info.l2.dstMac .. info.l2.srcMac .. tempDsaTag .. tempVlanTag .. tempHsrTag .. info.payload
end

local function calcLsduSize(payload)
    -- 4 and not 6 because without the 2 bytes of the ethertype
    return 4 + (string.len(payload) / 2)
end

local function checkDdlFirstCounter(counterType,hportCounters)
    local isError = false
    local typeToCounterMap = {
        First = 1,
        Second = 2,
        Third = 3,
    }
    
    local index = typeToCounterMap[counterType]
    
    if  index ~= nil then -- first  counter
        if (hportCounters[1] ~= 0 and index ~= 1) then
            isError = true
            printLog("First Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[2] ~= 0 and index ~= 2) then
            isError = true
            printLog("Second Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[3] ~= 0 and index ~= 3) then
            isError = true
            printLog("Third Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[index] == 0) then
            isError = true
            printLog("Counter failed (ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
        end
    else
        if (hportCounters[1] ~= 0) then
            printLog("First Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
        if (hportCounters[2] ~= 0) then
            printLog("Second Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
        if (hportCounters[3] ~= 0) then
            printLog("Third Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
    end
    
    return isError
end
local function checkDdlCounters(expected_ddlCounters,hportCounters,hport)
    if hport == hport_map[ringA] then
        return checkDdlFirstCounter(expected_ddlCounters.ringA,hportCounters)
    elseif hport == hport_map[ringB] then
        return checkDdlFirstCounter(expected_ddlCounters.ringB,hportCounters)
    elseif hport == hport_map[interlink] then
        return checkDdlFirstCounter(expected_ddlCounters.interlink,hportCounters)
    end
end

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

local function hport_DdlCountersCheck(hPort,expected_ddlCounters)
    local apiName = "cpssDxChHsrPrpHPortEntryGet"
    
    printLog("check discard counters for hPort = ",to_string(hPort))

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "hPort",    hPort },
        { "OUT",    "CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC",    "infoPtr"},
        { "OUT",    "GT_U64",    "firstCounterPtr"},
        { "OUT",    "GT_U64",    "secondCounterPtr"},
        { "OUT",    "GT_U64",    "thirdCounterPtr"}
    })

    --printLog("values = ",to_string(values))
    --printLog("values.firstCounterPtr = ",to_string(values["firstCounterPtr"]))
    --printLog("values.secondCounterPtr = ",to_string(values["secondCounterPtr"]))
    --printLog("values.thirdCounterPtr = ",to_string(values["thirdCounterPtr"]))
    local hportCounters = {
        [1]=values["firstCounterPtr"].l[0],
        [2]=values["secondCounterPtr"].l[0],
        [3]=values["thirdCounterPtr"].l[0]}

    --printLog("hportCounters = ",to_string(hportCounters))

    if not isError and expected_ddlCounters ~= nil then
        -- compare counters
        if(checkDdlCounters(expected_ddlCounters,hportCounters,hPort) == true) then
            isError = true
        end
    end
    
    return isError
end

local function globalDdlCountersCheck(outgoingPacketsDb_onPort,srcPort)
    local rc = 0
    local expected_ddlCounters = outgoingPacketsDb_onPort[1].ddlCounters
    
    if not expected_ddlCounters then
        -- not checking the ddlCounters
        return 0 --GT_OK
    end
    
    --printLog("part 1 : expected_ddlCounters",to_string(expected_ddlCounters))
    
    for _dummy2, egressInfo in pairs(outgoingPacketsDb_onPort) do
        local hPort = hport_map[egressInfo.trgPort]
        if true == hport_DdlCountersCheck(hPort,expected_ddlCounters) then
            rc = "failed"
        end
    end -- end of loop on egress ports
    
    --printLog("part 2 : expected_ddlCounters",to_string(expected_ddlCounters))
    local hPort = hport_map[srcPort]
    if true == hport_DdlCountersCheck(hPort,expected_ddlCounters) then
        rc = "failed"
    end

    return rc
end

local function clearDDE(index)
    local apiName = "cpssDxChHsrPrpDdeEntryInvalidate"

    if clearDdeNotOk then
        printLog("WARNING : DO NOT clean DDE index " .. index)
        return
    end

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
-- delete the DDE entry for MAC-A with seqNum = 1001 
-- delete before changing the partition mode 
local function clearDDE_8448()
    clearDDE(8448)
end
-- delete the DDE entry for MAC-A with seqNum = 1002 
-- delete before changing the partition mode 
local function clearDDE_17361()
    clearDDE(17361)
end
-- delete the DDE entry for MAC-B with seqNum = 1003
-- delete before changing the partition mode 
local function clearDDE_5952()
    clearDDE(5952)
end
-- delete the DDE entry for MAC-C1 with seqNum = 5003
-- delete before changing the partition mode 
local function clearDDE_5504()
    clearDDE(5504)
end

-- delete the DDE entry for MAC-B with seqNum = 5005
-- delete before changing the partition mode 
local function clearDDE_5488()
    clearDDE(5488)
end

local function wrongLsduSizeCommandSet(allowError)
    local apiName = "cpssDxChHsrPrpExceptionCommandSet"
    local command
    
    if allowError then
        command = "CPSS_PACKET_CMD_FORWARD_E"
    else
        command = "CPSS_PACKET_CMD_DROP_HARD_E"
    end

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT",    "type",    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E" },
        { "IN",     "CPSS_PACKET_CMD_ENT",    "command",    command }
    })
end

-- info accordint to :
-- https://sp.marvell.com/sites/EBUSites/Switching/Architecture/Shared%20Documents%20-%20arch%20sharepoint/Projects/Ironman/Verification%20use%20cases/HSR%20and%20PRP%20Use%20Cases.pdf?csf=1

local incomingPacketsDb = 
{
   {packetNum = "1-1" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "New address (SeqNr=0)"},
   {packetNum = "1-2" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false , remark = "Existing address (SeqNr=1)"},
   {packetNum = "1-3" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "Existing address (SeqNr=2)"},
   {packetNum = "1-4" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_20736},
   {packetNum = "1-5" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from same port"},
   {packetNum = "1-6" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from other port"},
   {packetNum = "1-7" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from other port"},
   {packetNum = "1-8" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1001, HsrTag = true  , vlanTag = false , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_8448},
   {packetNum = "1-9" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1001, HsrTag = true  , vlanTag = false , remark = "Same packet from other port"},
   {packetNum = "1-10", srcPort = ringA     , srcMac = macB , dstMac = macA , SeqNr = 1002, HsrTag = true  , vlanTag = true  , remark = "First packet 2me from Ring"},
   {packetNum = "1-11", srcPort = ringA     , srcMac = macB , dstMac = macA , SeqNr = 1002, HsrTag = true  , vlanTag = true  , remark = "Same packet 2me other port"},
   {packetNum = "1-12", srcPort = ringA     , srcMac = macA , dstMac = macB , SeqNr = 1003, HsrTag = true  , vlanTag = true  , remark = "Packet from me"},
   {packetNum = "1-13", srcPort = interlink , srcMac = macA1, dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "No ready entry"},
   {packetNum = "1-14-A", srcPort = ringA     , srcMac = macB , dstMac = macK1, SeqNr = 1004, HsrTag = true  , vlanTag = true  , remark = "Wrong LSDU size , defaul command (drop)" , wrongLsduSize = true},
   {packetNum = "1-14-B", srcPort = ringA     , srcMac = macB , dstMac = macK2, SeqNr = 1005, HsrTag = true  , vlanTag = true  , remark = "Wrong LSDU size , forward command (no drops)" , wrongLsduSize = true,
    preTestFunc  = wrongLsduSizeCommandSet , preTestParams = true  ,
    postTestFunc = wrongLsduSizeCommandSet , postTestParams = false},
}
--test that need 'simulation log'
local ssl_test = nil--"5-8"
local sslw = false

local outgoingPacketsDb =
{
    ["1-1"] = {
        {trgPort = ringA , SeqNr = 0 , HsrTag = true , vlanTag = true , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = 1 , HsrTag = true , vlanTag = true , }
    }
   ,["1-2"] = {
        {trgPort = ringA , SeqNr = 2 , HsrTag = true , vlanTag = false , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = 3 , HsrTag = true , vlanTag = false , }
    }
   ,["1-3"] = {
        {trgPort = ringA , SeqNr = 4 , HsrTag = true , vlanTag = true , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = 5 , HsrTag = true , vlanTag = true , }
    }
   ,["1-4"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "First" , ringA = "-" }}
       ,{trgPort = ringB    , SeqNr=1000, HsrTag = true  , vlanTag = true }
    }
   ,["1-5"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second"  ,ringB = "Second" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true}
    }
   ,["1-6"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Third"  ,ringB =   "NA" , ringA = "First" }}
       ,{trgPort = ringA    , SeqNr=1000, HsrTag = true  , vlanTag = true }
    }
   ,["1-7"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "NA" , ringA = "Second" }}
       ,{trgPort = ringA    , notEgress = true }
    }
   ,["1-8"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = false , ddlCounters = {interlink = "First"  ,ringB = "First" , ringA = "-" }}
       ,{trgPort = ringB    , SeqNr=1001, HsrTag = true  , vlanTag = false }
    }
   ,["1-9"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second"  ,ringB = "NA" , ringA = "First" }}
       ,{trgPort = ringA    , SeqNr=1001, HsrTag = true  , vlanTag = false }
    }
   ,["1-10"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "-" , ringA = "-" }}
    }
   ,["1-11"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-12"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-13"] = {
        {trgPort = ringA    , notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }


   ,["1-14-A"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-14-B"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "-" , ringA = "-" }}
    }
}

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

local function showDde()
    executeStringCliCommands("do shell-execute cpssDxChHsrPrpFdbDump ${dev}") 
end

local function forceExplicitDsaInfo(explicitDsa)
    local isExplicitDsa = explicitDsa and "1" or "0"
    --force/unset to packet that send by the test from the CPU to use explicit DSA tag as the test will define inside the packet
    executeStringCliCommands("do shell-execute prvTgfTrafficForceExplicitDsaInfo " .. isExplicitDsa ) 
--[[GT_STATUS prvTgfTrafficForceExplicitDsaInfo(
    GT_U32         force
)]]
end


local incomingPacketsDb_toFromCpu = {
    {packetNum = "5-1" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = ringA , remark = " From_CPU to Ring A"},
    {packetNum = "5-2" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = ringA , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,   
    postTestFunc = setDsaFromCpuTrgVlanTagged , postTestParams = false ,
    hiddenRxBytes = 4 --4 bytes of vlan tag hidden in the DSA 
    },
    {packetNum = "5-3" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = " From_CPU to Ring B with HSR tag"},
    {packetNum = "5-4" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,   
    postTestFunc = setDsaFromCpuTrgVlanTagged , postTestParams = false ,
    hiddenRxBytes = 4 --4 bytes of vlan tag hidden in the DSA 
    },
    {packetNum = "5-5" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true   , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,   
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
    {packetNum = "5-6" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,   
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
    {packetNum = "5-7" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true   , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,   
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
   {packetNum = "5-8" , srcPort = ringA     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, HsrTag = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-9" , srcPort = ringB     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, HsrTag = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-10" , srcPort = ringA    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, HsrTag = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504,
    --postTestFunc = showDde , postTestParams = nil ,
   },
   {packetNum = "5-11" , srcPort = ringB    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, HsrTag = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504
   },
   {packetNum = "5-12" , srcPort = ringA     , srcMac = macB , dstMac = macA2 , SeqNr = 5004, HsrTag = true  , vlanTag = true  , remark = "No DSA - Forwarded to CPU",
   },
   {packetNum = "5-13" , srcPort = ringB     , srcMac = macB , dstMac = macA2 , SeqNr = 5004, HsrTag = true  , vlanTag = true  , remark = "No DSA - Forwarded to CPU",
   },
   {packetNum = "5-14" , srcPort = ringA     , srcMac = macB , dstMac = MC1   , SeqNr = 5005, HsrTag = true  , vlanTag = false , remark = "No DSA - Forwarded to CPU",
    inBetweenSendFunc = clearDDE_5488
   },
   {packetNum = "5-15" , srcPort = ringB     , srcMac = macB , dstMac = MC1   , SeqNr = 5005, HsrTag = true  , vlanTag = false , remark = "No DSA - Forwarded to CPU",
   },
}


local outgoingPacketsDb_toFromCpu = {
    ["5-1"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = false , vlanTag = false},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-2"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = false , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-3"] = {
        {trgPort = ringB  , SeqNr = 5000 , HsrTag = true , vlanTag = false},
        {trgPort = ringA  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-4"] = {
        {trgPort = ringB  , SeqNr = 5000 , HsrTag = true , vlanTag = true},
        {trgPort = ringA  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-5"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , SeqNr = 1 , HsrTag = true , vlanTag = true},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = true}
    },
    ["5-6"] = {
        {trgPort = ringA  , SeqNr = 3 , HsrTag = true , vlanTag = false},
        {trgPort = ringB  , SeqNr = 4 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = false}
    },
    ["5-7"] = {
        {trgPort = ringA  , SeqNr = 6 , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , SeqNr = 7 , HsrTag = true , vlanTag = true},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = true}
    },
    ["5-8"] = {
        {trgPort = portCPU , SeqNr = 5002 , HsrTag = true , vlanTag = true},
        {trgPort = ringB   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-9"] = {
        {trgPort = portCPU , SeqNr = 5002 , HsrTag = true , vlanTag = true},
        {trgPort = ringA   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-10"] = {
        {trgPort = portCPU , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = ringB   , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, HsrTag = false , vlanTag = false}
    },
    ["5-11"] = {
        {trgPort = portCPU , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = ringA   , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, notEgress = true}
    },
    ["5-12"] = {
        {trgPort = portCPU , SeqNr = 5004 , HsrTag = false , vlanTag = true , 
            dsaTag = {note = "CPU get FRW"}},
        {trgPort = ringB   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-13"] = {
        {trgPort = portCPU , notEgress = true},
        {trgPort = ringA   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-14"] = {
        {trgPort = ringB    , SeqNr = 5005   , HsrTag = true  , vlanTag = false},
        {trgPort = interlink, HsrTag = false , vlanTag = false},
        {trgPort = portCPU  , HsrTag = false , vlanTag = false}
    },
    ["5-15"] = {
        {trgPort = portCPU , notEgress = true},
        {trgPort = ringA   , SeqNr = 5005   , HsrTag = true  , vlanTag = false},
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

local ingressLanId = 0

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
        
        ingressLanId = (ingressLanId + 1) % 16
        
        if(1 == (ingressLanId % 2)) then
            ingressPayload = payload_short
        else
            ingressPayload = payload_long
        end   
        
        local ingressLsduSize = calcLsduSize(ingressPayload)
        
        transmitInfo.portNum            = ingressInfo.srcPort

        local forceCaptureOnTransmitPort = false

        if (ingressInfo.srcPort == portCPU) then
            -- we send from the CPU to 'target port'
            if ingressInfo.trgPort ~= "vidx" then
                transmitInfo.portNum            = ingressInfo.trgPort
            end
            forceCaptureOnTransmitPort      = true
        end


        if ingressInfo.wrongLsduSize then
            ingressLsduSize = ingressLsduSize + 0x110
        end 
    --[[
    build packet info:
         l2 = {srcMac = , dstMac = }
        ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
        ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
        ,payload = 
        ,dsaTag = {vidx = }
    ]]
        local packetInfo = { 
            l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}
           ,hsrTag = { exists = ingressInfo.HsrTag , SeqNr = ingressInfo.SeqNr , lanId = ingressLanId ,lsduSize = ingressLsduSize}
           ,vlanTag = {exists = ingressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
           ,payload = ingressPayload
           ,dsaTag = ingressInfo.dsaTag
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
        
        if(ingressInfo.preTestFunc)then
            doFunction(ingressInfo.preTestFunc,ingressInfo.preTestParams)
        end

        --printLog("fullPacket=" ,to_string(transmitInfo.pktInfo.fullPacket));

        egressInfoTable = {}
        local index = 1
        for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do
     
            local egressLanId,egressLsduSize
            
            if ingressInfo.srcPort == ringA or  ingressInfo.srcPort == ringB then
                egressLanId = ingressLanId -- keep the same LanId
                egressLsduSize = ingressLsduSize
            else
                egressLanId = 0 -- egress with lanId = 0 if came from interlink
                egressLsduSize = calcLsduSize(ingressPayload)
            end
            
           
            egressInfoTable[index] = {}
            
            egressInfoTable[index].portNum = egressInfo.trgPort
            
            if egressInfo.trgPort == portCPU then
                egressInfoTable[index].portNum = "CPU"
            end
            
            if (ingressInfo.srcPort == portCPU and 
                 egressInfo.trgPort == ingressInfo.trgPort) then
                egressInfoTable[index].portNum = "CPU" 
                if egressInfo.HsrTag and (ingressInfo.trgPort == ringA or  ingressInfo.trgPort == ringB) then
                    egressLanId = ingressLanId -- keep the same LanId
                    egressLsduSize = ingressLsduSize
                end
            end

            --[[
            build packet info:
                 l2 = {srcMac = , dstMac = }
                ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
                ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
                payload = 
            ]]
            local packetInfo = { 
                l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}--[[the L2 kept]]
               ,hsrTag = { exists = egressInfo.HsrTag , SeqNr = egressInfo.SeqNr , lanId = egressLanId ,lsduSize = egressLsduSize}
               ,vlanTag = {exists = egressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
               ,payload = ingressPayload--[[the payload kept]]
               ,dsaTag = egressInfo.dsaTag
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
        
        if (clearDdeNotOk) and ingressInfo.HsrTag and transmitInfo.inBetweenSendFunc --[[clear dde]] then
            -- the GM hold cache that is not removed by the 'clearDDE' function
            -- therefor we should get drop on the second ring port
            local index = 1
            for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do
                
                if egressInfo.HsrTag then
                    egressInfoTable[index].packetCount = 1   -- only to count once 
                    egressInfoTable[index].pktInfo     = nil -- and not send second iteration
                    break--[[no more ports with hsr tag]]
                end
                
                index = index + 1
            end
        end
        

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

--        printLog("transmitInfo", to_string(transmitInfo))
--        printLog("egressInfoTable", to_string(egressInfoTable))

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           transmitInfo, egressInfoTable)
        local result_string = caseName

        if forceCaptureOnTransmitPort then
            -- restore the default
            forceCaptureOnTransmitPortSet(false)
        end

        if ingressInfo.packetNum == ssl_test then
            shell_execute("nssl")
        end

        local rc1 = globalDdlCountersCheck(OUT_Packets[ingressInfo.packetNum],ingressInfo.srcPort)

        if rc ~= 0 or rc1 ~= 0 then
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
        
    end -- loop on IN_Packets[]
end -- end of function doMainLogic

-------------- do the test --------------
executeLocalConfig("dxCh/examples/configurations/hsr_san.txt")
if wrlCpssIsAsicSimulation() or isGmUsed() then
    ddeSameKeyExpirationTimeSet(5000) -- allow to run ssl (simulation LOG)
else
    ddeSameKeyExpirationTimeSet(1000)
end
if not skip_part_1 then
    doMainLogic(incomingPacketsDb,outgoingPacketsDb)
end

if not skip_part_2 then
    executeLocalConfig("dxCh/examples/configurations/hsr_san_cpu.txt")
    
    doMainLogic(incomingPacketsDb_toFromCpu,outgoingPacketsDb_toFromCpu)
    
    executeLocalConfig("dxCh/examples/configurations/hsr_san_cpu_deconfig.txt")
end

ddeSameKeyExpirationTimeSet(2)-- restore the default after init
executeLocalConfig("dxCh/examples/configurations/hsr_san_deconfig.txt")
testPrintResultSummary(testName)


