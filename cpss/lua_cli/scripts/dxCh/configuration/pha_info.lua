--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pha_info.lua
--*
--* DESCRIPTION:
--*     Allow to manipulate complex PHA APIs.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
    list of the commands in this file:

    CLI_addCommand("config", "no pha thread-id-info", {
    CLI_addCommand("config", "pha thread-id-info ioam-ingress-switch ipv4", {
    CLI_addCommand("config", "pha thread-id-info ioam-ingress-switch ipv6", {
    CLI_addCommand("config", "pha thread-id-info ioam-transit-switch ipv4", {
    CLI_addCommand("config", "pha thread-id-info ioam-transit-switch ipv6", {
]]--

--includes

local help_thread_id = "the PHA FW Thread-Id (1..255)"

--constants

-- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
local CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E = 1024


local help_string_no_phaThreadId_info                           = "set the thread-id to not do any modification"
local help_string_phaThreadId_info                              = "set enhanced egress FW thread-id info"
local help_string_ingress_IOAM                                  = "set 'ingress switch' IOAM related info"
local help_string_transit_IOAM                                  = "set 'transit switch' IOAM related info"
local help_string_mirroring_IOAM                                = "set 'INT IOAM Mirroring' related info"
local help_string_egress_IOAM                                   = "set 'INT IOAM Egress' related info"
local help_string_egress_IOAM_IPv6                              = "set 'IOAM Egress IPv6' related info"
local help_string_egress_mirroring_metadata                     = "set 'Egress mirroring metadata' related info"
local help_string_ptp_one_step                                  = "set 'PTP one STEP' related info"
local help_string_MPLS_SR                                       = "set 'MPLS Segment Routing' related info"
local help_string_ingress_IOAM_ipv4                             = help_string_ingress_IOAM .. " with ipv4 TS"
local help_string_ingress_IOAM_ipv6                             = help_string_ingress_IOAM .. " with ipv6 TS"
local help_string_transit_IOAM_ipv4                             = help_string_transit_IOAM .. " with ipv4 tunnel"
local help_string_transit_IOAM_ipv6                             = help_string_transit_IOAM .. " with ipv6 tunnel"
local help_string_MPLS_SR_NO_EL                                 = help_string_MPLS_SR .. " with no Entropy Label"
local help_string_MPLS_SR_ONE_EL                                = help_string_MPLS_SR .. " with one Entropy Label"
local help_string_MPLS_SR_TWO_EL                                = help_string_MPLS_SR .. " with two Entropy Label"
local help_string_MPLS_SR_THREE_EL                              = help_string_MPLS_SR .. " with three Entropy Label"
local help_string_UNIFIED_SR                                    = "set 'Unified Segment Routing' related info"
local help_string_SRV6_SRC_NODE_1_SEGMENT                       = "set 'SRv6 source node(1 segment)' related info"
local help_string_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS         = "set 'SRv6 source node first pass(2/3 segments)' related info"
local help_string_CLASSIFIER_NSH_OVER_ETHERNET                  = "set 'Classifier NSH over Ethernet' related info"
local help_string_SFF_NSH_VXLAN_GPE_TO_ETHERNET                 = "set 'SFF NSH VXLAN GPE to Ethernet' related info"
local help_string_SFF_NSH_ETHERNET_TO_VXLAN_GPE                 = "set 'SFF NSH Ethernet to VXLAN GPE' related info"
local help_string_SRv6_END_NODE                                 = "set 'SRv6 end node' related info"
local help_string_SRv6_PENULTIMATE_NODE                         = "set 'SRv6 Penultimate node' related info"
local help_string_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS          = "set 'SRv6 source node second pass(2 segments)' related info"
local help_string_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS          = "set 'SRv6 source node second pass(3 segments)' related info"
local help_string_SGT_NET_ADD_MSB                               = "set 'SGT network add MSB' related info"
local help_string_SGT_NET_FIX                                   = "set 'SGT network fix' related info"
local help_string_SGT_NET_REMOVE                                = "set 'SGT network remove' related info"
local help_string_SGT_EDSA_FIX                                  = "set 'SGT eDSA fix' related info"
local help_string_SGT_EDSA_REMOVE                               = "set 'SGT eDSA remove' related info"
local help_string_SGT_GBP_FIX_IPV4                              = "set 'SGT GBP fix IPv4' related info"
local help_string_SGT_GBP_FIX_IPV6                              = "set 'SGT GBP fix IPv6' related info"
local help_string_SGT_GBP_REMOVE_IPV4                           = "set 'SGT GBP remove IPv4' related info"
local help_string_SGT_GBP_REMOVE_IPV6                           = "set 'SGT GBP remove IPv6' related info"
local help_string_SRC_DEV_MIRRORING_ERSPAN_Type_II              = "set 'Source Device Mirroring ERSPAN Type II' related info"
local help_string_TRG_DEV_LC_MIRRORING_IPV4_ERSPAN_Type_II      = "set 'Target Device LC Mirroring IPV4 ERSPAN Type II' related info"
local help_string_TRG_DEV_LC_MIRRORING_IPV6_ERSPAN_Type_II      = "set 'Target Device LC Mirroring IPV6 ERSPAN Type II' related info"
local help_string_TRG_DEV_DIRECT_MIRRORING_IPV4_ERSPAN_Type_II  = "set 'Target Device Direct Mirroring IPV4 ERSPAN Type II' related info"
local help_string_TRG_DEV_DIRECT_MIRRORING_IPV6_ERSPAN_Type_II  = "set 'Target Device Direct Mirroring IPV6 ERSPAN Type II' related info"
local help_string_TRG_DEV_MIRRORING_IPV4_SHARED_INFO_ERSPAN_Type_II = "set 'Target Device Mirroring IPV4 in Shared Memory ERSPAN Type II' related info"
local help_string_TRG_DEV_MIRRORING_IPV6_SHARED_INFO_ERSPAN_Type_II = "set 'Target Device Mirroring IPV6 in Shared Memory ERSPAN Type II' related info"
local help_string_SHARED_INFO_GLOBAL_ERSPAN_Type_II             = "set 'Global Parameters:Device Id in Shared Memory ERSPAN Type II' related info"
local help_string_VXLAN_GBP_SOURCE_GROUP_POLICY_ID              = "set 'VXLAN-GBP Source Group Policy ID' related info"
local help_string_SRV6_BE                                       = "set 'SRV6 BE Tunnels' related info"
local help_string_SRV6_1_CONTAINER                              = "set 'SRV6 G-SID Tunnels Single Pass with 1 SRH container' related info"
local help_string_SRV6_PASS1                                    = "set 'SRV6 G-SID Tunnels First Pass with 2/3 SRH containers' related info"
local help_string_SRV6_PASS2_2_CONTAINER                        = "set 'SRV6 G-SID Tunnels Second Pass with 2 SRH containers' related info"
local help_string_SRV6_PASS2_3_CONTAINER                        = "set 'SRV6 G-SID Tunnels Second Pass with 3 SRH containers' related info"
local help_string_SRV6_COC32_GSID                               = "set 'SRV6 G-SID Tunnels DIP with the next G-SID from SRH' related info"
local help_string_IPV4_TTL_INCREMENT                            = "set 'IPV4 TTL field increment' related info"
local help_string_IPV6_HOP_LIMIT_INCREMENT                      = "set 'IPV6 Hop Limit field increment' related info"
local help_string_CLEAR_OUTGOING_MTAG_CMD                       = "set 'Clear Outgoing Mtag Command' related info"

CLI_addHelp("config", "pha", "(sip6) Set enhanced egress processing commands (using FW (firmware))")
CLI_addHelp("config", "no pha", "(sip6) unset enhanced egress processing commands (using FW (firmware))")
CLI_addHelp("config", "no pha thread-id-info", help_string_no_phaThreadId_info)
CLI_addHelp("config", "pha thread-id-info", help_string_phaThreadId_info)
CLI_addHelp("config", "pha thread-id-info ioam-ingress-switch", help_string_ingress_IOAM)
CLI_addHelp("config", "pha thread-id-info ioam-transit-switch", help_string_transit_IOAM)
CLI_addHelp("config", "pha thread-id-info int-ioam-mirroring", help_string_mirroring_IOAM)
CLI_addHelp("config", "pha thread-id-info int-ioam-egress", help_string_egress_IOAM)
CLI_addHelp("config", "pha thread-id-info ioam-egress-ipv6", help_string_egress_IOAM_IPv6)
CLI_addHelp("config", "pha thread-id-info egress-mirroring-metadata", help_string_egress_mirroring_metadata)
CLI_addHelp("config", "pha thread-id-info ptp-one-step", help_string_ptp_one_step)
CLI_addHelp("config", "pha thread-id-info ioam-ingress-switch ipv4", help_string_ingress_IOAM_ipv4)
CLI_addHelp("config", "pha thread-id-info ioam-ingress-switch ipv6", help_string_ingress_IOAM_ipv6)
CLI_addHelp("config", "pha thread-id-info ioam-transit-switch ipv4", help_string_transit_IOAM_ipv4)
CLI_addHelp("config", "pha thread-id-info ioam-transit-switch ipv6", help_string_transit_IOAM_ipv6)
CLI_addHelp("config", "pha thread-id-info unified-sr", help_string_UNIFIED_SR)
CLI_addHelp("config", "pha thread-id-info classifier-nsh-over-ethernet",  help_string_CLASSIFIER_NSH_OVER_ETHERNET)
CLI_addHelp("config", "pha thread-id-info sff-nsh-vxlan-gpe-to-ethernet", help_string_SFF_NSH_VXLAN_GPE_TO_ETHERNET)
CLI_addHelp("config", "pha thread-id-info sff-nsh-ethernet-to-vxlan-gpe",  help_string_SFF_NSH_ETHERNET_TO_VXLAN_GPE)
CLI_addHelp("config", "pha thread-id-info srv6-end-node", help_string_SRv6_END_NODE)
CLI_addHelp("config", "pha thread-id-info srv6-penultimate-node", help_string_SRv6_PENULTIMATE_NODE)
CLI_addHelp("config", "pha thread-id-info srv6-source-node-second-pass-three-segment", help_string_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS)
CLI_addHelp("config", "pha thread-id-info srv6-source-node-second-pass-two-segment", help_string_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS)
CLI_addHelp("config", "pha thread-id-info sgt-network-add-msb", help_string_SGT_NET_ADD_MSB)
CLI_addHelp("config", "pha thread-id-info sgt-network-fix", help_string_SGT_NET_FIX)
CLI_addHelp("config", "pha thread-id-info sgt-network-remove", help_string_SGT_NET_REMOVE)
CLI_addHelp("config", "pha thread-id-info sgt-edsa-fix", help_string_SGT_EDSA_FIX)
CLI_addHelp("config", "pha thread-id-info sgt-edsa-remove", help_string_SGT_EDSA_REMOVE)
CLI_addHelp("config", "pha thread-id-info sgt-gbp-fix-ipv4", help_string_SGT_GBP_FIX_IPV4)
CLI_addHelp("config", "pha thread-id-info sgt-gbp-fix-ipv6", help_string_SGT_GBP_FIX_IPV6)
CLI_addHelp("config", "pha thread-id-info sgt-gbp-remove-ipv4", help_string_SGT_GBP_REMOVE_IPV4)
CLI_addHelp("config", "pha thread-id-info sgt-gbp-remove-ipv6", help_string_SGT_GBP_REMOVE_IPV6)
CLI_addHelp("config", "pha thread-id-info erspan-type-II-src-dev-mirroring", help_string_SRC_DEV_MIRRORING_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info erspan-type-II-trg-dev-lc-ipv4-mirroring", help_string_TRG_DEV_LC_MIRRORING_IPV4_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info erspan-type-II-trg-dev-lc-ipv6-mirroring", help_string_TRG_DEV_LC_MIRRORING_IPV6_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info erspan-type-II-trg-dev-direct-ipv4-mirroring", help_string_TRG_DEV_DIRECT_MIRRORING_IPV4_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info erspan-type-II-trg-dev-direct-ipv6-mirroring", help_string_TRG_DEV_DIRECT_MIRRORING_IPV6_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info ipv4-template-shared-memory-erspan-type-II-trg-dev-mirroring", help_string_TRG_DEV_MIRRORING_IPV4_SHARED_INFO_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info ipv6-template-shared-memory-erspan-type-II-trg-dev-mirroring", help_string_TRG_DEV_MIRRORING_IPV6_SHARED_INFO_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info shared-memory-global-erspan-type-II", help_string_SHARED_INFO_GLOBAL_ERSPAN_Type_II)
CLI_addHelp("config", "pha thread-id-info vxlan-gbp-source-group-policy-id", help_string_VXLAN_GBP_SOURCE_GROUP_POLICY_ID)
CLI_addHelp("config", "pha thread-id-info srv6-be-tunnel", help_string_SRV6_BE)
CLI_addHelp("config", "pha thread-id-info srv6-gsid-single-pass-one-container", help_string_SRV6_1_CONTAINER)
CLI_addHelp("config", "pha thread-id-info srv6-gsid-first-pass-with-multi-containers", help_string_SRV6_PASS1)
CLI_addHelp("config", "pha thread-id-info srv6-gsid-second-pass-with-two-containers", help_string_SRV6_PASS2_2_CONTAINER)
CLI_addHelp("config", "pha thread-id-info srv6-gsid-second-pass-with-three-containers", help_string_SRV6_PASS2_3_CONTAINER)
CLI_addHelp("config", "pha thread-id-info srv6-dip-with-gsid", help_string_SRV6_COC32_GSID)
CLI_addHelp("config", "pha thread-id-info ipv4-ttl-increment", help_string_IPV4_TTL_INCREMENT)
CLI_addHelp("config", "pha thread-id-info ipv6-hop-limit-increment", help_string_IPV6_HOP_LIMIT_INCREMENT)
CLI_addHelp("config", "pha thread-id-info clear-outgoing-mtag-cmd", help_string_CLEAR_OUTGOING_MTAG_CMD)

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E'
]]--
local function per_device__pha_no_thread(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    -- ignore the option to control the unused thread-IDs
    local common = {
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E
    }

    local dummyValue = {0}

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", "CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E" },
        { "IN", "CPSS_DXCH_PHA_THREAD_INFO_UNT", "extInfoPtr", dummyValue --[[don't care , but must be 'struct']] }
    })
end
--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E'
]]--
local function pha_no_thread(params)
    generic_all_device_func(per_device__pha_no_thread,params)
end
CLI_addCommand("config", "no pha thread-id-info", {
    func=function(params)
        return pha_no_thread(params)
    end,
    help   = help_string_no_phaThreadId_info,
    params = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },
    mandatory = {"phaThreadId"}
    }}
})



--[[ common parameters for several commands]]
local params_pha_ioam_ingress = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC ]]--
    {format="2.ioam.01.IOAM_Trace_Type  %GT_U32",   name="IOAM_Trace_Type" , help = "16 bits value"},
    {format="2.ioam.02.Maximum_Length   %GT_U32",   name="Maximum_Length"  , help = " 8 bits value"},
    {format="2.ioam.03.Flags            %GT_U32",   name="Flags"           , help = " 8 bits value"},
    {format="2.ioam.04.Hop_Lim          %GT_U32",   name="Hop_Lim"         , help = " 8 bits value"},
    {format="2.ioam.05.node_id          %GT_U32",   name="node_id"          , help = "24 bits value"},
    {format="2.ioam.06.Type1            %GT_U32",   name="Type1"           , help = " 8 bits value , 'IOAM'"},
    {format="2.ioam.07.IOAM_HDR_len1    %GT_U32",   name="IOAM_HDR_len1"   , help = " 8 bits value"},
    {format="2.ioam.08.Reserved1        %GT_U32",   name="Reserved1"       , help = " 8 bits value"},
    {format="2.ioam.09.Next_Protocol1   %GT_U32",   name="Next_Protocol1"  , help = " 8 bits value"},
    {format="2.ioam.10.Type2           %GT_U32",    name="Type2"           , help = " 8 bits value , 'IOAM_E2E'"},
    {format="2.ioam.11.IOAM_HDR_len2   %GT_U32",    name="IOAM_HDR_len2"   , help = " 8 bits value"},
    {format="2.ioam.12.Reserved2       %GT_U32",    name="Reserved2"       , help = " 8 bits value"},
    {format="2.ioam.13.Next_Protocol2  %GT_U32",    name="Next_Protocol2"  , help = " 8 bits value"},

    mandatory = {"phaThreadId"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E'
]]--
local function per_device__pha_ioam_ingress(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end


    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        IOAM_Trace_Type    = params.IOAM_Trace_Type,
        Maximum_Length     = params.Maximum_Length,
        Flags              = params.Flags,
        Hop_Lim            = params.Hop_Lim,
        node_id            = params.node_id,
        Type1              = params.Type1,
        IOAM_HDR_len1      = params.IOAM_HDR_len1,
        Reserved1          = params.Reserved1,
        Next_Protocol1     = params.Next_Protocol1,
        Type2              = params.Type2,
        IOAM_HDR_len2      = params.IOAM_HDR_len2,
        Reserved2          = params.Reserved2,
        Next_Protocol2     = params.Next_Protocol2
    }

    --print("info" , to_string(info))

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT  , "extInfoPtr", info }
    })
end

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E'
]]--
local function pha_ioam_ingress(params)
    generic_all_device_func(per_device__pha_ioam_ingress,params)
end

CLI_addCommand("config", "pha thread-id-info ioam-ingress-switch ipv4", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC"--"CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC"
        return pha_ioam_ingress(params)
    end,
    help   = help_string_ingress_IOAM_ipv4,
    params = params_pha_ioam_ingress --[[ common parameters with other commands]]
})
CLI_addCommand("config", "pha thread-id-info ioam-ingress-switch ipv6", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC"
        return pha_ioam_ingress(params)
    end,
    help   = help_string_ingress_IOAM_ipv6,
    params = params_pha_ioam_ingress --[[ common parameters with other commands]]
})


--[[ common parameters for several commands]]
local params_pha_ioam_transit = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC ]]--
    {format="2.ioam.02.node_id          %GT_U32",   name="node_id"         , help = "24 bits value"},

    mandatory = {"phaThreadId"}
}}


--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E'
]]--
local function per_device__pha_ioam_transit(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        node_id            = params.node_id,
    }

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT, "extInfoPtr", info }
    })
end
--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E'
]]--
local function pha_ioam_transit(params)
    generic_all_device_func(per_device__pha_ioam_transit,params)
end

CLI_addCommand("config", "pha thread-id-info ioam-transit-switch ipv4", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC"
        return pha_ioam_transit(params)
    end,
    help   = help_string_transit_IOAM_ipv4,
    params = params_pha_ioam_transit --[[ common parameters with other commands]]
})
CLI_addCommand("config", "pha thread-id-info ioam-transit-switch ipv6", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC"
        return pha_ioam_transit(params)
    end,
    help   = help_string_transit_IOAM_ipv6,
    params = params_pha_ioam_transit --[[ common parameters with other commands]]
})


--[[ common parameters for several commands]]
local params_pha_default = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    mandatory = {"phaThreadId"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E'
]]--
local function per_device__pha_default(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local dummyValue = {0}

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", "CPSS_DXCH_PHA_THREAD_INFO_UNT", "extInfoPtr", dummyValue --[[don't care , but must be 'struct']] }
    })
end

local function pha_default(params)
    generic_all_device_func(per_device__pha_default,params)
end

CLI_addCommand("config", "pha thread-id-info int-ioam-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_mirroring_IOAM,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info int-ioam-egress", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_egress_IOAM,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_type_dict["copyReservedLsb"] = {
    checker = CLI_check_param_number,
    min=0,
    max=17,
    help="Copy Reserved first bit used for Policy ID insertion, allowed range 0-17"
}
CLI_type_dict["copyReservedMsb"] = {
    checker = CLI_check_param_number,
    min=0,
    max=17,
    help="Copy Reserved last bit used for Policy ID insertion, allowed range 0-17"
}

--[[ common parameters for SGT with template]]
local params_pha_vxlan_gbp_with_template = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC ]]--
    {format="2.vxlanGbp.01.copyReservedLsb      %GT_U8",      name="copyReservedLsb"      , help = "First bit of Copy Reserved for Policy ID"},
    {format="2.vxlanGbp.02.copyReservedMsb      %GT_U8",      name="copyReservedMsb"      , help = "Last bit of Copy Reserved for Policy ID"},

    mandatory = {"phaThreadId"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E'
]]--

local function per_device__pha_vxlan_gbp(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        copyReservedLsb   = params.copyReservedLsb,
        copyReservedMsb   = params.copyReservedMsb,
    }
    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT  , "extInfoPtr", info }
    })
end

local function pha_vxlan_gbp(params)
    generic_all_device_func(per_device__pha_vxlan_gbp,params)
end

CLI_addCommand("config", "pha thread-id-info vxlan-gbp-source-group-policy-id", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC"
        return pha_vxlan_gbp(params)
    end,
    help   = help_string_VXLAN_GBP_SOURCE_GROUP_POLICY_ID,
    params = params_pha_vxlan_gbp_with_template --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info egress-mirroring-metadata", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_egress_mirroring_metadata,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info ptp-one-step", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_ptp_one_step,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info mpls-sr-no-el", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_MPLS_SR_NO_EL,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info mpls-sr-one-el", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_MPLS_SR_ONE_EL,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info mpls-sr-two-el", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_MPLS_SR_TWO_EL,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info mpls-sr-three-el", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_MPLS_SR_THREE_EL,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info classifier-nsh-over-ethernet", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_CLASSIFIER_NSH_OVER_ETHERNET,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info sff-nsh-vxlan-gpe-to-ethernet", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SFF_NSH_VXLAN_GPE_TO_ETHERNET,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info sff-nsh-ethernet-to-vxlan-gpe", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SFF_NSH_ETHERNET_TO_VXLAN_GPE,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-end-node", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRv6_END_NODE,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-penultimate-node", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRv6_PENULTIMATE_NODE,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-source-node-second-pass-three-segment", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-source-node-second-pass-two-segment", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-be-tunnel", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRV6_BE,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-gsid-second-pass-with-two-containers", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRV6_PASS2_2_CONTAINER,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-gsid-second-pass-with-three-containers", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SRV6_PASS2_3_CONTAINER,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info ipv4-ttl-increment", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_IPV4_TTL_INCREMENT,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info ipv6-hop-limit-increment", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_IPV6_HOP_LIMIT_INCREMENT,
    params = params_pha_default --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info clear-outgoing-mtag-cmd", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_CLEAR_OUTGOING_MTAG_CMD,
    params = params_pha_default --[[ common parameters with other commands]]
})

--[[ common parameters for several commands]]
local params_pha_sr_ipv6 = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC ]]--
    {format="2.srcAddr %ipv6", name="srcAddr", help = "IPv6 source address"},

    mandatory = {"phaThreadId"}
}}

--[[ common parameters for several commands]]
local params_pha_sr_ipv6_gsid_coc32 = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC ]]--
    {format="2.dipCommonPrefixLength %dipLength", name="dipLength", help = "IPv6 DIP Common Prefix Length"},

    mandatory = {"phaThreadId"}
}}


--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR' \
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E' \
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E'
]]--
local function per_device__pha_SR_IPv6(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        srcAddr = params.srcAddr
    }

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT, "extInfoPtr", info }
    })
end

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_COC32_GSID_E'
]]--
local function per_device__pha_SR_IPv6_GSID_COC32(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        dipLength = params.dipCommonPrefixLength
    }

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT, "extInfoPtr", info }
    })
end


--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E'
]]--

local function pha_sr_ipv6(params)
    generic_all_device_func(per_device__pha_SR_IPv6,params)
end

local function pha_sr_ipv6_gsid_coc32(params)
    generic_all_device_func(per_device__pha_SR_IPv6_GSID_COC32,params)
end

CLI_addCommand("config", "pha thread-id-info srv6-source-node-first-pass-2-3-segments", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS,
    params = params_pha_sr_ipv6 --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-source-node-one-segment", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_SRV6_SRC_NODE_1_SEGMENT,
    params = params_pha_sr_ipv6 --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info unified-sr", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_UNIFIED_SR,
    params = params_pha_sr_ipv6 --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-gsid-single-pass-one-container", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_SRV6_1_CONTAINER,
    params = params_pha_sr_ipv6 --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-gsid-first-pass-with-multi-containers", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_SRV6_PASS1,
    params = params_pha_sr_ipv6 --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info srv6-dip-with-gsid", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC"
        return pha_sr_ipv6(params)
    end,
    help   = help_string_SRV6_COC32_GSID,
    params = params_pha_sr_ipv6_gsid_coc32 --[[ common parameters with other commands]]
})

--[[ IOAM egress IPv6 command APIs ]]
local params_pha_ioam_egress_ipv6 = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC ]]--
    {format="2.node_id %GT_U32", name="nodeId", help = "Node ID"},

    mandatory = {"phaThreadId"}
}}


--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E'
]]--
local function per_device__pha_ioam_egress_ipv6(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        node_id = params.nodeId
    }

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT, "extInfoPtr", info }
    })
end


--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E'
]]--

local function pha_ioam_egress_ipv6(params)
    generic_all_device_func(per_device__pha_ioam_egress_ipv6,params)
end

CLI_addCommand("config", "pha thread-id-info ioam-egress-ipv6", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC"
        return pha_ioam_egress_ipv6(params)
    end,
    help   = help_string_egress_IOAM_IPv6,
    params = params_pha_ioam_egress_ipv6 --[[ common parameters with node ID]]
})

CLI_type_dict["etherType"] = {
    checker = CLI_check_param_hexnum,
    min=4,
    max=4,
    help="The ether type of SGT Tag, mandatory two bytes in hex (i.e. 0x8909, etc)"
}

CLI_type_dict["GT_U12"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4095,
    help="12 bit unsigned integer value"
}

--[[ common parameters for SGT with template]]
local params_pha_sgt_with_template = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC ]]--
    {format="2.sgtNetwork.01.etherType    %etherType",  name="etherType"    , help = "Ether Type value"},
    {format="2.sgtNetwork.02.version      %GT_U8",      name="version"      , help = "Version"},
    {format="2.sgtNetwork.03.length       %GT_U12",     name="length"       , help = "Length"},
    {format="2.sgtNetwork.04.optionType   %GT_U12",     name="optionType"   , help = "Option Type"},

    mandatory = {"phaThreadId"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E'
]]--

local function per_device__pha_sgt(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local info = {
        etherType   = params.etherType,
        version     = params.version,
        length      = params.length,
        optionType  = params.optionType
    }
    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT  , "extInfoPtr", info }
    })
end

local function pha_sgt(params)
    generic_all_device_func(per_device__pha_sgt,params)
end

CLI_addCommand("config", "pha thread-id-info sgt-network-add-msb", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC"
        return pha_sgt(params)
    end,
    help   = help_string_SGT_NET_ADD_MSB,
    params = params_pha_sgt_with_template --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info sgt-network-fix", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC"
        return pha_sgt(params)
    end,
    help   = help_string_SGT_NET_FIX,
    params = params_pha_sgt_with_template --[[ common parameters with other commands]]
})

CLI_addCommand("config", "pha thread-id-info sgt-network-remove", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_NET_REMOVE,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-edsa-fix", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_EDSA_FIX,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-edsa-remove", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_EDSA_REMOVE,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-gbp-fix-ipv4", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_GBP_FIX_IPV4,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-gbp-fix-ipv6", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_GBP_FIX_IPV6,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-gbp-remove-ipv4", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_GBP_REMOVE_IPV4,
    params = params_pha_default --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info sgt-gbp-remove-ipv6", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "notNeeded"
        return pha_default(params)
    end,
    help   = help_string_SGT_GBP_REMOVE_IPV6,
    params = params_pha_default --[[ common parameters]]
})

--[[ common parameters for ERSPAN with template]]
local params_pha_erspan_with_template = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="analyzer-index %analyzer_index", name="analyzerIndex", help = "analyzer index" },
    {format="thread-id %pha___thread_id",   name="phaThreadId", help = help_thread_id },

    --[[ info from CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC ]]--
    {format="1.common.busyStallMode %pha___busyStallMode",   name="busyStallMode", help = "The action taken when the PHA is busy" },
    {format="1.common.statisticalProcessingFactor %GT_U32",    name="statisticalProcessingFactor", help = "Statistical processing decision threshold.If 0, packet is always processed by the PHA. If 255 the packet has a 1 in 256 chance of being processed" },
    {format="1.common.stallDropCode %cpu_code_number",   name="stallDropCode", help = "Drop Code to set when the packet is dropped due to PHA overload" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC ]]--
    {format="2.erspan.01.macDa    %mac-address",   name="macDa"    , help = "ERSPAN Tunnel MAC DA"},
    {format="2.erspan.02.macSa    %mac-address",   name="macSa"    , help = "ERSPAN Tunnel MAC SA"},
    {format="2.erspan.03.tpid     %tpid",          name="tpid"     , help = "ERSPAN Tunnel L2 TPID"},
    {format="2.erspan.04.up       %up",            name="up"       , help = "ERSPAN Tunnel L2 UP"},
    {format="2.erspan.05.cfi      %cfi",           name="cfi"      , help = "ERSPAN Tunnel L2 CFI"},
    {format="2.erspan.06.vid      %GT_U12",        name="vid"      , help = "ERSPAN Tunnel VLAN ID"},

    mandatory = {"phaThreadId"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E'\
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E'
]]--

local function per_device__pha_erspan(command_data,devNum,params)
    local phaThreadId = params.phaThreadId

    if not params.stallDropCode then
        -- defaults for needed values
        params.stallDropCode = 0
    end

    local common = {
        statisticalProcessingFactor = params.statisticalProcessingFactor ,
        busyStallMode = params.busyStallMode,
        -- use the 1024 which is 'CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E'
        stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + tonumber(params.stallDropCode)
    }

    local dummyValue = {0}

    if params.analyzerIndex ~= nil and params.phaThreadId == 10 then
      print("PHA thread id does not accept analyzer index ")
      return {}
    end
    if params.analyzerIndex == nil and params.phaThreadId ~= 10 then
      params.analyzerIndex = 0 -- use default analyze index as 0
    end

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaThreadIdEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "IN", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfoPtr"  , common },
        { "IN", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType", params.CPSS_DXCH_PHA_THREAD_TYPE_ENT },
        { "IN", params.CPSS_DXCH_PHA_THREAD_INFO_UNT  , "extInfoPtr", dummyValue --[[don't care , but must be 'struct']] }
    })
end

local function pha_erspan(params)
    generic_all_device_func(per_device__pha_erspan,params)
end

CLI_addCommand("config", "pha thread-id-info erspan-type-II-src-dev-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC"
        return pha_erspan(params)
    end,
    help   = help_string_SRC_DEV_MIRRORING_ERSPAN_Type_II,
    params = params_pha_erspan_with_template --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info erspan-type-II-trg-dev-lc-ipv4-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC"
        return pha_erspan(params)
    end,
    help   = help_string_TRG_DEV_LC_MIRRORING_IPV4_ERSPAN_Type_II,
    params = params_pha_erspan_with_template --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info erspan-type-II-trg-dev-lc-ipv6-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC"
        return pha_erspan(params)
    end,
    help   = help_string_TRG_DEV_LC_MIRRORING_IPV6_ERSPAN_Type_II,
    params = params_pha_erspan_with_template --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info erspan-type-II-trg-dev-direct-ipv4-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC"
        return pha_erspan(params)
    end,
    help   = help_string_TRG_DEV_DIRECT_MIRRORING_IPV4_ERSPAN_Type_II,
    params = params_pha_erspan_with_template --[[ common parameters]]
})

CLI_addCommand("config", "pha thread-id-info erspan-type-II-trg-dev-direct-ipv6-mirroring", {
    func=function(params)
        params.CPSS_DXCH_PHA_THREAD_TYPE_ENT = "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E"
        params.CPSS_DXCH_PHA_THREAD_INFO_UNT = "CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC"
        return pha_erspan(params)
    end,
    help   = help_string_TRG_DEV_DIRECT_MIRRORING_IPV6_ERSPAN_Type_II,
    params = params_pha_erspan_with_template --[[ common parameters]]
})

CLI_type_dict["up"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help="TCI Priority Level"
}

CLI_type_dict["cfi"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    help="TCI Canonical Format Identifier"
}

CLI_type_dict["dscp"] = {
    checker = CLI_check_param_number,
    min=0,
    max=63,
    help="IPv4 DSCP"
}

CLI_type_dict["flags"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help="IPv4 Flags"
}

--[[ parameters for ERSPAN with IPV4 template]]
local params_pha_erspan_with_ipv4_template = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="analyzer-index %analyzer_index", name="analyzerIndex", help = "analyzer index" },

    --[[ info from CPSS_DXCH_PHA_THREAD_INFO_TYPE_ERSPAN_STC ]]--
    {format="1.erspan.01.macDa          %mac-address",   name="macDa"            , help = "ERSPAN Tunnel MAC DA"},
    {format="1.erspan.02.macSa          %mac-address",   name="macSa"            , help = "ERSPAN Tunnel MAC SA"},
    {format="1.erspan.03.tpid           %tpid",          name="tpid"             , help = "ERSPAN Tunnel L2 TPID"},
    {format="1.erspan.04.up             %up",            name="up"               , help = "ERSPAN Tunnel L2 UP"},
    {format="1.erspan.05.cfi            %cfi",           name="cfi"              , help = "ERSPAN Tunnel L2 CFI"},
    {format="1.erspan.06.vid            %GT_U12",        name="vid"              , help = "ERSPAN Tunnel VLAN ID"},
    {format="2.erspan_ipv4.01.dscp      %GT_U8",         name="dscp"             , help = "IPv4 dscp"},
    {format="2.erspan_ipv4.02.flags     %GT_U8",         name="flags"            , help = "IPv4 Flags"},
    {format="2.erspan_ipv4.03.ttl       %GT_U8",         name="ttl"              , help = "IPv4 Ttl"},
    {format="2.erspan_ipv4.04.sipAddr   %ipv4",          name="sip"              , help = "Source IPv4 address"},
    {format="2.erspan_ipv4.05.dipAddr   %ipv4",          name="dip"              , help = "Destination IPv4 address"},
    {format="2.erspan_ipv4.05.isid      %GT_U16",        name="isid"             , help = "Ingress session id"},
    {format="2.erspan_ipv4.05.esid      %GT_U16",        name="esid"             , help = "Egress session id"},

    mandatory = {"analyzerIndex"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E'
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E'
]]--

local function per_device_analyzer_index__pha_erspan_ipv4(command_data,devNum,params)

    local entry = {
        l2Info            = {
                            macDa   = params.macDa,
                            macSa   = params.macSa,
                            tpid    = params.tpid,
                            up      = params.up,
                            cfi     = params.cfi,
                            vid     = params.vid
                            },
        protocol          = CPSS_IP_PROTOCOL_IPV4_E,
        ipInfo            = {
                            ipv4 = {
                                 dscp              = params.dscp,
                                 flags             = params.flags,
                                 ttl               = params.ttl,
                                 sip               = params.sip,
                                 dip               = params.dip
                                   }},
        ingressSessionId  = params.isid,
        egressSessionId   = params.esid
    }
    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "analyzerIndex" , analyzerIndex },
        { "IN", "CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC", "entryPtr"  , entry }
    })
end

CLI_type_dict["flowLabel"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1048575,
    help="IPv6 Flow Label"
}

--[[ parameters for ERSPAN with IPV6 template]]
local params_pha_erspan_with_ipv6_template = {{
    type="named",
    {format="device %devID",name="devID", help="device number" },
    {format="analyzer-index %analyzer_index", name="analyzerIndex", help = "analyzer index" },

    --[[ CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC ]]--
    {format="1.erspan.01.macDa              %mac-address",   name="macDa"            , help = "ERSPAN Tunnel MAC DA"},
    {format="1.erspan.02.macSa              %mac-address",   name="macSa"            , help = "ERSPAN Tunnel MAC SA"},
    {format="1.erspan.03.tpid               %tpid",          name="tpid"             , help = "ERSPAN Tunnel L2 TPID"},
    {format="1.erspan.04.up                 %up",            name="up"               , help = "ERSPAN Tunnel L2 UP"},
    {format="1.erspan.05.cfi                %cfi",           name="cfi"              , help = "ERSPAN Tunnel L2 CFI"},
    {format="1.erspan.06.vid                %GT_U12",        name="vid"              , help = "ERSPAN Tunnel VLAN ID"},
    {format="2.erspan_ipv6.01.tc            %GT_U8",         name="tc"               , help = "IPv6 tc"},
    {format="2.erspan_ipv6.02.flowLabel     %GT_U32",        name="flowLabel"        , help = "IPv6 FlowLable"},
    {format="2.erspan_ipv6.03.hopLimit      %GT_U8",         name="hopLimit"         , help = "IPv6 HopLimit"},
    {format="2.erspan_ipv6.04.sipAddr       %ipv6",          name="sip"              , help = "Source IPv6 address"},
    {format="2.erspan_ipv6.05.dipAddr       %ipv6",          name="dip"              , help = "Destination IPv6 address"},
    {format="2.erspan_ipv6.05.isid          %GT_U16",        name="isid"             , help = "Ingress session id"},
    {format="2.erspan_ipv6.05.esid          %GT_U16",        name="esid"             , help = "Egress session id"},

    mandatory = {"analyzerIndex"}
}}

--[[
    function to build info for
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E'
        'CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E'
]]--

local function per_device_analyzer_index__pha_erspan_ipv6(command_data,devNum,params)

    local entry = {
        l2Info            = {
                            macDa   = params.macDa,
                            macSa   = params.macSa,
                            tpid    = params.tpid,
                            up      = params.up,
                            cfi     = params.cfi,
                            vid     = params.vid
                            },
        protocol          = CPSS_IP_PROTOCOL_IPV6_E,
        ipInfo            = {
                            ipv6 = {
                                 tc              = params.tc,
                                 flowLable       = params.flowLable,
                                 hopLimit        = params.hopLimit,
                                 sip             = params.sip,
                                 dip             = params.dip
                                   }},
        ingressSessionId  = params.isid,
        egressSessionId   = params.esid
    }
    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "analyzerIndex" , analyzerIndex },
        { "IN", "CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC", "entryPtr"  , entry }
    })
end

local function pha_erspan_ipv4(params)
    generic_all_device_func(per_device_analyzer_index__pha_erspan_ipv4,params)
end

local function pha_erspan_ipv6(params)
    generic_all_device_func(per_device_analyzer_index__pha_erspan_ipv6,params)
end

--[[ erspan IP template shared memory configfurations ]]
CLI_addCommand("config", "pha thread-id-info ipv4-template-shared-memory-erspan-type-II-trg-dev-mirroring", {
    func=function(params)
        return pha_erspan_ipv4(params)
    end,
    help   = help_string_TRG_DEV_MIRRORING_IPV4_SHARED_INFO_ERSPAN_Type_II,
    params = params_pha_erspan_with_ipv4_template --[[ ipv4 parameters]]
})

CLI_addCommand("config", "pha thread-id-info ipv6-template-shared-memory-erspan-type-II-trg-dev-mirroring", {
    func=function(params)
        return pha_erspan_ipv6(params)
    end,
    help   = help_string_TRG_DEV_MIRRORING_IPV6_SHARED_INFO_ERSPAN_Type_II,
    params = params_pha_erspan_with_ipv6_template --[[ ipv6 parameters]]
})

--[[ parameters for ERSPAN Global]]
local params_pha_erspan_global = {{
    type="named",
    {format="device %devID",              name="devID",       help="device number" },
    {format="erspan-device %erspanDevID", name="erspanDevID", help = "erspan device number" },
    {format="isVoQ %isVoQ",               name="isVoQ",       help = "LC has VoQ or not" },

    mandatory = {"erspanDevID"}
}}

--[[
    function to build info for PHA shared memory global parameters
]]--

local function per_device__pha_erspan_global(command_data,devNum,params)

    local erspanDevId = params.erspanDevID
    local isVoQ       = params.isVoQ

    if not params.isVoQ then
        -- defaults for needed values
        params.isVoQ = 1
    end

    genericCpssApiWithErrorHandler(command_data,"cpssDxChPhaSharedMemoryErspanGlobalConfigSet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U16", "erspanDevId" ,  erspanDevId},
        { "IN", "GT_BOOL","isVoQ"       ,  isVoQ}
    })
end

local function pha_erspan_global(params)
    generic_all_device_func(per_device__pha_erspan_global,params)
end

--[[ erspan shared memory global configfurations ]]
CLI_addCommand("config", "pha thread-id-info shared-memory-global-erspan-type-II", {
    func=function(params)
        return pha_erspan_global(params)
    end,
    help   = help_string_SHARED_INFO_GLOBAL_ERSPAN_Type_II,
    params = params_pha_erspan_global --[[ global parameters]]
})
