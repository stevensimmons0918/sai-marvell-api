--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_status.lua
--*
--* DESCRIPTION:
--*       showing of interface status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_interfaces_status
--        @description  clears statistics on an interface
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_status(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_interface_mode, port_interface_mode_string
    local port_link, port_link_string
    local port_speed, port_speed_string
    local port_duplex, port_duplex_string
    local header_string, footer_string
    local loopbackNum, loopback_mode_string
    local enumSerdes={
   CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E = "None",
   CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E = "SD analogTx2rx",
   CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E = "SD digitalTX2RX",
   CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E = "SD rx2tx"
    }
    local port_manager_string
    local port_autoneg_string
    local port_fec_string
    local port_linkscan_string
    local skipRes



    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization
    header_string =
        "\n" ..
        "Dev/Port     Mode    Link   Speed  Duplex    Loopback      Autoneg      FEC        Link Scan   Port Manager\n" ..
        "---------  --------  -----  -----  ------  -------------  ---------  ----------   -----------  ------------\n"
    footer_string = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            port_interface_mode_string = "n/a"
            port_link_string = "n/a"
            port_speed_string = "n/a"
            port_duplex_string = "n/a"
            loopback_mode_string = "n/a"
            port_manager_string  = "N/A"
            port_autoneg_string = "N/A"
            port_fec_string     = "N/A"
            port_linkscan_string = "N/A"
            -- Port interface mode getting.
            if true == command_data["local_status"] then
                result, values =
                    cpssPerPortParamGet("cpssDxChPortInterfaceModeGet",
                                                     devNum, portNum, "mode",
                                                     "CPSS_PORT_INTERFACE_MODE_ENT")
                if        0 == result then
                    port_interface_mode = interfaceStrGet(values["mode"])
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port interface mode getting is " ..
                                            "not allowed in device %d port %d.",
                                            devNum, portNum)
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                elseif 4 == result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("There is no port information to show.\n"..
                                          "Port %d of device %d does not exists.",
                                          portNum, devNum, returnCodes[result])
                    
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port interface mode " ..
                                          "getting of device %d port %d: %s",
                                          devNum, portNum, returnCodes[result])
                end

                if 0 == result then
                    port_interface_mode_string = port_interface_mode
                else
                    port_interface_mode_string = "n/a"
                end
            end

            -- Port link getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChPortLinkStatusGet",
                                                     devNum, portNum, "link",
                                                     "GT_BOOL")
                if        0 == result then
                    port_link = values["link"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port link getting is not " ..
                                            "allowed in device %d port %d.",
                                            devNum, portNum)
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port link getting of " ..
                                          "device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_link_string = boolUpDownStrGet(port_link)
                else
                    port_link_string = "n/a"
                end
            end

            -- Port speed getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChPortSpeedGet",
                                                     devNum, portNum, "speed",
                                                     "CPSS_PORT_SPEED_ENT")
                if        0 == result then
                    port_speed = values["speed"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port speed getting is not " ..
                                            "allowed in device %d port %d.",
                                            devNum, portNum)
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port speed getting of " ..
                                          "device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_speed_string = speedStrGet(port_speed)
                else
                    port_speed_string = "n/a"
                end
            end

            -- Port duplex mode getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChPortDuplexModeGet",
                                                     devNum, portNum, "dMode",
                                                     "CPSS_PORT_DUPLEX_ENT")
                if        0 == result then
                    port_duplex = values["dMode"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port duplex mode getting is not allowed in device %d " ..
                                            "port %d.", devNum, portNum)
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port mode getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end

                if 0 == result then
                    port_duplex_string = duplexStrGet(port_duplex)
                else
                    port_duplex_string = "n/a"
                end
            end


            -- Loopback mode getting.
            loopback_mode_string="None"
            if true == command_data["local_status"] then	--check internal
                result, values = cpssPerPortParamGet("cpssDxChPortInternalLoopbackEnableGet",
                                                     devNum, portNum, "enablePtr",
                                                     "GT_BOOL")
                if        0 == result then
                    loopbackNum = values["enablePtr"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Loopback internal mode getting is not allowed in device %d " ..
                                            "port %d.", devNum, portNum)
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port

                elseif 0x12 == result then
                          loopback_mode_string="n/a"
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at loopback internal getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end
                --print(loopbackNum)
                if 0 == result and true==loopbackNum then
                    loopback_mode_string = "Internal"
                    loopbackNum=false
                else	--check serdes

                    if port_interface_mode_string ~= "n/a" and port_interface_mode_string ~= "HCD" then
                        result, values = myGenWrapper("cpssDxChPortSerdesLoopbackModeGet",{
                                                    {"IN","GT_U8","devNum",devNum},
                                                    {"IN","GT_U32","portNum,",portNum},
                                                    {"IN","GT_U32","laneBmp",0},
                                                    {"OUT","CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT","modePtr"}
                                                    })
                        if  0 == result then
                            loopbackNum = values["modePtr"]
                        elseif 0x10 == result then
                            command_data:setFailPortStatus()
                            command_data:addWarning("Loopback serdes mode getting is not allowed in device %d " ..
                                                    "port %d.", devNum, portNum)
                        elseif 4 == result then
                          loopback_mode_string="n/a"
                        elseif 0 ~= result then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at loopback serdes getting of device %d " ..
                                                  "port %d: %s", devNum, portNum,
                                                  returnCodes[result])
                        end

                        if 0 == result  then
                            loopback_mode_string = enumSerdes[loopbackNum]
                            loopbackNum=false
                        end
                    end
                end
            end
            if true == command_data["local_status"] then	--check internal
                result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                                {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                                 { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
                if result == 0 then
                    if(values.enablePtr) then
                        result, values = myGenWrapper("cpssDxChPortManagerStatusGet",
                                    {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                                     { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                     { "OUT","CPSS_PORT_MANAGER_STATUS_STC", "portStagePtr"}})
                        if result == 0 then
                            if values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_LINK_UP_E" then
                                port_manager_string = "Link-Up"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_LINK_DOWN_E" then
                                port_manager_string = "Link-Down"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_RESET_E" then
                                port_manager_string = "Reset"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E" then
                                port_manager_string = "Init"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E" then
                                port_manager_string = "Mac-Link-Down"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_FAILURE_E" then
                                port_manager_string = "Failed"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_DEBUG_E" then
                                port_manager_string = "Debug"
                            elseif values.portStagePtr.portState == "CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E" then
                                port_manager_string = "Force-Link-Down"                                
                            else
                                port_manager_string = "N/A"
                            end
                            -- check pm status
                            if port_interface_mode == "n/a" and port_manager_string ~= "N/A" then
                                port_interface_mode_string = interfaceStrGet(values.portStagePtr.ifMode)
                                port_speed_string = speedStrGet(values.portStagePtr.speed)
                            end
                        end
                        port_autoneg_string = "No"
                        port_linkscan_string = "None"
                        result, values = myGenWrapper("cpssDxChPortManagerPortParamsGet",
                                            {{ "IN", "GT_U8"  , "devNum", devNum},
                                             { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                             { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})
                        if result== 0 then
                            if values.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                                 skipRes = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["skipRes"]
                                --print("skipap" ..to_string(skipRes))
                                if skipRes ~= true then
                                    port_autoneg_string = "Yes"
                                end
                            elseif values.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E" then
                                if values.portParamsStcPtr.portParamsType.regPort.portAttributes.pmOverFw == true then
                                    port_linkscan_string = "SW"
                                end
                            end
                        end
                    else
                        port_autoneg_string = "No"
                        if true == command_data["local_status"] then
                            result, values = myGenWrapper("cpssDxChPortApPortConfigGet",
                                        {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                                         { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                         { "OUT", "GT_BOOL", "apEnablePtr"},
                                         { "OUT", "CPSS_DXCH_PORT_AP_PARAMS_STC", "apParamsPtr"}})

                            if 0 == result then
                                if values.apEnablePtr then
                                    port_autoneg_string = "Yes"
                                end
                            elseif 4 == result and portNum == 63 then
                            -- skip CPU port
                            elseif 0 ~= result then
                                command_data:setFailPortAndLocalStatus()
                                command_data:addWarning("Port autoneg config get failed on device %d " ..
                                                "port %d.", devNum, portNum)
                            end
                        end
                    end
                end
            end
            port_fec_string = "None"
            if true == command_data["local_status"] then
                result, values = myGenWrapper("cpssDxChPortFecModeGet",
                                    {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                                     { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                     { "OUT", "CPSS_DXCH_PORT_FEC_MODE_ENT", "modePtr"}})

                if 0 == result then
                    if values.modePtr == "CPSS_DXCH_PORT_FEC_MODE_ENABLED_E" then
                        port_fec_string = "FC"
                    elseif values.modePtr == "CPSS_DXCH_PORT_FEC_MODE_DISABLED_E" then
                        port_fec_string = "None"
                    elseif values.modePtr == "CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E" then
                        port_fec_string = "RS"
                    elseif values.modePtr == "CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E" then
                        port_fec_string = "RS-544"
                    end
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addWarning("Port fec mode get failed on device %d " ..
                                            "port %d.", devNum, portNum)
                end
            end


            command_data:updateStatus()

            -- Resulting string formatting and adding.
            devPort=tostring(devNum).."/"..tostring(portNum)
            
            if (port_interface_mode_string ~= "n/a" or
                port_link_string ~= "n/a" or
                port_speed_string ~= "n/a" or
                port_duplex_string ~= "n/a") then
            
                command_data["result"] =
                    string.format("%-11s%-10s%-7s%-7s %-7s %-14s %-9s %-10s %-14s %-14s", devPort,
                                  alignLeftToCenterStr(port_interface_mode_string,
                                                       9),
                                  alignLeftToCenterStr(port_link_string, 5),
                                  alignLeftToCenterStr(port_speed_string, 5),
                                  port_duplex_string, alignLeftToCenterStr(loopback_mode_string,13), alignLeftToCenterStr(port_autoneg_string, 5),
                                  alignLeftToCenterStr(port_fec_string,8), alignLeftToCenterStr(port_linkscan_string,15), alignLeftToCenterStr(port_manager_string,10))

                command_data:addResultToResultArray()
            end

            

            command_data:updatePorts()
        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
                                         footer_string,
                                         "There is no port information to show.\n")

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-------------------------------------------------
-- command registration: show interfaces status
-------------------------------------------------
CLI_addCommand("exec", "show interfaces status", {
    func = show_interfaces_status,
    help = "Ports information",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})
