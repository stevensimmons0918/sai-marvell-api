--********************************************************************************
--*              (c), Copyright 2020, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* traffic_generator_with_options.lua
--*
--* DESCRIPTION:
--*       'emulate' external 'traffic generator' such as : smartBits / IXIA,
--*       to inject packet(s) into specific ingress port of the device with customized user input
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--*********************************************************
--********** Traffic-gen utility Help strings *************
--*********************************************************
local cust                    = "Send customized "
local help_string_traffic_gen = "Traffic generator"
local help_string_l2          = cust .. "L2 header,            Packet: <DA> <SA> <VLAN> <Custom-Tags> <Payload>"
local help_string_l3          = cust .. "L2 and L3 header,     Packet: <L2> + <IP Control flags> <SIP> <DIP> <Payload>"
local help_string_l4          = cust .. "L2, L3 and L4 header, Packet: <L2> + <L3> + <S-Port> <D-Port> <TCP/UDP Control Flags>"
local help_string_l2_in_l2    = cust .. "Packet: <L2>           + <L2>           + <Payload>"
local help_string_l2_in_l3    = cust .. "Packet: <L2> <L3>      + <L2>           + <Payload>"
local help_string_l2_in_l4    = cust .. "Packet: <L2> <L3> <L4> + <L2>           + <Payload>"
local help_string_l3_in_l2    = cust .. "Packet: <L2>           + <L2> <L3>      + <Payload>"
local help_string_l3_in_l3    = cust .. "Packet: <L2> <L3>      + <L2> <L3>      + <Payload>"
local help_string_l3_in_l4    = cust .. "Packet: <L2> <L3> <L4> + <L2> <L3>      + <Payload>"
local help_string_l4_in_l2    = cust .. "Packet: <L2>           + <L2> <L3> <L4> + <Payload>"
local help_string_l4_in_l3    = cust .. "Packet: <L2> <L3>      + <L2> <L3> <L4> + <Payload>"
local help_string_l4_in_l4    = cust .. "Packet: <L2> <L3> <L4> + <L2> <L3> <L4> + <Payload>"
local help_string_control_arp = cust .. "ARP Packet"
local help_string_control_icmp= cust .. "ICMP Packet"
CLI_addHelp("traffic", "traffic-gen tunnel", "Send customized(Inner/Outer) tunnel packets")
-- CLI_addHelp("traffic", "traffic-gen control", cust .. "control packets")
CLI_addHelp("traffic", "traffic-gen", cust .. "traffic with options")

-- L2 Default values
local def_SA            = "00 00 00 00 00 01"
local def_DA            = "00 00 00 DD DD DD"

-- L3(IPv4/IPv6) Default values
local def_ipv4_cntl_flag= "45 00 00 4a 00 00 00 00 40 2f 5e 6a"
local def_ipv4_sip      = "01.01.01.01"
local def_ipv4_dip      = "02.02.02.02"
local def_ipv6_cntl_flag= "60 00 00 00 00 74 11 14"
local def_ipv6_sip      = "3001:3002:3003:3004:3005:3006:3007:3008"
local def_ipv6_dip      = "2001:2002:2003:2004:2005:2006:2007:2008"

-- L4(TCP/UDP) Default values
local def_tcp_s_port    = "0x0001"
local def_tcp_d_port    = "0x0002"
local def_tcp_cntl_flags= "00 00 00 00 00 00 00 00"
local def_udp_s_port    = "0x2000"
local def_udp_d_port    = "0xbcef"
local def_udp_cntl_flags= "0000"

--*********************************************************
--************* Traffic-gen utility APIs ******************
--*********************************************************
local newLineSep = "\n    "
local cksm_note  = "(Checksum will be recomputed while sending packet)"
local function format_display(desc, frame_temp, frame_desc, frame_data)
    return (desc .. newLineSep .. frame_temp .. newLineSep .. frame_desc .. newLineSep .. frame_data .. newLineSep)
end

--*********************************************************
--************* Traffic-gen dictionaries ******************
--*********************************************************
local l2_custom_tag_temp            = "<---------4 Bytes----------> |<----------4 Bytes---------> |"
local l2_custom_tag_desc            = "<TPID(2Bytes)> <TCI(2Bytes)> |<TPID(2Bytes)> <TCI(2Bytes)> |"
local l2_custom_tag_format_1        = "88 A8          00 01         |81 00          00 02         |----- (Double Tag)"
local l2_custom_tag_format          = "81 00          00 01         |                             |----- (Single tag)" .. newLineSep .. l2_custom_tag_format_1
local ipv4_control_flag_temp        = "<--------------------4 Bytes-------------------> |<--------------4 Bytes-------------> |<-----------4 Bytes-------------> |"
local ipv4_control_flag_desc        = "<Version(4)/IHL(4)/TOS(6)/ECN(2)> <TotalLen(16)> |<ID(16)/Flags(3)/FragmentOffset(13)> |<TTL(8)> <PROTOCOL(8)> <cksm(16)> |"
local ipv4_control_flag_format      = "45 00                             00 4a          |00 00 00 00                          |40       2f            5e 6a      |"
local ipv6_control_flag_temp        = "<----------------4 Bytes-----------------> |<------------------4 Bytes-------------------> |"
local ipv6_control_flag_desc        = "<Version(4)/TOS(6)/ECN(2)> <FlowLabel(20)> |<payloadLen(16)> <NextHeader(8)> <HopLimit(8)> |"
local ipv6_control_flag_format      = "60 0                        0 00 00        |00 74            11              14            |"

local tcp_control_flag_temp         = "<-----4 Bytes------> |<------4 Bytes------> |<----------------------4 Bytes---------------------> |<---------4 Bytes--------> |"
local tcp_control_flag_desc         = "<Sequence Num(32)>   |<Ackn Num(32)>        |<Data Offset()>/<Reserved()>/<Flags()> <windows(16)> |<cksm(16)> <urgentPointer> |"
local tcp_control_flag_format       = "0000 0000 0000 0001  |0000 0000 0000 0002   |0000                                   0000          |EEEE       0000            |"
local udp_control_flag_temp         = "<-------4 Bytes------->"
local udp_control_flag_desc         = "<Length(16)> <cksm(16)>"
local udp_control_flag_format       = "0xFFFF       0xEEEE"

local TUNNEL_INNER_PKT_SUB_STRING   = "inner_"
local TUNNEL_OUTER_PKT_SUB_STRING   = "outer_"

CLI_type_dict["stream-options"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Packet stream type",
    enum = {
       ["incr-sa-mac"] = { value =   1  , help = "Incremental SA MAC address" },
       ["rand-sa-mac"] = { value =   2  , help = "Unique Random SA MAC address" },
       ["incr-sa-ip"]  = { value =   3  , help = "Incremental source IP address" },
       ["rand-sa-ip"]  = { value =   4  , help = "Random source IP address" },
       ["input-pkt"]   = { value =   5  , help = "Same input packet will be sent <count> number of times, default 1 packet" },
    }
}

-- L2 tag validation API
local function CLI_check_param_l2_tag(s, name, desc)
    local i, c, len

    len = string.len(s)
	if desc.min ~= nil and desc.min > len then
		return false, "L2 Tag too small"
	end
	if desc.max ~= nil and desc.max < len then
		return false, "L2 Tag too big"
	end
    if len ~= 8 and len ~= 16 and len ~= 24 and len ~= 32 then
		return false, "L2 Tag should be <4/8/12/16> Bytes in length"
	end
	for i = 1,string.len(s) do
		c = string.sub(s, 1, 1)
		if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
           (c >= 'A' and c <= 'F'))  ) then
			return false, "Found a non-numeric character"
		end
	end
	return true, s
end

-- L2(MAC, VLAN) Format
local function complete_param_l2(param, name, desc)
     local values =
        {{"81000001",  "Single Tagged"},
         {"8100000181000002",  "Double Tagged"}}
     return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["l2-custom-tag"] = {
    min=8,
    max=32,
    checker = CLI_check_param_l2_tag,
    complete = complete_param_l2,
    help = format_display("\nSample L2 custom tags:", l2_custom_tag_temp, l2_custom_tag_desc, l2_custom_tag_format)
}

-- IPv4 Format
function complete_param_ipv4(param, name, desc)
     local values =
        {{"4500004a00000000402f5e6a", "Sample IPv4 custom options"}}
     return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["cntr-flag-ipv4"] = {
    complete = complete_param_ipv4,
    help = format_display("\nSample IPv4 Control flags:" .. cksm_note, ipv4_control_flag_temp, ipv4_control_flag_desc, ipv4_control_flag_format)
}

-- IPv6 Format
function complete_param_ipv6(param, name, desc)
     local values =
        {{"6000000000741114", "Sample IPv6 custom options"}}
     return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["cntr-flag-ipv6"] = {
    complete = complete_param_ipv6,
    help = format_display("\nSample IPv6 Control flags:" .. cksm_note, ipv6_control_flag_temp, ipv6_control_flag_desc, ipv6_control_flag_format)
}

-- TCP Format
function complete_param_tcp(param, name, desc)
     local values =
        {{"0000000000000001000000000000000200000000EEEE0000 ", "Sample TCP custom options"}}
     return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["cntr-flag-tcp"] = {
    complete = complete_param_tcp,
    help = format_display("\nSample TCP Control flags" .. cksm_note , tcp_control_flag_temp, tcp_control_flag_desc, tcp_control_flag_format)
}

-- UDP Format
function complete_param_udp(param, name, desc)
     local values =
        {{"FFFFEEEE", "Sample UDP custom options"}}
     return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["cntr-flag-udp"] = {
    complete = complete_param_udp,
    help = format_display("\nSample UDP Control flags" .. cksm_note, udp_control_flag_temp, udp_control_flag_desc, udp_control_flag_format)
}

local function convertToArray(value, size)
    local customTag = {}
    local start = 1, curr_byte
    for ii = 0,size-1 do
        curr_byte = string.sub(value, start, start + 1)
        customTag[ii] = byteAs2NibblesToNumeric(curr_byte)
        start = start + 2
    end
    return customTag
end

--*********************************************************
--*  API to do: (enhanced UT c API):
--*     1. Get the input data to info structure,
--*        (for prefix holds starting part of the variable name)
--*********************************************************
local function get_pkt_details(params, info, prefix)
    local size

    -- L2 Parameters
    info.vlanId             = params[prefix .. "vlanId"];
    info.saMac              = params[prefix .. "saMac"] or convertMacAddrStringTo_GT_ETHERADDR(def_SA);
    info.daMac              = params[prefix .. "daMac"] or convertMacAddrStringTo_GT_ETHERADDR(def_DA);
    if params[prefix .. "customTagL2"] then
        size                = (string.len(params[prefix .. "customTagL2"])/2)
        info.customTagL2    = convertToArray(params[prefix .. "customTagL2"], size);
        info.l2tagSize      = size
    else
        info.l2tagSize      = 0
    end

    -- L3 Parameters
    if params[prefix .. "ipv6_sip"] or params[prefix .. "ipv6_dip"] or params[prefix .. "cntr_flag_ipv6"] then
        info.isIpv6         = GT_TRUE;
        info.ipSrcAddr      = {ipv6Addr = params[prefix .. "ipv6_sip"] or def_ipv6_sip};
        info.ipDstAddr      = {ipv6Addr = params[prefix .. "ipv6_dip"] or def_ipv6_dip};
        if params[prefix .. "cntr_flag_ipv6"] then
            size                = (string.len(params[prefix .. "cntr_flag_ipv6"])/2)
            info.customTagL3    = {ipv6 = convertToArray(params[prefix .. "cntr_flag_ipv6"], size)};
            info.isCustomTagL3  = GT_TRUE
        end
    else
        info.isIpv6         = GT_FALSE;
        info.ipSrcAddr      = {ipv4Addr = params[prefix .. "ipv4_sip"] or def_ipv4_sip};
        info.ipDstAddr      = {ipv4Addr = params[prefix .. "ipv4_dip"] or def_ipv4_dip};
        if params[prefix .. "cntr_flag_ipv4"] then
            size                = (string.len(params[prefix .. "cntr_flag_ipv4"])/2)
            info.customTagL3    = {ipv4 = convertToArray(params[prefix .. "cntr_flag_ipv4"], size)};
            info.isCustomTagL3  = GT_TRUE
        end
    end

    -- L4 Parameters
    if params[prefix .. "udp_s_port"] or params[prefix .. "udp_d_port"] or params[prefix .. "cntr_flag_udp"] then
        info.isTcp          = GT_FALSE;
        info.udpDstPort     = params[prefix .. "udp_d_port"] or tonumber(def_udp_d_port);
        info.udpSrcPort     = params[prefix .. "udp_s_port"] or tonumber(def_udp_s_port);
        if params[prefix .. "cntr_flag_udp"] then
            size                = (string.len(params[prefix .. "cntr_flag_udp"])/2)
            info.customTagL4    = {udp=convertToArray(params[prefix .. "cntr_flag_udp"], size)};
            info.isCustomTagL4  = GT_TRUE
        end
    else
        info.isTcp          = GT_TRUE;
        info.tcpSrcPort     = params[prefix .. "tcp_s_port"] or tonumber(def_tcp_s_port);
        info.tcpDstPort     = params[prefix .. "tcp_d_port"] or tonumber(def_tcp_d_port);
        if params[prefix .. "cntr_flag_tcp"] then
            size                = (string.len(params[prefix .. "cntr_flag_tcp"])/2)
            info.customTagL4    = {udp=convertToArray(params[prefix .. "cntr_flag_tcp"], size)};
            info.isCustomTagL4  = GT_TRUE
        end
    end
end

--*********************************************************
--*  API to do: (enhanced UT c API):
--*     1. Make interface UP
--*     2. Frame the packet according to input params.
--*     3. Display packet on the console
--*     4. Send the packet to the input port(s)
--*     5. Display sent status on the console
--*********************************************************
local function func_traffic_gen__per_port(command_data, devNum, portNum, params)
    local apiName,result
    local size

    -- In case of Tunnel its the inner packet details, also holds the common parameters like port, stream etc.
    local info = {}
    local tunnel_info = {}

    -- Common parameters
    info.pktType        = params.pktType;
    info.streamType     = params.streamType;
    info.port           = portNum;
    info.pktSize        = params.pktSize;
    if params["pktCnt"] then
        info.pktCnt     = params.pktCnt
    else
        info.pktCnt     = 1
    end

    if params.pktType <= 2 then
        get_pkt_details(params, info, '')
    else
        get_pkt_details(params, info,        TUNNEL_INNER_PKT_SUB_STRING)
        get_pkt_details(params, tunnel_info, TUNNEL_OUTER_PKT_SUB_STRING)
    end

    -- Call API for traffic send
    apiName = "prvTgfTrafficGenWithOptions"
	result = myGenWrapper(apiName, {
	    { "IN", "TGF_TRAFFIC_GEN_PARAMS_STC", "trafficGenParam", info },
	    { "IN", "TGF_TRAFFIC_GEN_PARAMS_STC", "trafficGenParam", tunnel_info },
	});

    command_data:handleCpssErrorDevPort(result, apiName.."()")
	return 0
end

local function func_traffic_gen(params)
    local command_data = Command_Data()
    params.all_interfaces = "all"
    params.ethernet = params.port
    generic_port_range_func(func_traffic_gen__per_port,params)
end

CLI_addParamDict("common_options", {
    { format="pkt-count %GT_U32"        ,name = "pktCnt"     ,help="The number of packets need to be sent" },
    { format="size %GT_U32"             ,name = "pktSize"    ,help="packet size in bytes reached by zero-padding" },
    { format="stream %stream-options"   ,name = "streamType" ,help="The stream option(Default - input-pkt)" },
    { format="port %port-range"         ,name = "port"       ,help="The device and port number" },
})

CLI_addParamDict("inner_l2_options", {
    { format="inner_l2_custom_tag %l2-custom-tag"          ,name="inner_customTagL2"    ,help="Inner custom Tag data           (default - Untagged)" },
    { format="inner_l2_daMac %mac-address-uc"              ,name="inner_daMac"          ,help="Inner Destination MAC address   (Default - "..def_DA..")" },
    { format="inner_l2_saMac %mac-address-uc"              ,name="inner_saMac"          ,help="Inner Source MAC Address        (Default - "..def_SA..")" },
    { format="inner_l2_vlan %GT_U16"                       ,name="inner_vlanId"         ,help="Inner VLAN ID                   (Default - Untagged)" },
})
CLI_addParamDict("inner_l3_options", {
    { format="inner_l3_ipv4_control_flags %cntr-flag-ipv4" ,name="inner_cntr_flag_ipv4" ,help="Inner IPv4 header control flags (Default - "..def_ipv4_cntl_flag..")" },
    { format="inner_l3_ipv6_control_flags %cntr-flag-ipv6" ,name="inner_cntr_flag_ipv6" ,help="Inner IPv6 header control flags (Default - "..def_ipv6_cntl_flag..")" },
    { format="inner_l3_ipv4_dip %ipv4"                     ,name="inner_ipv4_dip"       ,help="Inner Destination IPv4 Address  (Default - "..def_ipv4_dip..")" },
    { format="inner_l3_ipv6_dip %ipv6"                     ,name="inner_ipv6_dip"       ,help="Inner Destination IPv6 Address  (Default - "..def_ipv6_dip..")" },
    { format="inner_l3_ipv4_sip %ipv4"                     ,name="inner_ipv4_sip"       ,help="Inner Source IPv4 Address       (Default - "..def_ipv4_sip..")" },
    { format="inner_l3_ipv6_sip %ipv6"                     ,name="inner_ipv6_sip"       ,help="Inner Source IPv6 Address       (Default - "..def_ipv6_sip..")" },
})

CLI_addParamDict("inner_l4_options", {
    { format="inner_l4_tcp_control_flags %cntr-flag-tcp"   ,name="inner_cntr_flag_tcp"  ,help="Inner TCP control flags         (Default - "..def_tcp_cntl_flags..")" },
    { format="inner_l4_udp_control_flags %cntr-flag-udp"   ,name="inner_cntr_flag_udp"  ,help="Inner UDP control flags         (Default - "..def_udp_cntl_flags..")" },
    { format="inner_l4_udp_d_port %GT_U16"                 ,name="inner_udp_d_port"     ,help="Inner UDP destination port      (Default - "..def_udp_d_port..")" },
    { format="inner_l4_tcp_d_port %GT_U16"                 ,name="inner_tcp_d_port"     ,help="Inner TCP Destination port      (Default - "..def_tcp_d_port..")" },
    { format="inner_l4_tcp_s_port %GT_U16"                 ,name="inner_tcp_s_port"     ,help="Inner TCP Source port           (Default - "..def_tcp_s_port..")" },
    { format="inner_l4_udp_s_port %GT_U16"                 ,name="inner_udp_s_port"     ,help="Inner UDP source port           (Default - "..def_udp_s_port..")" },
})
CLI_addParamDict("outer_l2_options", {
    { format="outer_l2_custom_tag %l2-custom-tag"          ,name="outer_customTagL2"    ,help="Outer custom Tag data           (default - Untagged)" },
    { format="outer_l2_daMac %mac-address-uc"              ,name="outer_daMac"          ,help="Outer Destination MAC address   (Default - "..def_DA..")" },
    { format="outer_l2_saMac %mac-address-uc"              ,name="outer_saMac"          ,help="Outer Source MAC Address        (Default - "..def_SA..")" },
    { format="outer_l2_vlan %GT_U16"                       ,name="outer_vlanId"         ,help="Outer VLAN ID                   (Default - Untagged)" },
})
CLI_addParamDict("outer_l3_options", {
    { format="outer_l3_ipv4_control_flags %cntr-flag-ipv4" ,name="outer_cntr_flag_ipv4" ,help="Outer IPv4 header control flags (Default - "..def_ipv4_cntl_flag..")" },
    { format="outer_l3_ipv6_control_flags %cntr-flag-ipv6" ,name="outer_cntr_flag_ipv6" ,help="Outer IPv6 header control flags (Default - "..def_ipv6_cntl_flag..")" },
    { format="outer_l3_ipv4_dip %ipv4"                     ,name="outer_ipv4_dip"       ,help="Outer Destination IPv4 Address  (Default - "..def_ipv4_dip..")" },
    { format="outer_l3_ipv6_dip %ipv6"                     ,name="outer_ipv6_dip"       ,help="Outer Destination IPv6 Address  (Default - "..def_ipv6_dip..")" },
    { format="outer_l3_ipv4_sip %ipv4"                     ,name="outer_ipv4_sip"       ,help="Outer Source IPv4 Address       (Default - "..def_ipv4_sip..")" },
    { format="outer_l3_ipv6_sip %ipv6"                     ,name="outer_ipv6_sip"       ,help="Outer Source IPv6 Address       (Default - "..def_ipv6_sip..")" },
})
CLI_addParamDict("outer_l4_options", {
    { format="outer_l4_tcp_control_flags %cntr-flag-tcp"   ,name="outer_cntr_flag_tcp"  ,help="Outer TCP control flags         (Default - "..def_tcp_cntl_flags..")" },
    { format="outer_l4_udp_control_flags %cntr-flag-udp"   ,name="outer_cntr_flag_udp"  ,help="Outer UDP control flags         (Default - "..def_udp_cntl_flags..")" },
    { format="outer_l4_tcp_d_port %GT_U16"                 ,name="outer_tcp_d_port"     ,help="Outer TCP Destination port      (Default - "..def_tcp_d_port..")" },
    { format="outer_l4_udp_d_port %GT_U16"                 ,name="outer_udp_d_port"     ,help="Outer UDP destination port      (Default - "..def_udp_d_port..")" },
    { format="outer_l4_tcp_s_port %GT_U16"                 ,name="outer_tcp_s_port"     ,help="Outer TCP Source port           (Default - "..def_tcp_s_port..")" },
    { format="outer_l4_udp_s_port %GT_U16"                 ,name="outer_udp_s_port"     ,help="Outer UDP source port           (Default - "..def_udp_s_port..")" },
})
CLI_addParamDict("l2_options", {
    { format="l2-custom-tag %l2-custom-tag"          ,name="customTagL2"    ,help="Custom Tag data           (default - Untagged)" },
    { format="l2-da-mac %mac-address-uc"             ,name="daMac"          ,help="Destination MAC address   (Default - "..def_DA..")" },
    { format="l2-sa-mac %mac-address-uc"             ,name="saMac"          ,help="Source MAC Address        (Default - "..def_SA..")" },
    { format="l2-vlan %vlanId"                       ,name="vlanId"         ,help="VLAN ID                   (Default - Untagged)" },
})
CLI_addParamDict("l3_options", {
    { format="l3-ipv4-control-flags %cntr-flag-ipv4" ,name="cntr_flag_ipv4" ,help="IPv4 header control flags (Default - "..def_ipv4_cntl_flag..")" },
    { format="l3-ipv6-control-flags %cntr-flag-ipv6" ,name="cntr_flag_ipv6" ,help="IPv6 header control flags (Default - "..def_ipv6_cntl_flag..")" },
    { format="l3-ipv4-dip %ipv4"                     ,name="ipv4_dip"       ,help="Destination IPv4 Address  (Default - "..def_ipv4_dip..")" },
    { format="l3-ipv6-dip %ipv6"                     ,name="ipv6_dip"       ,help="Destination IPv6 Address  (Default - "..def_ipv6_dip..")" },
    { format="l3-ipv4-sip %ipv4"                     ,name="ipv4_sip"       ,help="Source IPv4 Address       (Default - "..def_ipv4_sip..")" },
    { format="l3-ipv6-sip %ipv6"                     ,name="ipv6_sip"       ,help="Source IPv6 Address       (Default - "..def_ipv6_sip..")" },
})
CLI_addParamDict("l4_options", {
    { format="l4-tcp-control-flags %cntr-flag-tcp"   ,name="cntr_flag_tcp"  ,help="TCP control flags         (Default - "..def_tcp_cntl_flags..")" },
    { format="l4-tcp-d-port %GT_U16"                 ,name="tcp_d_port"     ,help="TCP Destination port      (Default - "..def_tcp_d_port..")" },
    { format="l4-tcp-s-port %GT_U16"                 ,name="tcp_s_port"     ,help="TCP Source port           (Default - "..def_tcp_s_port..")" },
    { format="l4-udp-control-flags %cntr-flag-udp"   ,name="cntr_flag_udp"  ,help="UDP control flags         (Default - "..def_udp_cntl_flags..")" },
    { format="l4-udp-d-port %GT_U16"                 ,name="udp_d_port"     ,help="UDP destination port      (Default - "..def_udp_d_port..")" },
    { format="l4-udp-s-port %GT_U16"                 ,name="udp_s_port"     ,help="UDP source port           (Default - "..def_udp_s_port..")" },
})

--*************************************************************************
--********** Traffic-gen commands input params and validation *************
--*************************************************************************
local params_traffic_gen_l2 = {{
    type="named",
    '#l2_options',
    '#common_options',
    mandatory = {"port"}
}}

local params_traffic_gen_l3 = {{
    type="named",
    '#l2_options',
    '#l3_options',
    '#common_options',
    mandatory = {"port"}
}}

local params_traffic_gen_l4 = {{
    type="named",
    '#l2_options',
    '#l3_options',
    '#l4_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l2_in_l2 = {{
    type="named",
    '#inner_l2_options',
    '#outer_l2_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l2_in_l3 = {{
    type="named",
    '#inner_l2_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l2_in_l4 = {{
    type="named",
    '#inner_l2_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#outer_l4_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l3_in_l2 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#outer_l2_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l3_in_l3 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l3_in_l4 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#outer_l4_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l4_in_l2 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#inner_l4_options',
    '#outer_l2_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l4_in_l3 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#inner_l4_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#common_options',
    mandatory = {"port"}
}}
local params_traffic_gen_l4_in_l4 = {{
    type="named",
    '#inner_l2_options',
    '#inner_l3_options',
    '#inner_l4_options',
    '#outer_l2_options',
    '#outer_l3_options',
    '#outer_l4_options',
    '#common_options',
    mandatory = {"port"}
}}

-- local params_traffic_gen_control = {{
--     type="named",
--     { format="dip %ipv4" ,name="control_ipv4_dip"       ,help="Destination IPv4 Address  (Default - "..def_ipv4_dip..")" },
--     { format="sip %ipv4" ,name="control_ipv4_sip"       ,help="Source IPv4 Address  (Default - "..def_ipv4_sip..")" },
--     '#common_options',
--     mandatory = {"port"}
-- }}

--*********************************************************
--******* Traffic-gen utility Commands definition *********
--*********************************************************
CLI_addCommand("traffic", "traffic-gen l2", {
    func = function(params)
        params.pktType = 0 -- "TGF_TRAFFIC_GEN_PKT_TYPE_L2_E"
        return func_traffic_gen(params)
    end,
    help = help_string_l2,
    params = params_traffic_gen_l2
})

CLI_addCommand("traffic", "traffic-gen l3", {
    func = function(params)
        params.pktType = 1 -- "TGF_TRAFFIC_GEN_PKT_TYPE_L3_E"
        return func_traffic_gen(params)
    end,
    help = help_string_l3,
    params = params_traffic_gen_l3
})

CLI_addCommand("traffic", "traffic-gen l4", {
    func = function(params)
        params.pktType = 2 -- "TGF_TRAFFIC_GEN_PKT_TYPE_L4_E"
        return func_traffic_gen(params)
    end,
    help = help_string_l4,
    params = params_traffic_gen_l4
})

CLI_addCommand("traffic", "traffic-gen tunnel l2_in_l2", {
    func = function(params)
        params.pktType = 3 -- "TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L2_E"
        return func_traffic_gen(params)
    end,
    help = help_string_l2_in_l2,
    params = params_traffic_gen_l2_in_l2
})
CLI_addCommand("traffic", "traffic-gen tunnel l2_in_l3", {
    func = function(params)
        params.pktType = 4 -- TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L3_E
        return func_traffic_gen(params)
    end,
    help = help_string_l2_in_l3,
    params = params_traffic_gen_l2_in_l3
})
CLI_addCommand("traffic", "traffic-gen tunnel l2_in_l4", {
    func = function(params)
        params.pktType = 5 -- TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L4_E
        return func_traffic_gen(params)
    end,
    help = help_string_l2_in_l4,
    params = params_traffic_gen_l2_in_l4
})
CLI_addCommand("traffic", "traffic-gen tunnel l3_in_l2", {
    func = function(params)
        params.pktType = 6 -- TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L2_E
        return func_traffic_gen(params)
    end,
    help = help_string_l3_in_l2,
    params = params_traffic_gen_l3_in_l2
})
CLI_addCommand("traffic", "traffic-gen tunnel l3_in_l3", {
    func = function(params)
        params.pktType = 7 -- TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L3_E
        return func_traffic_gen(params)
    end,
    help = help_string_l3_in_l3,
    params = params_traffic_gen_l3_in_l3
})
CLI_addCommand("traffic", "traffic-gen tunnel l3_in_l4", {
    func = function(params)
        params.pktType = 8 -- TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L4_E
        return func_traffic_gen(params)
    end,
    help = help_string_l3_in_l4,
    params = params_traffic_gen_l3_in_l4
})
CLI_addCommand("traffic", "traffic-gen tunnel l4_in_l2", {
    func = function(params)
        params.pktType = 9 -- TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L2_E
        return func_traffic_gen(params)
    end,
    help = help_string_l4_in_l2,
    params = params_traffic_gen_l4_in_l2
})
CLI_addCommand("traffic", "traffic-gen tunnel l4_in_l3", {
    func = function(params)
        params.pktType = 10 -- TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L3_E
        return func_traffic_gen(params)
    end,
    help = help_string_l4_in_l3,
    params = params_traffic_gen_l4_in_l3
})
CLI_addCommand("traffic", "traffic-gen tunnel l4_in_l4", {
    func = function(params)
        params.pktType = 11 -- TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E
        return func_traffic_gen(params)
    end,
    help = help_string_l4_in_l4,
    params = params_traffic_gen_l4_in_l4
})

-- Control traffic yet to be supported
-- CLI_addCommand("traffic", "traffic-gen control arp", {
--     func = function(params)
--         params.pktType = 12 -- TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ARP_E
--         return func_traffic_gen(params)
--     end,
--     help = help_string_control_arp,
--     params = params_traffic_gen_control
-- })
-- CLI_addCommand("traffic", "traffic-gen control icmp", {
--     func = function(params)
--         params.pktType = 13 -- TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ICMP_E
--         return func_traffic_gen(params)
--     end,
--     help = help_string_control_icmp,
--     params = params_traffic_gen_control
-- })
