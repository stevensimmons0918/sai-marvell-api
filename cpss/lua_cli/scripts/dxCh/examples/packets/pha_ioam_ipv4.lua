--
-- packets for test of ipv4-udp-vxlan-gpe-ioam test
-- packets for 'network port' - ethernet (UNI port)
-- packets for 'carrier port' - ethernet over ipv4-udp-vxlan-gpe-ioam (NNI port)
-- based on 'vxlan.lua' packets
-------------------------------------
--  about the test :
--  'ingress device' test case 
--  port1 is UNI (network)
--  port2 is UNI (network)
--  port3 is NNI (egress TS - tunnel start , on ingress TT - tunnel terminated)
--  port4 is NNI (egress TS - tunnel start , on ingress TT - tunnel terminated)
--  'direction 1' is from port 1 to all other ports
--  
--  'transit device' test case 
--  port5 is transit (with tunnel , not TTI, no TS)
--  port6 is transit (with tunnel , not TTI, no TS)
--  'direction 1' is from port 5 to port 6 (always 'known UC' - mac Da of 'next hop')
--  
-------------------------------------
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]
local port6   = devEnv.port[6]

local uni_mac_da   = "00112233445A";
local uni_mac_sa   = "000000240001";
local uni_mac_vtag = "81000065";
local uni_eth_type = "3333";
local uni_l2_payload =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"
-- network port
local vxlan_direction1_packet_from_port_1 =
    uni_mac_da .. uni_mac_sa .. uni_mac_vtag .. uni_eth_type .. uni_l2_payload;

local nni_mac_da_port3   = "000000B01A11";
local nni_mac_da_port4   = "000000B01A12";
local nni_mac_sa   = "000000102030";
local ipv4_eth_type = "0800"
local ipv4_hdr_part1 =  "4500" -- version 5, header length - words, DSCP 0, ECN - 0
-- ommited 16 bit total length
local ipv4_hdr_part2 =  ""
    .. "00000000" -- identification(16), flags(3), fragment offset(13)
    .. "4011" -- time to live(8), UDP-protocol(8)
local ipv4_hdr_before_cs_port_3,ipv4_hdr_before_cs_port_4;
local ipv4_hdr_after_cs_port_3,ipv4_hdr_after_cs_port_4;
local nni_port3_local_ip  = "C0B00A0A"
local nni_port3_remote_ip = "C0B00A0B"
local nni_port4_local_ip  = "C0B00A0A"
local nni_port4_remote_ip = "C0B00A0C"

local mac_sa_da_size    = 12
local ether_type_size   = 2
local ipv4_header_size  = 20
local udp_header_size   = 8
local vxlan_header_size = 8
local ioam_header_size  = 8

local function stringToByteCount(stringInfo)
    return math.floor(string.len(stringInfo) / 2)
end

local function printLogPacket(name,packetPart)
    printLog(name,to_string(packetPart))
end

local nni_untagged_payload =
    uni_mac_da .. uni_mac_sa .. --[[untagged]]  uni_eth_type .. uni_l2_payload;

local nni_tagged_payload =
    uni_mac_da .. uni_mac_sa .. uni_mac_vtag .. uni_eth_type .. uni_l2_payload;


local phaThreadId_port3 = 156 --used in the configuration file
local isOk , phaEntry_port3 = lua_cpssDxChPhaThreadIdEntryGet(nil--[[command_data]],devNum,phaThreadId_port3)
local phaThreadId_port4 = 200 --used in the configuration file
local isOk , phaEntry_port4 = lua_cpssDxChPhaThreadIdEntryGet(nil--[[command_data]],devNum,phaThreadId_port4)

local phaEntry_port3_ioam_ipv4 = phaEntry_port3.extInfo.ioamIngressSwitchIpv4
local phaEntry_port4_ioam_ipv4 = phaEntry_port4.extInfo.ioamIngressSwitchIpv4

--printLog("phaEntry_port3",to_string(phaEntry_port3))
--printLog("phaEntry_port4",to_string(phaEntry_port4))
--[[
      Type2=109,
      node_id=104,
      Type1=105,
      Reserved1=107,
      IOAM_Trace_Type=46688,
      Hop_Lim=103,
      Maximum_Length=101,
      Flags=102,
      IOAM_HDR_len2=110,
      Next_Protocol1=108,
      IOAM_HDR_len1=106,
      Next_Protocol2=112,
      Reserved2=111
]]--    
    
-- the VSI (VNI) value that used in the configuration file
-- VXLAN Network Identifier (VNI)
local vniValue  = 1000000 --[["0F4240"]]    
-- value of 24 bits (6 nibbles)
local vniString = string.format("%06X", vniValue)
--[[
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |R|R|Ver|I|P|B|O|       Reserved                |Next Protocol  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                VXLAN Network Identifier (VNI) |   Reserved    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
-- set next bits in the 'vxlan gpe' header :
-- I (bit 4) , P (bit 5)
local flags = "0C"              -- 8 bits
local Reserved_16_bits = "0000" -- 16 bits
local Reserved_8_bits  = "00"   -- 8 bits 
local Next_Protocol_ioam_trace   = --[[8 bits]] string.format("%02X", getVxlanGpeNextProtocol("ioam_trace"))  --ioam_Trace protocol
--[[8 bytes]]
local nni_vxlan_gpe_header = --[[first 4 bytes]] flags .. Reserved_16_bits .. Next_Protocol_ioam_trace .. 
                             --[[next  4 bytes]] vniString .. Reserved_8_bits
--[[
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Type =IOAM    | IOAM HDR len  | Reserved      | Next Protocol |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         IOAM-Trace-Type       |NodeLen|  Flags  | RemainingLen |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
-- port 3
--[[word 0]]
local word0_ioam_trace_header_port_3 = string.format("%02X",phaEntry_port3_ioam_ipv4.Type1) .. 
                                       string.format("%02X",phaEntry_port3_ioam_ipv4.IOAM_HDR_len1) .. 
                                       string.format("%02X",phaEntry_port3_ioam_ipv4.Reserved1) .. 
                                       string.format("%02X",phaEntry_port3_ioam_ipv4.Next_Protocol1)
                                       
local word1_ioam_trace_header_port_3 = string.format("%04X",phaEntry_port3_ioam_ipv4.IOAM_Trace_Type) ..
                                       string.format("%02X",phaEntry_port3_ioam_ipv4.Maximum_Length) ..
                                       string.format("%02X",phaEntry_port3_ioam_ipv4.Flags)
--[[word 1]]
-- TraceType_HopLimAndNodeId   = true 
-- TraceType_IngEgrInterface   = false
-- TraceType_TimestampSec      = true 
-- TraceType_TimestampNanoSec  = true 
-- TraceType_TransitDelay      = false
-- TraceType_AppData           = true 
-- TraceType_QueueDepth        = true 
-- TraceType_OpaqueData        = false
-- TraceType_HopLimAndNodeIdWide= false
-- TraceType_IngEgrInterfaceWide= true
-- TraceType_AppDataWide        = true
--[[8 bytes]]
local ioam_trace_header_port_3 = word0_ioam_trace_header_port_3 .. 
                                 word1_ioam_trace_header_port_3
                          
                          
--[[
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Hop_Lim     |              node_id                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local Hop_Lim_port_3 = phaEntry_port3_ioam_ipv4.Hop_Lim -- from vxlan_gre_ioam.txt
local node_id_port_3 = phaEntry_port3_ioam_ipv4.node_id -- from vxlan_gre_ioam.txt
local TraceType_HopLimAndNodeId_port_3  = string.format("%02X", Hop_Lim_port_3) .. string.format("%06X", node_id_port_3)
local TraceType_TimestampSec_port_3     = "12345678"--[[unknown !!!]]
local TraceType_TimestampNanoSec_port_3 = "87654321"--[[unknown !!!]]
local TraceType_AppData_port_3          = "00000000"--the FW set it to Zero !
local TraceType_QueueDepth_port_3       = "00000000"--the FW set it to Zero !

--[[
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       ingress_if_id                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       egress_if_id                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local TraceType_IngEgrInterfaceWide_port_3 = string.format("%08X", port1) ..
                                             string.format("%08X", port3)
local TraceType_AppDataWide_port_3  = "00000000"--the FW set it to Zero ! 
                                   .. "00000000"--the FW set it to Zero !

--printLogPacket("TraceType_HopLimAndNodeId_port_3",TraceType_HopLimAndNodeId_port_3)
--printLogPacket("TraceType_TimestampSec_port_3",TraceType_TimestampSec_port_3)
--printLogPacket("TraceType_TimestampNanoSec_port_3",TraceType_TimestampNanoSec_port_3)
--printLogPacket("TraceType_AppData_port_3",TraceType_AppData_port_3)
--printLogPacket("TraceType_QueueDepth_port_3",TraceType_QueueDepth_port_3)
--printLogPacket("TraceType_IngEgrInterfaceWide_port_3",TraceType_IngEgrInterfaceWide_port_3)
--printLogPacket("TraceType_AppDataWide_port_3",TraceType_AppDataWide_port_3)
                                   
                                   
local ioam_added_fields_port_3 = TraceType_HopLimAndNodeId_port_3 ..
                                 TraceType_TimestampSec_port_3 ..
                                 TraceType_TimestampNanoSec_port_3 ..
                                 TraceType_AppData_port_3 ..
                                 TraceType_QueueDepth_port_3 ..
                                 TraceType_IngEgrInterfaceWide_port_3 ..
                                 TraceType_AppDataWide_port_3

                          
--port 4 
local word0_ioam_trace_header_port_4 = string.format("%02X",phaEntry_port4_ioam_ipv4.Type1) .. 
                                       string.format("%02X",phaEntry_port4_ioam_ipv4.IOAM_HDR_len1) .. 
                                       string.format("%02X",phaEntry_port4_ioam_ipv4.Reserved1) .. 
                                       string.format("%02X",phaEntry_port4_ioam_ipv4.Next_Protocol1)
                                       
local word1_ioam_trace_header_port_4 = string.format("%04X",phaEntry_port4_ioam_ipv4.IOAM_Trace_Type) ..
                                       string.format("%02X",phaEntry_port4_ioam_ipv4.Maximum_Length) ..
                                       string.format("%02X",phaEntry_port4_ioam_ipv4.Flags)



--[[word 1]]
-- TraceType_HopLimAndNodeId   = false 
-- TraceType_IngEgrInterface   = true
-- TraceType_TimestampSec      = false 
-- TraceType_TimestampNanoSec  = true 
-- TraceType_TransitDelay      = false
-- TraceType_AppData           = true 
-- TraceType_QueueDepth        = false 
-- TraceType_OpaqueData        = false
-- TraceType_HopLimAndNodeIdWide= true
-- TraceType_IngEgrInterfaceWide= false
-- TraceType_AppDataWide        = true
local ioam_trace_type_port_4        = "54A0"      --16 bits : set 5 bits: 1,3,5,8,10

--[[8 bytes]]
local ioam_trace_header_port_4 = word0_ioam_trace_header_port_4 ..
                            --word1
                                word1_ioam_trace_header_port_4

--[[
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Hop_Lim     |              node_id                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local Hop_Lim_port_4 = phaEntry_port4_ioam_ipv4.Hop_Lim -- from vxlan_gre_ioam.txt
local node_id_port_4 = phaEntry_port4_ioam_ipv4.node_id -- from vxlan_gre_ioam.txt

--[[
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     ingress_if_id             |         egress_if_id          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local ingress_if_id_port_1_to_port_4    = port1 --[[ ingress physical interface]]
local egress_if_id_port_4               = port4 --[[ egress  physical interface]]
local TraceType_IngEgrInterface_port_4  = string.format("%04X%04X", ingress_if_id_port_1_to_port_4 , egress_if_id_port_4)
local TraceType_TimestampNanoSec_port_4 = "87654321"--[[unknown !!!]]
local TraceType_AppData_port_4          = "00000000"--the FW set it to Zero !
--[[
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Hop_Lim     |              node_id                          ~
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   ~                         node_id (contd)                       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local TraceType_HopLimAndNodeIdWide_port_4 = string.format("%02X", Hop_Lim_port_4) .. "000000" ..
                                             "00" .. string.format("%06X", node_id_port_4)
                                             
-- the format of the field is not as expected above
local bug_in_HopLimAndNodeIdWide = true

if bug_in_HopLimAndNodeIdWide then
    TraceType_HopLimAndNodeIdWide_port_4 = string.format("%02X", Hop_Lim_port_4) .. string.format("%06X", node_id_port_4) ..
                                             "00000000"
end
                                             
                                             
local TraceType_AppDataWide_port_4  = "00000000"--the FW set it to Zero ! 
                                   .. "00000000"--the FW set it to Zero !
                                   
local ioam_added_fields_port_4 = TraceType_IngEgrInterface_port_4 ..
                                 TraceType_TimestampNanoSec_port_4 ..
                                 TraceType_AppData_port_4 ..
                                 TraceType_HopLimAndNodeIdWide_port_4 ..
                                 TraceType_AppDataWide_port_4
                          
--[[
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Type=IOAM_E2E  |   IOAM HDR len  |  Reserved   | Next Protocol |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      E2E Option data field determined by IOAM-E2E-Type        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--

--[[word 0]]
local word0_ioam_e2e_header_port_4 = string.format("%02X",phaEntry_port4_ioam_ipv4.Type2) .. 
                                     string.format("%02X",phaEntry_port4_ioam_ipv4.IOAM_HDR_len2) .. 
                                     string.format("%02X",phaEntry_port4_ioam_ipv4.Reserved2) .. 
                                     string.format("%02X",phaEntry_port4_ioam_ipv4.Next_Protocol2)
--[[word 1]]
local word1_ioam_e2e_header_port_4 = "00000000"--from the 'TS profile' and unchanged by the FW
--[[word 2]]
local word2_ioam_e2e_header_port_4 = "00000000"-- set to LM counter by the FW 'muxed' with 'timestamp'
-- so if not set bind to 'LM counter' it will hold 'timestamp'

local ioam_e2e_header_port_4  = word0_ioam_e2e_header_port_4 .. 
                                word1_ioam_e2e_header_port_4 ..
                                word2_ioam_e2e_header_port_4

--------------------------------------------------------------
local vxlan_direction1_expected_packet_to_port_2 = nni_untagged_payload;

local nni_udp_total_length_tagged    = udp_header_size  + 
    stringToByteCount(nni_vxlan_gpe_header) + 
    stringToByteCount(ioam_trace_header_port_3) +
    stringToByteCount(ioam_added_fields_port_3) + 
    stringToByteCount(nni_tagged_payload)
local nni_udp_total_length_untagged    = udp_header_size  + 
    stringToByteCount(nni_vxlan_gpe_header) + 
    stringToByteCount(ioam_trace_header_port_4) +
    stringToByteCount(ioam_added_fields_port_4) + 
    stringToByteCount(ioam_e2e_header_port_4) + 
    stringToByteCount(nni_untagged_payload)
local nni_ipv4_total_length_tagged   = ipv4_header_size + nni_udp_total_length_tagged
local nni_ipv4_total_length_untagged = ipv4_header_size + nni_udp_total_length_untagged


ipv4_hdr_before_cs_port_3 = ipv4_hdr_part1 .. string.format("%04X", nni_ipv4_total_length_tagged) .. ipv4_hdr_part2;
ipv4_hdr_before_cs_port_4 = ipv4_hdr_part1 .. string.format("%04X", nni_ipv4_total_length_untagged) .. ipv4_hdr_part2;
ipv4_hdr_after_cs_port_3 = nni_port3_local_ip .. nni_port3_remote_ip
ipv4_hdr_after_cs_port_4 = nni_port4_local_ip .. nni_port4_remote_ip

local vxlan_gpe_udpPort = getUdpPort_vxlan_gpe()

local nni_udp_header_tagged = 
       "0000" --Src port(16) (not compared),
    .. string.format("%02X",vxlan_gpe_udpPort)  -- "12B6" Dst port(16)
    .. string.format("%04X", nni_udp_total_length_tagged)
    .. "0000" -- Check Sum Placeholder

--printLogPacket("nni_udp_header_tagged",nni_udp_header_tagged);
--printLogPacket("nni_vxlan_gpe_header",nni_vxlan_gpe_header);
--printLogPacket("ioam_trace_header_port_3",ioam_trace_header_port_3);
--printLogPacket("ioam_added_fields_port_3",ioam_added_fields_port_3);
local vxlan_direction1_expected_packet_to_port_3 = 
       nni_mac_da_port3 .. nni_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs_port_3 .. "0000"--[[checksum]] .. ipv4_hdr_after_cs_port_3
    .. nni_udp_header_tagged
    .. nni_vxlan_gpe_header
    .. ioam_trace_header_port_3
    .. ioam_added_fields_port_3
    .. nni_tagged_payload;

local nni_udp_header_untagged = 
       "0000" --Src port(16) (not compared),
    .. string.format("%02X",vxlan_gpe_udpPort) -- "12B6" Dst port(16)
    .. string.format("%04X", nni_udp_total_length_untagged)
    .. "0000" -- Check Sum Placeholder

--printLogPacket("nni_udp_header_untagged",nni_udp_header_untagged);
--printLogPacket("nni_vxlan_gpe_header",nni_vxlan_gpe_header);
--printLogPacket("ioam_trace_header_port_4",ioam_trace_header_port_4);
--printLogPacket("ioam_added_fields_port_4",ioam_added_fields_port_4);
--printLogPacket("ioam_e2e_header_port_4",ioam_e2e_header_port_4);


local vxlan_direction1_expected_packet_to_port_4 = 
       nni_mac_da_port4 .. nni_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs_port_4 .. "0000"--[[checksum]] .. ipv4_hdr_after_cs_port_4
    .. nni_udp_header_untagged
    .. nni_vxlan_gpe_header
    .. ioam_trace_header_port_4
    .. ioam_added_fields_port_4
    .. ioam_e2e_header_port_4
    .. nni_untagged_payload;

local vxlan_transit_direction1_packet_from_port_5 = vxlan_direction1_expected_packet_to_port_3

-- function insert into 'string_to_insert_to' a string 'string_to_be_added'
-- insertion point is 'start_point' (from start of 'string_to_insert_to')
local function insertIntoString(string_to_insert_to,string_to_be_added,start_point)
    --printLogPacket("string_to_insert_to",string_to_insert_to)
    --printLogPacket("string_to_be_added",string_to_be_added)
    --printLog("start_point",to_string(start_point))
    local part1 = string.sub(string_to_insert_to,1, start_point)
    local part2 = string.sub(string_to_insert_to,start_point+1)
    
    return part1 .. string_to_be_added .. part2
end

local phaThreadId_port5 = 150 --used in the configuration file
local isOk , phaEntry_port5 = lua_cpssDxChPhaThreadIdEntryGet(nil--[[command_data]],devNum,phaThreadId_port5)
local phaThreadId_port6 = 160 --used in the configuration file
local isOk , phaEntry_port6 = lua_cpssDxChPhaThreadIdEntryGet(nil--[[command_data]],devNum,phaThreadId_port6)

local phaEntry_port5_ioam_ipv4 = phaEntry_port5.extInfo.ioamTransitSwitchIpv4
local phaEntry_port6_ioam_ipv4 = phaEntry_port6.extInfo.ioamTransitSwitchIpv4
--[[
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Hop_Lim     |              node_id                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
]]--
local ttl_in_ipv4_header = 0x40
local Hop_Lim_port_6 = ttl_in_ipv4_header -- unlike the 'ingress device' taken from the 'ttl' in the descriptor and not from the 'configuration'
local node_id_port_6 = phaEntry_port6_ioam_ipv4.node_id -- from vxlan_gre_ioam.txt
local TraceType_HopLimAndNodeId_port_6  = string.format("%02X", Hop_Lim_port_6) .. string.format("%06X", node_id_port_6)

local TraceType_TimestampSec_port_6     = "11223344"--[[unknown !!!]]
local TraceType_TimestampNanoSec_port_6 = "55667788"--[[unknown !!!]]
local TraceType_AppData_port_6          = "00000000"--the FW set it to Zero !
local TraceType_QueueDepth_port_6       = "00000000"--the FW set it to Zero !
local TraceType_IngEgrInterfaceWide_port_6 = string.format("%08X", port5) ..
                                             string.format("%08X", port6)
local TraceType_AppDataWide_port_6  = "00000000"--the FW set it to Zero ! 
                                   .. "00000000"--the FW set it to Zero !

--printLogPacket("TraceType_HopLimAndNodeId_port_6",TraceType_HopLimAndNodeId_port_6)
--printLogPacket("TraceType_TimestampSec_port_6",TraceType_TimestampSec_port_6)
--printLogPacket("TraceType_TimestampNanoSec_port_6",TraceType_TimestampNanoSec_port_6)
--printLogPacket("TraceType_AppData_port_6",TraceType_AppData_port_6)
--printLogPacket("TraceType_QueueDepth_port_6",TraceType_QueueDepth_port_6)
--printLogPacket("TraceType_IngEgrInterfaceWide_port_6",TraceType_IngEgrInterfaceWide_port_6)
--printLogPacket("TraceType_AppDataWide_port_6",TraceType_AppDataWide_port_6)
                                   
local ioam_added_fields_port_6 = TraceType_HopLimAndNodeId_port_6 ..
                                 TraceType_TimestampSec_port_6 ..
                                 TraceType_TimestampNanoSec_port_6 ..
                                 TraceType_AppData_port_6 ..
                                 TraceType_QueueDepth_port_6 ..
                                 TraceType_IngEgrInterfaceWide_port_6 ..
                                 TraceType_AppDataWide_port_6

local vxlan_transit_direction1_packet_to_port_6_addition = 
    ioam_added_fields_port_6
    
-- part 1 is till end of 'ioam_trace_header_port_3' part (BEFORE the ioam added fields from 'previous device')
local port_5_part_1_length    = 
    mac_sa_da_size   +
    ether_type_size  +
    ipv4_header_size +
    udp_header_size  + 
    stringToByteCount(nni_vxlan_gpe_header) + 
    stringToByteCount(ioam_trace_header_port_3)

-- add the extra ioam fields that 'transit' will add on top of those from 'ingress device'
local vxlan_transit_direction1_packet_to_port_6 = insertIntoString(
    vxlan_transit_direction1_packet_from_port_5 , 
    vxlan_transit_direction1_packet_to_port_6_addition,
    2 * port_5_part_1_length);

local function addLength(oldString,numBytesAdded)
    return numBytesAdded + tonumber("0x" .. oldString)
end
    
-- update the ipv4 'total length' and the IOAM 'IOAM HDR len'
local function updateByteCountInPacketFields(packetString,ipv4Offset,udpOffset,ioamOffset,numBytesAdded)
    local ipv4_total_length_offset = ipv4Offset + 2
    local udp_length_offset        = udpOffset  + 4
    local ioam_length_offset       = ioamOffset + 1

    --printLogPacket("packetString",packetString)
    --printLog("ipv4_total_length_offset",ipv4_total_length_offset)
    --printLog("ioamOffset",ioamOffset)

    local old_ipv4_total_length_str  = string.sub(packetString , 1+ ipv4_total_length_offset * 2 , (ipv4_total_length_offset * 2 + 2 * 2))
    local old_udp_length_str         = string.sub(packetString , 1+ udp_length_offset        * 2 , (udp_length_offset        * 2 + 2 * 2))
    local old_ioam_header_length_str = string.sub(packetString , 1+ ioam_length_offset       * 2 , (ioam_length_offset       * 2 + 1 * 2))

    --printLogPacket("old_ipv4_total_length_str",old_ipv4_total_length_str)
    --printLogPacket("old_ioam_header_length_str",old_ioam_header_length_str)

    local new_ipv4_total_length_str  = addLength(old_ipv4_total_length_str ,numBytesAdded)
    local new_udp_length_str         = addLength(old_udp_length_str        ,numBytesAdded)
    local new_ioam_header_length_str = addLength(old_ioam_header_length_str,numBytesAdded / 4--[[words!]])
    
    new_ipv4_total_length_str  = string.format("%04X",new_ipv4_total_length_str )
    new_udp_length_str         = string.format("%04X",new_udp_length_str        )
    new_ioam_header_length_str = string.format("%02X",new_ioam_header_length_str)
    
    local part1 = string.sub(packetString,1                                    ,(ipv4_total_length_offset * 2))
    local part2 = string.sub(packetString,1+(ipv4_total_length_offset + 2) * 2 ,(udp_length_offset        * 2))
    local part3 = string.sub(packetString,1+(udp_length_offset        + 2) * 2 ,(ioam_length_offset       * 2))
    local part4 = string.sub(packetString,1+(ioam_length_offset       + 1) * 2 , nil                          )
    
    --printLogPacket("new_ipv4_total_length_str",new_ipv4_total_length_str)
    --printLogPacket("new_ioam_header_length_str",new_ioam_header_length_str)

    --printLogPacket("part1",part1)
    --printLogPacket("part2",part2)
    --printLogPacket("part3",part3)
    
    return part1 .. new_ipv4_total_length_str  .. 
           part2 .. new_udp_length_str         ..
           part3 .. new_ioam_header_length_str ..
           part4
    
end

local numBytesAdded_port_6 = 9*4 -- 7 fields (2 are wide) --> 9 'words'
local ipv4Offset_port_6 = stringToByteCount(nni_mac_da_port3 .. nni_mac_sa .. ipv4_eth_type)
local udpOffset_port_6  = ipv4Offset_port_6 + ipv4_header_size
local ioamOffset_port_6 = udpOffset_port_6  + udp_header_size + vxlan_header_size

vxlan_transit_direction1_packet_to_port_6 = updateByteCountInPacketFields(
    vxlan_transit_direction1_packet_to_port_6 , 
    ipv4Offset_port_6 , 
    udpOffset_port_6 , 
    ioamOffset_port_6 ,
    numBytesAdded_port_6)

--printLogPacket("vxlan_transit_direction1_packet_to_port_6",vxlan_transit_direction1_packet_to_port_6)
--printLogPacket("lower",string.lower(vxlan_transit_direction1_packet_to_port_6))
    
return
{
    -- ingress device : 
    -- port1 send 'flood' to other ports 
    port1_flood = { sender = string.lower(vxlan_direction1_packet_from_port_1),
              port2 = string.lower(vxlan_direction1_expected_packet_to_port_2),
              port3 = string.lower(vxlan_direction1_expected_packet_to_port_3),
              port4 = string.lower(vxlan_direction1_expected_packet_to_port_4)
            },
            
            
    -- transit device : 
    -- port5 send 'known UC' to port6
    -- the packet that ingress port5 , is the one that egress from port 3 ! ('transit device' got packet from 'ingress device')
    port5_transit = { sender = string.lower(vxlan_transit_direction1_packet_from_port_5),
                       port6 = string.lower(vxlan_transit_direction1_packet_to_port_6)
            }
}


