--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* debug.lua
--*
--* DESCRIPTION:
--*       setting port to debug mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

function isPortInDebugMode(devNum, portNum)
    local ret, val
    ret, val = myGenWrapper("cpssDxChPortManagerStatusGet",
                        {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                         { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                         { "OUT","CPSS_PORT_MANAGER_STATUS_STC", "portStagePtr"}})
    if ret == 0 then
        if val.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_DEBUG_E" then
            return ret, true
        else
            return ret, false
        end
    end
    return ret, false
end

local function debug_func_main(params)
    local ret, val, val_pm
    local portStagePtr={}
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local portParamStc
    local portMgrEvent = {}
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    if params.flagNo then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            ret, val = isPortInDebugMode(devNum, portNum)
            if ret ~= 0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at Port Manager status get failed: device %d " ..
                                      "port %d: %s", devNum, portNum,
                                      returnCodes[ret])
                return ret
            end
            if true==val then 
                portMgrEvent.portEvent =  "CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E"
                ret = myGenWrapper("cpssDxChPortManagerEventSet",{
                            {"IN","GT_U8","devNum",devNum},
                            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                            {"IN","CPSS_PORT_MANAGER_STC", "portEventStcPtr", portMgrEvent}
                            })
                if ret ~= 7 and ret ~= 0 then --GT_BAD_STATE
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at Port Manager Event Set: device %d " ..
                                          "port %d: lane %d: %s", devNum, portNum, laneNum,
                                          returnCodes[ret])
                    return ret
                elseif ret == 7 then

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
                end
            else
                 --print("Port not in debug mode")
            end
        end
    else
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            ret, val = isPortInDebugMode(devNum, portNum)
            if ret ~= 0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at Port Manager status get failed: device %d " ..
                                      "port %d: %s", devNum, portNum,
                                      returnCodes[ret])
                return ret
            end
            if false==val then 
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
            else
                --print("Port in Debug Mode")
            end
        end
    end
end

--------------------------------------------------------------------------------
-- command registration: debug
--------------------------------------------------------------------------------
CLI_addCommand("interface", "debug", {
  func   = debug_func_main,
  help   = "Set the port in debug mode",
})

CLI_addCommand("interface", "no debug", {
  func = function(params)
    params.flagNo = true
    return debug_func_main(params)
    end,
  help = "set port out of debug mode"
})
