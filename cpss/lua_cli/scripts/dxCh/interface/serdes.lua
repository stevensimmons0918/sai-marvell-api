--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes.lua
--*
--* DESCRIPTION:
--*     Configure specific parameters of serdes
--*
--*
--*
--*
--********************************************************************************

--includes


--constants

local OR, XOR, AND = 1, 3, 4
local function bitoper(a, b, oper)
   local r, m, s = 0, 2^52
   repeat
      s,a,b = a+b+m, a%m, b%m
      r,m = r + m*oper%(s-a-b), m/2
   until m < 1
   return r
end

--------------------------------------------
-- type registration: calibration_mode
--------------------------------------------
CLI_type_dict["calibration_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "port calibration mode",
    enum = {
        ["copper"] = { value="CPSS_PORT_MANAGER_DEFAULT_CALIBRATION_TYPE_E", help="default calibration -DAC,copper"},
        ["optical"]   = { value="CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E", help="optical calibration"         }
    }
}

CLI_type_dict["min_threshold"] = {
    checker = CLI_check_param_number,
    help="<>    min threshold value"
}

CLI_type_dict["max_threshold"] = {
    checker = CLI_check_param_number,
    help="<>    max threshold value"
}

CLI_type_dict["rx_tune"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="Run rx training",
    enum = {
        ["adaptive"] = { value="CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E", help="run adaptive rx-training"},
        ["edge-detect"]   = { value="CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E", help="enable edge-detect and training"}
    }
}

-- ************************************************************************
---
--  serdes_calibration_func
--        @description  configure calibration parameters of serdes
--
--
--
--        @return       error message if fails
--

local function serdes_calibration_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local calibrationType, minEoThreshold, maxEoThreshold
    local bitMapEnable = 0

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    calibrationType= params.calibration_mode

    if params.flagNo == true then
        bitMapEnable = 0
    else
        if params.eye_opening~=nil then
            minEoThreshold = params.eye_opening[1]
            bitMapEnable = bitoper(bitMapEnable, 0x10, OR)
            maxEoThreshold = params.eye_opening[2]
            bitMapEnable = bitoper(bitMapEnable, 0x20, OR)
        end
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        result, values = myGenWrapper("cpssDxChSamplePortManagerOpticalCalibrationSet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"IN","CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT", "calibrationType", calibrationType},
                  {"IN","GT_U32", "minEoThreshold", minEoThreshold},
                  {"IN","GT_U32", "maxEoThreshold", maxEoThreshold},
                  {"IN","GT_U32", "bitMapEnable", bitMapEnable}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command: cpssDxChSamplePortManagerOpticalCalibrationSet :%s", result)
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function serdes_etl_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local minLF, maxLF

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    if params.etl_val~=nil then
        minLF = params.etl_val[1]
        maxLF = params.etl_val[2]
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        result, values = myGenWrapper("cpssDxChSamplePortManagerEnhanceTuneOverrideSet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"IN","GT_U8", "minLf", minLF},
                  {"IN","GT_U8", "maxLf", maxLF}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command: cpssDxChSamplePortManagerEnhanceTuneOverrideSet :%s", result)
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function serdes_precode_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local portExtraOperation

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    if params.flagNo == true then
        portExtraOperation = 0
    else
        portExtraOperation = 1
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        result, values = myGenWrapper("cpssDxChSamplePortManagerAttrExtOperationParamsSet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"IN","GT_U32", "portExtraOperation", portExtraOperation}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command: cpssDxChSamplePortManagerAttrExtOperationParamsSet :%s", result)
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function serdes_tune_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local trainModeType, adaptRxTrainSupp, edgeDetectSupported

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        if params.flagNo == true then
            if params.auto_tune_rx == nil then
                trainModeType = "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E"
                adaptRxTrainSupp = GT_FALSE
                edgeDetectSupported = GT_FALSE
            elseif params.auto_tune_rx == "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E" then
                trainModeType = "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E"
                adaptRxTrainSupp = GT_FALSE

                result, values = myGenWrapper("cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet",{
                                             {"IN", "GT_U8","devNum",devNum},
                                             {"IN", "GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                             {"IN", "GT_BOOL", "adaptRxTrainSupp", adaptRxTrainSupp}})
            else
                edgeDetectSupported = GT_FALSE
                result, values = myGenWrapper("cpssDxChSamplePortManagerEdgeDetectSuppSet",{
                                             {"IN", "GT_U8","devNum",devNum},
                                             {"IN", "GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                             {"IN", "GT_BOOL", "edgeDetectSupported", edgeDetectSupported}})
            end
        else
            if params.auto_tune_rx ~= nil then
                trainModeType = params.auto_tune_rx
                if params.auto_tune_rx == "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E" then
                    adaptRxTrainSupp = GT_TRUE
                    result, values = myGenWrapper("cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet",{
                                                 {"IN", "GT_U8","devNum",devNum},
                                                 {"IN", "GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                                 {"IN", "GT_BOOL", "adaptRxTrainSupp", adaptRxTrainSupp}})
                else
                    edgeDetectSupported = GT_TRUE
                    result, values = myGenWrapper("cpssDxChSamplePortManagerEdgeDetectSuppSet",{
                                                 {"IN", "GT_U8","devNum",devNum},
                                                 {"IN", "GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                                 {"IN", "GT_BOOL", "edgeDetectSupported", edgeDetectSupported}})
                end
                if result == GT_NOT_INITIALIZED then
                    print("Error : Port not initialized \n")
                elseif result == GT_BAD_STATE then
                    print("Error : Port is not in reset state \n")
                elseif result~=GT_OK then
                    print("Error at command :%s", result)
                end
            else
                trainModeType = params.auto_tune_tx
            end
        end


        result, values = myGenWrapper("cpssDxChSamplePortManagerTrainModeSet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"IN","CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT", "trainModeType", trainModeType}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command: cpssDxChSamplePortManagerTrainModeSet :%s", result)
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function interconnect_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local interconnectProfile

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        if params.flagNo == true then
            interconnectProfile = 0
            result, values = myGenWrapper("cpssDxChSamplePortManagerOverrideInterconnectProfileSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT", "interconnectProfile", interconnectProfile}})
        else
           if params.ic_profile ~= nil then
                if params.ic_profile == "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E" then
                    interconnectProfile = 1
                elseif params.ic_profile == "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_2_E" then
                    interconnectProfile = 2
                else
                    interconnectProfile = 0
                end
            else
                interconnectProfile = 0
            end
            result, values = myGenWrapper("cpssDxChSamplePortManagerOverrideInterconnectProfileSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT", "interconnectProfile", interconnectProfile}})
        end
        if result == GT_NOT_INITIALIZED then
            print("Error : Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command :%s", result)
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------
-- command registration: serdes calibration
--------------------------------------------
CLI_addCommand("interface", "serdes calibration ", {
  func   = serdes_calibration_func,
  help   = "serdes calibration mode",
  params = {
     { type="values", "%calibration_mode" },
     { type="named",
             { format = "low-frequency  %min_threshold   %max_threshold", name="low_frequency", multiplevalues=true, help = " min threshold of LF" },
             { format = "high-frequency %min_threshold   %max_threshold", name="high_frequency", multiplevalues=true, help = "Configure min and max threshold of HF"},
             { format = "eye-opening    %min_threshold   %max_threshold", name="eye_opening", multiplevalues=true, help = "Configure min and max threshold of eye-opening"},
             { format = "time-out %GT_U32", name="time-out",     help = "Configure calibration timeout value"},
     }}}
)

----------------------------------------------
-- command registration: no serdes calibration
----------------------------------------------
CLI_addCommand("interface", "no serdes calibration ", {
  func = function(params)
    params.flagNo = true
    return serdes_calibration_func(params)
    end,
  help   = "serdes calibration mode"
})

--------------------------------------------
-- command registration: serdes etl
--------------------------------------------
CLI_addCommand("interface", "serdes etl ", {
  func   = serdes_etl_func,
  help   = "serdes eye min and max threshold",
  params = {
     { type="named",
             { format = "    %min_threshold    %max_threshold", name="etl_val", multiplevalues=true }
     }}}
)
--------------------------------------------
-- command registration: no serdes etl
--------------------------------------------
CLI_addCommand("interface", "no serdes etl ", {
  func = function(params)
    params.flagNo = true
    return serdes_etl_func(params)
    end,
  help   = "serdes eye min and max threshold"
})

--------------------------------------------
-- command registration: serdes precode
--------------------------------------------
CLI_addCommand("interface", "serdes precode ", {
  func   = serdes_precode_func,
  help   = "enable serdes precoding",
  params = {}
  }
)

--------------------------------------------
-- command registration: no serdes precode
--------------------------------------------
CLI_addCommand("interface", "no serdes precode ", {
  func = function(params)
    params.flagNo = true
    return serdes_precode_func(params)
    end,
  help   = "enable serdes precoding",
  }
)

--------------------------------------------
-- command registration: serdes tune
--------------------------------------------
CLI_addCommand("interface", "serdes tune ", {
  func   = serdes_tune_func,
  help   = "serdes auto tuning modes",
  params = {
     { type="named",
             { format = "rx-training %rx_tune", name="auto_tune_rx", help = "run rx training" },
             { format = "tx-training", name="auto_tune_tx", help = "run tx training" }
     }}}
)

--------------------------------------------
-- command registration: no serdes tune
--------------------------------------------
CLI_addCommand("interface", "no serdes tune ", {
  func = function(params)
    params.flagNo = true
    return serdes_tune_func(params)
    end,
  help   = "serdes auto tuning modes",
  params = {
     { type="named",
             { format = "rx-training %rx_tune", name="auto_tune_rx", help = "run rx training" },
             { format = "tx-training", name="auto_tune_tx", help = "run tx training" }
     }}}
)

--------------------------------------------
-- command registration: interconnect profile
--------------------------------------------
CLI_addCommand("interface", "interconnect ", {
  func   = interconnect_func,
  help   = "interconnect profile",
  params = {
     { type="named",
             { format="profile %ic_profile", name="ic_profile", help="Set Interconnect profile"}
     }}}
)

--------------------------------------------
-- command registration: no interconnect profile
--------------------------------------------
CLI_addCommand("interface", "no interconnect ", {
  func = function(params)
    params.flagNo = true
    return interconnect_func(params)
    end,
  help   = "interconnect profile",
  params = {
     { type="named",
             { format="profile %ic_profile", name="ic_profile", help="Set Interconnect profile back to default"}
     }}}
)
