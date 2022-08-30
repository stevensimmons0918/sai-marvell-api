--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* loopback.lua
--*
--* DESCRIPTION:
--*       configure loopback mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

function serdesLoopBackToVal(sl)
     if sl == "CPSS_PORT_SERDES_LOOPBACK_DISABLE_E" or sl =="CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E" then
         return 0
     elseif sl == "CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E" or sl == "CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E"then
         return 1
     elseif sl == "CPSS_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E" or sl == "CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E"then
         return 2
     elseif sl == "CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E" or sl == "CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E" then
         return 3
     else
         print("error at serded loopback type",sl)
     end
end

-- ************************************************************************
---
--  loopbackHandler
--        @description  enable/disable loopback
--
--        @param params             - params["flagNo"]: no command property
--                    params["loopback_mode"]: internal/serdes
--                    params[params["loopback_mode"]]: internal/0/1/2.
--                    0: serdes rx2tx
--                    1: serdes analogTx2rx
--                    2: serdes digitalTx2rx
--
--        @return       true on success, otherwise false and error message
--
local function loopbackHandler(params)
    local isOk = true

    local isGm = wrLogWrapper("wrlCpssIsGmUsed")

    local serdesModesStr={
        CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E  = "analogTx2rx",
        CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E = "digitalTx2rx",
        CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E = "rx2tx" }

    local enable         = not params.flagNo
    local configInternalOnly = params["loopback_mode"] == "internal"
    local enableInternal = enable and params["loopback_mode"] == "internal"
    local enableSerdes   = enable and not enableInternal

    local serdesMode     = enableSerdes and params[params["loopback_mode"]]
        or "CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E"



    local all_ports, dev_ports, devNum
    all_ports = getGlobal("ifRange")--get table of ports

    for devNum, dev_ports in pairs(all_ports) do--disable internal
        local isDevBobcat2 = is_sip_5(devNum)

        for k, port in pairs(dev_ports) do
            if isGm and isDevBobcat2 then
                local ret = myGenWrapper(
                    "simulateExternalLoopbackPortSet", {
                        {"IN", "GT_U8", "devNum", devNum},
                        {"IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                        {"IN", "GT_BOOL", "enable", enable}
                })
                if ret ~= 0 then
                    print(string.format("%d/%d, simulate external loopback: %s",
                                        devNum, port, returnCodes[ret]))
                    isOk = false
                end
            else
                local ret,waNeeded = prvLuaTgfIsPortForceLinkUpWaReq(devNum, port)
                if waNeeded == true then
                    ret = prvLuaTgfPortLoopbackModeEnableSet(devNum, port, enableInternal)
                else
                    ret = myGenWrapper(
                        "cpssDxChPortInternalLoopbackEnableSet",{  --enable internal
                            {"IN","GT_U8","devNum",devNum},
                            {"IN","GT_U32","portNum",port},
                            {"IN","GT_BOOL","enable", enableInternal}
                    })
                end
                if ret ~= 0 then
                    print(string.format("%d/%d, loopback internal : %s",
                                        devNum, port, returnCodes[ret]))
                    isOk = false
                end

                if not configInternalOnly then 
                    local ret = myGenWrapper(
                        "cpssDxChPortSerdesLoopbackModeSet",{--enable serdes
                            {"IN","GT_U8","devNum",devNum},
                            {"IN","GT_U32","portNum",port},
                            {"IN","GT_U32","laneBmp",0xffffffff},
                            {"IN","CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT","mode", serdesMode}
                    })
                    if ret ~= 0 and enable then -- ignore result for NO command. Some devices does not support it.
                        print(string.format("%d/%d, loopback serdes %s : %s",
                                        devNum, port,
                                        serdesModesStr[serdesMode],
                                        returnCodes[ret]))
                        isOk = false
                    end
                end
            end
        end
    end
    return isOk
end

local function forceLinkUpSet(params)
    if prvLuaTgfPortForceLinkUpEnableSet == nil then
        print("Not supported, CPSS 4.1 required")
        return false
    end
    local all_ports, dev_ports, devNum, k, port
    all_ports = getGlobal("ifRange")--get table of ports
    local rc, ret = true
    for devNum, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do
            rc = prvLuaTgfPortForceLinkUpEnableSet(devNum,port,params.enable)
            if rc ~= 0 then
                print("prvLuaTgfPortForceLinkUpEnableSet("..
                        tostring(devNum)..", "..to_string(port)..", "..
                        tostring(params.enable)..") failed, rc="..to_string(rc))
                ret = false
            end
        end
    end
    return ret
end

local function loopbackHandler_port_mgr(params)
    local result, values
    local command_data = Command_Data()
    local devNum, portNum
    local iterator
    local GT_OK=0
    local GT_NOT_INITIALIZED=0x12
    local loopbackType, macLoopbackEnable
    local serdesMode
    local doOnlyMacLoopbackWa = false

    if params.flagNo then
        macLoopbackEnable = GT_FALSE
        serdesMode = "CPSS_PORT_SERDES_LOOPBACK_DISABLE_E"
    else
        if params.loopback_mode == "internal" then
            loopbackType = "CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E"
            macLoopbackEnable = GT_TRUE
            serdesMode = "CPSS_PORT_SERDES_LOOPBACK_DISABLE_E"
        else
            loopbackType = "CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E"
            macLoopbackEnable = GT_FALSE
            serdesMode   =  serdesLoopBackToVal(params[params["loopback_mode"]])
        end
    end

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()
    for iterator, devNum, portNum in command_data:getPortIterator() do
		local result,waNeeded = prvLuaTgfIsPortForceLinkUpWaReq(devNum, portNum)
        if waNeeded == true then
            local enable         = not params.flagNo
            local enableInternal = params["loopback_mode"] == "internal"
            doOnlyMacLoopbackWa = not enable or enableInternal -- true when 'no loopback' or when 'loopback serdes'
            -- prvLuaTgfPortLoopbackModeEnableSet supports the portMgr 
            -- (but also support the WA needed for 'force link up' and the relation with internal 'port loopback')
            result = prvLuaTgfPortLoopbackModeEnableSet(devNum, portNum, enableInternal)
            if result ~= 0 then
                print(string.format("Error : portNum %d/%d, loopback internal : %s",
                                    devNum, portNum, returnCodes[result]))
            end
        end

        if not doOnlyMacLoopbackWa then 
            result, values = myGenWrapper("cpssDxChSamplePortManagerLoopbackSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT", "loopbackType", loopbackType},
                           { "IN", "GT_BOOL" , "macLoopbackEnable", macLoopbackEnable},
                           { "IN", "CPSS_PORT_SERDES_LOOPBACK_MODE_ENT", "serdesLoopbackMode", serdesMode},
                           { "IN", "GT_BOOL", "enableRegularTrafic", GT_FALSE}})

            if result == GT_NOT_INITIALIZED then
                print("Error : port %d not initialized \n", portNum)
            elseif result == GT_BAD_STATE then
                print("Error : Port %d  not in reset state \n", portNum)
            elseif result~=GT_OK then
                print("Error at command: cpssDxChSamplePortManagerFecModeSet :%s", result)
            end
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function loopbackHandler_main(params)
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
        loopbackHandler_port_mgr(params)
    else
        loopbackHandler(params)
    end
end
--------------------------------------------------------------------------------
-- command registration: loopback mode
--------------------------------------------------------------------------------
CLI_addHelp("interface", "loopback", "Configure loopback")
CLI_addCommand("interface", "loopback mode", {
  func   = loopbackHandler_main,
  help   = "Define loopback mode",
  params = {{ type = "named", "#loopback", mandatory = {"loopback_mode"}}}
})

--------------------------------------------------------------------------------
-- command registration: no loopback
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no loopback", {
    func=function(params)
        params.flagNo=true
        return loopbackHandler_main(params)
    end,
    help   = "Disable loopback",
})


--------------------------------------------------------------------------------
-- command registration: force link up enable
--------------------------------------------------------------------------------
CLI_addHelp("interface", "force", "force link state")
CLI_addHelp("interface", "force link", "force link state")
CLI_addCommand("interface", "force link up", {
    func = function(params)
        params.enable=true
        return forceLinkUpSet(params)
    end,
    help = "force link up state on selected ports",
})

--------------------------------------------------------------------------------
-- command registration: no force link up
--------------------------------------------------------------------------------
CLI_addHelp("interface", "no force", "don't force link state")
CLI_addHelp("interface", "no force link", "don't force link state")
CLI_addCommand("interface", "no force link up", {
    func = function(params)
        params.enable=false
        return forceLinkUpSet(params)
    end,
    help = "don't force link up state on selected ports",
})
