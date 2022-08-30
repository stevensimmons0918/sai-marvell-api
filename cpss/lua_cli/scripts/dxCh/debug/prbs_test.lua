--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* prbs_test.lua
--*
--* DESCRIPTION:
--*       enable, disable and show prbs
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChDiagPrbsTimeInfoGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChDiagPrbsErrorCntrGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChDiagPrbsClearDb")
--constants

__prbs_lane_invert   = {}
-- function to set PRBS test start time
-- ************************************************************************
-- ************************************************************************
local function get_start_time(devNum, laneNum)
    local result, startTimeSec, startTimeNsec
    local readTimeSec, readTimeNsec
    result,startTimeSec,startTimeNsec,readTimeSec, readTimeNsec = wrLogWrapper("wrlCpssDxChDiagPrbsTimeInfoGet","(devNum,laneNum)", devNum, laneNum)
    return (startTimeSec + (startTimeNsec / 1000000000))
end

-- function to get PRBS test read time
-- ************************************************************************
local function get_read_time(devNum, laneNum)
    local result, startTimeSec, startTimeNsec
    local readTimeSec, readTimeNsec
    result,startTimeSec,startTimeNsec,readTimeSec, readTimeNsec = wrLogWrapper("wrlCpssDxChDiagPrbsTimeInfoGet","(devNum,laneNum)", devNum, laneNum)
    return (readTimeSec + (readTimeNsec / 1000000000))
end

-- ************************************************************************
local function clear_all(devNum,portNum,laneNum,serdesNum)
    local ret,val

-- to clear counters
    ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},

                    {"OUT","GT_BOOL","enablePtr"}
                    })
    if ret ~= 0 then
        return 0
    end

    if val.enablePtr == true then
            ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesStatusGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","GT_BOOL","lockedPtr"},
                {"OUT","GT_U32","errorCntrPtr"},
                {"OUT","GT_U64","patternCntrPtr"}
                })
    end
    if ret ~= 0 then
        return ret
    end
    ret = wrLogWrapper("wrlCpssDxChDiagPrbsClearDb","(devNum,laneNum)", devNum, serdesNum)
    return 0
end

-- ************************************************************************
local function get_error_cntr(devNum, laneNum)
    local ret, errorCntr
    ret, errorCntr = wrLogWrapper("wrlCpssDxChDiagPrbsErrorCntrGet","(devNum,laneNum)", devNum, laneNum)
    return errorCntr
end

local function get_lane_invert(laneNum)
    return __prbs_lane_invert[laneNum]
end

local function set_lane_invert(laneNum)
    __prbs_lane_invert[laneNum] = true
end
local function clear_lane_invert(laneNum)
    __prbs_lane_invert[laneNum] = false
end
-- ************************************************************************
local function isPrbsEnable(command_data,devNum, portNum, laneNum)
    local ret=0
    local val
    ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"OUT","GT_BOOL","enablePtr"}
                    })

    return ret,val["enablePtr"]
end

local function SetLanePolarityInvert(command_data,devNum,serdesNum)

    local ret, val
    local invertTx = false
    local invertRx = false
    ret,val = myGenWrapper("cpssDxChPortSerdesLanePolarityGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","portGroupNum",0},
            {"IN","GT_U32","laneNum", serdesNum},
            {"OUT","GT_BOOL","invertTx"},
            {"OUT","GT_BOOL","invertRx"}})

    if ret ~= 0 then
       --print(" Error at cpssDxChPortSerdesLanePolarityGet ret : " .. ret)
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at Lane Polarity get Set: device %d " ..
                          "port %d: lane %d: %s", devNum, portNum, serdesNum,
                          returnCodes[ret])
       return ret
    end

    if val["invertTx"] == invertTx then
        invertTx = true
    end
    if val["invertRx"] == invertRx then
        invertRx = true
    end
    ret = myGenWrapper("cpssDxChPortSerdesLanePolaritySet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","portGroupNum",0},
            {"IN","GT_U32","laneNum", serdesNum},
            {"IN","GT_BOOL","invertTx", invertTx},
            {"IN","GT_BOOL","invertRx", invertRx}})

    if ret ~= 0 then
       --print(" Error at cpssDxChPortSerdesLanePolaritySet ret : " .. ret)
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at Lane Polarity get Set: device %d " ..
                          "port %d: lane %d: %s", devNum, portNum, serdesNum,
                          returnCodes[ret])
        return ret
    end
    return 0

end
-- ************************************************************************

local function PrbsSetDisable(command_data,devNum, portNum,laneNum)

    local ret, val
    local serdesNum = GetPortSerdesNum(devNum, portNum, laneNum)
    local portMgrEvent = {}

    ret,val=isPrbsEnable(command_data,devNum, portNum, laneNum)
    if val == true then
        --print("laneinvert set: " ..to_string(get_lane_invert(serdesNum)))
        if true == get_lane_invert(serdesNum) then
            SetLanePolarityInvert(command_data, devNum, serdesNum)
            clear_lane_invert(serdesNum)
        end
        ret = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","GT_BOOL","enable",0}
                })

        if 0~=ret then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error cpssDxChDiagPrbsSerdesTestEnableSet: device %d " ..
                                  "port %d: lane %d: %s", devNum, portNum, laneNum,
                                  returnCodes[ret])
        end
        clear_all(devNum, portNum, laneNum, serdesNum)
    else
         if ret ~= 0 then
             print(devNum.."/"..portNum..": is disabled")
         end
        --print(devNum.."/"..portNum.."/" ..laneNum..": PRBS is already disabled")
    end
    return ret

end

-- ************************************************************************

local function PrbsSetEnable(command_data,devNum,portNum,laneNum,mode,inverted)

    local ret=0
    local val,val_mode
    local serdesNum = GetPortSerdesNum(devNum, portNum, laneNum)
    local portMgrEvent = {}

    ret,val=isPrbsEnable(command_data,devNum, portNum, laneNum)
    if ret==0 then
        if val == true then
            ret, val_mode = myGenWrapper("cpssDxChDiagPrbsSerdesTransmitModeGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","GT_U32","laneNum",laneNum},
            {"OUT","CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT","modePtr"}
            })
            if ret ~= 0 then
                print("Error at cpssDxChDiagPrbsSerdesTransmitModeGet")
            end
            if val_mode["modePtr"] ~= mode then
                val = false
            end
            --print("test AP prbs true")
        end
        if false==val then
            --print("prbs mode:" ..mode)
            if mode == "CPSS_DXCH_DIAG_TRANSMIT_MODE_CYCLIC_E" or mode == "CPSS_DXCH_DIAG_TRANSMIT_MODE_ONES_E" or
               mode == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E" or mode == "CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E" or
               mode == "CPSS_DXCH_DIAG_TRANSMIT_MODE_ZEROS_E" then
               print("prbs mode not supported for "..devNum .."/" ..portNum)
               return 0
            end
            clear_all(devNum, portNum, laneNum, serdesNum)

            ret = myGenWrapper("cpssDxChDiagPrbsSerdesTransmitModeSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT","mode",mode}
                })

            if 0~=ret then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error cpssDxChDiagPrbsSerdesTransmitModeSet: device %d " ..
                                      "port %d: lane %d: %s", devNum, portNum, laneNum,
                                      returnCodes[ret])
            else
                ret = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"IN","GT_BOOL","enable",1}
                    })

            end
            --print("Invert " .. to_string(inverted))
            if inverted == true then
                set_lane_invert(serdesNum)
                SetLanePolarityInvert(command_data, devNum, serdesNum)
            end
        else
            --print(devNum.."/"..portNum.. "lane: " ..laneNum.. ": PRBS is already enabled")
        end
    else
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at PRBS status getting isPrbsEnable: device %d " ..
                              "port %d: lane %d: %s", devNum, portNum, laneNum,
                              returnCodes[ret])
    end

    return ret
end

local function GetLaneSpeedToSpeedInBits(laneSpeed)
    if laneSpeed == "_26_5625G_PAM4" then
        return 53.125 -- 2*26.5625
    elseif laneSpeed == "_25_78125G" then
        return 25.78125
    elseif laneSpeed == "_10_3125G" then
        return 10.3125
    elseif laneSpeed == "_1_25G" then
        return 1.25
    else
        return 0
    end
end

local function getLanePrbsBer(start_time,read_time,laneSpeed,errCnt)
    local ber, speedbits, elapsed_time, bitcnt

    if start_time > read_time then
        print("Error in reading time")
        return 1
    end
    elapsed_time = read_time - start_time
    speedbits    = GetLaneSpeedToSpeedInBits(laneSpeed)
    bitcnt = (elapsed_time *(speedbits * 1000000000))
    ber = errCnt/bitcnt
    return ber
end

-- ************************************************************************

local function PrbsPrintLocked(command_data,devNum, portNum,laneNum,laneSpeed,devPort,val,
				           mode,singlePort,readTimePrev)
    local serdesNum = GetPortSerdesNum(devNum, portNum, laneNum)
    local lockedPtr=val["lockedPtr"]
    local errorCntrPtr=val["errorCntrPtr"]
    local patternCntrPtrStr
    local lockPtrStr, statusStr
    local start_time = get_start_time(devNum, serdesNum)
    local read_time  = get_read_time(devNum, serdesNum)
    local totalerror = get_error_cntr(devNum, serdesNum)
    local ber, totalber

    if readTimePrev == 0 then
        readTimePrev = start_time
    end
    ber      = getLanePrbsBer(readTimePrev,read_time,laneSpeed,errorCntrPtr)
    totalber = getLanePrbsBer(start_time,read_time,laneSpeed,totalerror)

    if lockedPtr == true then
        lockPtrStr =  string.format("Locked  ")
    else
        lockPtrStr =  string.format("UnLocked")
    end

    if errorCntrPtr == 0 then
        statusStr  =  string.format("Passed")
    else
        statusStr  =  string.format("Failed")
    end
    command_data:updateStatus()
    if singlePort == true then
        command_data["result"] = string.format("  Lane %-3s: ",laneNum).."  "
              ..string.format(" Start time  :%-15s",start_time).." " ..string.format(" Total errors :%-12s ",totalerror).. " "
              ..string.format("Total BER :%-28e ",totalber).. "\n"
              .."               "..string.format("Time Period :%-15s ",(read_time-readTimePrev)).. " "
             ..string.format("Errors       :%-12s ",errorCntrPtr).. " " ..string.format("BER       :%-28e \n",ber)
    else
        command_data["result"] = string.format("%-9s |",devPort).."  " ..string.format("%-3s |",laneNum).."   " .. mode .. "   | "
        .. statusStr .. "    | " .. lockPtrStr .. "  | " ..string.format("%-11s",errorCntrPtr).."| "  ..string.format("%-25e ",ber).. "|"
    end
    command_data:addResultToResultArray()
    return 0
end

-- ************************************************************************

local function PrbsPrintLane(command_data,devNum,portNum,laneNum,singlePort)

    local devPort
    local ret,val
    local mode, val_mode
    local laneSpeed
    local readTimePrev
    local serdesNum = GetPortSerdesNum(devNum, portNum, laneNum)

    ret,val=isPrbsEnable(command_data,devNum, portNum, laneNum)
    devPort=tostring(devNum).."/"..tostring(portNum)

    laneSpeed = GetPortLaneSpeed(devNum, portNum, laneNum)

    if 0==ret then
        if true==val then
            ret,val_mode = myGenWrapper("cpssDxChDiagPrbsSerdesTransmitModeGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U32","laneNum",laneNum},
                        {"OUT","CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT","modePtr"}
                        })

            if 0~=ret then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error cpssDxChDiagPrbsSerdesTransmitModeSet: device %d " ..
                                  "port %d: lane %d: %s", devNum, portNum, laneNum,
                                  returnCodes[ret])
            end

            if val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E" then
                mode = "PRBS_1T"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E" then
                mode = "PRBS_2T"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E" then
                mode = "PRBS_5T"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E" then
                mode = "PRBS_10T"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E" then
                mode = "PRBS_7"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E" then
                mode = "PRBS_9"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS11_E" then
                mode = "PRBS_11"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS13_E" then
                mode = "PRBS_13"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E" then
                mode = "PRBS_15"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E" then
                mode = "PRBS_23"
            elseif val_mode.modePtr == "CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E" then
                mode = "PRBS_31"
            else
                mode = "NA"
            end

            readTimePrev = get_read_time(devNum, serdesNum)
            ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesStatusGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","GT_BOOL","lockedPtr"},
                {"OUT","GT_U32","errorCntrPtr"},
                {"OUT","GT_U64","patternCntrPtr"}

                })	
            if 0 == ret then
                ret = PrbsPrintLocked(command_data,devNum,portNum,laneNum,laneSpeed,devPort,val,
                                      mode,singlePort,readTimePrev)
            else
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error cpssDxChDiagPrbsSerdesStatusGet: device %d " ..
                                      "port %d: lane %d: %s", devNum, portNum, laneNum,
                                      returnCodes[ret])

            end
        else
            command_data:updateStatus()
            command_data["result"] = string.format("%-6s",devPort).."         Disabled            "
            command_data:addResultToResultArray()
        end
    else
        command_data:updateStatus()
        command_data["result"] = string.format("%-6s",devPort).."         Disabled            "
        command_data:addResultToResultArray()
        return 0
    end
    return ret
end

local function PrbsClear(command_data,devNum,portNum,laneNum)
    local serdesNum = GetPortSerdesNum(devNum, portNum, laneNum)

    clear_all(devNum, portNum, laneNum, serdesNum)

    return 0
end
--------------------------------------------------------------------------------
local function SerdeLaneHandler(command_data,devNum,portNum,laneNum,state,mode,inverted,singlePort)

    local ret=0

    if state == "show" then
        ret = PrbsPrintLane(command_data,devNum,portNum,laneNum,singlePort)
        return ret
    elseif state == "prbs_enable" then
        ret = PrbsSetEnable(command_data,devNum,portNum,laneNum,mode,inverted)
        return ret
    elseif state == "clear" then
        ret = PrbsClear(command_data,devNum,portNum,laneNum)
        return ret
    else -- prbs_no_enable
        ret = PrbsSetDisable(command_data,devNum,portNum,laneNum)
        return ret
    end  
    return ret

end

local function setPortToDebugMode(command_data,devNum,portNum, force_debug)
    local ret, val
    local portMgrEvent = {}

    ret, val = isPortInDebugMode(devNum, portNum)
    if ret ~=0 then
        return ret
    end
    --print("val" ..to_string(val) .."force_debug " ..to_string(force_debug))
    if val == false and force_debug == true then
        portMgrEvent.portEvent =  "CPSS_PORT_MANAGER_EVENT_PORT_DEBUG_E"
        ret = myGenWrapper("cpssDxChPortManagerEventSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PORT_MANAGER_STC", "portEventStcPtr", portMgrEvent}
                 })
        if ret ~= 0 then
            print(" Error at cpssDxChPortManagerEventSet for debug mode ret : " .. ret)
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at Port Manager Event Set: device %d " ..
                                  "port %d: lane %d: %s", devNum, portNum, laneNum,
                                  returnCodes[ret])
            return ret
        end
        --delay(5000)
    elseif val == false then
        print(devNum.. "/" ..portNum .." not in debug mode")
        return 1
    end
    return ret
end

local function setPortToNormalMode(command_data,devNum,portNum)
    local ret, val, val_pm
    local portMgrEvent = {}

    ret, val = isPortInDebugMode(devNum, portNum)
    if ret ~= 0 then
        print("Error at setting port in normal mode ")
    end
    if val == true then
        ret, val_pm = myGenWrapper("cpssDxChPortManagerPortParamsGet",
                         {{ "IN", "GT_U8"  , "devNum", devNum},
                          { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                          { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})

        if ret ~= 0 then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at Port Manager params Set: device %d " ..
                                  "port %d: %s", devNum, portNum,
                                  returnCodes[ret])
            return ret
        end
        portMgrEvent.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
        ret = myGenWrapper("cpssDxChPortManagerEventSet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","CPSS_PORT_MANAGER_STC", "portEventStcPtr", portMgrEvent}
                        })
        if ret ~= 0 then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at Port Manager Event Set: device %d " ..
                                  "port %d: lane %d: %s", devNum, portNum, laneNum,
                                  returnCodes[ret])
            return ret
        end
        ret, val = myGenWrapper("cpssDxChPortManagerPortParamsSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr", val_pm["portParamsStcPtr"]}})

        if ret ~= 0 then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at Port Manager params Set: device %d " ..
                                  "port %d: %s", devNum, portNum,
                                  returnCodes[ret])
            return ret
        end
        portMgrEvent.portEvent =  "CPSS_PORT_MANAGER_EVENT_CREATE_E"
        ret = myGenWrapper("cpssDxChPortManagerEventSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PORT_MANAGER_STC", "portEventStcPtr", portMgrEvent}
                    })
        if ret ~= 0 then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at Port Manager Event Set: device %d " ..
                                  "port %d: lane %d: %s", devNum, portNum, laneNum,
                                  returnCodes[ret])
            return ret
        end 
    else
        --print("Port Not in debug mode")
    end
    return ret
end
-- ************************************************************************

--------------------------------------------------------------------------------
--[[
local function CounterClearOnReadEnableSet(params, devNum,portNum,laneNum)

    local enable = -1
    local ret = 0

    if "no-enable-clear-on-read-counter" == params.enableClearOnReadCounter then
        enable = 0
    else
        enable = 1
    end
    if enable ~= -1 then
        ret = myGenWrapper("cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet",
                {
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","GT_BOOL","enable",enable}
                })
        if ret ~= 0 then
            if 1 == enable then
                print("device "..devNum.."   port "..portNum.."  lane "..laneNum.. ": Cant't set Prbs Counter Clear On Read enable")
            else
                print("device "..devNum.."   port "..portNum.."  lane "..laneNum.. ": Cant't set Prbs Counter Clear On Read disable")
            end
        end
    end
    return ret

end
--]]
-- ************************************************************************

function prbsCommonDevPortLoop(params)

    local portNum,devNum, devPort
    local laneNum
    local ret=0
    local ret1, ret2, ret3
    local val
    local errorMsg=""
    local result, values
    local state=""
    local numOfSerdesLanesPtr=0
    local mode=0
    local header_string=""
    local footer_string=""

    -- Common variables declaration
    local result, values
    local command_data = Command_Data() 
    local singlePort
    local inverted = false
    local force_debug = false

    --print("prbsCommonDevPortLoop params  = "..to_string(params))
    setGlobal("ifType", "ethernet")
    setGlobal("ifRange", params["devPort"])
    --print(to_string(params))
    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:clearResultArray()

    -- Common variables initialization
    state=params.state
    mode=0

--[[    if params.debug ~= nil then
        force_debug = true
    else
        force_debug = false
    end--]]

    if params.inverted ~= nil then
        inverted = true
    else
        inverted = false
    end

    if params.ber ~= nil then
        singlePort = true
    else
        singlePort = false
    end

    if state == "prbs_enable" then
        mode=params["mode"]
    elseif state == "show" and singlePort == false then
        -- Command specific variables initialization
        header_string = 
           "\n" ..
           "Interface | Lane | Polynomial | Status    | Lock      | Errors     | BER                       |\n" ..
           "----------+------+------------+-----------+-----------+------------+---------------------------+\n"
        footer_string = "\n"
    else
    end

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    if true == command_data["status"] then

        for iterator, devNum, portNum in command_data:getPortIterator() do
            --command_data:clearPortStatus()
            if params["ber"] ~= nil then
                command_data["result"] = string.format("Interface %s/%s:",tostring(devNum),tostring(portNum))
                command_data:addResultToResultArray()
            end
            if state == "prbs_enable" then
              ret = setPortToDebugMode(command_data,devNum,portNum, force_debug)
            end
            if ret == 0 then
                ret1,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
                if ret1 == 0  then
                    for i=0,numOfSerdesLanesPtr-1,1 do
                        laneNum=i
                        ret2=SerdeLaneHandler(command_data,devNum,portNum,laneNum,state,mode, inverted, singlePort)
                        if 0 == ret then
                        --ret=CounterClearOnReadEnableSet(params, devNum,portNum,laneNum)
                        end
                    end
                end
            end
            --if state == "prbs_no_enable" and force_debug == true then
                --ret = setPortToNormalMode(command_data,devNum,portNum)
            --end
        end -- for iterator, devNum, portNum

    end -- if true == command_data["status"] then

    if state == "show" then

        command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"], footer_string)
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

    else
        command_data:setResultArrayToResultStr()
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
    end 

    return ret
end

local function prbsShowHandler(params)
    params.state="show"
    return prbsCommonDevPortLoop(params)
end

local function prbsClearHandler(params)
    params.state="clear"
    return prbsCommonDevPortLoop(params)
end
--------------------------------------------------------------------------------
local function prbsEnableHandler(params)

    if params.flagNo == true then
        params.state="prbs_no_enable"
    else
        params.state="prbs_enable"
    end
    return prbsCommonDevPortLoop(params)
end

--------------------------------------------------------------------------------
-- command registration: prbs enable
--------------------------------------------------------------------------------
CLI_addHelp("debug", "link prbs", "Link PRBS diag")
CLI_addCommand("debug", "link prbs interface", {
    func=prbsEnableHandler,
    help="Configure PRBS",
    params={
        {   type="named",
            { format=" ethernet %port-range", name="devPort", help="Ethernet interface to configure" },
            { format=" polynomial %prbs_mode", name="mode", help="PRBS mode"},
            { format=" inverted ", name="inverted", help="PRBS inverted pattern"},
            --{ format=" force-debug", name="debug", help="Force port in Debug mode"},
            mandatory={"mode", "devPort"}
        }
    }
})

CLI_addHelp("debug", "link prbs clear", "Clear PRBS counters")
CLI_addCommand("debug", "link prbs clear interface", {
    func=prbsClearHandler,
    help="Clear PRBS counters",
    params={
        {   type="named",
            { format=" ethernet %port-range", name="devPort", help="Ethernet interface to clear prbs counter" },
            mandatory={"devPort"}
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no prbs
--------------------------------------------------------------------------------
CLI_addCommand("debug", "no link prbs interface", {
    func=function(params)
        params.flagNo=true
        return prbsEnableHandler(params)
    end,
    help   = "Disable prbs",
    params ={{type= "named", { format=" ethernet %port-range", name="devPort", help="interface port" },
                             --{ format=" force-debug", name="debug", help="Force port in Debug mode"},
                             mandatory={"devPort"}}

  }}
)

--------------------------------------------------------------------------------
-- command registration: prbs show
--------------------------------------------------------------------------------
CLI_addHelp("debug", "link prbs show", "Show PRBS counters")
CLI_addCommand("debug", "link prbs show interface", {
  func   = prbsShowHandler,
  help   = "Show PRBS counter",
  params={
        {   type="named",
            { format=" ethernet %port-range", name="devPort", help="Ethernet interface to show" },
            { format=" ber", name="ber", help="per port ber" },
            mandatory={"devPort"}
        }
   }
})
--------------------------------------------------------------------------------




