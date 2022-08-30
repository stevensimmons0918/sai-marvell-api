--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc_arp_index.lua
--* NOTE: logic copied from ipv4_routing_basic.lua + CNC bounding to ARP index
--*
--* DESCRIPTION:
--*       Test basic IPv4 unicast inter VLAN routing + CNC bounding to ARP index
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pkt
local pktInfo
local payload
local status

local transmitInfo;
local egressInfoTable;
local rc = 0;
local nextHopIndex = 101

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)

-- function return 'true' if we expect to get no cnc counting on ARP index !
local function is_cnc_arp_index_fail()
    -- expect fail on wrong counting at wrong index on this case
    return (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E")
end

--SUPPORTED_FEATURE_DECLARE(devNum, "IP_UC_ROUTING")
local firstArpIndex = 2
--generate test packet
do
    status, payload = pcall(dofile, "dxCh/examples/packets/ipv4_routing_basic.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

printLog("========================== Set configuration ===========================")

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_basic.txt")

arp_table_print()

local configuration_5 = [[
end
configure
counters ARP_NAT device ]] .. to_string(devNum) .. [[

//counters nat-offset device 0 offset 0
end
]]
local configuration_6 = [[
end
configure
no counters ARP_NAT device ]] .. to_string(devNum) .. [[

//no counters nat-offset device 0
end
]]


--Get hwDevNum
local ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgHwDevNumGet")
    setFailState()
end
local hw_device_id_number = val["hwDevNumPtr"];

--Write MAC entry
local mac_entry = {
        key = {
            entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
            key = {
                macVlan = {
                    macAddr = "00:00:00:01:01:01",
                    vlanId = 200
                }
            }
        },
        dstInterface = {
            type = "CPSS_INTERFACE_PORT_E",
            devPort = {
                hwDevNum = hw_device_id_number,
                portNum = 61 -- tunnel, will be overriden
            }
        },
        isStatic = true,
        daRoute  = true,
        daCommand = "CPSS_MAC_TABLE_FRWRD_E"
    }

local ret, val = mac_entry_set(devNum, mac_entry);
if ret ~= 0 then
    printLog ("Error in mac_entry_set")
    setFailState()
end

-- Set ECMP indirect next hop entry to verify that
-- next hop entry directly taken from next hop table
if is_sip_5_25(devNum) then
    local ret, val = myGenWrapper("cpssDxChIpEcmpIndirectNextHopEntrySet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","indirectIndex", nextHopIndex},
        {"IN","GT_U32","nextHopIndex", nextHopIndex+1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpEcmpIndirectNextHopEntrySet")
        setFailState()
    end
end

--configuration 5: check CNC counters
executeStringCliCommands(configuration_5)

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to egress out port 2.

transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payload} }

egressInfoTable = {
    {portNum = port2  , packetCount = 1 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- check CNC counters
local params = {}
params["devID"] = devNum
params["arpNatCncIdx"] = firstArpIndex
local numberOfPackets, numberOfBytes
local ret, showVals = showCountersArpNat(params)
if (ret == false) then
    printLog ("\n Error while reading ARP counter \n")
    setFailState()
else
    if showVals["pass"] == nil or showVals["pass"][1] == nil then
        numberOfPackets = 0
        numberOfBytes = 0
    else
        numberOfPackets = showVals["pass"][1]["Packet-Counter"]
        numberOfBytes = showVals["pass"][1]["Byte-count counter"]
    end

    if not (numberOfPackets >= 1 and numberOfBytes >= 64) then
        printLog ("\n Error : ARP counter was not incremented \n")

        if is_cnc_arp_index_fail() then
            printLog ("\n ERROR : Be aware to ERRATUM on Falcon regarding ARP/TS/TRG_EPORT and CNC \n")
        else
            setFailState()
        end
    end
end

--configuration 6: remove CNC counters
executeStringCliCommands(configuration_6)

printLog("======================== Restore configuration =========================")

--Delete MAC entry
local ret, val = mac_entry_delete(devNum, mac_entry);
if ret ~= 0 then
    printLog ("Error in mac_entry_delete")
    setFailState()
end

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_basic_deconfig.txt")


