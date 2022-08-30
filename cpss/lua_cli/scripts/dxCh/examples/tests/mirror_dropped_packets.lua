--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mirror_dropped_packets.lua
--*
--* DESCRIPTION:
--*       The test for testing mirror dropped packets
--*       NOTE : the feature of 'mirror-on-drop' is NOT used
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6") 

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pkt
local pktInfo
local payloads
local status

executeLocalConfig("dxCh/examples/configurations/mirror_dropped_packets.txt")

local transmitInfo;
local egressInfoTable;
local rc = 0;

local learnSaPacket =
    "00AABBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000" 

printLog("Do SA mac learning on port " .. to_string(port1))
transmitInfo = {portNum = port1 ,
                pktInfo = {fullPacket = learnSaPacket} }
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)


local singleDestinationPacket =
    "000000003402004444444444"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000" 

local function updateAction(newAction)
    local apiName = "cpssDxChPclRuleActionGet"
    local command_data = Command_Data()
    local ruleSize = "CPSS_PCL_RULE_SIZE_30_BYTES_E"
    local ruleIndex = 2100
    
    local isError , result, values = genericCpssApiWithErrorHandler(command_data,apiName,
        {
            {"IN", "GT_U8", "devNum", devNum},
            {"IN", "GT_U32", "tcamIndex", 0},
            {"IN", "CPSS_PCL_RULE_SIZE_ENT", "ruleSize", ruleSize},
            {"IN", "GT_U32", "ruleIndex", ruleIndex},
            {"IN", "CPSS_PCL_DIRECTION_ENT", "direction", "CPSS_PCL_DIRECTION_EGRESS_E"},
            {"OUT", "CPSS_DXCH_PCL_ACTION_STC", "actionPtr"}
        })
        
    local action = values.actionPtr

    if newAction.pktCmd then 
        action.pktCmd = newAction.pktCmd
    end
    
    apiName = "cpssDxChPclRuleActionUpdate"
    
    genericCpssApiWithErrorHandler(command_data,apiName,
        {
            {"IN", "GT_U8", "devNum", devNum},
            {"IN", "GT_U32", "tcamIndex", 0},
            {"IN", "CPSS_PCL_RULE_SIZE_ENT", "ruleSize", ruleSize},
            {"IN", "GT_U32", "ruleIndex", ruleIndex},
            {"IN", "CPSS_DXCH_PCL_ACTION_STC", "actionPtr", action}
        })
end


local function epclLookup(port,pktType,enable)
    local apiName = "cpssDxCh2EgressPclPacketTypesSet"
    local command_data = Command_Data()
    genericCpssApiWithErrorHandler(command_data,apiName,
        {
            {"IN", "GT_U8", "devNum", devNum},
            {"IN", "GT_PHYSICAL_PORT_NUM", "port", port},
            {"IN", "CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT", "pktType", pktType},
            {"IN",  "GT_BOOL", "enable", enable}
        })
end

printLog("Send UC traffic from port " .. to_string(port3) .. " to port " .. to_string(port1) .. " expect not egress - due to EPCL soft drop")

-- from port 3 to port 1
transmitInfo = {portNum = port3 ,
                pktInfo = {fullPacket = singleDestinationPacket} }
if is_sip_6_10(devNum) then
    egressInfoTable = {
         {portNum = port1 , packetCount = 0} -- the mirrored port (have EPCL rull that not allow to egress)
        ,{portNum = port2 , packetCount = 0} -- the analyzer port (not get copy because port1 not get packet)
        ,{portNum = port4 , packetCount = 0} -- the packet was targeted to port1
    }
else
    printLog("non-sip6.10 device allow to mirror also on hard/soft drop , so expect to egress on analyzer port " ..to_string(port2))
    egressInfoTable = {
         {portNum = port1 , packetCount = 0} -- the mirrored port (have EPCL rull that not allow to egress)
        ,{portNum = port2 , pktInfo = {fullPacket = singleDestinationPacket}} -- the analyzer port 
        ,{portNum = port4 , packetCount = 0} -- the packet was targeted to port1
    }
end
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


printLog("update the EPCL action : 'hard-drop' instead of 'soft-drop'")
updateAction({pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E"})

printLog("Send UC traffic from port " .. to_string(port3) .. " to port " .. to_string(port1) .. " expect not egress - due to EPCL hard drop")
if not is_sip_6_10(devNum) then
    printLog("non-sip6.10 device allow to mirror also on hard/soft drop , nut the EPCL ignored the mirror-to-analyzer for the 'hard-drop'")
    printLog("so unlike the previous case of soft drop , there is no replication to the analyzer port")
end

-- from port 3 to port 1
transmitInfo = {portNum = port3 ,
                pktInfo = {fullPacket = singleDestinationPacket} }
egressInfoTable = {
     {portNum = port1 , packetCount = 0} -- the mirrored port (have EPCL rull that not allow to egress)
    ,{portNum = port2 , packetCount = 0} -- the analyzer port (not get copy because port1 not get packet)
    ,{portNum = port4 , packetCount = 0} -- the packet was targeted to port1
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("update the EPCL action : 'forward' instead of 'hard-drop'")
updateAction({pktCmd = "CPSS_PACKET_CMD_FORWARD_E"})

-- from port 3 to port 1
transmitInfo = {portNum = port3 ,
                pktInfo = {fullPacket = singleDestinationPacket} }
egressInfoTable = {
     {portNum = port1 , pktInfo = {fullPacket = singleDestinationPacket}} -- the target port (also mirrored port)
    ,{portNum = port2 , pktInfo = {fullPacket = singleDestinationPacket}} -- the analyzer port 
    ,{portNum = port4 , packetCount = 0} -- the packet was targeted to port1
}

printLog("Send UC traffic from port " .. to_string(port3) .. " to port " .. to_string(port1) .. " expect to egress - due to EPCL forward + egress to analyzer port " .. port2)
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("disable EPCL lookup on port " .. port1)
epclLookup(port1,"CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E",false)

-- from port 3 to port 1
transmitInfo = {portNum = port3 ,
                pktInfo = {fullPacket = singleDestinationPacket} }
egressInfoTable = {
     {portNum = port1 , pktInfo = {fullPacket = singleDestinationPacket}} -- the target port (not mirroed because no action)
    ,{portNum = port2 , packetCount = 0} -- no analyzer because no epcl action to mirror
    ,{portNum = port4 , packetCount = 0} -- the packet was targeted to port1
}

printLog("Send UC traffic from port " .. to_string(port3) .. " to port " .. to_string(port1) .. " expect to egress - no analyzer port because no EPCL that send to analyzer")
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


printLog("re-enable the EPCL lookup (so the 'deconfig' file can disable it)")
epclLookup(port1,"CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E",true)

executeLocalConfig("dxCh/examples/configurations/mirror_dropped_packets_deconfig.txt")



