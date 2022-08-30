--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_pha_info.lua
--*
--* DESCRIPTION:
--*     show the PHA info
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
local   help_string_show_pha_info                       = "(DXCH) show PHA info"
local   help_string_show_pha_info_common                = help_string_show_pha_info .. " - common"
local   help_string_show_pha_info_ioam                  = help_string_show_pha_info .. " - IOAM"
local   help_string_show_pha_info_srv6                  = help_string_show_pha_info .. " - SRv6"
local   help_string_show_pha_info_sgt                   = help_string_show_pha_info .. " - SGT"
local   help_string_show_pha_ip_shared_info_erspan      = help_string_show_pha_info .. " - ERSPAN IP SHARED INFO"
local   help_string_show_pha_global_shared_info_erspan  = help_string_show_pha_info .. " - ERSPAN GLOBAL SHARED INFO"
local   help_string_show_pha_info_vxlan_gbp             = help_string_show_pha_info .. " - VXLAN GBP"
local   help_string_show_pha_info_srv6_gsid             = help_string_show_pha_info .. " - SRv6 GSID"

--------------------------------------------------------------------------------
-- command registration: show pha-info
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show pha-info ",                     help_string_show_pha_info)
CLI_addHelp("exec", "show pha-info common",               help_string_show_pha_info_common)
CLI_addHelp("exec", "show pha-info ioam",                 help_string_show_pha_info_ioam)
CLI_addHelp("exec", "show pha-info srv6",                 help_string_show_pha_info_srv6)
CLI_addHelp("exec", "show pha-info sgt",                  help_string_show_pha_info_sgt)
CLI_addHelp("exec", "show pha-ip-shared-info erspan",     help_string_show_pha_ip_shared_info_erspan)
CLI_addHelp("exec", "show pha-global-shared-info erspan", help_string_show_pha_global_shared_info_erspan)
CLI_addHelp("exec", "show pha-info vxlan-gbp",            help_string_show_pha_info_vxlan_gbp)
CLI_addHelp("exec", "show pha-info srv6-gsid",             help_string_show_pha_info_srv6_gsid)

--[[
    returns 2 parameters :
    1. isOk - bool indication that no error occured
    2. the entry info in next format:
    {
        commonInfo = {statisticalProcessingFactor , busyStallMode , stallDropCode},
        extType,
        extInfo = { notNeeded , ioamIngressSwitchIpv4 , ioamIngressSwitchIpv6 , ioamTransitSwitchIpv4 , ioamTransitSwitchIpv6, unifiedSRIpv6}
    }
]]--
function lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
    if(command_data == nil) then
        command_data = Command_Data()
    end

    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssDxChPhaThreadIdEntryGet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "phaThreadId" , phaThreadId },
        { "OUT", "CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC", "commonInfo"},
        { "OUT", "CPSS_DXCH_PHA_THREAD_TYPE_ENT", "extType"},
        { "OUT", "CPSS_DXCH_PHA_THREAD_INFO_UNT", "extInfo"}
        })

    if did_error then
        --error indication
        return false , nil
    end

    return true , OUT_values
end

--[[
    returns 2 parameters :
    1. isOk - bool indication that no error occured
    2. the entry info in next format:
    {
        entryInfo = {l2Info, protocol , ipInfo , ingressSessionId , egressSessionId}
    }
]]--
function lua_cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(command_data,devNum,phaAnalyzerId)
    if(command_data == nil) then
        command_data = Command_Data()
    end

    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet", {
        { "IN", "GT_U8" , "devNum"      , devNum },
        { "IN", "GT_U32", "analyzerIndex" , phaAnalyzerId },
        { "OUT", "CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC", "entryInfo"}
        })

    if did_error then
        --error indication
        return false , nil
    end

    return true , OUT_values
end

--[[
    returns 2 parameters :
    1. isOk - bool indication that no error occured
    2. the entry info in next format:
    {
        entryInfo = {erspanDevIdPtr,isVoQ}
    }
]]--
function lua_cpssDxChPhaSharedMemoryErspanDeviceIdGet(command_data,devNum)
    if(command_data == nil) then
        command_data = Command_Data()
    end

    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssDxChPhaSharedMemoryErspanGlobalConfigGet", {
        { "IN",  "GT_U8" , "devNum"      , devNum },
        { "OUT", "GT_U16", "erspanDevId"},
        { "OUT", "GT_BOOL","isVoQ"}
        })

    if did_error then
        --error indication
        return false , nil
    end

    return true , OUT_values
end

local threadType_convertCpssToShow = {
     ["CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E"                  ]                       = "unused"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E"]                       = "ioam ingress ipv4"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E"]                       = "ioam ingress ipv6"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E"]                       = "ioam transit ipv4"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E"]                       = "ioam transit ipv6"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E"      ]                       = "ioam mirroring"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E"  ]                       = "ioam egress"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E"]                                  = "MPLS SR no EL"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E"]                                 = "MPLS SR one EL"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E"]                                 = "MPLS SR two EL"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E"]                               = "MPLS SR three EL"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR"]                                       = "Unified SR"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E"]                   = "Classifier NSH over ethernet"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E"]                  = "Classifier NSH over VXLAN GPE"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E"]                  = "SFF NSH VXLAN GPE to ethernet"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E"]                  = "SFF NSH ethernet to VXLAN GPE"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E"]                        = "IOAM Egress IPv6"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E"]                                  = "SRv6 End Node"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E"]                      = "SRv6 Penultimate Node"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E"]                        = "SRv6 source node(1 segment)"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E"]          = "SRv6 source node first pass"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E"]           = "SRv6 Source node second pass(3 segments)"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E"]           = "SRv6 Source node second pass(2 segments)"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E"]                            = "SGT Network ADD MSB"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E"]                                = "SGT Network Fix"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E"]                             = "SGT Network Remove"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E"]                                   = "SGT eDSA Fix"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E"]                                = "SGT eDSA Remove"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E"]                               = "SGT GBP Fix IPv4"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E"]                               = "SGT GBP Fix IPv6"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E"]                            = "SGT GBP Remove IPv4"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E"]                            = "SGT GBP Remove IPv6"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E"]                                 = "PTP 1 Step"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E"]                      = "Egress Mirroring Metadata"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E"]                      = "Source Device Mirroring ERSPAN Type II"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E"]              = "Target Device LC Mirroring IPV4 ERSPAN Type II"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E"]              = "Target Device LC Mirroring IPV6 ERSPAN Type II"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E"]          = "Target Device Direct Mirroring IPV4 ERSPAN Type II"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E"]          = "Target Device Direct Mirroring IPV6 ERSPAN Type II"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E"]               = "VXLAN GBP Source Group Policy ID"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E"]                               = "SRv6 Best Effort Tunnels"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E"]                      = "SRv6 G-SID Tunnels Single Pass with 1 SRH Container"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E"]        = "SRv6 G-SID Tunnels First Pass with 2/3 SRH Containers"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E"]         = "SRv6 G-SID Tunnels Second Pass with 2 SRH Containers"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E"]         = "SRv6 G-SID Tunnels Second Pass with 3 SRH Containers"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E"]                       = "SRv6 G-SID Tunnels DIP with the next G-SID from SRH"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E"]                             = "IPv4 TTL Increment"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E"]                       = "IPv6 Hop Limit Increment"
    ,["CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E"]                    = "Clear Outgoing Mtag Command"
}

local bussyMode_convertCpssToShow = {
     ["CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E"     ]  = "push back"
    ,["CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E"          ]  = "drop"
    ,["CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E"]  = "ordered bypass"
}


local CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E = 1024
local CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 255
local CPSS_NET_FIRST_USER_DEFINED_E       = 500
local CPSS_NET_LAST_USER_DEFINED_E        = CPSS_NET_FIRST_USER_DEFINED_E + 63
local HW_CPU_CODE_USER_DEFINED            = 192
local vxlanPhaThreadId                    = 11

-- cpu code / drop code holds the
--  'enum' string , like  : CPSS_NET_TARGET_NULL_PORT_E
-- but also may hold 'hex' value , like : "value 0x478" (0x400 + 0x78 --> HW value of 120)
-- or 'hex' value , like : value 0x1fc -- in range of CPSS_NET_FIRST_USER_DEFINED_E (500) ... CPSS_NET_LAST_USER_DEFINED_E (563)
-- need to convert "value 0x478" to "hw 120"
-- need to convert "value 0x1fc" to "hw 200"
local function get_cpu_code_string(origCpuCodeString)
    local removeValue = string.gsub(origCpuCodeString, "value ", "")
    local numberValue = tonumber(removeValue)

    if(nil == numberValue) then
        return origCpuCodeString
    end

    if (numberValue >= CPSS_NET_FIRST_USER_DEFINED_E and
        numberValue <= CPSS_NET_LAST_USER_DEFINED_E) then
        numberValue = HW_CPU_CODE_USER_DEFINED + (numberValue - CPSS_NET_FIRST_USER_DEFINED_E)
    elseif numberValue >= CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E and
           numberValue <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E then
        numberValue = numberValue - CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E
    else
        -- unknown value ?!
        numberValue = nil
    end

    if numberValue then
        return "hw " .. tostring(numberValue)
    else
        return origCpuCodeString
    end
 end

-- per_device__show_pha_info_common_func
--  per device iterator for PHA common info
--
local function per_device_per_phaThreadId__show_pha_info_common_func(command_data,devNum,phaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..phaThreadId
    local myString

    local statisticalProcessingFactor = currInfo.commonInfo.statisticalProcessingFactor
    local busyStallMode = bussyMode_convertCpssToShow[currInfo.commonInfo.busyStallMode]
    local stallDropCode = currInfo.commonInfo.stallDropCode
    local extType = threadType_convertCpssToShow[currInfo.extType]

    if(nil == busyStallMode) then busyStallMode = "ERROR" end
    if(nil == extType)       then extType       = "ERROR" end

    -- stallDropCode holds the 'enum' string or : "value 0x437" (0x400 + 0x37 --> HW value of 0x35)
    -- need to convert "value 0x437" to "hw 0x37"
    stallDropCode = get_cpu_code_string(stallDropCode)

    myString = string.format("%-8s %-15s %-15s ", tostring(statisticalProcessingFactor), tostring(busyStallMode), tostring(stallDropCode))
    myString = string.format("%-42s ", extType) .. myString
    myString = string.format("%-13s ", prefix)  .. myString

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end
-- per_device__show_pha_info_common_func
--  per device iterator for PHA common info
--
local function per_device__show_pha_info_common_func(command_data,devNum,params)
    for phaThreadId = pha___thread_id__min , pha___thread_id__max do
        local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
        if isOk == true then
            if currInfo.extType == "CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E" and params.show_dummy ~= true then
                -- we ignore this type of entry
            else
                -- we print the common part of the entry
                per_device_per_phaThreadId__show_pha_info_common_func(command_data,devNum,phaThreadId,currInfo)
            end
        end
    end
end

--  PHA common info
 local function show_pha_info_common_func(params)

    --====================
    --====================
    params.header_string =
        "\n" ..
        "PHA common info: \n" ..
        "\n" ..
        "Dev/threadId | info type                                | factor | stallMode     | dropCode          \n" ..
        "-------------|------------------------------------------|--------|---------------|------------------"

    generic_all_device_show_func(per_device__show_pha_info_common_func,params)
end

CLI_addCommand("exec", "show pha-info common", {
  func   = show_pha_info_common_func,
  help   = help_string_show_pha_info_common,
  params = {
      { type = "named",
          "#all_device",
        --[[ currently we not need this option :
            {format="show-dummy  %bool",  name="show_dummy"  , help = "by default not showing info about dummy thread-Ids"},
            ]]
      }
  }
})

local function isSrv6(extType)
    if extType == "CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E" then
       return true
    end
    return false
end

local function isSrv6iGsid(extType)
    if extType == "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E" then
       return true
    end
    return false
end

local function isIoamIngress(extType)
    if extType == "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E" then
       return true
    end
    return false
end

local function isIoamTransit(extType)
    if extType == "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E" or
       extType == "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E" then
       return true
    end
    return false
end

local function isIoamEgress(extType)
    if extType == "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E" then
       return true
    end
    return false
end

-- support value = nil -- presented as '  -  ' (spaces , with 'minus')
local function buildStringForPrint_HEX(numBits , value)
    local numOfHexNibels = math.floor((numBits+3)/4)
    local stringFormat = "0x%"..numOfHexNibels.."."..numOfHexNibels.."x "
    if value then
        stringFormat = "0x%"..numOfHexNibels.."."..numOfHexNibels.."x "
        return string.format(stringFormat, value)
    end
    --use the needed number of ' ' (spaces) + 2 spaces (that replace the '0x') + 1 space (from original)
    return string.sub("  -     ", 0 ,numOfHexNibels+3)

end

-- per_device_per_phaThreadId__show_pha_info_ioam_func
--  print PHA ioam info
--
local function per_device_per_phaThreadId__show_pha_info_ioam_func(command_data,devNum,phaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..phaThreadId

    local extType = threadType_convertCpssToShow[currInfo.extType]
    if(nil == extType)       then extType       = "ERROR" end
    -- remove the 'ioam '
    local extType = prefix_del("ioam ",extType)

    local IOAM_Trace_Type
    local Maximum_Length
    local Flags
    local Hop_Lim
    local node_id
    local Type1
    local IOAM_HDR_len1
    local Reserved1
    local Next_Protocol1
    local Type2
    local IOAM_HDR_len2
    local Reserved2
    local Next_Protocol2


    if isIoamIngress(currInfo.extType) then
        IOAM_Trace_Type =  currInfo.extInfo.ioamIngressSwitchIpv4.IOAM_Trace_Type;
        Maximum_Length  =  currInfo.extInfo.ioamIngressSwitchIpv4.Maximum_Length;
        Flags           =  currInfo.extInfo.ioamIngressSwitchIpv4.Flags;
        Hop_Lim         =  currInfo.extInfo.ioamIngressSwitchIpv4.Hop_Lim;
        node_id         =  currInfo.extInfo.ioamIngressSwitchIpv4.node_id;
        Type1           =  currInfo.extInfo.ioamIngressSwitchIpv4.Type1;
        IOAM_HDR_len1   =  currInfo.extInfo.ioamIngressSwitchIpv4.IOAM_HDR_len1;
        Reserved1       =  currInfo.extInfo.ioamIngressSwitchIpv4.Reserved1;
        Next_Protocol1  =  currInfo.extInfo.ioamIngressSwitchIpv4.Next_Protocol1;
        Type2           =  currInfo.extInfo.ioamIngressSwitchIpv4.Type2;
        IOAM_HDR_len2   =  currInfo.extInfo.ioamIngressSwitchIpv4.IOAM_HDR_len2;
        Reserved2       =  currInfo.extInfo.ioamIngressSwitchIpv4.Reserved2;
        Next_Protocol2  =  currInfo.extInfo.ioamIngressSwitchIpv4.Next_Protocol2;
    elseif isIoamTransit(currInfo.extType) then -- transit
        node_id         =  currInfo.extInfo.ioamTransitSwitchIpv4.node_id;
    else -- Egress
        node_id         =  currInfo.extInfo.ioamEgressSwitchIpv6.node_id;
    end


    local myString = ""
    myString = myString .. string.format("%-13s ", prefix)
    myString = myString .. string.format("%-13s ", extType)

    myString = myString .. buildStringForPrint_HEX(16,IOAM_Trace_Type)
    myString = myString .. buildStringForPrint_HEX( 8,Maximum_Length )
    myString = myString .. buildStringForPrint_HEX( 8,Flags          )
    myString = myString .. buildStringForPrint_HEX( 8,Hop_Lim        )
    myString = myString .. buildStringForPrint_HEX(24,node_id        )
    myString = myString .. buildStringForPrint_HEX( 8,Type1          )
    myString = myString .. buildStringForPrint_HEX( 8,IOAM_HDR_len1  )
    myString = myString .. buildStringForPrint_HEX( 8,Reserved1      )
    myString = myString .. buildStringForPrint_HEX( 8,Next_Protocol1 )
    myString = myString .. buildStringForPrint_HEX( 8,Type2          )
    myString = myString .. buildStringForPrint_HEX( 8,IOAM_HDR_len2  )
    myString = myString .. buildStringForPrint_HEX( 8,Reserved2      )
    myString = myString .. buildStringForPrint_HEX( 8,Next_Protocol2 )

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_pha_info_ioam_func
--  per device iterator for PHA IOAM info
--
local function per_device__show_pha_info_ioam_func(command_data,devNum,params)
    for phaThreadId = pha___thread_id__min , pha___thread_id__max do
        local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
        if isOk == true then
            if isIoamIngress(currInfo.extType) or isIoamTransit(currInfo.extType) or isIoamEgress(currInfo.extType) then
                -- we print the ioam part of the entry
                per_device_per_phaThreadId__show_pha_info_ioam_func(command_data,devNum,phaThreadId,currInfo)
            end
        end
    end
end

local show_pha_info_ioam_firstTime_abbreviation = true
local show_pha_info_ioam_abbreviation = "IOAM_Trace_Type - TT \n" ..
    "Maximum_Length  - ML \n" ..
    "Flags           - F  \n" ..
    "Hop_Lim         - HL \n" ..
    "node_id         - NI \n" ..
    "Type1           - T1 \n" ..
    "IOAM_HDR_len1   - L1 \n" ..
    "Reserved1       - R1 \n" ..
    "Next_Protocol1  - P1 \n" ..
    "Type2           - T2 \n" ..
    "IOAM_HDR_len2   - L2 \n" ..
    "Reserved2       - R2 \n" ..
    "Next_Protocol2  - P2 \n"

--  PHA ioam info
 local function show_pha_info_ioam_func(params)
    local curr_abbreviation

    if show_pha_info_ioam_firstTime_abbreviation == true then
        show_pha_info_ioam_firstTime_abbreviation = false
        curr_abbreviation = show_pha_info_ioam_abbreviation
    else
        curr_abbreviation = ""
    end

    --====================
    --====================
    params.header_string =
        "\n" ..
        "PHA IOAM info: \n" ..
        curr_abbreviation ..
        "\n" ..
        "Dev/threadId | type        | TT   | ML |  F | HL | NI     | T1 | L1 | R1 | P1 | T1 | L1 | R1 | P1 \n" ..
        "-------------|-------------|------|----|----|----|--------|----|----|----|----|----|----|----|----"


    generic_all_device_show_func(per_device__show_pha_info_ioam_func,params)
end


CLI_addCommand("exec", "show pha-info ioam", {
  func   = show_pha_info_ioam_func,
  help   = help_string_show_pha_info_ioam,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})

-- per_device_per_phaThreadId__show_pha_info_srv6_func
--  print PHA SRv6 info
--
local function per_device_per_phaThreadId__show_pha_info_srv6_func(command_data,devNum,phaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..phaThreadId

    local extType = threadType_convertCpssToShow[currInfo.extType]
    srcAddr =  currInfo.extInfo.unifiedSRIpv6.srcAddr;


    local myString = ""
    myString = myString .. string.format("%-13s ", prefix)
    myString = myString .. string.format("%-28s ", extType)
    myString = myString .. string.format("%-13s ", srcAddr)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end


-- per_device__show_pha_info_srv6_func
--  per device iterator for PHA SRv6 info
--
local function per_device__show_pha_info_srv6_func(command_data,devNum,params)
    for phaThreadId = pha___thread_id__min , pha___thread_id__max do
        local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
        if isOk == true then
            if isSrv6(currInfo.extType) then
                -- we print the SRv6 part of the entry
                per_device_per_phaThreadId__show_pha_info_srv6_func(command_data,devNum,phaThreadId,currInfo)
            end
        end
    end
end

--  PHA Unified SR info
 local function show_pha_info_srv6_func(params)
    params.header_string =
        "\n" ..
        "PHA SRv6 info: \n" ..
        "\n" ..
        "Dev/threadId | type                       | srcAddr  \n" ..
        "-------------|----------------------------|----------"

    generic_all_device_show_func(per_device__show_pha_info_srv6_func,params)
end


CLI_addCommand("exec", "show pha-info srv6", {
  func   = show_pha_info_srv6_func,
  help   = help_string_show_pha_info_srv6,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})

-- per_device_per_phaThreadId__show_pha_info_srv6_gsid_func
--  print PHA SRv6 GSID info
--
local function per_device_per_phaThreadId__show_pha_info_srv6_gsid_func(command_data,devNum,phaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..phaThreadId

    local extType = threadType_convertCpssToShow[currInfo.extType]
    dipCommonPrefixLength =  currInfo.extInfo.srv6Coc32GsidCommonPrefix.dipCommonPrefixLength;


    local myString = ""
    myString = myString .. string.format("%-13s ", prefix)
    myString = myString .. string.format("%-28s ", extType)
    myString = myString .. string.format("%-13s ", dipCommonPrefixLength)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end


-- per_device__show_pha_info_srv6_gsid_func
--  per device iterator for PHA SRv6 GSID info
--
local function per_device__show_pha_info_srv6_gsid_func(command_data,devNum,params)
    for phaThreadId = pha___thread_id__min , pha___thread_id__max do
        local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
        if isOk == true then
            if isSrv6Gsid(currInfo.extType) then
                -- we print the SRv6 part of the entry
                per_device_per_phaThreadId__show_pha_info_srv6_gsid_func(command_data,devNum,phaThreadId,currInfo)
            end
        end
    end
end

--  PHA SRv6 GSID info
 local function show_pha_info_srv6_gsid_func(params)
    params.header_string =
        "\n" ..
        "PHA SRv6 GSID info: \n" ..
        "\n" ..
        "Dev/threadId | type                       | dipCommonPrefixLength  \n" ..
        "-------------|----------------------------|------------------------"

    generic_all_device_show_func(per_device__show_pha_info_srv6_gsid_func,params)
end


CLI_addCommand("exec", "show pha-info srv6gsid", {
  func   = show_pha_info_srv6_gsid_func,
  help   = help_string_show_pha_info_srv6_gsid,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})


-- per_device_per_phaThreadId__show_pha_info_sgt_func
--  print PHA SGT info
--
local function per_device_per_phaThreadId__show_pha_info_sgt_func(command_data,devNum,phaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..phaThreadId

    local extType = threadType_convertCpssToShow[currInfo.extType]

    local myString = ""
    myString = myString .. string.format("%-13s ", currInfo.extInfo.sgtNetwork.etherType)
    myString = myString .. string.format("%-9s ", currInfo.extInfo.sgtNetwork.version)
    myString = myString .. string.format("%-8s ", currInfo.extInfo.sgtNetwork.length)
    myString = myString .. string.format("%-12s ", currInfo.extInfo.sgtNetwork.optionType)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_pha_info_sgt_func
--  per device iterator for PHA SGT info
--
local function per_device__show_pha_info_sgt_func(command_data,devNum,params)
    for phaThreadId = pha___analyzer_id__min , pha___analyzer_id__max do
        local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,phaThreadId)
        if isOk == true then
            if isSrv6(currInfo.extType) then
                -- we print the SGT part of the entry
                per_device_per_phaThreadId__show_pha_info_sgt_func(command_data,devNum,phaThreadId,currInfo)
            end
        end
    end
end

--  PHA Unified SR info
 local function show_pha_info_sgt_func(params)
    params.header_string =
        "\n" ..
        "PHA SGT info: \n" ..
        "\n" ..
        "Dev/threadId | type                       | Ether Type  | Version | Length | Option Type\n" ..
        "-------------|----------------------------|-------------|---------|--------|-------------"

    generic_all_device_show_func(per_device__show_pha_info_sgt_func,params)
end


CLI_addCommand("exec", "show pha-info sgt", {
  func   = show_pha_info_sgt_func,
  help   = help_string_show_pha_info_sgt,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})

-- per_device_per_analyzerId__show_pha_ip_shared_info_erspan_func
--  print PHA ERSPAN ip shared info
--
local function per_device_per_analyzerId__show_pha_ip_shared_info_erspan_func(command_data,devNum,phaAnalyzerId,currInfo)
    local prefix=tostring(devNum).."/"..phaAnalyzerId

    local extType = threadType_convertCpssToShow[currInfo.extType]

    local myString = ""
    myString = myString .. string.format("%-12s ", prefix)
    myString = myString .. string.format("%-25s ", currInfo.entryInfo.protocol)
    myString = myString .. string.format("%-6s ", currInfo.entryInfo.ipInfo.ipv4.dscp)
    myString = myString .. string.format("%-8s ", currInfo.entryInfo.ipInfo.ipv4.flags)
    myString = myString .. string.format("%-5s ", currInfo.entryInfo.ipInfo.ipv4.ttl)
    myString = myString .. string.format("%-10s ", currInfo.entryInfo.ipInfo.ipv4.sipAddr)
    myString = myString .. string.format("%-10s ", currInfo.entryInfo.ipInfo.ipv4.dipAddr)
    myString = myString .. string.format("%-6s ", currInfo.entryInfo.ipInfo.ipv6.tc)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.ipInfo.ipv6.flowLabel)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.ipInfo.ipv6.hopLimit)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.ipInfo.ipv6.sipAddr)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.ipInfo.ipv6.sipAddr)
    myString = myString .. string.format("%-6s ", currInfo.entryInfo.ingressSessionId)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.egressSessionId)
    myString = myString .. "\n"
    myString = myString .. string.format("%-12s ", prefix)
    myString = myString .. string.format("%-25s ", currInfo.entryInfo.l2Info.macDa)
    myString = myString .. string.format("%-25s ", currInfo.entryInfo.l2Info.macSa)
    myString = myString .. string.format("%-6s  ", currInfo.entryInfo.l2Info.tpid)
    myString = myString .. string.format("%-5s ", currInfo.entryInfo.l2Info.up)
    myString = myString .. string.format("%-5s ", currInfo.entryInfo.l2Info.cfi)
    myString = myString .. string.format("%-12s ", currInfo.entryInfo.l2Info.vid)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_pha_ip_shared_info_erspan_func
--  per device iterator for PHA IP Shared ERSPAN info
--
local function per_device__show_pha_ip_shared_info_erspan_func(command_data,devNum,params)
    for phaAnalyzerId = pha___analyzer_id__min , pha___analyzer_id__max do
        local isOk , currInfo = lua_cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(command_data,devNum,phaAnalyzerId)
        if isOk == true then
            -- we print the ERSPAN IP part of the entry
            per_device_per_analyzerId__show_pha_ip_shared_info_erspan_func(command_data,devNum,phaAnalyzerId,currInfo)
        end
    end
end

--  PHA ERSPAN IPv4 Shared info
 local function show_pha_ip_shared_info_erspan_func(params)
    params.header_string =
    "\n" ..
    "PHA ERSPAN SHARED info: \n" ..
    "\n" ..
    "Dev/ErspanId|        Protocol       | DSCP | Flags | TTL |  IPv4 SIP | IPv4 DIP |  TC | FlowLabel | HopLimit |   IPv6 SIP  |   IPv6 DIP  | ISID | ESID \n" ..
    "------------|-----------------------|------|-------|-----|-----------|----------|-----|-----------|----------|-------------|-------------|------|------\n" ..
    "\n" ..
    "Dev/threadId |       MAC DA       |        MAC SA      | TPID | CFI |  UP |  VID \n" ..
    "-------------|--------------------|--------------------|------|-----|-----|--------"

    generic_all_device_show_func(per_device__show_pha_ip_shared_info_erspan_func,params)
end


CLI_addCommand("exec", "show pha-ip-shared-info erspan", {
  func   = show_pha_ip_shared_info_erspan_func,
  help   = help_string_show_pha_ip_shared_info_erspan,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})

-- per_device__show_pha_global_info_erspan_func
--  print PHA ERSPAN global info
--
local function per_device__show_pha_global_info_erspan_func(command_data,devNum,currInfo)
    local prefix=tostring(devNum)

    if currInfo.isVoQ == 1 then
       currInfo.isVoQ = "TRUE"
    else
       currInfo.isVoQ = "FALSE"
    end

    local myString = ""
    myString = myString .. string.format("%-6s ", prefix)
    myString = myString .. string.format("%-16s ", currInfo.erspanDevId)
    myString = myString .. string.format("%-15s ", currInfo.isVoQ)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_pha_global_shared_info_erspan_func
--  per device iterator for PHA GLOBAL Shared ERSPAN info
--
local function per_device__show_pha_global_shared_info_erspan_func(command_data,devNum,params)
    local isOk , currInfo = lua_cpssDxChPhaSharedMemoryErspanDeviceIdGet(command_data,devNum)
    if isOk == true then
        -- we print the ERSPAN GLOBAL part of the entry
        per_device__show_pha_global_info_erspan_func(command_data,devNum,currInfo)
    end
end

--  PHA ERSPAN GLOBAL Shared info
 local function show_pha_global_shared_info_erspan_func(params)
    params.header_string =
    "\n" ..
    "PHA ERSPAN SHARED info: \n" ..
    "\n" ..
    "Dev| ERSPAN Device ID | isVoQ \n" ..
    "---|------------------|--------"

    generic_all_device_show_func(per_device__show_pha_global_shared_info_erspan_func,params)
end


CLI_addCommand("exec", "show pha-global-shared-info erspan", {
  func   = show_pha_global_shared_info_erspan_func,
  help   = help_string_show_pha_global_shared_info_erspan,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})

-- per_device_per_phaThreadId__show_pha_info_vxlan_gbp_func
--  print PHA VXLAN GBP info
--
local function per_device_per_phaThreadId__show_pha_info_vxlan_gbp_func(command_data,devNum,vxlanPhaThreadId,currInfo)
    local prefix=tostring(devNum).."/"..vxlanPhaThreadId

    local extType = threadType_convertCpssToShow[currInfo.extType]

    local myString = ""
    myString = myString .. string.format("%-14s ", prefix)
    myString = myString .. string.format("%-20s ", currInfo.extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb)
    myString = myString .. string.format("%-9s ", currInfo.extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb)

    command_data["result"] = myString

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_pha_info_vxlan_gbp_func
--  per device iterator for PHA VXLAN GBP info
--
local function per_device__show_pha_info_vxlan_gbp_func(command_data,devNum,params)
    local isOk , currInfo = lua_cpssDxChPhaThreadIdEntryGet(command_data,devNum,vxlanPhaThreadId)
    if isOk == true then
        -- we print the VXLAN GBP part of the entry
        per_device_per_phaThreadId__show_pha_info_vxlan_gbp_func(command_data,devNum,vxlanPhaThreadId,currInfo)
    end
end

 local function show_pha_info_vxlan_gbp_func(params)
    params.header_string =
        "\n" ..
        "PHA VXLAN GBP info: \n" ..
        "\n" ..
        "Dev/threadId | CopyReservedLSB  | CopyReservedMSB \n" ..
        "-------------|------------------|----------------"

    generic_all_device_show_func(per_device__show_pha_info_vxlan_gbp_func,params)
end

CLI_addCommand("exec", "show pha-info vxlan-gbp", {
  func   = show_pha_info_vxlan_gbp_func,
  help   = help_string_show_pha_info_vxlan_gbp,
  params = {
      { type = "named",
          "#all_device",
      }
  }
})
