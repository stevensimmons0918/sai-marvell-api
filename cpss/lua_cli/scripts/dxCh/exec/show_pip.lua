--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_pip.lua
--*
--* DESCRIPTION:
--*       show global and interface configurations and counters related to PIP.
--*       Pre-Ingress Prioritization (PIP)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
    the get data functions are :

    -- for dev,port get drop counter :
    -- function returns isError , counter
    function pip_get_info_drop_port_counter(command_data, devNum, portNum)    

    -- for dev get all 4 priority drop counters :
    -- function returns isError , counterPrioVeryHigh,counterPrioHigh,counterPrioMedium,counterPrioLow
    function pip_get_info_drop_global_counter(command_data,devNum)
    
    -- for dev,port get info :
    -- function returns isError , portInfo
    function pip_get_info_port(command_data, devNum, portNum)
    
    -- for dev get info :
    -- function returns isError , devInfo
    function pip_get_info_device(command_data, devNum)
    
]]--
--includes

cmdLuaCLI_registerCfunction("wrlCpssDxChPortPipGlobalDropCounterGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChPortPipDropCounterGet")

-- function for debug purpose that allow to print to the terminal from 'show' commands
function printFromShow(...)
    local command_data = Command_Data()
    local my_table = {...}
    command_data:clearResultArray()

    for index,entry in pairs(my_table) do
        command_data["result"] =(to_string(entry))
        command_data:addResultToResultArray()
        command_data:updateEntries() 
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        command_data:clearResultArray()
    end 
end
                               --[[fit to 5 chars]]
local priorityNamesSip520Arr = { [0] = "VHigh" , [1] = "High" ,  [2] = "Medium", [3] = "Low" }
local priorityNamesSip515Arr = { [0] = "High"  , [1] = "Medium", [2] = "Low" }

local sip6GlobalCountersName = "global counter"

local NO_NAME = "no name"
--called from other file(s) too
function pip_getPriorityName(priority)
    if is_sip_5_20(devEnv.dev) then
        return priorityNamesSip520Arr[priority]
    else
        return priorityNamesSip515Arr[priority]
    end
end

--(sip6) called from other file(s) too
function pip_sip6_getPriorityName()
    return sip6GlobalCountersName
end


--##################################
--##################################
-- for dev,port get drop counter :
-- function returns isError , counter
function pip_get_info_drop_port_counter(command_data, devNum, portNum)
--[[    local apiName = "cpssDxChPortPipDropCounterGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",                  "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",   "portNum",    portNum },
        { "OUT",    "GT_U64",                 "counterPtr"}
    })
]]--    
    
    local isError
    local rc, dropCounterLo, dropCounterHi
    
    rc, dropCounterLo, dropCounterHi = wrLogWrapper("wrlCpssDxChPortPipDropCounterGet", "(devNum, portNum)", devNum, portNum)
    if rc == 0 then
      isError = false
    else
      isError = true
    end
    
    return isError, dropCounterLo, dropCounterHi
end

-- for dev get all 3(4) priority drop counters :
-- function returns isError , counterPrioHigh,counterPrioMedium,counterPrioLow
function pip_get_info_drop_global_counter(command_data,devNum)
    local isError

    if is_sip_6(devNum) then
        local rc , counter0_low ,counter0_high =  wrLogWrapper("wrlCpssDxChPortPipGlobalDropCounterGet","(devNum)",devNum)
        if rc == 0 then
            isError = false
        else
            isError = true
        end

        return    isError,  {l={[0]=counter0_low ,[1]=counter0_high}, name = pip_sip6_getPriorityName()}
                            ,{l={[0]=0            ,[1]=0            } , notValid = true          } 
                            ,{l={[0]=0            ,[1]=0            } , notValid = true          } 
                            ,{l={[0]=0            ,[1]=0            } , notValid = true          } 
    elseif is_sip_5_20(devNum) then
        local rc ,
            counter0_low ,counter0_high ,
            counter1_low ,counter1_high ,
            counter2_low ,counter2_high ,
            counter3_low ,counter3_high =  wrLogWrapper("wrlCpssDxChPortPipGlobalDropCounterGet","(devNum)",devNum)
        if rc == 0 then
            isError = false
        else
            isError = true
        end

        return    isError,  {l={[0]=counter0_low ,[1]=counter0_high}, name = pip_getPriorityName(0)} ,
                            {l={[0]=counter1_low ,[1]=counter1_high}, name = pip_getPriorityName(1)} ,
                            {l={[0]=counter2_low ,[1]=counter2_high}, name = pip_getPriorityName(2)} ,
                            {l={[0]=counter3_low ,[1]=counter3_high}, name = pip_getPriorityName(3)}
    else
        local rc ,
            counter0_low ,counter0_high ,
            counter1_low ,counter1_high ,
            counter2_low ,counter2_high  = wrLogWrapper("wrlCpssDxChPortPipGlobalDropCounterGet","(devNum)",devNum)
        if rc == 0 then
            isError = false
        else
            isError = true
        end

        return    isError,  {l={[0]=0            ,[1]=0            } , notValid = true          } ,-- emulate the 'very high'
                            {l={[0]=counter0_low ,[1]=counter0_high} , name = pip_getPriorityName(0)} ,
                            {l={[0]=counter1_low ,[1]=counter1_high} , name = pip_getPriorityName(1)} ,
                            {l={[0]=counter2_low ,[1]=counter2_high} , name = pip_getPriorityName(2)}
    end
end

-- for dev,port get info :
-- function returns isError , portInfo
--      portInfo --> {trusted,profile,vidClassificationEnable,defaultPriority,vidTpidPriority}
function pip_get_info_port(command_data, devNum, portNum)
    local portInfo = {}

    local apiName = "cpssDxChPortPipTrustEnableGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",     "GT_BOOL",    "enablePtr"}
    })
    
    portInfo.trusted = values.enablePtr
    
    if isError then
        return isError , portInfo
    end
 
    --====================
    --====================
    
    local apiName = "cpssDxChPortPipProfileGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",     "GT_U32",    "pipProfilePtr"}
    })
    
    if isError then
        return isError , portInfo
    end
    
    portInfo.profile = values.pipProfilePtr

    --====================
    --====================

    local apiName = "cpssDxChPortPipVidClassificationEnableGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",     "GT_BOOL",    "enablePtr"}
    })
    
    if isError then
        return isError , portInfo
    end
    
    portInfo.vidClassificationEnable = values.enablePtr
    
    --====================
    --====================

    local classification_type = "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E"
    local apiName = "cpssDxChPortPipPriorityGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT",    "type",    classification_type },
        { "OUT",     "GT_U32",    "priorityPtr"}
    })
    
    if isError then
        return isError , portInfo
    end
    
    portInfo.defaultPriority = values.priorityPtr

    --====================
    --====================

    local classification_type = "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E"
    local apiName = "cpssDxChPortPipPriorityGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT",    "type",    classification_type },
        { "OUT",     "GT_U32",    "priorityPtr"}
    })
    
    if isError then
        return isError , portInfo
    end

    portInfo.vidTpidPriority = values.priorityPtr

    return isError , portInfo
end    

-- for dev get info :
-- function returns isError , devInfo
--[[
    devInfo = {
        enabled = 
        vidClassification[0..3]= {etherType = , vid = }
        macDaClassification[0..3] = {macAddrValue = ,macAddrMask = , dbName = }
        udeClassification[0..3] = {etherType = , dbName = }
        thresholds[priority] = { value = }
        profiles[0..3][pip_type][0..max] = 'priority'
        
        -- addition in sip6
        drop_counter_config = { countPip = , countPbFl = ,
                                portNum = ,
                                priority_very_high = ,
                                priority_high = ,
                                priority_medium = ,
                                priority_low = ,
                                }
        pfc_tc_vector = pfc_tc_vector

        parserTpid[0..3] = {etherType = , tpidSize  = }
        
        latency_sensitive_Classification[0..3] = {etherType = , dbName = }
                                
    }
]]--
function pip_get_info_device(command_data, devNum)
    local devInfo = {}
    
    local function global__info()
        
        --====================
        --====================
        local apiName = "cpssDxChPortPipGlobalEnableGet"
        local isError , result, values = 
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "OUT",    "GT_BOOL",    "enablePtr"}
        })
        
        if isError then
            return isError , devInfo
        end
        
        devInfo.enabled = values.enablePtr
    end -- function global__info
    
    local function global__vid_classification()
        local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E"
        
        devInfo.vidClassification = {}
    
        for index = 0,3 do
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalVidClassificationGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_U32",    "index",    index },
                { "OUT",    "GT_U16",    "vidPtr"}
            })
            
            if isError then
                return isError
            end
            
            devInfo.vidClassification[index] = {}
            devInfo.vidClassification[index].vid = values.vidPtr
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",    protocol },
                { "IN",     "GT_U32",    "index",    index },
                { "OUT",    "GT_U16",    "etherTypePtr"}
            })
            
            if isError then
                return isError
            end

            devInfo.vidClassification[index].etherType = values.etherTypePtr
        end
        
        if is_sip_6(devNum) then 
            devInfo.parserTpid = {}

            for index = 0,3 do
                local apiName = "cpssDxChPortParserGlobalTpidGet"
                local isError , result, values = 
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",     "devNum",     devNum },
                    { "IN",     "GT_U32",    "index",       index },
                    { "OUT",     "GT_U16",   "etherTypePtr"       },
                    { "OUT",     "CPSS_BRG_TPID_SIZE_TYPE_ENT",    "tpidSizePtr"}
                })
                if isError then
                    return isError
                end

                devInfo.parserTpid[index] = {}
                devInfo.parserTpid[index].etherType = values.etherTypePtr
                devInfo.parserTpid[index].tpidSize  = values.tpidSizePtr
            end
        end
        
    end -- function global__vid_classification
    
    
    local function global__mac_da_classification()

        devInfo.macDaClassification = {}

        for index = 0,3 do
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalMacDaClassificationEntryGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_U32",    "index",    index },
                { "OUT",    "CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC",    "entryPtr"}
            })
            
            if isError then
                return isError
            end
            
            devInfo.macDaClassification[index] = {macAddrValue = values.entryPtr.macAddrValue,
                                                  macAddrMask = values.entryPtr.macAddrMask}
            
            local db_name = pip_db_mac_da_name_by_index_get(index)
            if db_name == nil then
                db_name = NO_NAME
            end
            
            devInfo.macDaClassification[index].dbName = db_name

            end
    end -- function global__mac_da_classification
    
    local function global__ude_classification()
        local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E"

        devInfo.udeClassification = {}

        for index = 0,3 do
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",    protocol },
                { "IN",     "GT_U32",    "index",    index },
                { "OUT",    "GT_U16",    "etherTypePtr"}
            })
            
            if isError then
                return isError
            end
            
            devInfo.udeClassification[index] = {}
            devInfo.udeClassification[index].etherType = values.etherTypePtr

            local db_name = pip_db_ude_name_by_index_get(index)
            
            if db_name == nil then
                db_name = NO_NAME
            end
            
            devInfo.udeClassification[index].dbName = db_name

            
        end
    end -- function global__ude_classification

    local function global__latency_sensitive_classification()
        local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E"

        devInfo.latency_sensitive_Classification = {}

        for index = 0,3 do
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",    protocol },
                { "IN",     "GT_U32",    "index",    index },
                { "OUT",    "GT_U16",    "etherTypePtr"}
            })
            
            if isError then
                return isError
            end
            
            devInfo.latency_sensitive_Classification[index] = {}
            devInfo.latency_sensitive_Classification[index].etherType = values.etherTypePtr

            local db_name = pip_db_latency_sensitive_name_by_index_get(index)
            
            if db_name == nil then
                db_name = NO_NAME
            end
            
            devInfo.latency_sensitive_Classification[index].dbName = db_name

            
        end
    end -- function global__latency_sensitive_classification
    
    local function global__thresholds()
        devInfo.thresholds = {}
        local maxPriority
        if is_sip_5_20(devEnv.dev) then
            maxPriority = 3
        else
            maxPriority = 2
        end

        for priority = 0,maxPriority do
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalBurstFifoThresholdsGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_DATA_PATH_BMP", "dataPathBmp",  0xFFFFFFFF },
                { "IN",     "GT_U32",    "priority",    priority },
                { "OUT",    "GT_U32",    "thresholdPtr"}
            })
            
            if isError then
                return isError
            end
            
            devInfo.thresholds[priority] = {}
            devInfo.thresholds[priority].value = values.thresholdPtr
            
        end

        if is_sip_6(devEnv.dev) then
            devInfo.thresholds_by_name = {}

            local thresholdType = {
                 [1] = {name = "latency sensitive"  ,id = "CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_LATENCY_SENSITIVE_E"}
                ,[2] = {name = "pfc fill level low" ,id = "CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_LOW_E"}
                ,[3] = {name = "pfc fill level high" ,id = "CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_HIGH_E"}
                ,[4] = {name = "pfc counter traffic" , id = "CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E"}
            }
            
            for index = 1,4 do
                local apiName = "cpssDxChPortPipGlobalThresholdGet"
                local isError , result, values = 
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "GT_PORT_GROUPS_BMP", "portGroupsBmp",  0xFFFFFFFF },
                    { "IN",     "CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT",    "thresholdType",    thresholdType[index].id },
                    { "OUT",    "GT_U32",    "thresholdPtr"}
                })
            
                if isError then
                    return isError
                end

                devInfo.thresholds_by_name[index] = {name = thresholdType[index].name , value = values.thresholdPtr}
            end

        end
        
        
    end -- function global__thresholds
    
    -- profile_info {pipProfile,max,field}
    local function global__single_profile(profile_info)
        --local profile_info = params.profile_info
    
        local pipProfile = profile_info.pipProfile
        local max_value = profile_info.max_value
        local pip_type = profile_info.field
        local ipv6_erratum
        
        if pip_is_device_ipv6_erratum(devNum) and pip_type == IPV6_TC then
            ipv6_erratum = true
        else
            ipv6_erratum = false
        end

        --printFromShow("profile_info" ,profile_info)
        
        local perProfilePerTypeSTC = devInfo.profiles[pipProfile][pip_type]
        
        for fieldIndex = 0,max_value do
            local cpssFieldIndex = fieldIndex
            
            if ipv6_erratum then
                cpssFieldIndex = fieldIndex % (pip_type_max_value_ipv6_erratum + 1)
            end
            --====================
            --====================
            local apiName = "cpssDxChPortPipGlobalProfilePriorityGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_U32",    "pipProfile",    pipProfile },
                { "IN",     "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT",    "type",    pip_type },
                { "IN",     "GT_U32",    "fieldIndex",    cpssFieldIndex },
                { "OUT",    "GT_U32",    "priorityPtr"}
            })
            
            if isError then
                return isError
            end
            
            -- devInfo.profiles[profile_info.pipProfile][pip_type][fieldIndex] = priority
            perProfilePerTypeSTC[fieldIndex] = values.priorityPtr
            --printFromShow("priority" ,values.priorityPtr) ; 
        end
    end -- function global__single_profile
    
    local function global__all_profiles()
        devInfo.profiles = {}

        local profile_info = {}
        for index = 0 , 3 do
            profile_info.pipProfile = index
            devInfo.profiles[index] = {}
            local per_profile = devInfo.profiles[index]
            
            for pip_type,max_value in pairs(pip_type_max_value) do
                profile_info.field = pip_type
                
                --max_value = 3
                
                profile_info.max_value = max_value
                
                per_profile[pip_type] = {}
                
                global__single_profile(profile_info)
            end
        end
    end -- function global__all_profiles

    local global__sip6_drop_counter_mode
    local global__sip6_PfcTcVector
    if(is_sip_6(devNum)) then
        -- addition in sip6
        local drop_counter_config = { countPip = false, countPbFl = false,
                                    portNum = nil,
                                    priority_very_high = false,
                                    priority_high = false,
                                    priority_medium = false,
                                    priority_low = false
                                }

        local function internal_sip6_drop_counter_mode()
            local apiName = "cpssDxChPortRxDmaGlobalDropCounterModeGet"
            local isError , result, values = 
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "OUT",     "CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC",    "counterModeInfoPtr"}
            })
            if isError then
                return isError
            end
            
            local counterModeInfo = values.counterModeInfoPtr
            local value
            
            value = bit_and(counterModeInfo.modesBmp , 0x1)
            if value ~= 0 then
                -- CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E
                drop_counter_config.countPip  = true
            end

            value = bit_and(counterModeInfo.modesBmp , 0x2)
            
            if value ~= 0 then
                -- CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E
                drop_counter_config.countPbFl = true
            end
            
            if counterModeInfo.portNumMask == 0xFFFFFFFF then
                drop_counter_config.portNum = counterModeInfo.portNumPattern
            else
                drop_counter_config.portNum = "all"
            end
            
            if counterModeInfo.priorityMask == 0 then
                drop_counter_config.priority_very_high = true
                drop_counter_config.priority_high = true
                drop_counter_config.priority_medium = true
                drop_counter_config.priority_low = true
            else
                value = bit_and(counterModeInfo.priorityMask , 0x1)
                drop_counter_config.priority_very_high = true
                drop_counter_config.priority_high = true
                drop_counter_config.priority_medium = true
                drop_counter_config.priority_low = true
                
                if value ~= 0 then
                    -- bit 0 forced by mask
                    value = bit_and(counterModeInfo.priorityPattern , 0x1)
                    if value ~= 0 then
                        -- pattern value 1 (medium or very-high)
                        drop_counter_config.priority_high = false
                        drop_counter_config.priority_low = false
                    else
                        -- pattern value 0 (low    or high)
                        drop_counter_config.priority_very_high = false
                        drop_counter_config.priority_medium = false
                    end
                end
                
                value = bit_and(counterModeInfo.priorityMask , 0x2)
                
                if value ~= 0 then
                    -- bit 1 forced by mask
                    value = bit_and(counterModeInfo.priorityPattern , 0x2)
                    if value ~= 0 then
                        -- pattern value 1 (high or very-high)
                        drop_counter_config.priority_medium = false
                        drop_counter_config.priority_low = false
                    else
                        -- pattern value 0 (low or medium)
                        drop_counter_config.priority_very_high = false
                        drop_counter_config.priority_high = false
                    end
                end
                
            end
            
            devInfo.drop_counter_config = drop_counter_config
        end-- of function
        
        global__sip6_drop_counter_mode = internal_sip6_drop_counter_mode
        
        local pfc_tc_vector = { 
             [1] = {name = "priority low"        , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_LOW_E"}
            ,[2] = {name = "priority medium"     , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_MEDIUM_E"}
            ,[3] = {name = "priority high"       , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_HIGH_E"}
            ,[4] = {name = "priority very high"  , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_VERY_HIGH_E"}
            ,[5] = {name = "no congestion"       , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_NO_CONGESTION_E"}
            ,[6] = {name = "heavy congestion"    , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E"}
        }
        local function internal_sip6_PfcTcVector()

            for index = 1,6 do
                local entry = pfc_tc_vector[index]
                local apiName = "cpssDxChPortPipGlobalPfcTcVectorGet"
                local isError , result, values = 
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT",    "vectorType" , entry.id},
                    { "OUT",     "GT_U32",    "tcVectorPtr"}
                })
                if isError then
                    return isError
                end
                -- save the info to the DB
                entry.value = values.tcVectorPtr
                
            end -- end of loop on index
        
            devInfo.pfc_tc_vector = pfc_tc_vector
            
        end -- end of local function
        global__sip6_PfcTcVector = internal_sip6_PfcTcVector
        
    end
    
    -- build all the info
    local isError = global__info()
    if isError then return isError , devInfo end

    local isError = global__vid_classification()
    if isError then return isError , devInfo end

    local isError = global__mac_da_classification()
    if isError then return isError , devInfo end

    local isError = global__ude_classification()
    if isError then return isError , devInfo end

    if is_sip_6(devNum) then 
        local isError = global__latency_sensitive_classification()
        if isError then return isError , devInfo end
    end

    local isError = global__thresholds()
    if isError then return isError , devInfo end

    local isError = global__all_profiles()
    if isError then return isError , devInfo end
    
    if global__sip6_drop_counter_mode then
        local isError = global__sip6_drop_counter_mode()
        if isError then return isError , devInfo end
    end
    
    if global__sip6_PfcTcVector then
        local isError = global__sip6_PfcTcVector()
        if isError then return isError , devInfo end
    end

    return isError , devInfo
end

--##################################
--##################################
local function per_port__show_pip_drop_port(command_data, devNum, portNum, params)
    if(is_sip_6(devNum)) then
        -- sip6 not supports 'per port' counter
        -- the show will 'show' no entries
        return
    end

    local isError , cpssCounterLo, cpssCounterHi = pip_get_info_drop_port_counter(command_data, devNum, portNum)
    if isError then
        return
    end
 
    local counter
    if(cpssCounterLo > 0) then
        counter = {low = string.format("0x%8.8x",cpssCounterLo)}
    else
        counter = {low = "0"}
    end
        
    -- Resulting string formatting and adding.
    local devPortStr=tostring(devNum).."/"..tostring(portNum)
    command_data["result"] =
        string.format("%-11s%-18s", devPortStr, counter.low)

    command_data:addResultToResultArray()
end


local function show_pip_drop_port(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        "Dev/Port   pip-drop-counter        \n" ..
        "---------  ----------------------  \n"
    footer_string = "\n"
    
    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_pip_drop_port,params,pip_supported_interfaces)
end

--constants


--[[
  all_interfaces="all",
  all="all",
  devID=0
  
  ["port-channel"]=1,
  all_interfaces="port-channel"  
  
  all_interfaces="ethernet",
  ethernet={
    [0]={ 2, 3, 4, 5 }  
  
]]--

CLI_addHelp("exec", "show pip", "show pip related configurations and drop counters")


--[[
Command to show the pip-drop counters of the ports.

Context – exec (no context)

Format: 
Show pip drop-port <all/Ethernet/port-channel >

Parameters:
  all                  Apply to all ports of specific device or all available devices
  ethernet             Ethernet interface to configure
  port-channel         Valid Port-Channel interface

]]--
local help_string_show_pip_drop_port = "show the pip-drop counters of the ports"
CLI_addHelp("exec", "show pip drop-port", help_string_show_pip_drop_port)
CLI_addCommand("exec", "show pip drop-port", {
  func   = show_pip_drop_port,
  help   = help_string_show_pip_drop_port,
  params = {
      { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
      }
  },
  preFunc = currentSystemPortType_RXDMA
})
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

local function global_counter_get(command_data,devNum)
    local isError
    local maxPriority
    local cpssCounters
    local isError,cpssCounter1,cpssCounter2,cpssCounter3,cpssCounter4 = pip_get_info_drop_global_counter(command_data,devNum)
    if isError then
        return
    end

    local counter = {}
    
    if(is_sip_6(devNum)) then
        -- Resulting string formatting and adding.
        local prefix=tostring(devNum).."/".."global counter"
        -- single counter 
        counter.low  = string.format(  "%8.8x",cpssCounter1.l[0])
        counter.high = string.format("0x%8.8x",cpssCounter1.l[1])
        
        command_data["result"] =
            string.format("%-16s  %-16s %s", prefix, counter.high , counter.low)

        command_data:addResultToResultArray()
        command_data:updateEntries() 

        return
    end
    
    cpssCounters = {[0] = cpssCounter1,[1]=cpssCounter2,[2]=cpssCounter3, [3]=cpssCounter4}
    maxPriority = 3

    local priority
    
    for priority = 0,maxPriority do
        if not cpssCounters[priority].notValid then
            counter.low  = string.format(  "%8.8x",cpssCounters[priority].l[0])
            counter.high = string.format("0x%8.8x",cpssCounters[priority].l[1])
            
            -- Resulting string formatting and adding.
            local prefix=tostring(devNum).."/"..cpssCounters[priority].name
            
            command_data["result"] =
                string.format("%-16s%-18s %s", prefix, counter.high , counter.low)

            command_data:addResultToResultArray()
            command_data:updateEntries() 
        end
    end

end

local function per_device__show_pip_drop_global(command_data,devNum,params)
    global_counter_get(command_data,devNum)
end

local function show_pip_drop_global(params)
    params.header_string =
        "\n" ..
        "Dev/Priority    pip-drop-counter (0xHigh Low) \n" ..
        "--------------  ----------------------------- \n"
    params.footer_string = "\n"

    return generic_all_device_show_func(per_device__show_pip_drop_global,params)
end
--[[
Command to show the global pip-drop counters (all pip-priorities).

Context – exec (no context)

Format: 
Show pip drop-global

Parameters:
None.
]]--
local help_string_show_pip_drop_global = "show the global pip-drop counters (all pip-priorities)"
CLI_addHelp("exec", "show pip drop-global", help_string_show_pip_drop_global)

CLI_addCommand("exec", "show pip drop-global", {
  func   = show_pip_drop_global,
  help   = help_string_show_pip_drop_global,
  params = {
      { type = "named",
          "#all_device",-- specific device or on all devices
      }
  }
})

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
local function per_port__show_pip_info_port(command_data, devNum, portNum, params)

    -- portInfo --> {trusted,profile,vidClassificationEnable,defaultPriority,vidTpidPriority}
    local isError , portInfo = pip_get_info_port(command_data, devNum, portNum)

    if isError then
        return
    end

    local trust
    if(portInfo.trusted == true ) then
        trust = "Enabled"
    else
        trust = "Disabled"
    end

    local profile = portInfo.profile
    
    local VidClassificationEnable
    if(portInfo.vidClassificationEnable == true ) then
        VidClassificationEnable = "Enabled"
    else
        VidClassificationEnable = "Disabled"
    end
    
    local default_priority = portInfo.defaultPriority
    local vid_tpid_priority = portInfo.vidTpidPriority
    
    --====================
    --====================
    
    -- Resulting string formatting and adding.
    local devPortStr=tostring(devNum).."/"..tostring(portNum)
    command_data["result"] =
        string.format("%-10s" .. --devPortStr
                      "%-12s" .. --trust
                      "%-5s" .. --profile
                      "%-16s" .. --default_priority
                      "%-15s" .. --VidClassificationEnable
                      "%-8s",  --vid_tpid_priority
                devPortStr, 
                trust,
                tostring(profile),
                pip_getPriorityName(default_priority),
                VidClassificationEnable,
                pip_getPriorityName(vid_tpid_priority)
                )

    command_data:addResultToResultArray()
 
end


local function show_pip_info_port(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        "Dev/Port  trust  profile default-priority  Vid+TPID  Vid+TPID priority  \n" ..
        "--------- ------ ------- ---------------- ---------- -----------------  \n"
    footer_string = "\n"
    
    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_pip_info_port,params,pip_supported_interfaces)
end

--[[
Command to show the pip-info of the ports.

Context – exec (no context)

Format: 
Show pip info-port <all/Ethernet/port-channel >

Parameters:
  all                  Apply to all ports of specific device or all available devices
  ethernet             Ethernet interface to configure
  port-channel         Valid Port-Channel interface
]]--
local help_string_show_pip_info_port = "show the pip-info of the ports"
CLI_addHelp("exec", "show pip info-port", help_string_show_pip_info_port)
CLI_addCommand("exec", "show pip info-port", {
  func   = show_pip_info_port,
  help   = help_string_show_pip_info_port,
  params = {
      { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
      }
  },
  preFunc = currentSystemPortType_RXDMA
})
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
local function per_device__show_pip_info_global__enabled(command_data,devNum,params)
    local enable
    if(params.devInfo.enabled == true ) then
        enable = "Enabled"
    else
        enable = "Disabled"
    end
    
    local devPortStr=tostring(devNum)
    
    command_data["result"] =
        string.format("%-13s".."%-8s",
                    devPortStr, enable)

    command_data:addResultToResultArray()
    command_data:updateEntries() 
    
end

local function per_device__show_pip_info_global__vid_classification(command_data,devNum,params)
    for index = 0,3 do
        local vid = string.format("0x%3.3x",params.devInfo.vidClassification[index].vid)
        local etherType = string.format("0x%4.4x",params.devInfo.vidClassification[index].etherType)

        --====================
        --====================
        local devPortStr=tostring(devNum).."/"..tostring(index)
        
        command_data["result"] =
            string.format("%-13s".."%-8s".."%-10s", 
                        devPortStr, vid , etherType)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end
-- sip6 support
local function per_device__show_pip_info_global__parserTpid(command_data,devNum,params)
    for index = 0,3 do
        local etherType = string.format("0x%4.4x",params.devInfo.parserTpid[index].etherType)
        local tpidSize  = params.devInfo.parserTpid[index].tpidSize-- already string

        --====================
        --====================
        local devPortStr=tostring(devNum).."/"..tostring(index)
        
        command_data["result"] =
            string.format("%-13s".."%-10s".."%-8s", 
                        devPortStr, etherType , tpidSize)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end

local function per_device__show_pip_info_global__mac_da_classification(command_data,devNum,params)
    for index = 0,3 do
        local mac  = params.devInfo.macDaClassification[index].macAddrValue
        local mask = params.devInfo.macDaClassification[index].macAddrMask
        local db_name = params.devInfo.macDaClassification[index].dbName
        
        local devPortStr=tostring(devNum).."/"..tostring(index)
        
        command_data["result"] =
            string.format("%-13s".."%-20s".."%-20s".."%-25s", 
                        devPortStr, mac , mask , db_name)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end

local function per_device__show_pip_info_global__ude_classification(command_data,devNum,params)
    for index = 0,3 do
        local etherType = string.format("0x%4.4x",params.devInfo.udeClassification[index].etherType)

        local db_name = params.devInfo.udeClassification[index].dbName

        local devPortStr=tostring(devNum).."/"..tostring(index)
        
        command_data["result"] =
            string.format("%-13s".."%-13s".."%-10s", 
                        devPortStr, etherType , db_name)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end
--sip6 function
local function per_device__show_pip_info_global__latency_sensitive_classification(command_data,devNum,params)
    for index = 0,3 do
        local etherType = string.format("0x%4.4x",params.devInfo.latency_sensitive_Classification[index].etherType)

        local db_name = params.devInfo.latency_sensitive_Classification[index].dbName

        local devPortStr=tostring(devNum).."/"..tostring(index)
        
        command_data["result"] =
            string.format("%-13s".."%-13s".."%-10s", 
                        devPortStr, etherType , db_name)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end

local function per_device__show_pip_info_global__thresholds(command_data,devNum,params)
    local maxPriority
    if is_sip_5_20(devEnv.dev) then
        maxPriority = 3
    else
        maxPriority = 2
    end
    for priority = 0,maxPriority do
        --====================
        --====================
        local threshold = tostring(params.devInfo.thresholds[priority].value)

        local prefix=tostring(devNum).."/"..pip_getPriorityName(priority)
        
        command_data["result"] =
            string.format("%-15s".."%-20s", 
                        prefix, threshold)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end
local function per_device__show_pip_info_global__thresholds_by_name(command_data,devNum,params)
    --sip6 filled this info 
    for index = 1,4 do
        local entry = params.devInfo.thresholds_by_name[index]
        local threshold = tostring(entry.value)
        local prefix=tostring(devNum).."/"..entry.name

        command_data["result"] =
            string.format("%-23s".."%-20s", 
                        prefix, threshold)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end

local function per_device__show_pip_info_global__per_profile_sub_value(command_data,devNum,params)
    local profile_info = params.profile_info
    
    local pipProfile = profile_info.pipProfile
    local max_value = profile_info.max
    local pip_type = profile_info.field
    
    -- we print up to 10 priorities per entry
    local max_per_entry = 10
    local firstLineLength
    
    if max_per_entry < (max_value+1) then
        firstLineLength = max_per_entry
    else
        firstLineLength = (max_value+1)
    end

    if pipProfile == 0 or (max_value+1) >= max_per_entry then
        command_data["result"] = "              "
        for ii = 0,(firstLineLength-1) do
            command_data["result"] = command_data["result"] .. ii .. "     "
        end 
    end
    
    command_data:addResultToResultArray()
    command_data:updateEntries() 

    
    local prefix=tostring(devNum).."/"..pipProfile
    local prefix_part_1,prefix_part_2

    --command_data:addResultToResultArray()
    --command_data:updateEntries() 
    command_data["result"] = ""

    local ipv6_erratum
    
    if pip_is_device_ipv6_erratum(devNum) and pip_type == IPV6_TC then
        ipv6_erratum = true
    else
        ipv6_erratum = false
    end

    --max_value = 3
    
    for fieldIndex = 0,max_value do
        local perProfilePerTypeSTC = params.devInfo.profiles[pipProfile][pip_type]
        local cpssPriority = perProfilePerTypeSTC[fieldIndex]
        
        local priority = string.format("%-5s",pip_getPriorityName(cpssPriority))
        
        command_data["result"] = command_data["result"] .. " " .. priority
        
        local needNewLine  = ((fieldIndex % max_per_entry) == (max_per_entry - 1))
        local lastLine = (fieldIndex == max_value)

        if needNewLine or lastLine then
            if lastLine and fieldIndex > max_per_entry then
                prefix_part_2 = tostring(max_value)..")"
                prefix= prefix_part_1 .. prefix_part_2
            end
        
            command_data["result"] = 
                string.format("%-13s", prefix) .. command_data["result"]

            command_data:addResultToResultArray()
            command_data:updateEntries() 

            command_data["result"] = ""
            
            prefix_part_1 = "  " .. "(".. tostring(fieldIndex+1) .. ".."
            prefix_part_2 = tostring(fieldIndex+max_per_entry)..")"
            
            prefix= prefix_part_1 .. prefix_part_2
        end
    end
end

local function per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
    for index = 0 , 3 do
        params.profile_info.pipProfile = index
        
        per_device__show_pip_info_global__per_profile_sub_value(command_data,devNum,params)
    end
end

local function per_device__show_pip_info_global__profile_mpls(command_data,devNum,params)
    params.profile_info = { max = 7 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_ipv4(command_data,devNum,params)
    params.profile_info = { max = 255 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_ipv6(command_data,devNum,params)
    params.profile_info = { max = 255 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E"}

    if pip_is_device_ipv6_erratum(devNum) then
        command_data["result"] = "IMPORTANT NOTE : " .. pip_ipv6_erratum_string .. 
                " So we get the same priority for TC 0,64,128,192 (and TC 1,65,129,193 ...)"

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_vlan_tag(command_data,devNum,params)
    params.profile_info = { max = 7 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_dsa_qos(command_data,devNum,params)
    params.profile_info = { max = 127 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_dsa_up(command_data,devNum,params)
    params.profile_info = { max = 7 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_ude(command_data,devNum,params)
    params.profile_info = { max = 3 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__profile_mac_da(command_data,devNum,params)
    params.profile_info = { max = 3 , field = "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E"}
    
    per_device__show_pip_info_global__profile_sub_value(command_data,devNum,params)
end

local function per_device__show_pip_info_global__drop_counter_mode(command_data,devNum,params)
    local drop_counter_config = params.devInfo.drop_counter_config 

    local str_countPip           = tostring(drop_counter_config.countPip)
    local str_countPbFl          = tostring(drop_counter_config.countPbFl)
    local str_portNum            = tostring(drop_counter_config.portNum)
    local str_priority_very_high = tostring(drop_counter_config.priority_very_high)
    local str_priority_high      = tostring(drop_counter_config.priority_high)
    local str_priority_medium    = tostring(drop_counter_config.priority_medium)
    local str_priority_low       = tostring(drop_counter_config.priority_low)
    
    local devPortStr=tostring(devNum)
    
    command_data["result"] =
        string.format("%-13s"   --devPortStr
                      .."%-6s"  --str_countPip 
                      .."%-7s"  -- str_countPbFl
                      .."%-6s"  -- str_portNum
                      .."%-9s"  -- str_priority_very_high
                      .."%-7s"  -- str_priority_high
                      .."%-9s"  -- str_priority_medium
                      .."%-7s"  -- str_priority_low
                      ,
                      devPortStr, str_countPip , str_countPbFl , str_portNum , str_priority_very_high , str_priority_high , str_priority_medium , str_priority_low)

    command_data:addResultToResultArray()
    command_data:updateEntries() 
end

local function per_device__show_pip_info_global__pfc_tc_vector(command_data,devNum,params)

    for index = 1,6 do
        local entry = params.devInfo.pfc_tc_vector[index]
        local value = tostring(entry.value)
        local prefix=tostring(devNum).."/"..entry.name

        command_data["result"] =
            string.format("%-23s".."0x%-20x", 
                        prefix, value)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end

end


local function show_pip_info_global(params)
    local command_data = Command_Data()
    local delemiter = "*-*-*-*-*-*-*-*-*-*"

    params.command_data = command_data
    params.bypass_final_return = true
    params.disablePausedPrinting = true

    params.footer_string = "\n"..delemiter
    
    local function for_show___pip_get_info_device(command_data,devNum,params)
        local isError,devInfo = pip_get_info_device(command_data,devNum)
        if isError then
            return
        end
        
        -- use params to deliver the info between the functions
        params.devInfo = devInfo
        
        --printFromShow(params.devInfo)
    end
    
    -- get the raw data
    generic_all_device_func(for_show___pip_get_info_device,params)
    
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for mpls-exp 0..7 \n" ..
        "------------  -------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_mpls,params)

    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for ipv4-tos 0..255 \n" ..
        "------------  ---------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_ipv4,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for ipv6-tc 0..255 \n" ..
        "------------  --------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_ipv6,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for vlan-tag-up 0..7 \n" ..
        "------------  ----------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_vlan_tag,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for dsa-qos-profile 0..127 \n" ..
        "------------  ----------------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_dsa_qos,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for dsa-up 0..7 \n" ..
        "------------  ------------------------ \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_dsa_up,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for user-defined-etherType 0..3 \n" ..
        "------------  ---------------------------------------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_ude,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "profiles info: \n" ..
        "\n" ..
        "Dev/profile   priority for mac-da 0..3 \n" ..
        "------------  ------------------------ \n"

    generic_all_device_show_func(per_device__show_pip_info_global__profile_mac_da,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "vlan classification info: \n" ..
        "Dev/index    vid    etherType \n" ..
        "-----------  -----  --------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__vid_classification,params)

    --====================
    --====================
    if params.devInfo.parserTpid then -- sip6
        params.header_string =
            "\n" ..
            "Parser TPID classification info: \n" ..
            "Dev/index    etherType  length \n" ..
            "-----------  ---------- ------ \n"

        generic_all_device_show_func(per_device__show_pip_info_global__parserTpid,params)
    end

    --====================
    --====================
    params.header_string =
        "\n" ..
        "mac-da classification info: \n" ..
        "Dev/index       mac-da                mask            'name'   \n" ..
        "-----------  -----------------  ------------------  ---------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__mac_da_classification,params)
    --====================
    --====================
    params.header_string =
        "\n" ..
        "user-defined-etherTypes classification info: \n" ..
        "Dev/index    etherType      'name'   \n" ..
        "-----------  -----------  ---------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__ude_classification,params)

    --====================
    --====================
    if is_sip_6() then
        params.header_string =
            "\n" ..
            "latency-sensitive-etherTypes classification info: \n" ..
            "Dev/index    etherType      'name'   \n" ..
            "-----------  -----------  ---------- \n"

        generic_all_device_show_func(per_device__show_pip_info_global__latency_sensitive_classification,params)
    end
    --====================
    --====================
    params.header_string =
        "\n" ..
        "thresholds (number of packets) info: \n" ..
        "Dev/priority  threshold   \n" ..
        "------------  ----------- \n"

    generic_all_device_show_func(per_device__show_pip_info_global__thresholds,params)
    --====================
    --====================
    if params.devInfo.thresholds_by_name then
        --sip6 filled this info 
        params.header_string =
            "\n" ..
            "thresholds (number of packets) info: \n" ..
            "Dev/type                threshold   \n" ..
            "----------------------  ----------- \n"

        generic_all_device_show_func(per_device__show_pip_info_global__thresholds_by_name,params)
    end
    --====================
    --====================
    params.header_string = 
        "\n" ..
        "Is pip enabled ? \n" ..
        "Dev          Enabled?  \n" ..
        "-----------  --------  \n"

    generic_all_device_show_func(per_device__show_pip_info_global__enabled,params)
    --====================
    --====================
    if is_sip_6() then
        params.header_string = 
            "\n" ..
            "RxDma drop counter modes \n" ..
            "PB-FL - packet buffer fill level \n" ..
            "Dev          pip   PB-FL  port  p_v_high p_high p_medium p_low \n" ..
            "-----------  ------------------------------------------------- \n"

        generic_all_device_show_func(per_device__show_pip_info_global__drop_counter_mode,params)
        
        --sip6 filled this info 
        params.header_string =
            "\n" ..
            "PFC vectors (bitmap of TC - traffic classes) info: \n" ..
            "Dev/type               TC vector   \n" ..
            "---------------        ----------- \n"
        generic_all_device_show_func(per_device__show_pip_info_global__pfc_tc_vector,params)
        
    end
    --====================
    --====================
    
    return command_data:getCommandExecutionResults()
end

--[[
Command to show the global pip-info.

Context – exec (no context)

Format: 
Show pip info-global

Parameters:
None.
]]--
local help_string_show_pip_info_global = "show the global pip-info"
CLI_addHelp("exec", "show pip info-global", help_string_show_pip_info_global)

CLI_addCommand("exec", "show pip info-global", {
  func   = show_pip_info_global,
  help   = help_string_show_pip_info_global,
  params = {
      { type = "named",
          "#all_device",-- specific device or on all devices
      }
  }
})
