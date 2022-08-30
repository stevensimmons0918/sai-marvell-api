--********************************************************************************
--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* remote_physical_ports_system_ingress_from_stack_port.lua
--*
--* DESCRIPTION:
--*       Remote physical ports; cpssInitSystem 36,1
--*       Set trunk and TPIDs on remote physical ports , and Cascade to other Sip5 device.
--*       in system of BC3 + 2 PIPEs set as 'Remote physical ports' system
--*       Using special 'appDemo' functions for TPID synch between DX and PX config.
--*       Using special 'appDemo' functions for Trunk synch for 'stacking' WA with other Sip5 device.
--*       traffic:
--*
--*       flood from stack port (cascade port) with 'forward eDSA' to vidx = 0xFFF in vlan 5 , 'origIsTrunk = true'
--*       this 'stack port' emulate a packet from neighbour sip5 device that have the 'TTI-1522' issue in the eDSA
--*       (similar to packet that egress the cascade from test 'remote_physical_ports_system_ingress_from_remote_port2.lua')
--*       this to check WA for limitation derived from behaviour of JIRA 'TTI-1522'
--*          a. not flood back to this port
--*          b. not flood back to other trunk member (remote port and BC3 native port)
--*          d. check that FDB learned on trunkId
--*          e. check egress tagging on egress ports , according to their egress tag state.
--*          f. special check : disable 'port isolation' (the 'WA') , and check that one of trunk member gets flood !
--*              (restore port isolation at the end)
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-------------------- local declarations --------------------
-- local declaration
local configFile             = "dxCh/examples/configurations/remote_physical_ports_common.txt"
local deconfigFile           = "dxCh/examples/configurations/remote_physical_ports_common_deconfig.txt"
local flush_fdb_only_dynamic = "dxCh/examples/configurations/flush_fdb_only_dynamic.txt"

local devNum = devEnv.dev
local port1  = devEnv.port[1] -- BC3 native port
local port2  = devEnv.port[2] -- (remote)port on PIPE#1  , set as trunk 15 member
local port3  = devEnv.port[3] -- (remote)port on PIPE#2 
local port4  = devEnv.port[4] -- BC3 native port
local port5  = devEnv.port[5] -- BC3 native port , set as trunk 15 member
local port6  = devEnv.port[6] -- BC3 native port , set as cascade 

-- skip test for unsupported 'systems' !
-- we need here the 'cpssInitSystem 36,1'
SUPPORTED_FEATURE_DECLARE(devNum, "REMOTE_PHYSICAL_PORTS_SYSTEM")

local isFailed = false
local totalFailCases = 0

local function local_setFail(errorStr)
    isFailed = true
    totalFailCases = totalFailCases + 1
    if errorStr then
        printLog("Error : " .. errorStr)
    end
    setFailState()
    return 
end

local macDaFlood = "ffffffffffff"
local macSaPort6 = "000999999999"
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f" 
local function basic_packet_flood_ingress_port_6(tagStr)
    if tagStr == nil then
        tagStr = ""
    end
    return macDaFlood .. macSaPort6 .. tagStr .. packetPayload
end
local vlanId = 1
local trunkId = 15
local function build_edsa_ingress_port_6(ingressTaggingType,tagStr)
    local srcIsTagged,tag0TpidIndex
    if ingressTaggingType == "untagged" then
        srcIsTagged = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"
        tag0TpidIndex = 0
    elseif ingressTaggingType == "tag0" then
        srcIsTagged = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E"
        tag0TpidIndex = 1
    elseif ingressTaggingType == "tag1" then
        srcIsTagged = "CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E"
        tag0TpidIndex = 0-- no tag0 ... so used 0
    elseif ingressTaggingType == "tag0,tag1" then
        srcIsTagged = "CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E"
        tag0TpidIndex = 1
    else
        local_setFail("build_edsa_ingress_port_6 : unknown tag")
        return nil
    end

    if tagStr == nil then
        tagStr = ""
    end
    
    local hwDevNum = 0x321 --[[801 decimal]] --[[ other device in the 'stack' ]]
    
    local edsaStc = {
      dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                     vpt = 0,
                     vid = vlanId },
      dsaInfo = {
        forward = {
          tag0TpidIndex        = tag0TpidIndex, 
          srcHwDev             = hwDevNum,
          srcIsTagged          = srcIsTagged,
          srcIsTrunk           = true,
          isTrgPhyPortValid    = false,
          dstInterface         ={ type="CPSS_INTERFACE_VID_E", vlanId=vlanId },
          phySrcMcFilterEnable = true,
          skipFdbSaLookup      = true,

          source = {trunkId = trunkId},-- src eport
          origSrcPhy = {portNum = 0}   -- 12 bits in eDSA because muxed with origTrunkId
        }
      }
    }
    local rc, dsaStr =  wrlDxChDsaToString(devNum, edsaStc)
    if (rc ~=0) then
        local_setFail("build_edsa_ingress_port_6:" .. to_string(edsaStc))
        return
    end     
    
    local afterDsaStr = ""
    local length = string.len(tagStr)
    if length > 8 then
        -- remove first 8 nibbles (that disappear in the eDSA)
        afterDsaStr = string.sub(tagStr,9,length)
    end
    
    return dsaStr..afterDsaStr
end

local tag0Str = "B1000001"
local tag1Str = "91000001"
local eDsaStr_tag0_tag1 = build_edsa_ingress_port_6("tag0,tag1",tag0Str..tag1Str)

local function run_traffic_ingress_stack_port_tag0_tag1()
    local transmitInfo = {
        devNum = devNum,
        portNum = port6,
        pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(eDsaStr_tag0_tag1)}
    }
    local egressInfoTable = {
        -- expected egress port to flood
        {portNum = port1, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(nil--[[untagged]])}}, -- untagged
        {portNum = port2, packetCount = 0 },--trunk member (same trunkId as in the eDSA from stack port)
        {portNum = port3, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(tag0Str..tag1Str)}},-- defined 'Tag0,Tag1'
        {portNum = port4, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(tag1Str)}},-- defined 'Tag1' 
        {portNum = port5, packetCount = 0 },--trunk member (same trunkId as in the eDSA from stack port)
    }
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test passed \n")
    else
        printLog ("Test failed \n")
        local_setFail()
    end 
end

local function portIsolationEnable(allow)
    local port_isolation_command 
    if(allow) then
        port_isolation_command     = 
        [[
        do cpss-api call cpssDxChNstPortIsolationEnableSet devNum ${dev} enable true
        ]]
    else
        port_isolation_command     = 
        [[
        do cpss-api call cpssDxChNstPortIsolationEnableSet devNum ${dev} enable false
        ]]
    end

    executeStringCliCommands(port_isolation_command)
end


local function run_traffic_ingress_stack_port_no_port_isolation_tag0_tag1()
    local transmitInfo = {
        devNum = devNum,
        portNum = port6,
        pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(eDsaStr_tag0_tag1)}
    }
    local egressInfoTable = {
        -- expected egress port to flood
        {portNum = port1, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(nil--[[untagged]])}}, -- untagged
        {portNum = port2, packetCount = 1 },--ERROR not expected to get packet because : trunk member (same trunkId as in the eDSA from stack port)
        {portNum = port3, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(tag0Str..tag1Str)}},-- defined 'Tag0,Tag1'
        {portNum = port4, pktInfo = {fullPacket = basic_packet_flood_ingress_port_6(tag1Str)}},-- defined 'Tag1' 
        {portNum = port5, packetCount = 0 },--(filtered by 'designated table') that kept 'port2' to get this unwanted packet !)
    }
    
    -- disable the port isolation
    portIsolationEnable(false)
    
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test passed \n")
    else
        printLog ("Test failed \n")
        local_setFail()
    end 
    
    --restore the port isolation
    portIsolationEnable(true)
    
end


------------------------------ MAIN PART ---------------------------------------
-- check that the entry is in the FDB with the expected MAC-SA
local function checkFdbEntry(srcMacAddr)
    
    -- do visualization of the FDB table .. for this mac address
    local command_showFdbEntry = 
    [[
    end
    show mac address-table all device ${dev}
    end
    ]]
    
    local expectedVid  = vlanId

    executeStringCliCommands(command_showFdbEntry)
    
    local mac_filter = 
    { ["key"] =
        { ["entryType"] = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
          ["key"] = 
           { ["macVlan"] =
               {
                  ["macAddr"]               = srcMacAddr,
                  ["vlanId"]                = expectedVid
               }
           }
        }
    } 

    local result, values, mac_entry_index, dummyVidx =
        wrlCpssDxChBrgIsEntryExists(devNum, 0--[[index -> ignored]], mac_filter) 
    if result ~= 0 or not values then
        local_setFail("FDB : Entry not found " .. to_string(mac_filter))
        return
    end
        
    local rc, values, skip_valid_aged, mac_entry = 
        next_valid_mac_entry_get(devNum,mac_entry_index-1)         

    -- check the entry for trunkId
    if (rc ==0 --[[GT_OK]] and mac_entry ~= nil) then 
        if mac_entry.dstInterface.type    ~= "CPSS_INTERFACE_TRUNK_E" or
           mac_entry.dstInterface.trunkId ~= trunkId then
           -- The entry not registered on the 'trunkId' 
            local_setFail("FDB : Mac entry association not as expected on trunk" .. to_string(mac_entry.dstInterface))
            return
        end
    else
        local_setFail("FDB : Mac entry not retrieved")
        return
    end
    
    return
end 

local fullResults = {}

local function checkFail(section)
    if isFailed then
        printLog("ERROR : section :",section,"FAILED")
        fullResults[section] = "FAILED"
    else
        printLog("section : ",section,"PASSED")
        fullResults[section] = "PASSED"
    end

    -- clear the fail indication 
    isFailed = false
end

local function run_test(test,section)
    printLog("Start : section",section)
    test()
    printLog("Ended : section",section)
    
    checkFail(section)
end

-- load configurations
executeLocalConfig(configFile)

-- run traffic ingress from the remote physical port
local section,test

section = "ingress stack port with 'src trunkId' packet to flood to all other ports"
test = run_traffic_ingress_stack_port_tag0_tag1
run_test(test,section)

section = "check FDB entry on trunkId " .. trunkId
checkFdbEntry(macSaPort6)
checkFail(section)

section = "special check : disable 'port isolation' (the 'WA') , and check that trunk member gets flood !"
test = run_traffic_ingress_stack_port_no_port_isolation_tag0_tag1
run_test(test,section)

printLog("\n\n")
printLog("test checks ended , start 'restore configurations'")
printLog("\n\n")
--flush the FDB from dynamic entries
executeLocalConfig(flush_fdb_only_dynamic)
-- unload configurations
executeLocalConfig(deconfigFile)

printLog("\n\n\n")
if totalFailCases ~= 0 then
    printLog("Total of "..to_string(totalFailCases).." Fails (see summary below)")
else
    printLog("All cases PASSED (see summary below)")
end

printLog("Summary",to_string(fullResults))
