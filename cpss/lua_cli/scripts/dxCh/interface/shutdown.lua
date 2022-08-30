--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shutdown.lua
--*
--* DESCRIPTION:
--*       disabling/restarting of an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  shutdown_func
--        @description  disables/restart's of an interface
--
--        @return       true on success, otherwise false and error message
--
cmdLuaCLI_registerCfunction("prvLuaTgfIsPortForceLinkUpWaReq")

local function shutdown_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_state,port_state_cg

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- System specific data initialization.
    wrlOsSaveCurrentStdOutFunction()
    wrlOsResetStdOutFunction()

    -- Command specific variables initialization
    port_state = command_data:getTrueIfFlagNoIsNil(params)
    port_state_cg = command_data:getTrueIfFlagNoIsNil(params)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            -- Port enabling.
            command_data:clearLocalStatus()

            if true == command_data["local_status"]             then
                local result, waNeeded = prvLuaTgfIsPortForceLinkUpWaReq(devNum, portNum)
                if waNeeded == false then
                    result =
                        cpssPerPortParamSet("cpssDxChPortForceLinkDownEnableSet",
                                            devNum, portNum, port_state, "state",
                                                         "GT_BOOL")
                    if     0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("\"shutdown\" is not supported on AP ports. " ..
                                                "In order to bring the AP port link down, use the \"no ap-port\" command");

                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at going to down port %d " ..
                                              "device %d: %s.", portNum, devNum,
                                              returnCodes[result])
                    end
                else
                -- reverse state for 'link up' logic
                    port_state = not port_state
                    result = prvLuaTgfPortForceLinkUpEnableSet(devNum, portNum, port_state)
                    if 0 ~= result then
                        command_data:addError("Error: prvLuaTgfPortForceLinkUpEnableSet port %d " ..
                                              "device %d: %s.", portNum, devNum,
                                              returnCodes[result])
                    end

                    if not is_sip_6(devNum)
                       then
                        result =
                        cpssPerPortParamSet("cpssDxChPortForceLinkDownEnableSet",
                                            devNum, portNum,port_state_cg, "state",
                                                         "GT_BOOL")
                        if     0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("\"shutdown\" is not supported on AP ports. " ..
                                                "In order to bring the AP port link down, use the \"no ap-port\" command");

                        elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at going to down port %d " ..
                                              "device %d: %s.", portNum, devNum,
                                              returnCodes[result])
                        end
                    end
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not going to down all processed ports.")
    end

    -- System specific data updating.
    wrlOsRestoreStdOutFunction()

    -- Command data postprocessing
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

local function shutdown_func_port_mgr(params)
    local result, values
    local devNum, portNum
    local iterator
    local GT_NOT_INITIALIZED = 0x12
    local event = {}
    local command_data = Command_Data()
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    if params.flagNo then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            result, values = myGenWrapper("cpssDxChPortManagerStatusGet",
                                    {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                                     { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                     { "OUT","CPSS_PORT_MANAGER_STATUS_STC", "portStagePtr"}})
            if result == GT_NOT_INITIALIZED then
                command_data:addError("Port %d Speed and Mode not Configured %s", portNum, returnCodes[result])
            elseif 0 ~= result then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at power up dev %d port %d %s", devNum, portNum, returnCodes[result])
            end

            if(values.portStagePtr.portUnderOperDisable)                or values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_RESET_E"then

                if values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_RESET_E" then
                    event.portEvent =  "CPSS_PORT_MANAGER_EVENT_CREATE_E"
                else
                    event.portEvent =  "CPSS_PORT_MANAGER_EVENT_ENABLE_E"
                end

                result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                                  {{ "IN", "GT_U8", "devNum", devNum},
                                   { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                   { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})
                if result == GT_NOT_INITIALIZED then
                    command_data:addError("Port %d Speed and Mode not Configured %s", portNum, returnCodes[result])
                elseif 0 ~= result then
		    if result > 0x1001F then
		       result = 1
		    end
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at power up dev %d port %d result %s", devNum, portNum, returnCodes[result])
                end
            else
                command_data:addError("Port %d not in reset/disabled state.", portNum, tostring(values.portStagePtr.portState))
            end
        end
    else
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DISABLE_E"
            result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                              {{ "IN", "GT_U8", "devNum", devNum},
                               { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                               { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})
        end
        command_data:updateStatus()
        command_data:updatePorts()
    end

    wrlOsRestoreStdOutFunction()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function shutdown_func_main(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    devNum, portNum = command_data:getFirstPort()

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if (values.enablePtr) then
        shutdown_func_port_mgr(params)
    else
        shutdown_func(params)
    end
end

--------------------------------------------------------
-- command registration: vlan-mtu
--------------------------------------------------------
CLI_addCommand("interface", "shutdown", {
  func   = shutdown_func_main,
  help   = "Shutdown the selected interface"
})

--------------------------------------------------------
-- command registration: vlan-mtu
--------------------------------------------------------
CLI_addCommand("interface", "no shutdown", {
  func   = function(params)
               params.flagNo = true
               shutdown_func_main(params)
           end,
  help   = "Restarting the selected interface"
})

