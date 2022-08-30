--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer.lua
--*
--* DESCRIPTION:
--*      Packet analyzer commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerLogicalKeyCreate")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupRuleAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupRuleFieldsAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerLogicalKeyFieldsAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerActionInverseSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerActionSamplingModeSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsRemove")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerSamplingAllInterfacesSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupRuleDelete")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerLogicalKeyDelete")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerManagerNumDevicesGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerNumOfInterfacesGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerInterfaceGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerNumOfFieldsGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerFieldGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerNumOfUdfsGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerUdfGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerNumOfUdssGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerUdsGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupActivateEnableGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerStageValidityGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerPreDefinedStageInfoGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerFieldEnumGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerIndexesForInterfaceAndInstanceGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPacketAnalyzerInstanceAllInterfacesGet")

local NOT_INITIALIZED = 0xFFFF
local DEFAULT = 0xFF
local DEFAULT_SIP6 = 0xFE
local MODIFY_MODE = 0xFD
local TABLE_TO_USE = DEFAULT
local INIT_PA_STRINGS = NOT_INITIALIZED

paDb = {}
paDb.paTable = NOT_INITIALIZED
paDb.paGrpTable = {}
paDb.activateGroup = {}
paDb.configuration = {}

local GT_BAD_PARAM = 4
local GT_BAD_STATE = 7
local GT_ALREADY_EXIST = 27
local GT_NOT_FOUND = 11
local GT_NOT_SUPPORTED = 16
local GT_NOT_INITIALIZED = 18

local CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E = 0
local CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E = 1
local CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E = 47
local CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS  = 128

local id_all = 1
local id_tti = 2
local id_ipcl = 3
local id_bridge = 4
local id_router = 5
local id_ioam = 6
local id_ipolicer = 7
local id_mll = 8
local id_equeue = 9
local id_filter = 10
local id_tqueue = 11
local id_header_alt = 12
local id_eoam = 13
local id_epcl = 14
local id_epolicer = 15
local id_timestamp = 16
local id_mac = 17
local id_uds0 = 18
local id_uds1 = 19
local id_uds2 = 20
local id_uds3 = 21
local id_uds4 = 22
local id_uds5 = 23
local id_uds6 = 24
local id_uds7 = 25
local id_uds8 = 26
local id_uds9 = 27
local id_uds10 = 28
local id_uds11 = 29
local id_uds12 = 30
local id_uds13 = 31
local id_uds14 = 32
local id_uds15 = 33
local id_uds16 = 34
local id_uds17 = 35
local id_uds18 = 36
local id_uds19 = 37
local id_pha = 38
local id_replication = 39

--update if stage added or removed
local id_uds_first=id_uds0
local id_uds_last=id_uds19
local id_last=id_replication

local paTable = {}
-- default manager 1
paTable[DEFAULT] = {}
-- default {manager ,device 0} only static parameters!!!
-- table index correlated to stage's enum order
paTable[DEFAULT][0] = {
                  [0] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",["id"]=id_tti,
                     ["att"]="TTI",["group"]=2},
                  [1] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",["id"]=id_ipcl,
                     ["att"]="IPCL",["group"]=2},
                  [2] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",["id"]=id_bridge,
                     ["att"]="BRIDGE",["group"]=2},
                  [3] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",["id"]=id_router,
                     ["att"]="ROUTER",["group"]=2},
                  [4] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",["id"]=id_ioam,
                     ["att"]="IOAM",["group"]=2},
                  [5] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",["id"]=id_ipolicer,
                     ["att"]="IPOLICER",["group"]=2},
                  [6] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",["id"]=id_mll,
                     ["att"]="MLL",["group"]=2},
                  [7] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",["id"]=id_equeue,
                     ["att"]="EQUEUE",["group"]=2},
                  [8] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",["id"]=id_filter,
                     ["att"]="FILTER",["group"]=2},
                  [9] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",["id"]=id_tqueue,
                     ["att"]="TQUEUE",["group"]=2},
                  [10] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",["id"]=id_header_alt,
                     ["att"]="HEADER ALT",["group"]=2},
                  [11] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",["id"]=id_eoam,
                     ["att"]="EOAM",["group"]=2},
                  [12] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",["id"]=id_epcl,
                     ["att"]="EPCL",["group"]=2},
                  [13] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",["id"]=id_epolicer,
                     ["att"]="EPOLICER",["group"]=2},
                  [14] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",["id"]=id_timestamp,
                     ["att"]="TIMESTAMP",["group"]=2},
                  [15] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",["id"]=id_mac,
                     ["att"]="MAC",["group"]=2},
                  [18] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",["id"]=id_uds0,
                     ["att"]="UDS0",["group"]=2},
                  [19] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",["id"]=id_uds1,
                     ["att"]="UDS1",["group"]=2},
                  [20] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",["id"]=id_uds2,
                     ["att"]="UDS2",["group"]=2},
                  [21] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",["id"]=id_uds3,
                     ["att"]="UDS3",["group"]=2},
                  [22] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",["id"]=id_uds4,
                     ["att"]="UDS4",["group"]=2},
                  [23] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",["id"]=id_uds5,
                     ["att"]="UDS5",["group"]=2},
                  [24] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",["id"]=id_uds6,
                     ["att"]="UDS6",["group"]=2},
                  [25] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",["id"]=id_uds7,
                     ["att"]="UDS7",["group"]=2},
                  [26] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",["id"]=id_uds8,
                     ["att"]="UDS8",["group"]=2},
                  [27] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",["id"]=id_uds9,
                     ["att"]="UDS9",["group"]=2},
                  [28] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",["id"]=id_uds10,
                     ["att"]="UDS10",["group"]=2},
                  [29] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",["id"]=id_uds11,
                     ["att"]="UDS11",["group"]=2},
                  [30] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",["id"]=id_uds12,
                     ["att"]="UDS12",["group"]=2},
                  [31] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",["id"]=id_uds13,
                     ["att"]="UDS13",["group"]=2},
                  [32] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",["id"]=id_uds14,
                     ["att"]="UDS14",["group"]=2},
                  [33] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",["id"]=id_uds15,
                     ["att"]="UDS15",["group"]=2},
                  [34] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",["id"]=id_uds16,
                     ["att"]="UDS16",["group"]=2},
                  [35] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",["id"]=id_uds17,
                     ["att"]="UDS17",["group"]=2},
                  [36] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",["id"]=id_uds18,
                     ["att"]="UDS18",["group"]=2},
                  [37] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E",["id"]=id_uds19,
                     ["att"]="UDS19",["group"]=2},
                  [40] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E",["id"]=id_all,
                     ["att"]="PIPLINE",["group"]=1},
    }

paTable[DEFAULT_SIP6] = {}
-- default {manager ,device 0} only static parameters!!!
paTable[DEFAULT_SIP6][0] = {
                  [0] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",["id"]=id_tti,
                     ["att"]="TTI",["group"]=2},
                  [1] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",["id"]=id_ipcl,
                     ["att"]="IPCL",["group"]=2},
                  [2] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",["id"]=id_bridge,
                     ["att"]="BRIDGE",["group"]=2},
                  [3] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",["id"]=id_router,
                     ["att"]="ROUTER",["group"]=2},
                  [4] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",["id"]=id_ioam,
                     ["att"]="IOAM",["group"]=2},
                  [5] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",["id"]=id_ipolicer,
                     ["att"]="IPOLICER",["group"]=2},
                  [6] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",["id"]=id_mll,
                     ["att"]="MLL",["group"]=2},
                  [7] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",["id"]=id_equeue,
                     ["att"]="EQUEUE",["group"]=2},
                  [8] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",["id"]=id_filter,
                     ["att"]="FILTER",["group"]=2},
                  [9] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",["id"]=id_header_alt,
                     ["att"]="HEADER ALT",["group"]=2},
                  [10] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",["id"]=id_eoam,
                     ["att"]="EOAM",["group"]=2},
                  [11] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",["id"]=id_epcl,
                     ["att"]="EPCL",["group"]=2},
                  [12] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",["id"]=id_epolicer,
                     ["att"]="EPOLICER",["group"]=2},
                  [13] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",["id"]=id_timestamp,
                     ["att"]="TIMESTAMP",["group"]=2},
                  [14] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",["id"]=id_pha,
                     ["att"]="PHA",["group"]=2},
                  [15] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E",["id"]=id_replication,
                     ["att"]="REPLICATION",["group"]=2},
                  [16] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",["id"]=id_tqueue,
                     ["att"]="TQUEUE",["group"]=2},
                  [17] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",["id"]=id_mac,
                     ["att"]="MAC",["group"]=2},
                  [18] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",["id"]=id_uds0,
                     ["att"]="UDS0",["group"]=2},
                  [19] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",["id"]=id_uds1,
                     ["att"]="UDS1",["group"]=2},
                  [20] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",["id"]=id_uds2,
                     ["att"]="UDS2",["group"]=2},
                  [21] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",["id"]=id_uds3,
                     ["att"]="UDS3",["group"]=2},
                  [22] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",["id"]=id_uds4,
                     ["att"]="UDS4",["group"]=2},
                  [23] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",["id"]=id_uds5,
                     ["att"]="UDS5",["group"]=2},
                  [24] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",["id"]=id_uds6,
                     ["att"]="UDS6",["group"]=2},
                  [25] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",["id"]=id_uds7,
                     ["att"]="UDS7",["group"]=2},
                  [26] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",["id"]=id_uds8,
                     ["att"]="UDS8",["group"]=2},
                  [27] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",["id"]=id_uds9,
                     ["att"]="UDS9",["group"]=2},
                  [28] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",["id"]=id_uds10,
                     ["att"]="UDS10",["group"]=2},
                  [29] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",["id"]=id_uds11,
                     ["att"]="UDS11",["group"]=2},
                  [30] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",["id"]=id_uds12,
                     ["att"]="UDS12",["group"]=2},
                  [31] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",["id"]=id_uds13,
                     ["att"]="UDS13",["group"]=2},
                  [32] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",["id"]=id_uds14,
                     ["att"]="UDS14",["group"]=2},
                  [33] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",["id"]=id_uds15,
                     ["att"]="UDS15",["group"]=2},
                  [34] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",["id"]=id_uds16,
                     ["att"]="UDS16",["group"]=2},
                  [35] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",["id"]=id_uds17,
                     ["att"]="UDS17",["group"]=2},
                  [36] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",["id"]=id_uds18,
                     ["att"]="UDS18",["group"]=2},
                  [37] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E",["id"]=id_uds19,
                     ["att"]="UDS19",["group"]=2},
                  [40] =
                    {["stage"]="CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E",["id"]=id_all,
                     ["att"]="PIPLINE",["group"]=1},
    }

-- Grps for default manager
paTable[DEFAULT]["pipeline"] = 1
paTable[DEFAULT]["stage"] = 2

-----------------------------------------------------------------------------------------------------------
local instances = {}
local instancesHelp = {}
local indexes = {}
local indexesHelp = {}

local function string_manipulations(name)
    name = string.gsub(name , "^%s$", "") -- remove spaces from the begining and end of the string
    name = string.gsub(name , " ", "-")   -- replace spaces with "-"
    name = string.lower(name)             -- change to lower case letters

    return name
end

local function fieldIndex_to_fieldName(field)

    local status = true
    local err
    local result
    local str

    -- check if fieldIndex is udf; then convert from 'name' to idebug field
    -- if PA fieldIndex then convert from index to idebug field
    result,firstUdf,lastUdf = wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet()
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet returned "..to_string(err))
        return status,err
    end
    if (tonumber(field,10) >= tonumber(firstUdf,10) and tonumber(field,10) <= tonumber(lastUdf,10)) then
      str = printPaFieldUdf[field]
    else
      str = printPaField[field]
    end

    return status, str

end

local function check_pa_field_value(param)

    local currentField

    -- convert from fieldIndex to idebug field
    result, currentField = fieldIndex_to_fieldName(field)
    if (result ~= true) then
        status = false
        err = returnCodes[result]
        print("fieldIndex_to_fieldName returned "..to_string(err))
        return status,err
    end

    if "ipv4_dip" == currentField or "ipv4_sip" == currentField then
        return check_param_ipv4(param, data)
    elseif "ipv6_dip" == currentField or "ipv6_sip" == currentField then
        return check_param_ipv6(param,data)
    elseif "mac_da" == currentField or "mac_sa" == currentField then
        return check_param_mac(param, data)
    else

        -- handle enumerations
        result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,currentField)
        if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
        end

        if numOfEnums ~= 0 then

          for i = 1, numOfEnums do
              result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,currentField,i-1)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
              end

              name = string_manipulations(name)

              if (name == param) then
                return true, param
              end

          end

          return false, "Invalid Value"

        end

        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerFieldSizeGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "fieldName", field },
                            { "OUT", "GT_U32", "lengthPtr", lengthPtr }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerFieldSizeGet returned "..err)
        end

        if (string.lower(string.sub(param,1,2))~="0x") then
          -- decimal
          for i = 1 ,string.len(param) do
              c = string.sub(param, i, i)
              if (not (c >= '0' and c <= '9')) then
                  return false, "Found a non-numeric character"
              end
          end
          if tonumber(param,10) <= 2^(values["lengthPtr"])-1 and tonumber(param, 10) >= 0 then
                return true, param
          end
        else
          -- hex
          for i = 3 ,string.len(param) do
              c = string.sub(param, i, i)
              if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
                     (c >= 'A' and c <= 'F'))  ) then
                  return false, "Found a non-numeric character"
              end
          end
          if tonumber(param, 16) <= 2^(values["lengthPtr"])-1 and tonumber(param, 16) >= 0 then
                return true, param
          end
        end

    end

    return false, "Invalid Value"
end

local function complete_param_ipv4(param)
    local values =
        {{"0.0.0.0",   "Default ipv4"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

local function complete_param_ipv6(param)
    local values =
        {{"0::0",   "Default ipv6"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

local function complete_param_packet_trace(param)
    local values =
        {{"1",   "Default packet-trace value"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

local function complete_pa_field_value(param)
    local help = {}
    local currentField

    -- convert from fieldIndex to idebug field
    result, currentField = fieldIndex_to_fieldName(field)
    if (result ~= true) then
        status = false
        err = returnCodes[result]
        print("fieldIndex_to_fieldName returned "..to_string(err))
        return status,err
    end

    if "ipv4_dip" == currentField or "ipv4_sip" == currentField then
        return complete_param_ipv4(param)
    elseif "ipv6_dip" == currentField or "ipv6_sip" == currentField then
        return complete_param_ipv6(param)
    elseif "mac_da" == currentField or "mac_sa" == currentField then
        return complete_param_mac(param)
    elseif "packet_trace" == currentField then
        return complete_param_packet_trace(param)
    else

        -- handle enumerations
        result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,currentField)
        if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
        end

        if numOfEnums ~= 0 then

          local values = {}
          for i = 1, numOfEnums do
              result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,currentField,i-1)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
              end

              name = string_manipulations(name)

              values[i] = {}
              values[i][1] = name               --string
              values[i][2] = tonumber(value,10) --help; convert string to digit

          end

          return complete_param_with_predefined_values(param,name,desc,values)

        end

        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerFieldSizeGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "fieldName", field },
                            { "OUT", "GT_U32", "lengthPtr", lengthPtr }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerFieldSizeGet returned "..err)
        end

        help.def = "a number between [0.."..string.format("0x%X", 2^(values["lengthPtr"])-1).."]"
        return {}, help

    end

end

CLI_type_dict["pa_field_value"] = {
    checker = check_pa_field_value,
    complete = complete_pa_field_value,
    help = "Packet-Analyzer field value"
}

local function check_pa_field_mask(param)
    local currentField

    -- convert from fieldIndex to idebug field
    result, currentField = fieldIndex_to_fieldName(field)
    if (result ~= true) then
        status = false
        err = returnCodes[result]
        print("fieldIndex_to_fieldName returned "..to_string(err))
        return status,err
    end

    if "ipv4_dip" == currentField or "ipv4_sip" == currentField then
        return check_param_ipv4_mask(param, data)
    elseif "ipv6_dip" == currentField or "ipv6_sip" == currentField then
        return check_param_ipv6_mask(param,data)
    elseif "mac_da" == currentField or "mac_sa" == currentField then
        return CLI_check_param_macMask(param, data)
    else

      -- handle enumerations
        result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,currentField)
        if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
        end

        if numOfEnums ~= 0 then

          for i = 1, numOfEnums do
              result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,currentField,i-1)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
              end

              if (string.format("0x%X", value) == param) then
                return true, param
              end

          end

          return false, "Invalid Value"

        end

        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerFieldSizeGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "fieldName", field },
                            { "OUT", "GT_U32", "lengthPtr", lengthPtr }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerFieldSizeGet returned "..err)
        end

        if (string.lower(string.sub(param,1,2))~="0x") then
          return false, "Mask value must start with \"0x\""
        end

        for i = 3 ,string.len(param) do
              c = string.sub(param, i, i)
              if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
                     (c >= 'A' and c <= 'F'))  ) then
                  return false, "Found a non-numeric character"
              end
        end
        if tonumber(param, 16) <= 2^(values["lengthPtr"])-1 and tonumber(param, 16) >= 0 then
                return true, param
        end

    end

    return false, "Invalid Value"
end

local function complete_param_ipv4_mask(param, name, desc)
    local values =
        {{"255.255.255.255",   "Default ipv4 32-bit mask"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

function complete_param_mac_mask(param, name, desc)
    local values =
        {{"FF:FF:FF:FF:FF:FF",  "Default mac-address mask"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

local function complete_param_packet_trace_mask(param, name, desc)
    local values =
        {{"0x1",   "Default packet-trace mask"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

local function complete_pa_field_mask(param)
    local currentField
    local help = {}
    local mask

    -- convert from fieldIndex to idebug field
    result, currentField = fieldIndex_to_fieldName(field)
    if (result ~= true) then
        status = false
        err = returnCodes[result]
        print("fieldIndex_to_fieldName returned "..to_string(err))
        return status,err
    end

    if "ipv4_dip" == currentField or "ipv4_sip" == currentField then
        return complete_param_ipv4_mask(param)
    elseif "ipv6_dip" == currentField or "ipv6_sip" == currentField then
        return complete_param_ipv6_mask(param)
    elseif "mac_da" == currentField or "mac_sa" == currentField then
        return complete_param_mac_mask(param)
    elseif "packet_trace" == currentField then
        return complete_param_packet_trace_mask(param)
    else

        -- handle enumerations
        result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,currentField)
        if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
        end

        if numOfEnums ~= 0 then

          local values = {}

          for i = 1, numOfEnums do
              result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,currentField,i-1)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
              end

              values[i] = {}
              values[i][1] = string.format("0x%X", value) --string
              values[i][2] = string.format("0x%X", value) --help
          end

          return complete_param_with_predefined_values(param,name,desc,values)

        end

        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerFieldSizeGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "fieldName", field },
                            { "OUT", "GT_U32", "lengthPtr", lengthPtr }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerFieldSizeGet returned "..err)
            return false, param
        end

        help.def = "a number between [0.."..string.format("0x%X", 2^(values["lengthPtr"])-1).."]"
        return {}, help

    end

end

CLI_type_dict["pa_field_mask"] = {
    checker = check_pa_field_mask,
    complete = complete_pa_field_mask,
    help = "Packet-Analyzer field mask"
}

local function check_pa_fields_for_stage(param, data)
    local status = true
    local err
    local result
    local applicStage
    local stages = {} -- all applicable stages

    -- prepare all applicable stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end
    if values["numOfApplicStagesPtr"] ~= 0 then
        for i = 1, values["numOfApplicStagesPtr"] do
            applicStaget = values["applicStagesListArr"][i-1]
            applicStage = printPaStageCpssInt[applicStaget]
            table.insert(stages ,printPaStageCpssIndex[applicStage])
        end
    end

    if data == "all" and param == "all" then
      return true, param
    end
    if data == "dump" and param == "dump" then
      return true, param
    end
    if data == "dump" and param == "all" then
      return true, param
    end

    if (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "pipeline" and paDb.configuration[paDb.managerId]["pipeline"] ~= "packet-trace") or
       (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "stage" and paDb.configuration[paDb.managerId]["stage"] ~= "packet-trace") or
       (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == nil) then

        for i=1, #stages do

            result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[i] },
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
            end

            for j=1, values["numOfFieldsPtr"] do
                fieldt = values["fieldsArr"][j-1]
                field = printPaFieldCpssInt[fieldt]
                if param == printPaField[field] then
                    return true, param
                end
            end
        end

   elseif (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "pipeline" and paDb.configuration[paDb.managerId]["pipeline"] == "packet-trace") or
          (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "stage" and paDb.configuration[paDb.managerId]["stage"] == "packet-trace") or
          (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == nil) then

       result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageFieldsGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" },
                            { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
        end

        if values["numOfFieldsPtr"] ~= 0 then
            for i = 1, values["numOfFieldsPtr"] do
                fieldt = values["fieldsArr"][i-1]
                field = printPaFieldCpssInt[fieldt]
                if param == printPaField[field] then
                    return true, param
                end
            end
        end

   else
       result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageFieldsGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", paDb.stage },
                            { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
        end

        if values["numOfFieldsPtr"] ~= 0 then
            for i = 1, values["numOfFieldsPtr"] do
                fieldt = values["fieldsArr"][i-1]
                field = printPaFieldCpssInt[fieldt]
                if printPaField[field] == "packet_trace" then
                    if (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" ) and paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" then
                        return false, "Invalid Field Check"
                    end
                end
                if param == printPaField[field] then
                    return true, param
                end
            end
        end

    end

    return false, "Invalid Field Check"
end

local function complete_pa_fields_for_stage(param,data)
    local status = true
    local err
    local result
    local applicStage
    local tCompl = {} --not sorted with duplications stages list
    local compl = {}  --sorted without duplications stages list
    local help = {}
    local stages = {} -- all applicable stages

    -- prepare all applicable stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end
    if values["numOfApplicStagesPtr"] ~= 0 then
        for i = 1, values["numOfApplicStagesPtr"] do
            applicStaget = values["applicStagesListArr"][i-1]
            applicStage = printPaStageCpssInt[applicStaget]
            table.insert(stages ,printPaStageCpssIndex[applicStage])
        end
    end

    if (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "pipeline" and paDb.configuration[paDb.managerId]["pipeline"] ~= "packet-trace") or
       (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "stage" and paDb.configuration[paDb.managerId]["stage"] ~= "packet-trace") or
       (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == nil) then

        for i=1, #stages do

            result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[i] },
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
            end

            for j=1, values["numOfFieldsPtr"] do
                fieldt = values["fieldsArr"][j-1]
                field = printPaFieldCpssInt[fieldt]
                fieldStr = printPaField[field]
                if fieldStr ~= "packet_trace" then
                    table.insert(tCompl ,fieldStr)
                end
            end

        end

        table.sort(tCompl)

        for key,value in ipairs(tCompl) do
            if value ~=tCompl[key+1] then
                table.insert(compl,value)
                table.insert(help," ")
            end
        end

    elseif (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "pipeline" and paDb.configuration[paDb.managerId]["pipeline"] == "packet-trace") or
           (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == "stage" and paDb.configuration[paDb.managerId]["stage"] == "packet-trace") or
           (paDb.stage == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.config == nil) then

        result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageFieldsGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" },
                            { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
        end

        if values["numOfFieldsPtr"] == 0 then
            help.def = "There are no fields for stage "..printPaStage["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E"]
            return {}, help
        end

        for i = 1, values["numOfFieldsPtr"] do
            fieldt = values["fieldsArr"][i-1]
            field = printPaFieldCpssInt[fieldt]
            fieldStr = printPaField[field]
            if fieldStr ~= "packet_trace" then
                table.insert(compl,fieldStr)
                table.insert(help," ")
            end
        end

    else

        result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageFieldsGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", paDb.stage },
                            { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
        end

        if values["numOfFieldsPtr"] == 0 then
            help.def = "There are no fields for stage "..printPaStage[paDb.stage]
            return {}, help
        end

        devFamily = wrlCpssDeviceFamilyGet(paDb.dev)
        for i = 1, values["numOfFieldsPtr"] do
            fieldt = values["fieldsArr"][i-1]
            field = printPaFieldCpssInt[fieldt]
            fieldStr = printPaField[field]
            if fieldStr == "packet_trace" then
                if (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" ) and paDb.stage ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" then
                    table.insert(compl,fieldStr)
                    table.insert(help," ")
                end
            else
                table.insert(compl,fieldStr)
                table.insert(help," ")
            end
        end
    end

    if data == "all" then
      table.insert(compl,"all")
      table.insert(help," ")
    end

    if data == "dump" then
      table.insert(compl,"all")
      table.insert(help," ")
      table.insert(compl,"dump")
      table.insert(help," ")
    end

    return compl, help
end

CLI_type_dict["pa_fields_for_stage"] = {
    checker = check_pa_fields_for_stage,
    complete = complete_pa_fields_for_stage,
    help = "Packet-Analyzer fields for stage"
}
local function check_pa_fields_for_stage_all(param, data)
    return check_pa_fields_for_stage(param,"all")
end

local function check_pa_fields_for_stage_dump(param, data)
    return check_pa_fields_for_stage(param,"dump")
end

local function complete_pa_fields_for_pipeline(param)
    paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
    return complete_pa_fields_for_stage(param,"dump")
end

CLI_type_dict["pa_fields_for_pipeline"] = {
    checker = check_pa_fields_for_stage_dump,
    complete = complete_pa_fields_for_pipeline,
    help = "Packet-Analyzer fields for pipeline"
}

local function complete_pa_fields_for_stage1(param)
    return complete_pa_fields_for_stage(param,"dump")
end

CLI_type_dict["pa_fields_for_stage1"] = {
    checker = check_pa_fields_for_stage_dump,
    complete = complete_pa_fields_for_stage1,
    help = "Packet-Analyzer fields for stage"
}

local function complete_pa_fields_for_stage_all(param)
    paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
    return complete_pa_fields_for_stage(param,"all")
end

local function complete_pa_fields_for_stage_dump(param)
    paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
    return complete_pa_fields_for_stage(param,"dump")
end

CLI_type_dict["pa_fields_for_stage_all"] = {
    checker = check_pa_fields_for_stage_all,
    complete = complete_pa_fields_for_stage_all,
    help = "Packet-Analyzer fields for all stages"
}

CLI_type_dict["pa_fields_for_stage_dump"] = {
    checker = check_pa_fields_for_stage_dump,
    complete = complete_pa_fields_for_stage_dump,
    help = "Packet-Analyzer fields for all stages"
}

local function check_pa_stage(param,data)
    local status = true
    local err
    local result
    local values
    local applicStage
    local stages = {}
    local help = {}

    -- prepare stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end

    if values["numOfApplicStagesPtr"] ~= 0 then
        for i = 1, values["numOfApplicStagesPtr"] do
            applicStaget = values["applicStagesListArr"][i-1]
            applicStage = printPaStageCpssInt[applicStaget]
            if data == "no-ipcl" then
                if applicStaget ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" then
                    if param == printPaStageIndex[applicStage] then
                        paDb.stage = printPaStageCpssIndex[applicStage]
                        return true, param
                    end
                end
            else
                if param == printPaStageIndex[applicStage] then
                    paDb.stage = printPaStageCpssIndex[applicStage]
                    return true, param
                end
            end
        end
    end

    return false, "Invalid Stage Check"

end

local function complete_pa_stage(param,data)
    local status = true
    local err
    local result
    local values
    local applicStage
    local stages = {}
    local help = {}

    -- prepare stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end
    if values["numOfApplicStagesPtr"] ~= 0 then
        for i = 1, values["numOfApplicStagesPtr"] do
            applicStaget = values["applicStagesListArr"][i-1]
            applicStage = printPaStageCpssInt[applicStaget]
            if data == "no-ipcl" then
                if applicStaget ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" then
                    table.insert(stages ,printPaStageIndex[applicStage])
                    table.insert(help,printPaStageIndexHelp[applicStage])
                end
            else
                table.insert(stages ,printPaStageIndex[applicStage])
                table.insert(help,printPaStageIndexHelp[applicStage])
            end
        end
    end

    return stages, help

end


CLI_type_dict["pa_stage"] = {
    checker = check_pa_stage,
    complete = complete_pa_stage,
    help = "PA stage"
}

local function complete_pa_stage_no_ipcl(param)
    return complete_pa_stage(param,"no-ipcl")
end

local function check_pa_stage_no_ipcl(param)
    return check_pa_stage(param,"no-ipcl")
end

CLI_type_dict["pa_stage_no_ipcl"] = {
    checker = check_pa_stage_no_ipcl,
    complete = complete_pa_stage_no_ipcl,
    help = "PA stage"
}

local function complete_pa_instances_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfInstances
    local help = {}
    local array = {}
    local tInstances = {} --not sorted with duplications instances list

    result, numOfInstances, array[1], array[2], array[3], array[4], array[5],
                            array[6], array[7], array[8], array[9], array[10],
                            array[11], array[12], array[13], array[14], array[15],
                            array[16], array[17], array[18], array[19], array[20],
                            array[21], array[22], array[23], array[24], array[25],
                            array[26], array[27], array[28], array[29], array[30],
                            array[31], array[32], array[33], array[34], array[35],
                            array[36], array[37], array[38], array[39], array[40],
                            array[41], array[42], array[43], array[44], array[45],
                            array[46], array[47], array[48], array[49], array[50]
    = wrlCpssDxChPacketAnalyzerInstanceAllInterfacesGet(paDb.managerId,paDb.interface)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerInstanceAllInterfacesGet returned "..err)
    end

    if numOfInstances ~= 0 then
        if numOfInstances > 50 then
          help.def = "Error: There are more then 50 instances for managerId "..to_string(paDb.managerId)
          return {}, help
        end
        -- clean tables
        for i = 1, #instances do
          table.remove(instances, i)
          table.remove(instancesHelp, i)
        end
        -- create tables
        for i = 1, numOfInstances do
            table.insert(tInstances ,array[i])
        end
    else
        help.def = "There are no instances for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    table.sort(tInstances)

    for key,value in ipairs(tInstances) do
        if value ~=tInstances[key+1] then
            table.insert(instances,value)
            table.insert(instancesHelp," ")
        end
    end

    return instances, instancesHelp
end

local function check_pa_instances_for_manager(param, data)

    --fill instances table if #instances is empty
    if #instances == 0 then
        complete_pa_instances_for_manager(param,data)
    end

    if #instances ~= 0 then
        for i = 1, #instances do
            if param == instances[i] then
                paDb.instance = param
                return true, param
            end
        end
    end

    return false, "Invalid Instance Check"
end

CLI_type_dict["pa_instances_for_manager"] = {
    checker = check_pa_instances_for_manager,
    complete = complete_pa_instances_for_manager,
    help = "Packet-Analyzer instances for interface"
}

local function complete_pa_indexes_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfIndexes
    local help = {}
    local array = {}
    local tIndexes = {} --not sorted with duplications indexes list

    result, numOfIndexes, array[1], array[2], array[3], array[4], array[5],
                          array[6], array[7], array[8], array[9], array[10]
    = wrlCpssDxChPacketAnalyzerIndexesForInterfaceAndInstanceGet(paDb.managerId,paDb.interface,paDb.instance)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerIndexesForInterfaceAndInstanceGet returned "..err)
    end

    if numOfIndexes ~= 0 then
      -- clean tables
      for i = 1, #indexes do
        table.remove(indexes, i)
        table.remove(indexesHelp, i)
      end
      -- create tables
      for i = 1, numOfIndexes do
          table.insert(tIndexes, array[i])
      end
    else
        help.def = "There are no legs for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    table.sort(tIndexes)

    for key,value in ipairs(tIndexes) do
        if value ~=tIndexes[key+1] then
            table.insert(indexes,value)
            table.insert(indexesHelp," ")
        end
    end

    return indexes, indexesHelp
end

local function check_pa_indexes_for_manager(param, data)
    local number = tonumber(param,10)

    --fill indexes table if #indexes is empty
    if #indexes == 0 then
        complete_pa_indexes_for_manager(param,data)
    end

    if #indexes ~= 0 then
        for i = 1, #indexes do
            if number == indexes[i] then
                return true, number
            end
        end
    end

    return false, "Invalid leg Check"

end

CLI_type_dict["pa_indexes_for_manager"] = {
    checker = check_pa_indexes_for_manager,
    complete = complete_pa_indexes_for_manager,
    help = "Packet-Analyzer leg for instance"
}

local function complete_pa_fields_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfFields
    local field
    local help = {}
    local fields = {}

    result, numOfFields = wrlCpssDxChPacketAnalyzerNumOfFieldsGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfFieldsGet returned "..err)
    end

    if numOfFields ~= 0 then
        for i = 1, numOfFields do
            result, field = wrlCpssDxChPacketAnalyzerFieldGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerFieldGet returned "..err)
            end
            table.insert(fields ,field)
            table.insert(help," ")
        end
    else
        help.def = "There are no fields for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    return fields, help
end

local function check_pa_fields_for_manager(param, data)
    local status = true
    local err
    local result
    local numOfFields
    local field

    result, numOfFields = wrlCpssDxChPacketAnalyzerNumOfFieldsGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfFieldsGet returned "..err)
    end

    if numOfFields ~= 0 then
        for i = 1, numOfFields do
            result, field = wrlCpssDxChPacketAnalyzerFieldGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerFieldGet returned "..err)
            end
            if param == field then
                return true, param
            end
        end
    end

    return false, "Invalid Field Check"
end

CLI_type_dict["pa_fields_for_manager"] = {
    checker = check_pa_fields_for_manager,
    complete = complete_pa_fields_for_manager,
    help = "Packet-Analyzer fields for manager"
}

local function complete_pa_udfs_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfFields
    local field
    local help = {}
    local fields = {}

    result, numOfFields = wrlCpssDxChPacketAnalyzerNumOfUdfsGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfUdfsGet returned "..err)
    end

    if numOfFields ~= 0 then
        for i = 1, numOfFields do
            result, field = wrlCpssDxChPacketAnalyzerUdfGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdfGet returned "..err)
            end
            table.insert(fields ,field)
            table.insert(help," ")
        end
    else
        help.def = "There are no udfs for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    return fields, help
end

local function check_pa_udfs_for_manager(param, data)
    local status = true
    local err
    local result
    local numOfFields
    local field

    result, numOfFields = wrlCpssDxChPacketAnalyzerNumOfUdfsGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfUdfsGet returned "..err)
    end

    if numOfFields ~= 0 then
        for i = 1, numOfFields do
            result, field = wrlCpssDxChPacketAnalyzerUdfGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdfGet returned "..err)
            end
            if param == field then
                return true, param
            end
        end
    end

    return false, "Invalid Udf Check"
end

CLI_type_dict["pa_udfs_for_manager"] = {
    checker = check_pa_udfs_for_manager,
    complete = complete_pa_udfs_for_manager,
    help = "Packet-Analyzer udfs for manager"
}

local function complete_pa_udss_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfStages
    local stage
    local help = {}
    local stages = {}

    result, numOfStages = wrlCpssDxChPacketAnalyzerNumOfUdssGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfUdssGet returned "..err)
    end

    if numOfStages ~= 0 then
        for i = 1, numOfStages do
            result, stage = wrlCpssDxChPacketAnalyzerUdsGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdsGet returned "..err)
            end
            table.insert(stages ,stage)
            table.insert(help," ")
        end
    else
        help.def = "There are no udss for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    return stages, help
end

local function check_pa_udss_for_manager(param, data)
    local status = true
    local err
    local result
    local numOfStages
    local stage

    result, numOfStages = wrlCpssDxChPacketAnalyzerNumOfUdssGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfUdssGet returned "..err)
    end

    if numOfStages ~= 0 then
        for i = 1, numOfStages do
            result, stage = wrlCpssDxChPacketAnalyzerUdsGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdsGet returned "..err)
            end
            if param == stage then
                return true, param
            end
        end
    end

    return false, "Invalid Uds Check"
end

CLI_type_dict["pa_udss_for_manager"] = {
    checker = check_pa_udss_for_manager,
    complete = complete_pa_udss_for_manager,
    help = "Packet-Analyzer udss for manager"
}

  local function complete_pa_interfaces_for_manager(param,data)
    local status = true
    local err
    local result
    local numOfInterfaces
    local interface
    local help = {}
    local tInterfaces = {} --not sorted with duplications interfaces list
    local interfaces = {}  --sorted without duplications interfaces list

    result, numOfInterfaces = wrlCpssDxChPacketAnalyzerNumOfInterfacesGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfInterfacesGet returned "..err)
    end

    if numOfInterfaces ~= 0 then
        for i = 1, numOfInterfaces do
            result, interface = wrlCpssDxChPacketAnalyzerInterfaceGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerInterfaceGet returned "..err)
            end
            table.insert(tInterfaces ,interface)
        end
    else
        help.def = "There are no interfaces for managerId "..to_string(paDb.managerId)
        return {}, help
    end

    table.sort(tInterfaces)

    for key,value in ipairs(tInterfaces) do
            if value ~=tInterfaces[key+1] then
                table.insert(interfaces,value)
                table.insert(help," ")
            end
        end

    return interfaces, help
end

local function check_pa_interfaces_for_manager(param, data)
    local status = true
    local err
    local result
    local numOfInterfaces
    local interface

    result, numOfInterfaces = wrlCpssDxChPacketAnalyzerNumOfInterfacesGet(paDb.managerId)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerNumOfInterfacesGet returned "..err)
    end

    if numOfInterfaces ~= 0 then
        for i = 1, numOfInterfaces do
            result, interface = wrlCpssDxChPacketAnalyzerInterfaceGet(paDb.managerId,i-1)
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerInterfaceGet returned "..err)
            end
            if param == interface then
                paDb.interface = param
                return true, param
            end
        end
    end

    return false, "Invalid Interface Check"
end

CLI_type_dict["pa_interfaces_for_manager"] = {
    checker = check_pa_interfaces_for_manager,
    complete = complete_pa_interfaces_for_manager,
    help = "Packet-Analyzer interfaces for manager"
}

local function checkSamplingState(actionId)
    local status = true
    local err

    -- no manager was initialized yet
    if paDb.paTable == NOT_INITIALIZED then
        err = returnCodes[GT_NOT_INITIALIZED]
        return status, err
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerActionGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "GT_U32", "actionId", actionId },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_ACTION_STC", "actionIdPtr", actionIdPtr }
        }
    )
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerActionGet returned "..err)
    end

    if values["actionIdPtr"].samplingEnable == GT_TRUE then
        result = GT_BAD_STATE
        err = returnCodes[result]
        print("Need to disable packet-analyzer first")
    end

    return status, err
end

-- ************************************************************************
--
--  initDbAndStringsSip6
--        @description  set lua DB and string tables with SIP6 stages order
--                      moved stage TRANSMIT_QUEUE from index 9 to 16
--                      added 2 stages in indexes 14 and 15
--        @return       true on success, otherwise false and error message
--
local function initDbAndStringsSip6()

    local status = true
    local ii
    local paStageStartIndex = 9   --index after CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E
    local paStageEndIndex = 13    --index of CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E
    local paLastStage =17         --new index of CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
    local udsLastStage =37        --new index of uds19
    local udsFirstStage =18       --new index of uds0
    local devFamily = wrlCpssDeviceFamilyGet(paDb.dev)

    --set the correct table name to use
    TABLE_TO_USE = DEFAULT_SIP6

    -- check if string tables needs to be change - only at first time
    if INIT_PA_STRINGS == NOT_INITIALIZED then

        --change the order and add 2 stages to all string tables
        for ii=paStageStartIndex,paStageEndIndex do
            printPaStageIndex[ii]=printPaStageIndex[ii+1]
            printPaStageIndexHelp[ii]=printPaStageIndexHelp[ii+1]
            printPaStageCpssIndex[ii]=printPaStageCpssIndex[ii+1]

        end

        printPaStageIndex[14]= "pha"
        printPaStageIndex[15]= "replication"
        printPaStageIndex[16]= "tx-queue"
        printPaStageIndex[17]= "mac"
        printPaStageIndexHelp[14]= "Pre programmable Header Alteration egress stage"
        printPaStageIndexHelp[15]= "Pre replication egress stage"
        printPaStageIndexHelp[16]= "Pre tx-Queue egress stage"
        printPaStageIndexHelp[17]= "Pre MAC egress stage"
        printPaStageCpssIndex[14]= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E"
        printPaStageCpssIndex[15]= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E"
        printPaStageCpssIndex[16]= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E"
        printPaStageCpssIndex[17]= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E"

        INIT_PA_STRINGS = DEFAULT_SIP6

        if (devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
            --remove MAC and PHA from tables
            table.remove(printPaStageIndex, 17)
            table.remove(printPaStageIndex, 14)
            table.remove(printPaStageIndexHelp, 17)
            table.remove(printPaStageIndexHelp, 14)
            table.remove(printPaStageCpssIndex, 17)
            table.remove(printPaStageCpssIndex, 14)

            --remove MAC and PHA FROM DEFAULT_SIP6 table
            table.remove(paTable[DEFAULT_SIP6][0], 17)
            table.remove(paTable[DEFAULT_SIP6][0], 14)

            INIT_PA_STRINGS = MODIFY_MODE
        end

        for key, val in pairs(paTable[DEFAULT_SIP6][0]) do
            printPaStageCpssInt[val["stage"]]=key
        end

    end

    return status
end

-- ************************************************************************
---
--  packetAnalyzerEnable
--        @description  enable packet analyzer on specific group
--
--        @param params         - group: specific group to enable
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerEnable(params)
    local status = true
    local actionId
    local groupId

    if params["group"] == "pipeline" then
        groupId = paTable[paDb.managerId]["pipeline"]
    else -- group stage
        groupId = paTable[paDb.managerId]["stage"]
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerGroupActivateEnableSet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "GT_U32", "groupId", groupId },
                        { "IN", "GT_BOOL", "enable", GT_TRUE }
        }
    )
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerGroupActivateEnableSet returned "..err)
        return status, err
    end

    --Clear sample data and counters on all rules
    for i,vals in pairs(paDb.paTable) do
      if vals["group"] == groupId then
        id = paDb.paTable[i]["id"]
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerSampledDataCountersClear", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "ruleId", id },
                            { "IN", "GT_U32", "groupId",groupId }
            }
        )
        if (result ~=0 and result ~=GT_NOT_FOUND) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerSampledDataCountersClear returned "..err)
        end
      end
    end

    for i,vals in pairs(paDb.paTable) do
        if vals["group"] == groupId then
            actionId = paDb.paTable[i]["id"]
            result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerActionSamplingEnableSet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "GT_U32", "actionId", actionId },
                                { "IN", "GT_BOOL", "enable", GT_TRUE }
                }
            )
            if (result ~=0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerActionSamplingEnableSet returned "..err)
                return status, err
            end
        end
    end

    paDb.activateGroup[paDb.managerId] = groupId

    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerDisable
--        @description  disable packet analyzer on activated group
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerDisable(params)
    local status = true
    local actionId
    local groupId

    -- no group was activated yet
    if paDb.activateGroup[paDb.managerId] == NOT_INITIALIZED then
        return status, err
    end

    groupId = paDb.activateGroup[paDb.managerId]

    for i,vals in pairs(paDb.paTable) do
        if vals["group"] == groupId then
            actionId = paDb.paTable[i]["id"]
            result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerActionSamplingEnableSet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "GT_U32", "actionId", actionId },
                                { "IN", "GT_BOOL", "enable", GT_FALSE }
                }
            )
            if (result ~=0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerActionSamplingEnableSet returned "..err)
                return status, err
            end
        end
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerGroupActivateEnableSet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "GT_U32", "groupId", groupId },
                        { "IN", "GT_BOOL", "enable", GT_FALSE }
        }
    )
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerGroupActivateEnableSet returned "..err)
    end

    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerClear
--        @description  clear counters and sampled data on activated group
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerClear(params)
    local enable = GT_TRUE
    local status = true
    local actionId = 1
    local groupId

    -- no group was activated yet
    if paDb.activateGroup[paDb.managerId] == NOT_INITIALIZED then
        return status, err
    end

    groupId = paDb.activateGroup[paDb.managerId]

    for i,vals in pairs(paDb.paTable) do
        if vals["group"] == groupId then
            actionId = paDb.paTable[i]["id"]
            result, values = checkSamplingState(actionId)
            if (result ~=true) then
                status = false
                err = values
                print("checkSamplingState returned "..err)
            end
            if result == true and values == returnCodes[GT_BAD_STATE] then
                return result,values
            end
        end
    end

    --Clear sample data and counters on all rules
    for i,vals in pairs(paDb.paTable) do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerSampledDataCountersClear", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "ruleId", vals["id"] },
                            { "IN", "GT_U32", "groupId",vals["group"] }
            }
        )
        if (result ~=0 and result ~=GT_NOT_FOUND) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerSampledDataCountersClear returned "..err)
        end
    end

    return status, err

end

local function deleteManager()
    local status = true
    local err
    local values
    local actionIds = {}

    local groupIds = {paDb.paGrpTable["pipeline"],paDb.paGrpTable["stage"]}

    for i=1,#groupIds do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupActivateEnableSet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "groupId", groupIds[i] },
                            { "IN", "GT_BOOL", "enable", GT_FALSE }
            }
        )
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupActivateEnableSet returned "..err)
            return status, err
        end
    end

    --Clear sample data and counters on all rules
    for i,vals in pairs(paDb.paTable) do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerSampledDataCountersClear", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "ruleId", vals["id"] },
                            { "IN", "GT_U32", "groupId",vals["group"] }
            }
        )
        if (result ~=0 and result ~=GT_NOT_FOUND) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerSampledDataCountersClear returned "..err)
            return status, err
        end
    end

    for i=1,#groupIds do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupDelete", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "groupId", groupIds[i] }
            }
        )
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupDelete returned "..err)
            return status, err
        end
    end

    j=1
    for i,vals in pairs(paDb.paTable) do
        actionIds[j] = vals["id"]
        j = j+1
    end

    for i=1,#actionIds do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerActionDelete", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "actionId", actionIds[i] }
            }
        )
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerActionDelete returned "..err)
            return status, err
        end
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerManagerDelete", {
                        { "IN", "GT_U32", "managerId", paDb.managerId}
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerManagerDelete returned "..err)
        return status, err
    end

    return status, err
end

-- ************************************************************************
---
--  noPacketAnalyzer
--        @description  deallocates packet analyzer or removes device from packet analyzer
--
--        @param params         - params["managerId"]: Manager ID (optinal)
--        @param params         - params["device"]: devNum

--        @return       true on success, otherwise false and error message
--
local function noPacketAnalyzer(params)
    local status = true
    local err
    local values
    local actionIds ={}

    if params["managerId"] ~= nil then
        paDb.managerId = params["managerId"]
    else
        --Set default manager ID value if parameter isn't provided
        paDb.managerId = 1
    end

    if params["device"] ~= nil then
        paDb.dev = params["device"]
    else
        --Set default device ID value if parameter isn't provided
        paDb.dev = 0
    end

    -- no manager was initialized yet
    if paDb.paTable == NOT_INITIALIZED then
        err = returnCodes[GT_NOT_INITIALIZED]
        return status, err
    end

    j=1
    for i,vals in pairs(paDb.paTable) do
        actionIds[j] = vals["id"]
        j = j+1
    end

    for i=1,#actionIds do
        result, values = checkSamplingState(actionIds[i])
        if (result ~=true) then
            status = false
            err = values
            print("checkSamplingState returned "..err)
        end
        if result == true and values == returnCodes[GT_BAD_STATE] then
            return result,values
        end
    end

    local groupIds = {paDb.paGrpTable["pipeline"],paDb.paGrpTable["stage"]}

    for i=1,#groupIds do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupActivateEnableSet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "groupId", groupIds[i] },
                            { "IN", "GT_BOOL", "enable", GT_FALSE }
            }
        )
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupActivateEnableSet returned "..err)
            return status, err
        end
    end

    result, numOfDevices = wrlCpssDxChPacketAnalyzerManagerNumDevicesGet(paDb.managerId)
    if (result ~= 0) then
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerManagerNumDevicesGet returned "..err)
        return false, err
    end

    if numOfDevices == 1 then
        result, values = deleteManager()
        if (result ~=true) then
            status = false
            err = values
            print("deleteManager returned "..err)
        end

        paDb.paTable = NOT_INITIALIZED
    else
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerManagerDeviceRemove", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U8",  "devNum", paDb.dev}
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerManagerDeviceRemove returned "..err)
            return status, err
        end
    end
    --clear paDb.configuration DB
    paDb.configuration[paDb.managerId]["stage"] = nil
    paDb.configuration[paDb.managerId]["pipeline"] = nil
    paTable[paDb.managerId] = nil

    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerSetField
--        @description  packet-analyzer.stage# set field in rule for stage
--
--        @param params         - params["field"]: field name
--                              - params["value"]: field value
--                              - params["mask"]: field mask (optional)
--                              - params["no"]: remove/add field to/from key & group
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerSetField(params)
    local status = true
    local err
    local groupId
    local id
    local field
    local index
    local ruleAttr = {}
    local keyAttr = {}

    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == paDb.stage then
            groupId = paDb.paTable[i]["group"]
            id = paDb.paTable[i]["id"]
            index = i
        end
    end

    result, values = checkSamplingState(id)
    if (result ~=true) then
        status = false
        err = values
        print("checkSamplingState returned "..err)
    end
    if result == true and values == returnCodes[GT_BAD_STATE] then
        return result,values
    end

    -- remove all stage's configuration
    if (params["no"] ~= nil and params["field"] == nil) then

        -- Delete rule from group
        result = wrlCpssDxChPacketAnalyzerGroupRuleDelete(paDb.managerId,id,groupId)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerGroupRuleDelete returned "..to_string(err))
            return status, err
        end

        -- Delete key
        result = wrlCpssDxChPacketAnalyzerLogicalKeyDelete(paDb.managerId,id)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerLogicalKeyDelete returned "..to_string(err))
            return status, err
        end

        -- Create empty key
        keyAttr.name = paDb.paTable[index]["att"]
        result = wrlCpssDxChPacketAnalyzerLogicalKeyCreate(paDb.managerId, id, keyAttr, paDb.stage)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerLogicalKeyCreate returned "..to_string(err))
            return status, err
        end

        local fieldsValues = {}
        fieldsValues[0] = {}

        -- Add empty rule to group
        ruleAttr.name = paDb.paTable[index]["att"]
        result = wrlCpssDxChPacketAnalyzerGroupRuleAdd(paDb.managerId, id, groupId, id, ruleAttr, 0, fieldsValues, id)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerGroupRuleAdd returned "..to_string(err))
        end

        return status, err
    end

    local currentField
    -- check if the field is udf; then convert from name to idebug field
    udfId = printPaFieldCpssEnum[params["field"]]
    if udfId ~= nil then -- this is udf field
      field = udfId
      currentField = printPaFieldUdf[printPaFieldCpssInt[udfId]]
    else -- this is regular PA field
      for i=1, #printPaField+1 do
        if printPaField[i-1] == params["field"] then
            field = printPaFieldCpss[i-1]
            break
        end
      end
      currentField = params["field"]
    end

    if (params["no"] == nil) then
        -- Update Key
        result = wrlCpssDxChPacketAnalyzerLogicalKeyFieldsAdd(paDb.managerId,id,field)
        if (result ~=0 and result ~= GT_ALREADY_EXIST) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerLogicalKeyFieldsAdd returned "..to_string(err))
            return status, err
        end

        -- Update Rule
        fieldsValues = {}
        fieldsValues[0] = {}
        fieldsValues[0].data = {}
        fieldsValues[0].msk = {}
        fieldsValues[0].name = field

        if currentField == "mac_da" or currentField == "mac_sa" then

            value0 = tonumber(params["value"][3], 16)
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][4], 16))
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][5], 16))
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][6], 16))

            value1 = tonumber(params["value"][1], 16)
            value1 = bit_or(bit_shl(value1,8),tonumber(params["value"][2], 16))

            fieldsValues[0].data[0] = value0
            fieldsValues[0].data[1] = value1

            if (params["mask"] ~= nil) then

                msk0 = tonumber(params["mask"][3], 16)
                msk0 = bit_or(bit_shl(msk0,8),tonumber(params["mask"][4], 16))
                msk0 = bit_or(bit_shl(msk0,8),tonumber(params["mask"][5], 16))
                msk0 = bit_or(bit_shl(msk0,8),tonumber(params["mask"][6], 16))

                msk1 = tonumber(params["mask"][1], 16)
                msk1 = bit_or(bit_shl(msk1,8),tonumber(params["mask"][2], 16))

                fieldsValues[0].msk[0] = msk0
                fieldsValues[0].msk[1] = msk1
            else
                fieldsValues[0].msk[0] = 0xFFFFFFFF
                fieldsValues[0].msk[1] = 0xFFFF
            end

        elseif currentField == "ipv4_dip" or currentField == "ipv4_sip" then

            local msk = 0xffffffff

            value0 = tonumber(params["value"][1], 10)
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][2], 10))
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][3], 10))
            value0 = bit_or(bit_shl(value0,8),tonumber(params["value"][4], 10))

            fieldsValues[0].data[0] = value0

            if (params["mask"] ~= nil) then
                msk = bit_shl(msk,32-params["mask"])

                msk3 = bit_and(bit_shr(msk,24),0xff)
                msk2 = bit_and(bit_shr(msk,16),0xff)
                msk1 = bit_and(bit_shr(msk,8),0xff)
                msk0 = bit_and(msk,0xff)

                msk0 = bit_or(bit_shl(msk0,8),msk1)
                msk0 = bit_or(bit_shl(msk0,8),msk2)
                msk0 = bit_or(bit_shl(msk0,8),msk3)

                fieldsValues[0].msk[0] = msk0
            else
                fieldsValues[0].msk[0] = 0xFFFFFFFF
            end

        elseif currentField == "ipv6_dip" or currentField == "ipv6_sip" then

            local msk = {}

            value0 = tonumber(params["value"].addr[7], 10)
            value0 = bit_or(bit_shl(value0,16),tonumber(params["value"].addr[8], 10))

            value1 = tonumber(params["value"].addr[5], 10)
            value1 = bit_or(bit_shl(value1,16),tonumber(params["value"].addr[6], 10))

            value2 = tonumber(params["value"].addr[3], 10)
            value2 = bit_or(bit_shl(value2,16),tonumber(params["value"].addr[4], 10))

            value3 = tonumber(params["value"].addr[1], 10)
            value3 = bit_or(bit_shl(value3,16),tonumber(params["value"].addr[2], 10))

            fieldsValues[0].data[0] = value0
            fieldsValues[0].data[1] = value1
            fieldsValues[0].data[2] = value2
            fieldsValues[0].data[3] = value3

            if (params["mask"] ~= nil) then

                numPrefixWords = math.floor(params["mask"]/32)
                if (params["mask"] % 32) > 0 then
                    numPrefixWords = numPrefixWords + 1
                end

                -- Write mask value for 4 register
                for wordItr=0,3 do

                    --last word
                    if (numPrefixWords == (wordItr + 1)) then

                        wordPrefix = params["mask"] % 32
                        if (wordPrefix ~= 0) then
                            value = bit_shl(bit_shl(1,wordPrefix) - 1,32 - wordPrefix)
                        else
                            value = 0xFFFFFFFF
                        end
                    else
                        if (numPrefixWords > wordItr) then
                            value = 0xFFFFFFFF
                        else
                            value = 0
                        end

                    end
                    msk[(wordItr*2)+1] = value
                    msk[wordItr*2] = value
                end

                fieldsValues[0].msk[0] = msk[0]
                fieldsValues[0].msk[1] = msk[1]
                fieldsValues[0].msk[2] = msk[2]
                fieldsValues[0].msk[3] = msk[3]
            else

                fieldsValues[0].msk[0] = 0xFFFFFFFF
                fieldsValues[0].msk[1] = 0xFFFFFFFF
                fieldsValues[0].msk[2] = 0xFFFFFFFF
                fieldsValues[0].msk[3] = 0xFFFFFFFF

            end

        else
            fieldsValues[0].data[0] = nil

            -- handle enumerations
            result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,currentField)
            if (result ~= 0) then
              status = false
              err = returnCodes[result]
              print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
            end

            if numOfEnums ~= 0 then

              local values = {}
              for i = 1, numOfEnums do
                  result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,currentField,i-1)
                  if (result ~= 0) then
                      status = false
                      err = returnCodes[result]
                      print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
                  end

                  name = string_manipulations(name)

                  if (name == params["value"]) then
                    fieldsValues[0].data[0] = tonumber(value)
                    break
                  end
              end

            end

            if fieldsValues[0].data[0] == nil then
              fieldsValues[0].data[0] = tonumber(params["value"])
            end
            if (params["mask"] ~= nil) then
                fieldsValues[0].msk[0] = tonumber(params["mask"])
            else
                local lengthPtr
                local msk = 0

                result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerFieldSizeGet", {
                                    { "IN", "GT_U32", "managerId", paDb.managerId},
                                    { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "fieldName", field },
                                    { "OUT", "GT_U32", "lengthPtr", lengthPtr }
                    }
                )
                if (result ~= 0) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerFieldSizeGet returned "..err)
                end
                for i = 0 ,tonumber(values["lengthPtr"]-1,10) do
                    msk = msk + 2^i
                end
                fieldsValues[0].msk[0] = msk
            end

        end

        result = wrlCpssDxChPacketAnalyzerGroupRuleFieldsAdd(paDb.managerId, id, groupId , 1, fieldsValues, id)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerGroupRuleFieldsAdd returned "..to_string(err))
            return status, err
        end

    else

        -- Remove field from rule & group
        result = wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsRemove(paDb.managerId,id,field,id,groupId)
        if (result ~=0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsRemove returned "..to_string(err))
        end

    end
    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerStage
--        @description  change CLI mode for specifed stage
--
--        @param params         - params["stage"]: stage ID
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerStage(params)
    local err
    local stage
    local result, values
    local applicStage
    local numOfMuxStages, muxStage
    local line
    local id
    local groupId
    local index
    local ruleAttr = {}
    local keyAttr = {}

    if (params["stage"] == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.configuration[paDb.managerId]["pipeline"] == "packet-trace" and params["packet_trace"] == nil)
        or
        (params["stage"] == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" and paDb.configuration[paDb.managerId]["pipeline"] == "regular" and params["packet_trace"] ~= nil)
    then
        result = GT_BAD_STATE
        err = returnCodes[result]

        print("Packet-trace can't co-exist with regular pipeline settings. Please revert recent changes using 'no configure pipeline'")
        return result,err
    end

    if (params["stage"] == "i-pcl" and paDb.configuration[paDb.managerId]["stage"] == "packet-trace" and params["packet_trace"] == nil)
        or
        (params["stage"] == "i-pcl" and paDb.configuration[paDb.managerId]["stage"] == "regular" and params["packet_trace"] ~= nil)
    then
        result = GT_BAD_STATE
        err = returnCodes[result]

        print("Packet-trace can't co-exist with regular stage i-pcl settings. Please revert recent changes using 'no configure stage i-pcl'")
        return result,err
    end

    if params["stage"] == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" then

        stage = printPaStage[params["stage"]]
        paDb.stage = params["stage"]


        if params["packet_trace"] == nil then
            paDb.configuration[paDb.managerId]["pipeline"] = "regular"

            CLI_prompts["packet-analyzer.stage"] = "(debug."..paDb.dev.."-pa-"..paDb.managerId.."-"..stage..")#"
            CLI_change_mode_push("packet-analyzer.stage")

        else

            params["field"] = "packet_trace"
            params["value"] = "1"
            params["mask"] = "0x1"
            result, values = packetAnalyzerSetField(params)
            if (result ~=true) then
                status = false
                err = values
                print("packetAnalyzerSetField returned "..err)
            end
            paDb.configuration[paDb.managerId]["pipeline"] = "packet-trace"

            CLI_prompts["packet-analyzer.stage"] = "(debug."..paDb.dev.."-pa-"..paDb.managerId.."-"..stage.."-packet-trace"..")#"
            CLI_change_mode_push("packet-analyzer.stage")
        end
        return true,err
    end

    params["stage"] = printPaStageCpssEnum[params["stage"]]
    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == params["stage"] then
            groupId = paDb.paTable[i]["group"]
            id = paDb.paTable[i]["id"]
            index = i
        end
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end

    if values["numOfApplicStagesPtr"] ~= 0 then
        for i = 1, values["numOfApplicStagesPtr"] do
            applicStaget = values["applicStagesListArr"][i-1]
            applicStage = printPaStageCpssInt[applicStaget]
            if printPaStageIndex[applicStage] == printPaStage[params["stage"]] then
                result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerMuxStageBind", {
                                    { "IN", "GT_U8", "devNum", paDb.managerId },
                                    { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", params["stage"] }
                    }
                )
                if (result ~= 0 and result ~= GT_BAD_STATE) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerMuxStageBind returned "..err)
                    return status,err
                end

                if (result == 0) then

                  stage = printPaStage[params["stage"]]
                  paDb.stage = params["stage"]

                  -- Create empty key
                  keyAttr.name = paDb.paTable[index]["att"]
                  result = wrlCpssDxChPacketAnalyzerLogicalKeyCreate(paDb.managerId, id, keyAttr, params["stage"])
                  if (result ~=0 and result ~=GT_ALREADY_EXIST) then
                      status = false
                      err = returnCodes[result]
                      print("wrlCpssDxChPacketAnalyzerLogicalKeyCreate returned "..to_string(err))
                      return status, err
                  end

                  local fieldsValues = {}
                  fieldsValues[0] = {}

                  -- Add empty rule to group
                  ruleAttr.name = paDb.paTable[index]["att"]
                  result = wrlCpssDxChPacketAnalyzerGroupRuleAdd(paDb.managerId, id, groupId, id, ruleAttr, 0, fieldsValues, id)
                  if (result ~=0 and result ~=GT_ALREADY_EXIST) then
                      status = false
                      err = returnCodes[result]
                      print("wrlCpssDxChPacketAnalyzerGroupRuleAdd returned "..to_string(err))
                  end

                  if params["packet_trace"] == nil then
                    if params["stage"] == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E" then
                      paDb.configuration[paDb.managerId]["stage"] = "regular"
                    end

                    CLI_prompts["packet-analyzer.stage"] = "(debug."..paDb.dev.."-pa-"..paDb.managerId.."-"..stage..")#"
                    CLI_change_mode_push("packet-analyzer.stage")

                  else
                    paDb.configuration[paDb.managerId]["stage"] = "packet-trace"

                    CLI_prompts["packet-analyzer.stage"] = "(debug."..paDb.dev.."-pa-"..paDb.managerId.."-"..stage.."-packet-trace"..")#"
                    CLI_change_mode_push("packet-analyzer.stage")
                  end
                  return true,err

                end
                break -- result == GT_BAD_STATE
            end
        end
    end

    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerMuxStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", params["stage"] },
                        { "INOUT", "GT_U32", "numOfMuxStagesPtr", 20 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[20]", "muxStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerMuxStagesGet returned "..err)
    end

    if params["stage"] == nil then
        status = false
        err = returnCodes[result]
        print("Wrong or incomplete option: stage")
        return false, err
    end

    line = "Stage "..printPaStage[params["stage"]].." is in-conflict with one or more stages ("
    for i = 1, values["numOfMuxStagesPtr"] do
      if i > 1 then
        line = line..", "
      end
      muxStaget = values["muxStagesListArr"][i-1]
      muxStage = printPaStageCpssInt[muxStaget]
      line = line..printPaStageIndex[muxStage]
    end
    line = line..")"
    print(line)

    return false, err

end

-- ************************************************************************
---
--  noPacketAnalyzerStage
--        @description  unset configuration for specifed stage
--
--        @param params         - params["stage"]: stage ID
--
--        @return       true on success, otherwise false and error message
--
local function noPacketAnalyzerStage(params)
    local status = true
    local err
    local id
    local groupId
    local index
    local ruleAttr = {}
    local keyAttr = {}

    if params["stage"] ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" then
        params["stage"] = printPaStageCpssEnum[params["stage"]]
        paDb.configuration[paDb.managerId]["stage"] = nil
    else
        paDb.configuration[paDb.managerId]["pipeline"] = nil
    end

    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == params["stage"] then
            groupId = paDb.paTable[i]["group"]
            id = paDb.paTable[i]["id"]
            index = i
        end
    end

    -- Delete rule from group
    result = wrlCpssDxChPacketAnalyzerGroupRuleDelete(paDb.managerId,id,groupId)
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerGroupRuleDelete returned "..to_string(err))
        return status, err
    end

    -- Delete key
    result = wrlCpssDxChPacketAnalyzerLogicalKeyDelete(paDb.managerId,id)
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerLogicalKeyDelete returned "..to_string(err))
        return status, err
    end

    if params["stage"] ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" then
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerMuxStageUnbind", {
                            { "IN", "GT_U8", "devNum", paDb.managerId },
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", params["stage"] }
            }
        )
        if (result ~= 0 and result ~= GT_BAD_STATE) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerMuxStageUnbind returned "..err)
            return status,err
        end
    else
      -- Create empty key
      keyAttr.name = paDb.paTable[index]["att"]
      result = wrlCpssDxChPacketAnalyzerLogicalKeyCreate(paDb.managerId, id, keyAttr, params["stage"])
      if (result ~=0 and result ~=GT_ALREADY_EXIST) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerLogicalKeyCreate returned "..to_string(err))
          return status, err
      end

      local fieldsValues = {}
      fieldsValues[0] = {}

      -- Add empty rule to group
      ruleAttr.name = paDb.paTable[index]["att"]
      result = wrlCpssDxChPacketAnalyzerGroupRuleAdd(paDb.managerId, id, groupId, id, ruleAttr, 0, fieldsValues, id)
      if (result ~=0 and result ~=GT_ALREADY_EXIST) then
          status = false
          err = returnCodes[result]
          print("wrlCpssDxChPacketAnalyzerGroupRuleAdd returned "..to_string(err))
      end

    end

    return status, err
end

local function addCommands()

    -- add commands according to family
    local devFamily = wrlCpssDeviceFamilyGet(paDb.dev)
    if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then

        --------------------------------------------
        -- command registration: pipeline
        --------------------------------------------
        CLI_addCommand("packet-analyzer", "configure pipeline", {
            func=function(params)
                params["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
                return packetAnalyzerStage(params)
            end,
            help="Select pipeline",
            params={
                {   type="named",
                        { format="packet-trace", optional = true, name="packet_trace", help="Enable Packet Trace"},
                }
            }
        })

        --------------------------------------------
        -- command registration: stage
        --------------------------------------------
        CLI_addCommand("packet-analyzer", "configure stage", {
            func=packetAnalyzerStage,
            help="Select stage",
            params={
                {
                    type="values",
                        { format="%pa_stage_no_ipcl", optional = false, name="stage", help=CLI_type_dict["pa_stage"].help}
                }
            }
        })

        CLI_addCommand("packet-analyzer", "configure stage i-pcl", {
            func=function(params)
                params["stage"] = "i-pcl"
                return packetAnalyzerStage(params)
            end,
            help="Pre PCL0,1,2 ingress stages",
            params={
                {   type="named",
                        { format="packet-trace", optional = true, name="packet_trace", help="Enable Packet Trace"},
                }
            }
        })

    end

    if (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E") then

        --------------------------------------------
        -- command registration: pipeline
        --------------------------------------------
        CLI_addCommand("packet-analyzer", "configure pipeline", {
            func=function(params)
                params["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
                return packetAnalyzerStage(params)
            end,
            help="Select pipeline"
        })

        --------------------------------------------
        -- command registration: stage
        --------------------------------------------
        CLI_addCommand("packet-analyzer", "configure stage", {
            func=packetAnalyzerStage,
            help="Select stage",
            params={
                {
                    type="values",
                        { format="%pa_stage", optional = false, name="stage", help=CLI_type_dict["pa_stage"].help}
                },
            }
        })

    end
end

-- ************************************************************************
---
--  packetAnalyzer
--        @description  allocates packet analyzer. Enters to packet-analyzer
--                      CLI mode
--
--        @param params         - params["managerId"]: Manager ID (optional)
--                              - params["device"]: Device ID (optional)
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzer(params)
    local status = true
    local err
    local groupAttrPtr = {}
    local actionPtr = {}
    local ruleAttr = {}
    local keyAttr = {}
    local devFamily

    if params["managerId"] ~= nil then
        paDb.managerId = params["managerId"]
    else
        --Set default manager ID value if parameter isn't provided
        paDb.managerId = 1
    end

    if params["device"] ~= nil then
        paDb.dev = params["device"]
    else
        --Set default device ID value if parameter isn't provided
        paDb.dev = devEnv.dev
    end

    result = myGenWrapper("prvCpssDxChPacketAnalyzerIsXmlPresent", {
                            { "IN", "GT_U8", "devNum", paDb.dev}})
    if (result == 0) then -- no xml present ,can't run packet analyzer
        status = false
        err = returnCodes[0x0B]
        print("prvCpssDxChPacketAnalyzerIsXmlPresent returned "..err)
        return status,err
    end

    devFamily = wrlCpssDeviceFamilyGet(paDb.dev)

    if paDb.paTable == NOT_INITIALIZED then
        paDb.paTable = {}
    end

    if paTable[paDb.managerId] == nil then
        paTable[paDb.managerId] = {}
        paTable[paDb.managerId][paDb.dev] = {}
        --change Lua DB and strings order
        if (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" or devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" or
            devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" or devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
            initDbAndStringsSip6()
        end

        -- create default table
        for i in pairs(paTable[TABLE_TO_USE][0]) do
            paTable[paDb.managerId][paDb.dev][i] = {}
            paTable[paDb.managerId][paDb.dev][i]["stage"] = paTable[TABLE_TO_USE][0][i]["stage"]
            paTable[paDb.managerId][paDb.dev][i]["id"] = paTable[TABLE_TO_USE][0][i]["id"]
            paTable[paDb.managerId][paDb.dev][i]["att"] = paTable[TABLE_TO_USE][0][i]["att"]
            paTable[paDb.managerId][paDb.dev][i]["group"] = paTable[TABLE_TO_USE][0][i]["group"]
            paTable[paDb.managerId][paDb.dev][i]["inverseEnable"] = false
            paTable[paDb.managerId][paDb.dev][i]["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E
        end

        paTable[paDb.managerId]["pipeline"] = paTable[DEFAULT]["pipeline"]
        paTable[paDb.managerId]["stage"] = paTable[DEFAULT]["stage"]
        paDb.activateGroup[paDb.managerId] = NOT_INITIALIZED
        paDb.configuration[paDb.managerId] = {}
        paDb.configuration[paDb.managerId]["pipeline"] = nil
        paDb.configuration[paDb.managerId]["stage"] = nil
    end

    paDb.paTable = paTable[paDb.managerId][paDb.dev]
    paDb.paGrpTable = paTable[paDb.managerId]
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerManagerDevicesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "OUT", "GT_BOOL", "deviceStatePtr", deviceStatePtr },
                        { "INOUT", "GT_U32", "numOfDevicesPtr", 30 },
                        { "OUT", "GT_U8[30]", "devicesArr"},
        }
    )
    if (result == GT_NOT_FOUND) then
        --Create manager
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerManagerCreate", {
                            { "IN", "GT_U32", "managerId", paDb.managerId}
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerManagerCreate returned "..err)
            return status,err
        end

    else if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerManagerDevicesGet returned "..err)
        else

            -- update action's values in DB
            for i,vals in pairs(paDb.paTable) do
                actionId = vals["id"]
                result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerActionGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "GT_U32", "actionId", actionId },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_ACTION_STC", "actionIdPtr", actionIdPtr }
                }
                )
                if (result ~=0) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerActionGet returned "..err)
                end

                if (values["actionIdPtr"]["samplingMode"] == "CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E") then
                  paDb.paTable[i]["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E
                else
                  paDb.paTable[i]["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E
                end
                paDb.paTable[i]["inverseEnable"] = values["actionIdPtr"]["inverseEnable"]
            end

            -- update active group
            local groupIds = {paDb.paGrpTable["pipeline"],paDb.paGrpTable["stage"]}

            for i=1, #groupIds do

                result,activateEnable = wrlCpssDxChPacketAnalyzerGroupActivateEnableGet(paDb.managerId, i)
                if (result == GT_BAD_PARAM) then
                    status = false
                    err = returnCodes[result]
                    print("wrlCpssDxChPacketAnalyzerGroupActivateEnableGet returned "..to_string(err))
                    return status,err
                end
                if (activateEnable == 1) then
                    paDb.activateGroup[paDb.managerId] = i
                end

            end

            -- update packet-trace enable/disable
            result,mask = wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck(paDb.managerId, id_ipcl, CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E, id_ipcl, paDb.paGrpTable["stage"])
            if (result ~= 0 and result ~= GT_NOT_FOUND) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck returned "..to_string(err))
                return status,err
            end

            if (result == 0 and mask == 1) then
                paDb.configuration[paDb.managerId]["stage"] = "packet-trace"
            else if (result == 0 and mask == 0) then
                paDb.configuration[paDb.managerId]["stage"] = "regular"
                end
            end

            result,mask = wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck(paDb.managerId, id_all, CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E, id_all, paDb.paGrpTable["pipeline"])
            if (result ~= 0 and result ~= GT_NOT_FOUND) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerGroupLogicalKeyFieldsExistCheck returned "..to_string(err))
                return status,err
            end
            if (result == 0 and mask == 1) then
                paDb.configuration[paDb.managerId]["pipeline"] = "packet-trace"
            else if (result == 0 and mask == 0) then
                paDb.configuration[paDb.managerId]["pipeline"] = "regular"
                end
            end


            -- update uds DB
            result,firstUds,lastUds = wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet()
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet returned "..to_string(err))
                return status,err
            end

            for udsId = firstUds, lastUds do
                result, values = myGenWrapper(
                      "cpssDxChPacketAnalyzerUserDefinedStageInfoGet", {
                                      { "IN", "GT_U32", "managerId", paDb.managerId},
                                      { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "udsId", udsId},
                                      { "OUT", "GT_BOOL", "validPtr", validPtr},
                                      { "OUT", "CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC", "udsAttrPtr", udsAttrPtr},
                                      { "OUT", "CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC", "interfaceAttrPtr", interfaceAttrPtr}
                      }
                  )

                  if (result ~= 0) then
                      status = false
                      err = returnCodes[result]
                      print("cpssDxChPacketAnalyzerUserDefinedStageInfoGet returned "..err)
                  end

                  if (values["validPtr"] == GT_TRUE) then
                        printPaStage[printPaStageCpssIndex[udsId]] = values["udsAttrPtr"].udsNameArr[0]
                        printPaStageIndex[udsId] = values["udsAttrPtr"].udsNameArr[0]
                        printPaStageCpssEnum[values["udsAttrPtr"].udsNameArr[0]] = printPaStageCpssIndex[udsId]
                  end

            end

            -- update udf DB
            result,firstUdf,lastUdf = wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet()
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet returned "..to_string(err))
                return status,err
            end

            for udfId = firstUdf, lastUdf do

                result, values = myGenWrapper(
                  "cpssDxChPacketAnalyzerUserDefinedFieldInfoGet", {
                                  { "IN", "GT_U32", "managerId", paDb.managerId},
                                  { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "udfId", udfId},
                                  { "OUT", "GT_BOOL", "validPtr", validPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC", "udfAttrPtr", udfAttrPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC", "fieldNamePtr", fieldNamePtr}
                  }
              )
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("cpssDxChPacketAnalyzerUserDefinedFieldInfoGet returned "..err)
              end

              if (values["validPtr"] == GT_TRUE) then
                  printPaField[udfId] = values["udfAttrPtr"].udfNameArr[0]
                  printPaFieldCpssEnum[values["udfAttrPtr"].udfNameArr[0]] = printPaFieldCpss[udfId]
                  printPaFieldUdf[udfId] = values["fieldNamePtr"].fieldNameArr[0]
              end
            end
        end
    end

    --Add device
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerManagerDeviceAdd", {
                        { "IN", "GT_U32", "managerId", paDb.managerId },
                        { "IN", "GT_U8", "devNum", paDb.dev }
        }
    )
    if (result ~= 0 and result ~= GT_ALREADY_EXIST) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerManagerDeviceAdd returned "..err)
        return status,err
    end

    if result == GT_ALREADY_EXIST then
        addCommands()
        CLI_prompts["packet-analyzer"] = "(debug.pa-"..paDb.managerId..")#"
        CLI_change_mode_push("packet-analyzer")
        return true,err
    end

    -- Enable/Disable Analyzer Manager
     result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerManagerEnableSet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "GT_BOOL", "enable", GT_TRUE }
        }
    )
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerManagerEnableSet returned "..err)
        return status,err
    end

    local groupIds = {paDb.paGrpTable["pipeline"],paDb.paGrpTable["stage"]}

    -- create groups
    for i=1, #groupIds do
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupCreate", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "groupId", groupIds[i]},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC", "groupAttrPtr", groupAttrPtr }
            }
        )
        if (result ~=0 and result ~= GT_ALREADY_EXIST) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupCreate returned "..err)
            return status,err
        end
    end

    --Create all actions
    for i,vals in pairs(paDb.paTable) do
        actionPtr = {
            samplingMode = vals["samplingMode"]
        }
        actionId = vals["id"]
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerActionCreate", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "actionId", actionId},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_ACTION_STC", "actionPtr", actionPtr }
            }
        )
        if (result ~=0 and result ~= GT_ALREADY_EXIST) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerActionCreate returned "..err)
            return status,err
        end
    end

    -- Create all keys
    for i,vals in pairs(paDb.paTable) do
        if (vals["id"] < id_uds_first or ((id_uds_last < vals["id"]) and (vals["id"] <= id_last)))  then
            keyAttr.name = vals["att"]
            result = wrlCpssDxChPacketAnalyzerLogicalKeyCreate(paDb.managerId, vals["id"], keyAttr, vals["stage"])
            if (result ~=0 and result ~= GT_ALREADY_EXIST and result ~= GT_BAD_PARAM) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerLogicalKeyCreate returned "..to_string(err))
                return status,err
            end
        end
    end

    fieldsValues = {}
    fieldsValues[0] = {}

    -- Add Rules to groups
    for i,vals in pairs(paDb.paTable) do
        if (vals["id"] < id_uds_first or ((id_uds_last < vals["id"]) and (vals["id"] <= id_last)))  then
            ruleAttr.name = vals["att"]
            result = wrlCpssDxChPacketAnalyzerGroupRuleAdd(paDb.managerId, vals["id"], vals["group"], vals["id"],
                                                           ruleAttr, 0, fieldsValues, vals["id"])
            if (result ~=0 and result ~= GT_ALREADY_EXIST and result ~= GT_NOT_FOUND) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerGroupRuleAdd returned "..to_string(err))
                return status,err
            end
        end
     end

    CLI_prompts["packet-analyzer"] = "(debug.pa-"..paDb.managerId..")#"

    if status == true then
        CLI_change_mode_push("packet-analyzer")
    end

    -- add commands according to family
    addCommands()

    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerSetInverse
--        @description  packet-analyzer.stage# set inverse mode
--
--        @param params         - params["inverseEnable"]: inverse enable
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerSetInverse(params)
    local status = true
    local err
    local actionId
    local index

    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == paDb.stage then
            groupId = paDb.paTable[i]["group"]
            actionId = paDb.paTable[i]["id"]
            index = i
        end
    end

    result, values = checkSamplingState(actionId)
    if (result ~=true) then
        status = false
        err = values
        print("checkSamplingState returned "..err)
    end
    if result == true and values == returnCodes[GT_BAD_STATE] then
        return result,values
    end

    -- update action inverse state
    result = wrlCpssDxChPacketAnalyzerActionInverseSet(paDb.managerId,actionId,params["inverseEnable"])
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerActionInverseSet returned "..to_string(err))
        return status, err
    end

    paDb.paTable[index]["inverseEnable"] = params["inverseEnable"]

    return status, err
end

-- ************************************************************************
---
--  packetAnalyzerSetSamplingMode
--        @description  packet-analyzer.stage# set sampling mode
--
--        @param params         - params["samplingMode"]: sampling mode
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerSetSamplingMode(params)
    local status = true
    local err
    local actionId
    local index

    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == paDb.stage then
            groupId = paDb.paTable[i]["group"]
            actionId = paDb.paTable[i]["id"]
            index = i
        end
    end

    result, values = checkSamplingState(actionId)
    if (result ~=true) then
        status = false
        err = values
        print("checkSamplingState returned "..err)
    end
    if result == true and values == returnCodes[GT_BAD_STATE] then
        return result,values
    end

    -- update action sampling mode
    result = wrlCpssDxChPacketAnalyzerActionSamplingModeSet(paDb.managerId,actionId,params["samplingMode"])
    if (result ~=0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerActionSamplingModeSet returned "..to_string(err))
        return status, err
    end

    if params["samplingMode"] == "CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E" then
      paDb.paTable[index]["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E
    else
      paDb.paTable[index]["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E
    end

    return status, err
end

--  packetAnalyzerUds
--        @description  add/remove user define stage
--
--        @param params         - params["stage"]: stage ID
--                              - params["instance"]: instance ID
--                              - params["interface"]: interface ID
--                              - params["leg"]: interface leg index
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerUds(params)
    local status = true
    local err
    local udsAttr = {}
    local interfaceAttr = {}
    local values
    local udsIndex

    -- clean instances and indexes tables used for uds input
    for i = 1, #instances do
        table.remove(instances, i)
        table.remove(instancesHelp, i)
    end

    for i = 1, #indexes do
        table.remove(indexes, i)
        table.remove(indexesHelp, i)
    end

    udsAttr = {
        udsNameArr = params["stage"]
    }

    if tonumber(string.len(params["stage"])) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS then
        print ("name not valid")
        return false
    end

    if (params["no"] == "yes") then

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerUserDefinedStageDelete", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC", "udsAttrPtr", udsAttr }

          }
      )
      if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerUserDefinedStageDelete returned "..err)
      end

      -- update DB
      udsId = printPaStageCpssEnum[udsAttr["udsNameArr"]]
      printPaStage[udsId] = printPaStageUdsDefault[udsId]
      printPaStageIndex[printPaStageCpssInt[udsId]] = printPaStageIndexUdsDefault[printPaStageCpssInt[udsId]]
      printPaStageCpssEnum[udsAttr["udsNameArr"]] = nil

    else

      interfaceAttr.instanceId = params["instance"]
      interfaceAttr.interfaceId = params["interface"]
      interfaceAttr.interfaceIndex = params["leg"]
      interfaceAttr.isValid = "GT_FALSE"

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerUserDefinedStageAdd", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC", "udsAttrPtr", udsAttr },
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC", "interfaceAttrPtr", interfaceAttr },
                          { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "udsIdPtr", udsIdPtr }
          }
      )
      if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerUserDefinedStageAdd returned "..err)
      end

      -- update DB
      printPaStage[values["udsIdPtr"]] = udsAttr["udsNameArr"]
      printPaStageIndex[printPaStageCpssInt[values["udsIdPtr"]]] = udsAttr["udsNameArr"]
      printPaStageCpssEnum[udsAttr["udsNameArr"]] = values["udsIdPtr"]

      --print the uds index
      udsIndex = string.gsub(printPaStageUdsDefault[values["udsIdPtr"]], "uds", "index ")
      print(udsIndex)


      --Clear sample data and counters on all rules
      for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == values["udsIdPtr"] then
          id = paDb.paTable[i]["id"]
          break
        end
      end

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerSampledDataCountersClear", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "GT_U32", "ruleId", id },
                          { "IN", "GT_U32", "groupId",paDb.paGrpTable["stage"] }
          }
      )
      if (result ~=0 and result ~=GT_NOT_FOUND) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerSampledDataCountersClear returned "..err)
      end

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerSampledDataCountersClear", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "GT_U32", "ruleId", id_all },
                          { "IN", "GT_U32", "groupId",paDb.paGrpTable["pipeline"] }
          }
      )
      if (result ~=0 and result ~=GT_NOT_FOUND) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerSampledDataCountersClear returned "..err)
      end

    end

end

--  packetAnalyzerUdf
--        @description  add/remove user define field
--
--        @param params         - params["field"]: iDebug field ID
--                              - params["name"]:  field name
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerUdf(params)
    local status = true
    local err
    local udfAttr = {}
    local fieldName = {}
    local values
    local udfIndex
    udfAttr = {
        udfNameArr = params["name"]
    }
    fieldName = {
        fieldNameArr = params["field"]
    }

    if tonumber(string.len(params["name"])) > CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS then
        print ("name not valid")
        return false
    end

    if (params["no"] == "yes") then

      paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"

      result, values = packetAnalyzerSetField(params)
      if (result ~=true) then
          status = false
          err = values
          print("packetAnalyzerSetField returned "..err)
      end

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerUserDefinedFieldDelete", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC", "udfAttrPtr", udfAttr }

          }
      )
      if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerUserDefinedFieldDelete returned "..err)
      end

      -- update DB
      udfId = printPaFieldCpssEnum[params["name"]]
      printPaFieldCpssEnum[params["name"]] = nil
      printPaFieldUdf[printPaFieldCpssInt[udfId]] = nil
      table.remove(printPaField,printPaFieldCpssInt[udfId])

    else

      result, values = myGenWrapper(
          "cpssDxChPacketAnalyzerUserDefinedFieldAdd", {
                          { "IN", "GT_U32", "managerId", paDb.managerId},
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC", "udfAttrPtr", udfAttr },
                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC", "fieldNamePtr", fieldName },
                          { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "udfIdPtr", udfIdPtr }
          }
      )
      if (result ~= 0) then
          status = false
          err = returnCodes[result]
          print("cpssDxChPacketAnalyzerUserDefinedFieldAdd returned "..err)
      end

      -- update DB
      printPaField[printPaFieldCpssInt[values["udfIdPtr"]]] = params["name"]
      printPaFieldCpssEnum[params["name"]] = values["udfIdPtr"]
      printPaFieldUdf[printPaFieldCpssInt[values["udfIdPtr"]]] = params["field"]

      --print the udf index
      udfIndex = string.gsub(printPaFieldUdfDefault[values["udfIdPtr"]], "udf", "index ")
      print(udfIndex)

    end
end

printPaFieldCpss =
{
    [0]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E",
    [1]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E",
    [2]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E",
    [3]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_EPORT_E",
    [4]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E",
    [5]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_IS_TRUNK_E",
    [6]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E",
    [7]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_IS_TRUNK_E",
    [8]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E",
    [9]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_TRUNK_ID_E",
    [10]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E",
    [11]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_PHY_PORT_E",
    [12]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E",
    [13]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E",
    [14]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_VID_E",
    [15]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EVIDX_E",
    [16]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E",
    [17]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_ETHER_TYPE_E",
    [18]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E",
    [19]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E",
    [20]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E",
    [21]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E",
    [22]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPX_PROTOCOL_E",
    [23]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E",
    [24]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E",
    [25]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E",
    [26]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E",
    [27]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV6_E",
    [28]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IP_LEGAL_E",
    [29]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPM_E",
    [30]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_L4_VALID_E",
    [31]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_TO_ME_E",
    [32]  = "CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E",
    [33] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PORT_E",
    [34] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E",
    [35] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_USE_VIDX_E",
    [36] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_DROP_E",
    [37] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_EN_E",
    [38] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_REGISTERED_E",
    [39] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_CPU_CODE_E",
    [40] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_E",
    [41] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_EXTENDED_E",
    [42] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_BRIDGE_E",
    [43] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_INGRESS_PIPE_E",
    [44] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_START_E",
    [45] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_TERMINATED_E",
    [46] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_PACKET_CMD_E",
    [47] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E",
    [48] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_OUTGOING_MTAG_CMD_E",
    [49] ="CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E",
    [50] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E",
    [51] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E",
    [52] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E",
    [53] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_3_E",
    [54] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_4_E",
    [55] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_5_E",
    [56] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_6_E",
    [57] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_7_E",
    [58] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_8_E",
    [59] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_9_E",
    [60] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_10_E",
    [61] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_11_E",
    [62] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_12_E",
    [63] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_13_E",
    [64] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_14_E",
    [65] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_15_E",
    [66] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_16_E",
    [67] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_17_E",
    [68] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_18_E",
    [69] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_19_E",
    [70] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_20_E",
    [71] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_21_E",
    [72] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_22_E",
    [73] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_23_E",
    [74] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_24_E",
    [75] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_25_E",
    [76] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_26_E",
    [77] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_27_E",
    [78] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_28_E",
    [79] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_29_E",
    [80] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_30_E",
    [81] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_31_E",
    [82] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_32_E",
    [83] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_33_E",
    [84] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_34_E",
    [85] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_35_E",
    [86] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_36_E",
    [87] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_37_E",
    [88] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_38_E",
    [89] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_39_E",
    [90] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_40_E",
    [91] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_41_E",
    [92] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_42_E",
    [93] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_43_E",
    [94] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_44_E",
    [95] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_45_E",
    [96] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_46_E",
    [97] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_47_E",
    [98] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_48_E",
    [99] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_49_E",
    [100] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_50_E",
    [101] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_51_E",
    [102] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_52_E",
    [103] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_53_E",
    [104] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_54_E",
    [105] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_55_E",
    [106] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_56_E",
    [107] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_57_E",
    [108] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_58_E",
    [109] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_59_E",
    [110] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_60_E",
    [111] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_61_E",
    [112] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_62_E",
    [113] = "CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_63_E",
}

printPaFieldCpssInt =
{
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E"] = 0,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E"] = 1,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E"] = 2,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_EPORT_E"] = 3,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E"] = 4,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_IS_TRUNK_E"] = 5,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E"] = 6,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_IS_TRUNK_E"] = 7,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E"] = 8,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_TRUNK_ID_E"] = 9,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E"] = 10,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_PHY_PORT_E"] = 11,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E"] = 12,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E"] = 13,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_VID_E"] = 14,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EVIDX_E"] = 15,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E"] = 16,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_ETHER_TYPE_E"] = 17,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E"] = 18,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E"] = 19,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E"] = 20,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E"] = 21,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPX_PROTOCOL_E"] = 22,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E"] = 23,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E"] = 24,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E"] = 25,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E"] = 26,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV6_E"] = 27,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IP_LEGAL_E"] = 28,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_IPM_E"] = 29,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_L4_VALID_E"] = 30,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_TO_ME_E"] = 31,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E"] = 32,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PORT_E"] = 33,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E"] = 34,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_USE_VIDX_E"] = 35,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_DROP_E"] = 36,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_EN_E"] = 37,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_REGISTERED_E"] = 38,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_CPU_CODE_E"] = 39,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_E"] = 40,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_EXTENDED_E"] = 41,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_BRIDGE_E"] = 42,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_INGRESS_PIPE_E"] = 43,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_START_E"] = 44,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_TERMINATED_E"] = 45,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_PACKET_CMD_E"] = 46,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E"] = 47,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_OUTGOING_MTAG_CMD_E"] = 48,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E"] = 49,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E"] = 50,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E"] = 51,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E"] = 52,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_3_E"] = 53,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_4_E"] = 54,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_5_E"] = 55,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_6_E"] = 56,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_7_E"] = 57,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_8_E"] = 58,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_9_E"] = 59,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_10_E"] = 60,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_11_E"] = 61,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_12_E"] = 62,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_13_E"] = 63,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_14_E"] = 64,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_15_E"] = 65,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_16_E"] = 66,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_17_E"] = 67,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_18_E"] = 68,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_19_E"] = 69,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_20_E"] = 70,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_21_E"] = 71,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_22_E"] = 72,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_23_E"] = 73,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_24_E"] = 74,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_25_E"] = 75,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_26_E"] = 76,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_27_E"] = 77,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_28_E"] = 78,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_29_E"] = 79,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_30_E"] = 80,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_31_E"] = 81,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_32_E"] = 82,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_33_E"] = 83,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_34_E"] = 84,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_35_E"] = 85,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_36_E"] = 86,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_37_E"] = 87,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_38_E"] = 88,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_39_E"] = 89,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_40_E"] = 90,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_41_E"] = 91,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_42_E"] = 92,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_43_E"] = 93,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_44_E"] = 94,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_45_E"] = 95,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_46_E"] = 96,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_47_E"] = 97,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_48_E"] = 98,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_49_E"] = 99,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_50_E"] = 100,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_51_E"] = 101,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_52_E"] = 102,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_53_E"] = 103,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_54_E"] = 104,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_55_E"] = 105,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_56_E"] = 106,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_57_E"] = 107,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_58_E"] = 108,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_59_E"] = 109,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_60_E"] = 110,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_61_E"] = 111,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_62_E"] = 112,
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_63_E"] = 113,

}

printPaField =
{
    [0]  = "local_dev_src_is_trunk",
    [1]  = "local_dev_src_port",
    [2]  = "local_dev_src_trunk_id",
    [3]  = "local_dev_src_eport",
    [4]  = "local_dev_trg_phy_port",
    [5]  = "orig_is_trunk",
    [6]  = "orig_src_dev",
    [7]  = "orig_src_phy_is_trunk",
    [8]  = "orig_src_phy_port_or_trunk_id",
    [9]  = "orig_src_trunk_id",
    [10]  = "trg_eport",
    [11]  = "trg_phy_port",
    [12]  = "mac_sa",
    [13]  = "mac_da",
    [14]  = "orig_vid",
    [15]  = "evidx",
    [16]  = "evlan",
    [17]  = "ether_type",
    [18]  = "ipv4_sip",
    [19]  = "ipv4_dip",
    [20]  = "ipv6_sip",
    [21]  = "ipv6_dip",
    [22]  = "ipx_protocol",
    [23]  = "dscp",
    [24]  = "byte_count",
    [25]  = "is_ip",
    [26]  = "is_ipv4",
    [27]  = "is_ipv6",
    [28]  = "ip_legal",
    [29]  = "ipm",
    [30]  = "l4_valid",
    [31]  = "mac_to_me",
    [32]  = "packet_cmd",
    [33]  = "queue_port",
    [34]  = "queue_priority",
    [35]  = "use_vidx",
    [36]  = "egress_filter_drop",
    [37]  = "egress_filter_en",
    [38]  = "egress_filter_registered",
    [39]  = "cpu_code",
    [40]  = "marvell_tagged",
    [41]  = "marvell_tagged_extended",
    [42]  = "bypass_bridge",
    [43]  = "bypass_ingress_pipe",
    [44]  = "tunnel_start",
    [45]  = "tunnel_terminated",
    [46]  = "egress_packet_cmd",
    [47]  = "packet_trace",
    [48]  = "outgoing_mtag_cmd",
    [49]  = "queue_offset",

}

printPaFieldUdfDefault =
{
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E"] = "udf0",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E"] = "udf1",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E"] = "udf2",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_3_E"] = "udf3",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_4_E"] = "udf4",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_5_E"] = "udf5",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_6_E"] = "udf6",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_7_E"] = "udf7",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_8_E"] = "udf8",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_9_E"] = "udf9",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_10_E"] = "udf10",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_11_E"] = "udf11",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_12_E"] = "udf12",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_13_E"] = "udf13",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_14_E"] = "udf14",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_15_E"] = "udf15",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_16_E"] = "udf16",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_17_E"] = "udf17",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_18_E"] = "udf18",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_19_E"] = "udf19",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_20_E"] = "udf20",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_21_E"] = "udf21",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_22_E"] = "udf22",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_23_E"] = "udf23",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_24_E"] = "udf24",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_25_E"] = "udf25",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_26_E"] = "udf26",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_27_E"] = "udf27",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_28_E"] = "udf28",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_29_E"] = "udf29",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_30_E"] = "udf30",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_31_E"] = "udf31",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_32_E"] = "udf32",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_33_E"] = "udf33",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_34_E"] = "udf34",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_35_E"] = "udf35",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_36_E"] = "udf36",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_37_E"] = "udf37",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_38_E"] = "udf38",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_39_E"] = "udf39",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_40_E"] = "udf40",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_41_E"] = "udf41",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_42_E"] = "udf42",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_43_E"] = "udf43",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_44_E"] = "udf44",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_45_E"] = "udf45",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_46_E"] = "udf46",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_47_E"] = "udf47",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_48_E"] = "udf48",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_49_E"] = "udf49",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_50_E"] = "udf50",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_51_E"] = "udf51",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_52_E"] = "udf52",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_53_E"] = "udf53",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_54_E"] = "udf54",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_55_E"] = "udf55",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_56_E"] = "udf56",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_57_E"] = "udf57",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_58_E"] = "udf58",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_59_E"] = "udf59",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_60_E"] = "udf60",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_61_E"] = "udf61",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_62_E"] = "udf62",
    ["CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_63_E"] = "udf63",
}

printPaFieldCpssEnum =
{

}

printPaFieldUdf =
{

}

printPaStage =
{
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E"] = "tti",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E"] = "i-pcl",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E"] = "bridge",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E"] = "router",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E"] = "i-oam",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E"] = "i-policer",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E"] = "mll",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E"] = "equeue",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E"] = "e-filter",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E"] = "tx-queue",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E"] = "header-alt",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E"] = "e-oam",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E"] = "e-pcl",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E"] = "e-policer",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E"] = "timestamp",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E"] = "mac",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E"] = "uds0",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E"] = "uds1",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E"] = "uds2",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E"] = "uds3",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E"] = "uds4",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E"] = "uds5",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E"] = "uds6",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E"] = "uds7",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E"] = "uds8",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E"] = "uds9",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E"] = "uds10",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E"] = "uds11",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E"] = "uds12",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E"] = "uds13",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E"] = "uds14",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E"] = "uds15",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E"] = "uds16",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E"] = "uds17",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E"] = "uds18",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E"] = "uds19",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"] = "pipeline",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E"] = "pha",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E"] = "replication",

}

printPaStageUdsDefault =
{
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E"] = "uds0",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E"] = "uds1",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E"] = "uds2",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E"] = "uds3",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E"] = "uds4",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E"] = "uds5",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E"] = "uds6",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E"] = "uds7",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E"] = "uds8",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E"] = "uds9",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E"] = "uds10",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E"] = "uds11",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E"] = "uds12",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E"] = "uds13",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E"] = "uds14",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E"] = "uds15",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E"] = "uds16",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E"] = "uds17",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E"] = "uds18",
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E"] = "uds19",
}

-- printPaStageIndex correlated to CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT order
printPaStageIndex =
{
    [0] = "tti",
    [1] = "i-pcl",
    [2] = "bridge",
    [3] = "router",
    [4] = "i-oam",
    [5] = "i-policer",
    [6] = "mll",
    [7] = "equeue",
    [8] = "e-filter",
    [9] = "tx-queue",
    [10] = "header-alt",
    [11] = "e-oam",
    [12] = "e-pcl",
    [13] = "e-policer",
    [14] = "timestamp",
    [15] = "mac",
    [16] = "pha",
    [17] = "replication",
    [18] = "uds0",
    [19] = "uds1",
    [20] = "uds2",
    [21] = "uds3",
    [22] = "uds4",
    [23] = "uds5",
    [24] = "uds6",
    [25] = "uds7",
    [26] = "uds8",
    [27] = "uds9",
    [28] = "uds10",
    [29] = "uds11",
    [30] = "uds12",
    [31] = "uds13",
    [32] = "uds14",
    [33] = "uds15",
    [34] = "uds16",
    [35] = "uds17",
    [36] = "uds18",
    [37] = "uds19",
    [38] = "all-ingress",
    [39] = "all-egress",
    [40] = "pipeline",
}

printPaStageIndexUdsDefault =
{
    [18] = "uds0",
    [19] = "uds1",
    [20] = "uds2",
    [21] = "uds3",
    [22] = "uds4",
    [23] = "uds5",
    [24] = "uds6",
    [25] = "uds7",
    [26] = "uds8",
    [27] = "uds9",
    [28] = "uds10",
    [29] = "uds11",
    [30] = "uds12",
    [31] = "uds13",
    [32] = "uds14",
    [33] = "uds15",
    [34] = "uds16",
    [35] = "uds17",
    [36] = "uds18",
    [37] = "uds19",
}

printPaStageIndexHelp =
{
    [0] = "Pre TTI0,1 ingress stages",
    [1] = "Pre PCL0,1,2 ingress stages",
    [2] = "Pre Bridge ingress stage",
    [3] = "Pre Router ingress stage",
    [4] = "Pre OAM ingress stage",
    [5] = "Pre Policer ingress stage",
    [6] = "Pre MLL ingress stage",
    [7] = "Pre e-Queue ingress stage",
    [8] = "Pre Filter egress stage",
    [9] = "Pre tx-Queue egress stage",
    [10] = "Pre Header Alteration egress stage",
    [11] = "Pre OAM egress stage",
    [12] = "Pre PCL egress stage",
    [13] = "Pre Policer egress stage",
    [14] = "Pre Timestamp egress stage",
    [15] = "Pre MAC egress stage",
    [16]= "Pre programmable Header Alteration egress stage",
    [17]= "Pre replication egress stage",
    [18] = "User Defined Stage index 0",
    [19] = "User Defined Stage index 1",
    [20] = "User Defined Stage index 2",
    [21] = "User Defined Stage index 3",
    [22] = "User Defined Stage index 4",
    [23] = "User Defined Stage index 5",
    [24] = "User Defined Stage index 6",
    [25] = "User Defined Stage index 7",
    [26] = "User Defined Stage index 8",
    [27] = "User Defined Stage index 9",
    [28] = "User Defined Stage index 10",
    [29] = "User Defined Stage index 11",
    [30] = "User Defined Stage index 12",
    [31] = "User Defined Stage index 13",
    [32] = "User Defined Stage index 14",
    [33] = "User Defined Stage index 15",
    [34] = "User Defined Stage index 16",
    [35] = "User Defined Stage index 17",
    [36] = "User Defined Stage index 18",
    [37] = "User Defined Stage index 19",
    [38] = "All Ingress Stages",
    [39] = "All Egress Stages",
    [40] = "Pipeline Stages",
}

printPaStageCpssIndex =
{
    [0] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",
    [1] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",
    [2] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",
    [3] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",
    [4] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",
    [5] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",
    [6] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",
    [7] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",
    [8] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",
    [9] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",
    [10] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",
    [11] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",
    [12] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",
    [13] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",
    [14] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",
    [15] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",
    [16] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",
    [17] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E",
    [18] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",
    [19] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",
    [20] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",
    [21] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",
    [22] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",
    [23] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",
    [24] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",
    [25] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",
    [26] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",
    [27] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",
    [28] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",
    [29] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",
    [30] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",
    [31] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",
    [32] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",
    [33] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",
    [34] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",
    [35] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",
    [36] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",
    [37] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E",
    [38] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E",
    [39] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E",
    [40] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E",
}

printPaStageCpssInt =
{
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E"] = 0,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E"] = 1,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E"] = 2,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E"] = 3,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E"] = 4,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E"] = 5,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E"] = 6,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E"] = 7,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E"] = 8,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E"] = 9,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E"] = 10,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E"] = 11,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E"] = 12,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E"] = 13,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E"] = 14,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E"] = 15,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E"] = 16,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E"] = 17,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E"] = 18,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E"] = 19,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E"] = 20,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E"] = 21,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E"] = 22,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E"] = 23,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E"] = 24,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E"] = 25,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E"] = 26,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E"] = 27,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E"] = 28,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E"] = 29,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E"] = 30,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E"] = 31,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E"] = 32,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E"] = 33,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E"] = 34,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E"] = 35,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E"] = 36,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E"] = 37,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E"] = 38,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E"] = 39,
    ["CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"] = 40,
}

printPaStageCpssEnum =
{
    ["tti"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",
    ["i-pcl"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",
    ["bridge"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",
    ["router"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",
    ["i-oam"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",
    ["i-policer"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",
    ["mll"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",
    ["equeue"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",
    ["e-filter"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",
    ["tx-queue"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",
    ["header-alt"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",
    ["e-oam"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",
    ["e-pcl"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",
    ["e-policer"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",
    ["timestamp"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",
    ["mac"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",
    ["uds0"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",
    ["uds1"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",
    ["uds2"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",
    ["uds3"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",
    ["uds4"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",
    ["uds5"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",
    ["uds6"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",
    ["uds7"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",
    ["uds8"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",
    ["uds9"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",
    ["uds10"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",
    ["uds11"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",
    ["uds12"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",
    ["uds13"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",
    ["uds14"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",
    ["uds15"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",
    ["uds16"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",
    ["uds17"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",
    ["uds18"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",
    ["uds19"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E",
    ["all-ingress"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E",
    ["all-egress"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E",
    ["pipeline"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E",
    ["pha"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",
    ["replication"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E",
}

local function printMac(fieldsValues,printMask)

    local mac,value1,value2,value3,mac_msk
    local value4,value5,value6

    value6 = bit_and(fieldsValues[0].data[0],0xFF)
    value5 = bit_and(bit_shr(fieldsValues[0].data[0],8),0xFF)
    value4 = bit_and(bit_shr(fieldsValues[0].data[0],16),0xFF)
    value3 = bit_and(bit_shr(fieldsValues[0].data[0],24),0xFF)

    value2 = bit_and(fieldsValues[0].data[1],0xFF)
    value1 = bit_and(bit_shr(fieldsValues[0].data[1],8),0xFF)

    mac = string.format("%02x:",value1)
    mac = mac..string.format("%02x:",value2)
    mac = mac..string.format("%02x:",value3)
    mac = mac..string.format("%02x:",value4)
    mac = mac..string.format("%02x:",value5)
    mac = mac..string.format("%02x",value6)
    mac=string.format("%-29s",mac)

    if printMask == true then

        value6 = bit_and(fieldsValues[0].msk[0],0xFF)
        value5 = bit_and(bit_shr(fieldsValues[0].msk[0],8),0xFF)
        value4 = bit_and(bit_shr(fieldsValues[0].msk[0],16),0xFF)
        value3 = bit_and(bit_shr(fieldsValues[0].msk[0],24),0xFF)

        value2 = bit_and(fieldsValues[0].msk[1],0xFF)
        value1 = bit_and(bit_shr(fieldsValues[0].msk[1],8),0xFF)

        mac_msk = string.format("%02X:",value1)
        mac_msk = mac_msk..string.format("%02X:",value2)
        mac_msk = mac_msk..string.format("%02X:",value3)
        mac_msk = mac_msk..string.format("%02X:",value4)
        mac_msk = mac_msk..string.format("%02X:",value5)
        mac_msk = mac_msk..string.format("%02X",value6)
        mac_msk=string.format("mask = 0x%-27s",mac_msk)
        mac=mac..mac_msk

    end

    return mac
end

local function printIpv4(fieldsValues,printMask)

    local Ipv4,value1,value2,value3,value4,msk

    value4 = bit_and(fieldsValues[0].data[0],0xFF)
    value3 = bit_and(bit_shr(fieldsValues[0].data[0],8),0xFF)
    value2 = bit_and(bit_shr(fieldsValues[0].data[0],16),0xFF)
    value1 = bit_and(bit_shr(fieldsValues[0].data[0],24),0xFF)

    Ipv4 = string.format("%03u.",value1)
    Ipv4 = Ipv4..string.format("%03u.",value2)
    Ipv4 = Ipv4..string.format("%03u.",value3)
    Ipv4 = Ipv4..string.format("%03u",value4)
    Ipv4 = string.format("%-29s",Ipv4)

    if printMask == true then

        value4 = bit_and(fieldsValues[0].msk[0],0xFF)
        value3 = bit_and(bit_shr(fieldsValues[0].msk[0],8),0xFF)
        value2 = bit_and(bit_shr(fieldsValues[0].msk[0],16),0xFF)
        value1 = bit_and(bit_shr(fieldsValues[0].msk[0],24),0xFF)
        msk=""
        msk = msk..string.format("%03u.",value1)
        msk = msk..string.format("%03u.",value2)
        msk = msk..string.format("%03u.",value3)
        msk = msk..string.format("%03u",value4)
        Ipv4=Ipv4..string.format("mask = %-29s",msk)
    end

    return Ipv4
end

local function printIpv6(fieldsValues,printMask)

    local Ipv6
    local value1,value2,value3,value4,mskGen
    local value5,value6,value7,value8
    local foundZero = nil
    local firstDigit = nil

    value8 = bit_and(fieldsValues[0].data[0],0xFFFF)
    value7 = bit_and(bit_shr(fieldsValues[0].data[0],16),0xFFFF)
    value6 = bit_and(fieldsValues[0].data[1],0xFFFF)
    value5 = bit_and(bit_shr(fieldsValues[0].data[1],16),0xFFFF)
    value4 = bit_and(fieldsValues[0].data[2],0xFFFF)
    value3 = bit_and(bit_shr(fieldsValues[0].data[2],16),0xFFFF)
    value2 = bit_and(fieldsValues[0].data[3],0xFFFF)
    value1 = bit_and(bit_shr(fieldsValues[0].data[3],16),0xFFFF)

    msk8 = bit_and(fieldsValues[0].msk[0],0xFFFF)
    msk7 = bit_and(bit_shr(fieldsValues[0].msk[0],16),0xFFFF)
    msk6 = bit_and(fieldsValues[0].msk[1],0xFFFF)
    msk5 = bit_and(bit_shr(fieldsValues[0].msk[1],16),0xFFFF)
    msk4 = bit_and(fieldsValues[0].msk[2],0xFFFF)
    msk3 = bit_and(bit_shr(fieldsValues[0].msk[2],16),0xFFFF)
    msk2 = bit_and(fieldsValues[0].msk[3],0xFFFF)
    msk1 = bit_and(bit_shr(fieldsValues[0].msk[3],16),0xFFFF)

    if value1 == 0 then
        Ipv6 = "::"
        foundZero = true
    else
        Ipv6 = string.format("%x",value1)
    end
    if value2 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value2 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value2)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value2)
            end
        end
    end
    if value3 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value3 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value3)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value3)
            end
        end
    end
    if value4 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value4 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value4)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value4)
            end
        end
    end
    if value5 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value5 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value5)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value5)
            end
        end
    end
    if value6 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value6 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value6)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value6)
            end
        end
    end
    if value7 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value7 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value7)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value7)
            end
        end
    end
    if value8 == 0 and foundZero == nil then
        Ipv6 = Ipv6.."::"
        foundZero = true
    else if value8 ~= 0 then
            if foundZero == true and firstDigit == nil then
                Ipv6 = Ipv6..string.format("%x",value8)
                firstDigit = true
            else
                Ipv6 = Ipv6..string.format(":%x",value8)
            end
        end
    end

    Ipv6= string.format("%-29s",Ipv6)

    if printMask == true then
        mskGen=""
        mskGen = mskGen..string.format("%04X:",msk1)
        mskGen = mskGen..string.format("%04X:",msk2)
        mskGen = mskGen..string.format("%04X:",msk3)
        mskGen = mskGen..string.format("%04X:",msk4)
        mskGen = mskGen..string.format("%04X:",msk5)
        mskGen = mskGen..string.format("%04X:",msk6)
        mskGen = mskGen..string.format("%04X:",msk7)
        mskGen = mskGen..string.format("%04X",msk8)
        mskGen= string.format("mask = 0x%-27s",mskGen)
        Ipv6=Ipv6..mskGen
    end

    return Ipv6
end

local function printField(fieldStr,fieldsValues,printMask)

  local mac
  local ipv4
  local ipv6
  local udfId

  -- check if the field is udf; then convert from name to idebug field
  udfId = printPaFieldCpssEnum[fieldStr]
  if udfId ~= nil then -- this is udf field
    fieldStr = printPaFieldUdf[printPaFieldCpssInt[udfId]]
  end

  if fieldStr == "mac_sa" or fieldStr == "mac_da" then
      mac = printMac(fieldsValues,printMask)
      header1 = header1.."= "..mac
  elseif fieldStr == "ipv4_dip" or fieldStr == "ipv4_sip" then
      ipv4 = printIpv4(fieldsValues,printMask)
      header1 = header1.."= "..ipv4
  elseif fieldStr == "ipv6_dip" or fieldStr == "ipv6_sip" then
      ipv6 = printIpv6(fieldsValues,printMask)
      header1 = header1.."= "..ipv6
  else
      -- handle enumerations
      result, numOfEnums = wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet(paDb.dev,fieldStr)
      if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerFieldNumOfEnumsGet returned "..err)
      end

      if numOfEnums ~= 0 then
          for i = 1, numOfEnums do
              result, name, value = wrlCpssDxChPacketAnalyzerFieldEnumGet(paDb.dev,fieldStr,i-1)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerFieldEnumGet returned "..err)
              end

              name = string_manipulations(name)

              if (tonumber(value) == fieldsValues[0].data[0]) then
                header1 = header1..string.format("= %-29s",name)
                if printMask == true then
                  header1 = header1..string.format("mask = 0x%-27X",fieldsValues[0].msk[0])
                end
                return
              end
          end

      end

      header1 = header1..string.format("= %-29u",fieldsValues[0].data[0])
      if printMask == true then
        header1 = header1..string.format("mask = 0x%-27X",fieldsValues[0].msk[0])
      end
  end
end

local function printBlockOfData(dataToPrint)
    local lines = splitString(dataToPrint, "\n")

    for i = 1, #lines do
        print(lines[i])
    end
end

local function do_command_with_print_nice_iDebug(cmd, ...)
    print_nice_saved_print = print
    print_nice_lines = 0
    print_nice_interrupted = false
    if true == myCPSSGenWrapperPausedPrinting   then
        print = print_nice
    else
        print = print
    end
    local success, rc

    success, rc = pcall(...)

    print = print_nice_saved_print

    if success then
        return rc
    end
    if print_nice_interrupted then
        print_nice_interrupted = false
        return true
    end

    print("Failed in "..cmd..": "..rc)
    return false
end

-- ************************************************************************
---
--  packetAnalyzerShow
--        @description  allocates packet analyzer. Enters to packet-analyzer
--                      CLI mode
--
--        @param params         - params["managerId"]: Manager ID (optional)
--                              - params["device"]: Device ID (optional)
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerShowStage(params)
    local validStage
    local result, values, values1
    local i, j, index, found
    local command_data = Command_Data()
    local fieldsValues = {}
    local stages = {}
    local foundStage = GT_FALSE

    -- prepare stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end

    if values["numOfValidStagesPtr"] ~= 0 then
        for i = 1, values["numOfValidStagesPtr"] do
            validStage = values["validStagesListArr"][i-1]
            table.insert(stages ,validStage)
            if validStage == paDb.stage then
                foundStage = GT_TRUE
            end
        end
    end

    fieldsValues[0] = {}
    fieldsValues[0].data = {}
    fieldsValues[0].msk = {}

    -- Common variables initialization
    command_data:clearResultArray()

    -- Command specific variables initialization
    header_string = ""
    footer_string = ""

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    if paDb.stage ~= "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" then
      stages = {paDb.stage}
    else
        foundStage = GT_TRUE
    end

    if values["numOfValidStagesPtr"] == 0 then
        result = GT_BAD_STATE
        err = returnCodes[result]
        print("No valid stages")
        return false, err
    end

    if foundStage == GT_FALSE then
        result = GT_BAD_STATE
        err = returnCodes[result]
        print("Stage "..printPaStage[paDb.stage].." is not valid")
        return false, err
    end

    -- check that pipeline mode is not enabled
    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E" then
            id = paDb.paTable[i]["id"]
        end
    end

    result, values = checkSamplingState(id)
    if (result ~=true) then
      status = false
      err = values
      print("checkSamplingState returned "..err)
    end
    if result == true and values == returnCodes[GT_BAD_STATE] then
      return result,values
    end

    for k=1,#stages do

      header1 = ""
      for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == stages[k] then
            groupId = paDb.paTable[i]["group"]
            id = paDb.paTable[i]["id"]
            index = i
        end
      end

      result, values = checkSamplingState(id)
      if (result ~=true) then
          status = false
          err = values
          print("checkSamplingState returned "..err)
      end
      if result == true and values == returnCodes[GT_BAD_STATE] then
          return result,values
      end

      result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,id,groupId,stages[k])
      stageStr = printPaStage[stages[k]]

      header1 = header1..string.format("\nStage: %-15s",stageStr)
      header1 = header1.."Hits: "..numOfHits.."\n\n"

      if paDb.paTable[index]["inverseEnable"] == true then
            header1 = header1.."\tMatch (inverse):"
      else
            header1 = header1.."\tMatch :"
      end

      result, values = myGenWrapper(
         "cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "IN", "GT_U32", "keyId", id},
                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                        { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
         }
      )
      if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet returned "..err)
      end

      if values["numOfFieldsPtr"] == 0 then
            header1 = header1.." ANY\n"
      else
          header1 = header1.."\n"
          result, values1 = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupRuleGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "ruleId", id},
                            { "IN", "GT_U32", "groupId", groupId},
                            { "OUT", "GT_U32", "keyIdPtr", keyIdPtr},
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC", "ruleAttrPtr", ruleAttrPtr},
                            { "INOUT", "GT_U32", "numOfFieldsValuePtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "fieldsValueArr" },
                            { "OUT", "GT_U32", "ruleAttrPtr", actionIdPtr}
            }
         )
         if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupRuleGet returned "..err)
         end
         for i = 0, values["numOfFieldsPtr"]-1 do
            fieldt = values["fieldsArr"][i]
            field = printPaFieldCpssInt[fieldt]
            fieldStr = printPaField[field]
            header1 = header1..string.format("\t\t%-40s",fieldStr)
            for j=1, values1["numOfFieldsValuePtr"] do
                if values1["fieldsValueArr"][j-1].fieldName == fieldt then
                    fieldsValues[0].data[0] = values1["fieldsValueArr"][j-1].data[0]
                    fieldsValues[0].data[1] = values1["fieldsValueArr"][j-1].data[1]
                    fieldsValues[0].data[2] = values1["fieldsValueArr"][j-1].data[2]
                    fieldsValues[0].data[3] = values1["fieldsValueArr"][j-1].data[3]
                    fieldsValues[0].msk[0] = values1["fieldsValueArr"][j-1].msk[0]
                    fieldsValues[0].msk[1] = values1["fieldsValueArr"][j-1].msk[1]
                    fieldsValues[0].msk[2] = values1["fieldsValueArr"][j-1].msk[2]
                    fieldsValues[0].msk[3] = values1["fieldsValueArr"][j-1].msk[3]

                    printField(fieldStr,fieldsValues,true)
                    header1 = header1.."\n"
                    break
                end
            end
          end
        end

        header1 = header1.."\n\tSampled data"
        if paDb.paTable[index]["samplingMode"] == CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E then
            header1 = header1.." (last-match):"
        else
            header1 = header1.." (first-match):"
        end
        if numOfHits == 0 then
          header1 = header1.." NONE\n"
        else
          header1 = header1.."\n"
          if params["field"] == nil then --sampled fields

            if values["numOfFieldsPtr"] ~= 0 then

                result, values1 = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                    { "IN", "GT_U32", "managerId", paDb.managerId},
                                    { "IN", "GT_U32", "ruleId", id},
                                    { "IN", "GT_U32", "groupId", groupId},
                                    { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                    { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                    { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                    { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                    }
                )
                if (result ~= 0) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                end

                found = false
                for i = 1, values["numOfFieldsPtr"] do
                        fieldt = values["fieldsArr"][i-1]
                        field = printPaFieldCpssInt[fieldt]
                        fieldStr = printPaField[field]

                        for j=1, values1["numOfSampleFieldsPtr"] do
                            if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                                header1 = header1..string.format("\t\t%-40s",fieldStr)

                                fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                                fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                                fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                                fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                                found = true
                                printField(fieldStr,fieldsValues,false)
                                header1 = header1.."\n"
                                break
                            end
                        end
                end
                if found == false then
                    header1 = header1.."\t\tNONE\n"
                end
            end

          else --detailed fields (dump/all/single field)
            result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k] },
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
            end

            if values["numOfFieldsPtr"] ~= 0 then

                if params["field"] == "dump" then --dump all idebug fields
                      result, values1 = myGenWrapper(
                          "cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet", {
                                          { "IN", "GT_U32", "managerId", paDb.managerId},
                                          { "IN", "GT_U32", "ruleId", id},
                                          { "IN", "GT_U32", "groupId", groupId},
                                          { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                          { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                          { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 500 },
                                          { "OUT", "CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC[500]", "sampleFieldsValueArr" }
                          }
                      )
                    if (result ~= 0) then
                        status = false
                        err = returnCodes[result]
                        print("cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet returned "..err)
                    else
                        for j=1, values1["numOfSampleFieldsPtr"] do
                            test1 = string.format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                                values1["sampleFieldsValueArr"][j-1].fieldName[0], values1["sampleFieldsValueArr"][j-1].fieldName[1],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[2], values1["sampleFieldsValueArr"][j-1].fieldName[3],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[4], values1["sampleFieldsValueArr"][j-1].fieldName[5],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[6], values1["sampleFieldsValueArr"][j-1].fieldName[7],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[8], values1["sampleFieldsValueArr"][j-1].fieldName[9],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[10], values1["sampleFieldsValueArr"][j-1].fieldName[11],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[12], values1["sampleFieldsValueArr"][j-1].fieldName[13],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[14], values1["sampleFieldsValueArr"][j-1].fieldName[15],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[16], values1["sampleFieldsValueArr"][j-1].fieldName[17],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[18], values1["sampleFieldsValueArr"][j-1].fieldName[19],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[20], values1["sampleFieldsValueArr"][j-1].fieldName[21],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[22], values1["sampleFieldsValueArr"][j-1].fieldName[23],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[24], values1["sampleFieldsValueArr"][j-1].fieldName[25],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[26], values1["sampleFieldsValueArr"][j-1].fieldName[27],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[28], values1["sampleFieldsValueArr"][j-1].fieldName[29],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[30], values1["sampleFieldsValueArr"][j-1].fieldName[31],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[32], values1["sampleFieldsValueArr"][j-1].fieldName[33],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[34], values1["sampleFieldsValueArr"][j-1].fieldName[35],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[36], values1["sampleFieldsValueArr"][j-1].fieldName[37],
                                                values1["sampleFieldsValueArr"][j-1].fieldName[38], values1["sampleFieldsValueArr"][j-1].fieldName[39])
                            header1 = header1..string.format("\t\t%-40s",test1)

                            fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                            fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                            fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                            fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                            printField(test1,fieldsValues,false)
                            header1 = header1.."\n"
                        end
                    end
                else
                    result, values1 = myGenWrapper(
                        "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                        { "IN", "GT_U32", "managerId", paDb.managerId},
                                        { "IN", "GT_U32", "ruleId", id},
                                        { "IN", "GT_U32", "groupId", groupId},
                                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                        { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                        { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                        }
                    )
                    if (result ~= 0) then
                        status = false
                        err = returnCodes[result]
                        print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                    end

                    found = false
                    for i = 1, values["numOfFieldsPtr"] do
                        fieldt = values["fieldsArr"][i-1]
                        field = printPaFieldCpssInt[fieldt]
                        fieldStr = printPaField[field]

                        if params["field"] ~= "all" and params["field"] == fieldStr then --single field

                            for j=1, values1["numOfSampleFieldsPtr"] do
                                if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                                    header1 = header1..string.format("\t\t%-40s",fieldStr)

                                    fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                                    fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                                    fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                                    fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                                    printField(fieldStr,fieldsValues,false)
                                    header1 = header1.."\n"
                                    found = true
                                    break
                                end
                            end

                            if found == false then
                                header1 = header1.."\t\tNONE\n"
                            end
                            break

                        elseif params["field"] == "all" then --all fields
                            for j=1, values1["numOfSampleFieldsPtr"] do
                                if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                                    header1 = header1..string.format("\t\t%-40s",fieldStr)

                                    fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                                    fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                                    fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                                    fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                                    printField(fieldStr,fieldsValues,false)
                                    header1 = header1.."\n"
                                    found = true
                                    break
                                end
                            end
                        end --single field/all fields
                    end
                    if found == false then
                        header1 = header1.."\t\tNONE\n"
                    end
                end
            end
          end --detailed/sampled fields
        end

      header1 = header1.."\n"
      command_data["result"] = string.format("%s",header1)
      command_data:addResultToResultArray()

  end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()

    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    --command_data:printCommandExecutionResults()
    for index, item in pairs(command_data["result"]) do
        printBlockOfData(tostring(item),true)
    end

    return command_data:getCommandExecutionResults()
end

local function show_stage(params)
    return do_command_with_print_nice_iDebug(
                "packetAnalyzerShowStage()",
                packetAnalyzerShowStage,
                params)

end
-- ************************************************************************
---
--  packetAnalyzerShowPipeline
--        @description  allocates packet analyzer. Enters to packet-analyzer
--                      CLI mode
--
--        @param params         - params["managerId"]: Manager ID (optional)
--                              - params["device"]: Device ID (optional)
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerShowPipeline(params)
    local validStage
    local result, values, values1
    local i,j, index, found
    local command_data = Command_Data()
    local fieldsValues = {}
    local fields = {}
    local tFields = {}
    local field
    local stages = {}

    -- prepare stages list
    result, values = myGenWrapper(
        "cpssDxChPacketAnalyzerStagesGet", {
                        { "IN", "GT_U32", "managerId", paDb.managerId},
                        { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                        { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("cpssDxChPacketAnalyzerStagesGet returned "..err)
    end

    if values["numOfValidStagesPtr"] ~= 0 then
        for i = 1, values["numOfValidStagesPtr"] do
            validStage = values["validStagesListArr"][i-1]
            table.insert(stages ,validStage)
        end
    end

    fieldsValues[0] = {}
    fieldsValues[0].data = {}
    fieldsValues[0].msk = {}

    -- Common variables initialization
    command_data:clearResultArray()

    -- Command specific variables initialization
    header_string = ""
    footer_string = ""

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    if values["numOfValidStagesPtr"] == 0 then
        result = GT_BAD_STATE
        err = returnCodes[result]
        print("No valid stages")
        return false, err
    end

    -- check that stages mode is not enabled
    for k=1,#stages do

      for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == stages[k] then
            id = paDb.paTable[i]["id"]
        end
      end

      result, values = checkSamplingState(id)
      if (result ~=true) then
          status = false
          err = values
          print("checkSamplingState returned "..err)
      end
      if result == true and values == returnCodes[GT_BAD_STATE] then
          return result,values
      end
    end

    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == paDb.stage then
            groupId = paDb.paTable[i]["group"]
            id = paDb.paTable[i]["id"]
            index = i
        end
    end

    result, values = checkSamplingState(id)
    if (result ~=true) then
      status = false
      err = values
      print("checkSamplingState returned "..err)
    end
    if result == true and values == returnCodes[GT_BAD_STATE] then
      return result,values
    end

    header1 = "Hits:"
    for k=1,#stages do

      result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,id,groupId,stages[k])
      stageStr = printPaStage[stages[k]]
      header1 = header1..string.format("\n\t%-12s:",stageStr)
      header1 = header1..string.format("  %-30u",numOfHits)

    end

    if paDb.paTable[index]["inverseEnable"] == true then
        header1 = header1.."\n\nMatch (inverse):"
    else
        header1 = header1.."\n\nMatch :"
    end

    for k=1,#stages do

        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "keyId", id},
                            { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                            { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet returned "..err)
        end
        if values["numOfFieldsPtr"] ~= 0 then
            for i = 0, values["numOfFieldsPtr"]-1 do
                fieldt = values["fieldsArr"][i]
                field = printPaFieldCpssInt[fieldt]
                fieldStr = printPaField[field]
                table.insert(tFields ,fieldStr)
            end
        end
    end

    table.sort(tFields)
    for key,value in ipairs(tFields) do
        if value ~=tFields[key+1] then
            table.insert(fields,value)
        end
    end

    if #fields == 0 then
        header1 = header1.." ANY\n"
    else
        header1 = header1.."\n"
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerGroupRuleGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "IN", "GT_U32", "ruleId", id},
                            { "IN", "GT_U32", "groupId", groupId},
                            { "OUT", "GT_U32", "keyIdPtr", keyIdPtr},
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC", "ruleAttrPtr", ruleAttrPtr},
                            { "INOUT", "GT_U32", "numOfFieldsValuePtr", 120 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "fieldsValueArr" },
                            { "OUT", "GT_U32", "ruleAttrPtr", actionIdPtr}
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerGroupRuleGet returned "..err)
        end
        for key,value in ipairs(fields) do
            for i=1, #printPaField+1 do
                if printPaField[i-1] == value then
                    field = printPaFieldCpss[i-1]
                    break
                end
            end

            header1 = header1..string.format("\t%-40s",value)
            for i=1, values["numOfFieldsValuePtr"] do
                if values["fieldsValueArr"][i-1].fieldName == field then
                    fieldsValues[0].data[0] = values["fieldsValueArr"][i-1].data[0]
                    fieldsValues[0].data[1] = values["fieldsValueArr"][i-1].data[1]
                    fieldsValues[0].data[2] = values["fieldsValueArr"][i-1].data[2]
                    fieldsValues[0].data[3] = values["fieldsValueArr"][i-1].data[3]
                    fieldsValues[0].msk[0] = values["fieldsValueArr"][i-1].msk[0]
                    fieldsValues[0].msk[1] = values["fieldsValueArr"][i-1].msk[1]
                    fieldsValues[0].msk[2] = values["fieldsValueArr"][i-1].msk[2]
                    fieldsValues[0].msk[3] = values["fieldsValueArr"][i-1].msk[3]

                    printField(value,fieldsValues,true)
                    header1 = header1.."\n"
                    break
                end
            end
        end
    end

    header1 = header1.."\nSampled data"
    if paDb.paTable[index]["samplingMode"] == CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E then
        header1 = header1.." (last-match):"
    else
        header1 = header1.." (first-match):"
    end

    if params["field"] ~= nil and params["field"] ~= "all" and params["field"] ~= "dump" then --spesific field
     for k=1,#stages do

         stageStr = printPaStage[stages[k]]
         header1 = header1..string.format("\n\tStage %-15s: ",stageStr)

         result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,id,groupId,stages[k])
         if numOfHits == 0 then
            header1 = header1.."NONE\n"
         else
            result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k] },
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
            end
            if values["numOfFieldsPtr"] ~= 0 then

                result, values1 = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                    { "IN", "GT_U32", "managerId", paDb.managerId},
                                    { "IN", "GT_U32", "ruleId", id},
                                    { "IN", "GT_U32", "groupId", groupId},
                                    { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                    { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                    { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                    { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                    }
                )
                if (result ~= 0) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                end
                for i = 1, values["numOfFieldsPtr"] do
                    fieldt = values["fieldsArr"][i-1]
                    field = printPaFieldCpssInt[fieldt]
                    fieldStr = printPaField[field]
                    if params["field"] == fieldStr then --single field
                      header1 = header1..string.format("%-40s",fieldStr)
                      for j=1, values1["numOfSampleFieldsPtr"] do

                        if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                            fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                            fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                            fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                            fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                            printField(fieldStr,fieldsValues,false)
                            break
                        end
                      end
                      break

                    end
                end
            end
            header1 = header1.."\n"
         end
       end
    else
     for k=1,#stages do

        stageStr = printPaStage[stages[k]]
        header1 = header1..string.format("\n\tStage: %-15s",stageStr)

        result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,id,groupId,stages[k])
        if numOfHits == 0 then
          header1 = header1.."\n\t\tNONE\n"
        else
          header1 = header1.."\n"
          if params["field"] == nil then --sampled fields

                result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet", {
                                    { "IN", "GT_U32", "managerId", paDb.managerId},
                                    { "IN", "GT_U32", "keyId", id},
                                    { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                    { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                    { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                    }
                )
                if (result ~= 0) then
                    status = false
                    err = returnCodes[result]
                    print("cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet returned "..err)
                end
                if values["numOfFieldsPtr"] ~= 0 then

                    result, values1 = myGenWrapper(
                        "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                        { "IN", "GT_U32", "managerId", paDb.managerId},
                                        { "IN", "GT_U32", "ruleId", id},
                                        { "IN", "GT_U32", "groupId", groupId},
                                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                        { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                        { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                        }
                    )
                    if (result ~= 0) then
                        status = false
                        err = returnCodes[result]
                        print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                    end

                    found = false
                    for i = 1, values["numOfFieldsPtr"] do
                        fieldt = values["fieldsArr"][i-1]
                        field = printPaFieldCpssInt[fieldt]
                        fieldStr = printPaField[field]

                        for j=1, values1["numOfSampleFieldsPtr"] do
                            if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                                header1 = header1..string.format("\t\t%-40s",fieldStr)

                                fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                                fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                                fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                                fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                                found = true
                                printField(fieldStr,fieldsValues,false)
                                header1 = header1.."\n"
                                break
                            end
                        end
                    end
                    if found == false then
                        header1 = header1.."\t\tNONE\n"
                    end

                end

          elseif params["field"] == "dump" then --dump all idebug fields
              result, values1 = myGenWrapper(
                  "cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet", {
                                  { "IN", "GT_U32", "managerId", paDb.managerId},
                                  { "IN", "GT_U32", "ruleId", id},
                                  { "IN", "GT_U32", "groupId", groupId},
                                  { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                  { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                  { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 500 },
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC[500]", "sampleFieldsValueArr" }
                  }
              )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet returned "..err)
            else
                for j=1, values1["numOfSampleFieldsPtr"] do
                    test1 = string.format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                        values1["sampleFieldsValueArr"][j-1].fieldName[0], values1["sampleFieldsValueArr"][j-1].fieldName[1],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[2], values1["sampleFieldsValueArr"][j-1].fieldName[3],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[4], values1["sampleFieldsValueArr"][j-1].fieldName[5],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[6], values1["sampleFieldsValueArr"][j-1].fieldName[7],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[8], values1["sampleFieldsValueArr"][j-1].fieldName[9],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[10], values1["sampleFieldsValueArr"][j-1].fieldName[11],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[12], values1["sampleFieldsValueArr"][j-1].fieldName[13],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[14], values1["sampleFieldsValueArr"][j-1].fieldName[15],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[16], values1["sampleFieldsValueArr"][j-1].fieldName[17],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[18], values1["sampleFieldsValueArr"][j-1].fieldName[19],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[20], values1["sampleFieldsValueArr"][j-1].fieldName[21],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[22], values1["sampleFieldsValueArr"][j-1].fieldName[23],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[24], values1["sampleFieldsValueArr"][j-1].fieldName[25],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[26], values1["sampleFieldsValueArr"][j-1].fieldName[27],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[28], values1["sampleFieldsValueArr"][j-1].fieldName[29],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[30], values1["sampleFieldsValueArr"][j-1].fieldName[31],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[32], values1["sampleFieldsValueArr"][j-1].fieldName[33],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[34], values1["sampleFieldsValueArr"][j-1].fieldName[35],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[36], values1["sampleFieldsValueArr"][j-1].fieldName[37],
                                        values1["sampleFieldsValueArr"][j-1].fieldName[38], values1["sampleFieldsValueArr"][j-1].fieldName[39])
                    header1 = header1..string.format("\t\t%-40s",test1)

                    fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                    fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                    fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                    fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                    printField(test1,fieldsValues,false)
                    header1 = header1.."\n"
                end
            end
          else --params["field"] == "all" then --detailed fields

            result, values = myGenWrapper(
                    "cpssDxChPacketAnalyzerStageFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k] },
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageFieldsGet returned "..err)
            end
            if values["numOfFieldsPtr"] ~= 0 then

                  result, values1 = myGenWrapper(
                        "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                        { "IN", "GT_U32", "managerId", paDb.managerId},
                                        { "IN", "GT_U32", "ruleId", id},
                                        { "IN", "GT_U32", "groupId", groupId},
                                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", stages[k]},
                                        { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                        { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                        }
                    )
                    if (result ~= 0) then
                        status = false
                        err = returnCodes[result]
                        print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                    end

                    found = false
                    for i = 1, values["numOfFieldsPtr"] do
                        fieldt = values["fieldsArr"][i-1]
                        field = printPaFieldCpssInt[fieldt]
                        fieldStr = printPaField[field]

                        for j=1, values1["numOfSampleFieldsPtr"] do
                            if values1["sampleFieldsValueArr"][j-1].fieldName == fieldt then
                                header1 = header1..string.format("\t\t%-40s",fieldStr)

                                fieldsValues[0].data[0] = values1["sampleFieldsValueArr"][j-1].data[0]
                                fieldsValues[0].data[1] = values1["sampleFieldsValueArr"][j-1].data[1]
                                fieldsValues[0].data[2] = values1["sampleFieldsValueArr"][j-1].data[2]
                                fieldsValues[0].data[3] = values1["sampleFieldsValueArr"][j-1].data[3]

                                printField(fieldStr,fieldsValues,false)
                                header1 = header1.."\n"
                                found = true
                                break
                            end

                        end
                    end

                if found == false then
                    header1 = header1.."\t\tNONE\n"
                end
            end
          end
        end
      end
    end
    command_data["result"] = string.format("%s",header1)
    command_data:addResultToResultArray()

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()

    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    --command_data:printCommandExecutionResults()
    for index, item in pairs(command_data["result"]) do
        printBlockOfData(tostring(item),true)
    end

    return command_data:getCommandExecutionResults()
end

local function show_pipeline(params)
    return do_command_with_print_nice_iDebug(
                "packetAnalyzerShowPipeline()",
                packetAnalyzerShowPipeline,
                params)

end

-- ************************************************************************
---
--  packetAnalyzerShowUds
--        @description  Display packet analyzer configurtions for user defined stages
--
--        @param params         - params["all"]: display also pre defined stages (optional)
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerShowUds(params)
    local status = true
    local err
    local result
    local stages = {}

    header1 = string.format("\n%-30s   ","Stage Name")
    header1 = header1..string.format("%-45s   ","Interface")
    header1 = header1..string.format("%-50s   ","Instance")
    header1 = header1..string.format("%-3s   ","Leg")
    if (params["all"] == "yes") then
        header1 = header1..string.format("%-7s ","Status")

        header1 = header1..("\n------------------------------- ----------------------------------------------- ---------------------------------------------------- ----- ---------")
    else
        header1 = header1..string.format("%-7s   ","Status")
        header1 = header1..string.format("%-5s ","Index")
        header1 = header1..("\n------------------------------- ----------------------------------------------- ---------------------------------------------------- ----- --------- -------")
    end

    if (params["all"] == "yes") then

        -- prepare all applicable stages list
        result, values = myGenWrapper(
            "cpssDxChPacketAnalyzerStagesGet", {
                            { "IN", "GT_U32", "managerId", paDb.managerId},
                            { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                            { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                            { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
            }
        )
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("cpssDxChPacketAnalyzerStagesGet returned "..err)
        end
        if values["numOfApplicStagesPtr"] ~= 0 then
            for i = 1, values["numOfApplicStagesPtr"] do
                applicStaget = values["applicStagesListArr"][i-1]
                applicStage = printPaStageCpssInt[applicStaget]
                table.insert(stages ,printPaStageCpssIndex[applicStage])
            end
        end

        for i=1, #stages do

            result,interfaceId,instanceId,interfaceIndex,valid = wrlCpssDxChPacketAnalyzerPreDefinedStageInfoGet(paDb.managerId,paDb.dev,stages[i])
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("wrlCpssDxChPacketAnalyzerPreDefinedStageInfoGet returned "..to_string(err))
                return status,err
            end

            if (valid == 1) then
              validStr = "valid"
            else
              validStr = "invalid"
            end

            header1 = header1..string.format("\n%-30s   %-45s   %-50s   %-3s   %-7s",
                            printPaStage[stages[i]],interfaceId,instanceId,interfaceIndex,validStr)

        end

        header1 = header1.."\n"
        print(string.format("%s",header1))

    else

        result,firstUds,lastUds = wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet()
        if (result ~= 0) then
            status = false
            err = returnCodes[result]
            print("wrlCpssDxChPacketAnalyzerUdsFirstLastIndexGet returned "..to_string(err))
            return status,err
        end

        for udsId = firstUds, lastUds do

            result, values = myGenWrapper(
                  "cpssDxChPacketAnalyzerUserDefinedStageInfoGet", {
                                  { "IN", "GT_U32", "managerId", paDb.managerId},
                                  { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "udsId", udsId},
                                  { "OUT", "GT_BOOL", "validPtr", validPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC", "udsAttrPtr", udsAttrPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC", "interfaceAttrPtr", interfaceAttrPtr}
                  }
              )
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("cpssDxChPacketAnalyzerUserDefinedStageInfoGet returned "..err)
              end

              result,valid = wrlCpssDxChPacketAnalyzerStageValidityGet(paDb.managerId,paDb.dev,udsId)
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("wrlCpssDxChPacketAnalyzerStageValidityGet returned "..to_string(err))
                  return status,err
              end
              if (valid == 1) then
                  validStr = "valid"
              else
                  validStr = "invalid"
              end

              udsIndex = string.sub(printPaStageUdsDefault[printPaStageCpssIndex[udsId]],4)
              if (values["validPtr"] == GT_TRUE) then
                  header1 = header1..string.format("\n%-30s   %-45s   %-50s   %-3s   %-7s   %-5s",
                                values["udsAttrPtr"].udsNameArr[0],values["interfaceAttrPtr"].interfaceId[0],
                                values["interfaceAttrPtr"].instanceId[0],values["interfaceAttrPtr"].interfaceIndex,validStr,udsIndex)
              end
        end

        header1 = header1.."\n"
        print(string.format("%s",header1))

    end
end

-- ************************************************************************
---
--  packetAnalyzerShowUdf
--        @description  Display packet analyzer configurtions for user defined fields
--
--        @return       true on success, otherwise false and error message
--
local function packetAnalyzerShowUdf()
    local status = true
    local err
    local result

    header1 = string.format("\n%-30s   ","Field Name")
    header1 = header1..string.format("%-40s   ","Field Id")
    header1 = header1..string.format("%-5s ","Index")
    header1 = header1..("\n------------------------------- ------------------------------------------ -------")

    -- update udf DB
    result,firstUdf,lastUdf = wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet()
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
        print("wrlCpssDxChPacketAnalyzerUdfFirstLastIndexGet returned "..to_string(err))
        return status,err
    end

    for udfId = firstUdf, lastUdf do

        result, values = myGenWrapper(
                  "cpssDxChPacketAnalyzerUserDefinedFieldInfoGet", {
                                  { "IN", "GT_U32", "managerId", paDb.managerId},
                                  { "IN", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT", "udfId", udfId},
                                  { "OUT", "GT_BOOL", "validPtr", validPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC", "udfAttrPtr", udfAttrPtr},
                                  { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC", "fieldNamePtr", fieldNamePtr}
                  }
              )
              if (result ~= 0) then
                  status = false
                  err = returnCodes[result]
                  print("cpssDxChPacketAnalyzerUserDefinedFieldInfoGet returned "..err)
              end

        udfIndex = string.sub(printPaFieldUdfDefault[printPaFieldCpss[udfId]],4)
        if (values["validPtr"] == GT_TRUE) then
            header1 = header1..string.format("\n%-30s   %-40s   %-5s",
                        values["udfAttrPtr"].udfNameArr[0],values["fieldNamePtr"].fieldNameArr[0],udfIndex)
        end
    end

    header1 = header1.."\n"
    print(string.format("%s",header1))
end

--########################################################################################################
-------------------------------------------------------
-- command registration: packet-analyzer manager device
-------------------------------------------------------
CLI_addCommand("debug", "packet-analyzer manager", {
    func=packetAnalyzer,
    help="Configure packet analyzer",
     params={
        {   type="values",
                { format="%manager_id", optional = false, name="managerId", help=CLI_type_dict["manager_id"].help}
        },
        {   type="named",
                { format="device %devID", optional = false, name="device", help=CLI_type_dict["devID"].help},
                notMandatory = "device"
        }
    }
})

-------------------------------------------------------
-- command registration: packet-analyzer device
-------------------------------------------------------
CLI_addCommand("debug", "packet-analyzer device", {
    func=packetAnalyzer,
    help="Configure packet analyzer",
     params={
        {   type="values",
                { format="%devID", optional = false, name="device", help=CLI_type_dict["devID"].help},
                notMandatory = "device"
        }
    }
})

--------------------------------------------
-- command registration: no packet-analyzer
--------------------------------------------
CLI_addCommand("debug", "no packet-analyzer manager", {
    func=noPacketAnalyzer,
    help="Disable Packet Analyzer",
    params={
        {   type="values",
                { format="%manager_id", optional = false, name="managerId", help=CLI_type_dict["manager_id"].help}
        },
        {   type="named",
                { format="device %devID", optional = true, name="device", help=CLI_type_dict["devID"].help},
            notMandatory = "device"
        }
    }
})

--------------------------------------------
-- command registration: no packet-analyzer
--------------------------------------------
CLI_addCommand("debug", "no packet-analyzer device", {
    func=noPacketAnalyzer,
    help="Disable Packet Analyzer",
    params={
        {   type="values",
                { format="%devID", optional = false, name="device", help=CLI_type_dict["devID"].help},
                notMandatory = "device"
        }
    }
})

--------------------------------------------
-- command registration: packet-analyzer enable
--------------------------------------------
CLI_addCommand("packet-analyzer", "enable pipeline", {
    func=function(params)
        params["group"] = "pipeline"
        return packetAnalyzerEnable(params)
    end,
    help="Enable Packet Analyzer"
})

--------------------------------------------
-- command registration: packet-analyzer enable
--------------------------------------------
CLI_addCommand("packet-analyzer", "enable stage", {
    func=function(params)
        params["group"] = "stage"
        return packetAnalyzerEnable(params)
    end,
    help="Enable Packet Analyzer"
})

--------------------------------------------
-- command registration: packet-analyzer disable
--------------------------------------------
CLI_addCommand("packet-analyzer", "disable", {
    func=packetAnalyzerDisable,
    help="Disable Packet Analyzer"
})

--------------------------------------------
-- command registration: packet-analyzer clear
--------------------------------------------
CLI_addCommand("packet-analyzer", "clear", {
    func=packetAnalyzerClear,
    help="Clear Packet Analyzer"
})

--------------------------------------------
-- command registration: no stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "no configure stage", {
    func=noPacketAnalyzerStage,
    help="Remove stage configuration",
    params={
        {
            type="named",
            { format="%pa_stage", name="stage", help=CLI_type_dict["pa_stage"].help}
        }
    }
})

--------------------------------------------
-- command registration: no pipeline
--------------------------------------------
CLI_addCommand("packet-analyzer", "no configure pipeline", {
    func=function(params)
        params["stage"] = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
        return noPacketAnalyzerStage(params)
    end,
    help="Remove pipeline configuration"
})

--------------------------------------------
-- command registration: packet-analyzer show pipeline
--------------------------------------------
CLI_addCommand("packet-analyzer", "show pipeline", {
    func=function(params)
        paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
        paDb.config = "pipeline"
        return show_pipeline(params)
    end,
    help="Display packet analyzer configurtions for pipeline",
    params={
        {
          type="named",
            { format="field %pa_fields_for_pipeline", optional = true, name="field", help=CLI_type_dict["pa_fields_for_pipeline"].help}
        }
    }
})

--------------------------------------------
-- command registration: packet-analyzer show stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "show stage", {
    func=function(params)
        paDb.config = "stage"
        return show_stage(params)
    end,
    help="Display packet analyzer configurtions for stage",
    params={
        {
            type="values",
                { format="%pa_stage", optional = false, name="stage", help=CLI_type_dict["pa_stage"].help}
        },
        {
            type="named",
            { format="field %pa_fields_for_stage1", optional = true, name="field", help=CLI_type_dict["pa_fields_for_stage1"].help}
        }
    }
})

--------------------------------------------
-- command registration: packet-analyzer show stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "show stage all", {
    func=function(params)
        paDb.stage = "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E"
        paDb.config = "stage"
        return show_stage(params)
    end,
    help="all stages",
    params={
        {
          type="named",
            { format="field %pa_fields_for_stage_dump", optional = true, name="field", help=CLI_type_dict["pa_fields_for_stage_dump"].help}
        }
    }
})

--------------------------------------------
-- command registration: packet-analyzer show stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "show user-defined stage", {
    func=function(params)
        return packetAnalyzerShowUds(params)
    end,
    help="Display packet analyzer configurtions for user defined stages"
})

--------------------------------------------
-- command registration: packet-analyzer show stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "show user-defined stage all", {
    func=function(params)
        params["all"] = "yes"
        return packetAnalyzerShowUds(params)
    end,
    help="Display packet analyzer configurtions for user defined stages and pre defined stages"
})

--------------------------------------------
-- command registration: packet-analyzer show stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "show user-defined field", {
    func=function(params)
        return packetAnalyzerShowUdf(params)
    end,
    help="Display packet analyzer configurtions for user defined fields"
})

--------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------
-- command registration: stage match field
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "match", {
    func=packetAnalyzerSetField,
    help="Select field",
    params={
            { type = "values",
                { format="%pa_fields_for_stage", name ="field", help=CLI_type_dict["pa_fields_for_stage"].help},
                { format="%pa_field_value", name="value" , help=CLI_type_dict["pa_field_value"].help}
            },
            { type = "named",
                { format="mask %pa_field_mask", optional = true, name="mask" , help=CLI_type_dict["pa_field_mask"].help}
            },
            requirements = {value = {"field"}, mask = {"value"}},
            mandatory = {"value"}
    }
})

--------------------------------------------
-- command registration: stage no match field
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "no match", {
    func=function(params)
        params["no"] = "yes"
        return packetAnalyzerSetField(params)
    end,
    help="Unselect field",
    params={
            { type = "named",
                { format="%pa_fields_for_stage", optional = true, name ="field", help=CLI_type_dict["pa_fields_for_stage"].help}
            }
    }
})

--------------------------------------------
-- command registration: stage inverse match
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "inverse", {
    func=function(params)
        params["inverseEnable"] = true
        return packetAnalyzerSetInverse(params)
    end,
    help="Force inverse match"
})

--------------------------------------------
-- command registration: stage no inverse match
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "no inverse", {
    func=function(params)
        params["inverseEnable"] = false
        return packetAnalyzerSetInverse(params)
    end,
    help="Unset inverse match"
})

--------------------------------------------
-- command registration: stage sampling-mode
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "sampling-mode", {
    func=packetAnalyzerSetSamplingMode,
    help="Set sampling mode",
    params={
        {
            type="named",
            { format="%sampling_mode", name="samplingMode", help=CLI_type_dict["sampling_mode"].help}
        }
    }
})

--------------------------------------------
-- command registration: stage no sampling mode
--------------------------------------------
CLI_addCommand("packet-analyzer.stage", "no sampling-mode", {
    func=function(params)
        params["samplingMode"] = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E
        return packetAnalyzerSetSamplingMode(params)
    end,
    help="Unset sampling mode"
})

--------------------------------------------
-- command registration: user-defined stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "user-defined stage", {
    func=packetAnalyzerUds,
    help="add new user-defined stage",
    params={
        {   type="values",
                { format="%string", optional = false, name="stage", help="stage name"}
        },
        {   type="named",
                { format="interface %pa_interfaces_for_manager", optional = false, name="interface", help="interface ID"},
                { format="instance %pa_instances_for_manager", optional = false, name="instance", help="instance ID"},
                { format="leg %pa_indexes_for_manager", optional = false, name="leg", help="interface leg ID"},
                requirements = {["instance"] = {"interface"},["leg"] = {"instance"}},
        },
    }
})

--------------------------------------------
-- command registration: no user-defined stage
--------------------------------------------
CLI_addCommand("packet-analyzer", "no user-defined stage", {
    func=function(params)
        params["no"] = "yes"
        return packetAnalyzerUds(params)
    end,
    help="remove user-defined stage",
    params={
        {   type="values",
                { format="%pa_udss_for_manager", optional = false, name="stage", help="stage name"}
        }
    }
})

--------------------------------------------
-- command registration: user-defined field
--------------------------------------------
CLI_addCommand("packet-analyzer", "user-defined field", {
    func=packetAnalyzerUdf,
    help="add new user-defined field",
    params={
        {   type="values",
                { format="%string", optional = false, name="name", help="field name"}
        },
        {   type="named",
                { format="field-id %pa_fields_for_manager", optional = false, name="field", help="field id"},
        },
    }
})

--------------------------------------------
-- command registration: no user-defined field
--------------------------------------------
CLI_addCommand("packet-analyzer", "no user-defined field", {
    func=function(params)
        params["no"] = "yes"
        return packetAnalyzerUdf(params)
    end,
    help="remove user-defined field",
    params={
        {   type="values",
                { format="%pa_udfs_for_manager", optional = false, name="name", help="field name"}
        }
    }
})

