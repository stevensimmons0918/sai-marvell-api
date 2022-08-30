--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* storm_control.lua
--*
--* DESCRIPTION:
--*       set Ingress Port Storm Rate Limit Enforcement
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
require("dxCh/cli_types/storm_control_rate_limit_enum")

--constants


--
-- ************************************************************************
---
--  stormControl
--        @description  stormControl - Set Ingress Port Storm Rate Limit Enforcement
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function stormControlEnable(command_data, devNum, portNum, params)

    local command_data = params.command_data
    local did_error, result, values
    local portGfgPtr, brgRateLimitPtr

    local enable = true

    if (params.flagNo == true) then
        enable = false
    end
    ----- WRAPPER for cpssDxChBrgGenPortRateLimitGet
    did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitGet", {
                     {"IN", "GT_U8", "devNum", devNum},
                     {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                     {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr"}})

    portGfgPtr = values["portGfgPtr"]
    if enable == true then
        if params.broadcast then
                portGfgPtr.enableBc = true
        end
        if params.known then
                portGfgPtr.enableUcKnown = true
        end
        if params.unknown then
                portGfgPtr.enableUcUnk = true
        end
        if params.unregistered then
                portGfgPtr.enableMc = true
        end
        if params.registered then
                portGfgPtr.enableMcReg = true
        end
        if params.tcp_syn then  ----- WRAPPER for cpssDxChBrgGenPortRateLimitTcpSynSet

                genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynSet", {
                         {"IN", "GT_U8", "devNum", devNum},
                         {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                         {"IN", "GT_BOOL", "enable", GT_TRUE}})
        end

        if params.kbps then
    ----- WRAPPER for cpssDxChBrgGenRateLimitGlobalCfgGet
            did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgGet", {
                             {"IN", "GT_U8", "devNum", devNum},
                             {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr"}})

            brgRateLimitPtr = values["brgRateLimitPtr"]
            local windowSize = 1024;
            brgRateLimitPtr.rMode       = "CPSS_RATE_LIMIT_BYTE_BASED_E"
            brgRateLimitPtr.win10Mbps   = windowSize
            brgRateLimitPtr.win100Mbps  = windowSize
            brgRateLimitPtr.win1000Mbps = windowSize
            brgRateLimitPtr.win10Gbps   = windowSize
            brgRateLimitPtr.win100Gbps  = windowSize

            genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgSet",
            {
                         {"IN", "GT_U8", "devNum", devNum},
                         {"IN", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr", brgRateLimitPtr}})
      --      if params.level then

       --     end

            local rateGranularityInBytes, maxRateValue
            if is_sip_5(devNum) then
                rateGranularityInBytes = 1
                maxRateValue = 4194303
            else
                rateGranularityInBytes = 64
                maxRateValue = 65535
            end

            local tempLimit = (params.kbps * (windowSize/1000000) * 1000) / (rateGranularityInBytes * 8) -- we can reduce equsion to [params.kbps / (64 * 8)]
            if tempLimit <= maxRateValue then
                portGfgPtr.rateLimit = tempLimit
            else
                command_data:handleCpssErrorDevPort(
                result, string.format("Rate limit out of range [%s] for kbps = %s ", tempLimit, params.kbps))
            end
        end

        if params.pps then
            local maxRateValue, rateGranularityInPackets
            local windowSize = 1024
            if is_sip_5(devNum) then
                -- Rate limit field size is 22 bits
                maxRateValue = 4194303
                rateGranularityInPackets = 1
            else
                -- Rate limit field size is 16 bits
                maxRateValue = 65535
                rateGranularityInPackets = 64
            end
            -- Calculate number of packets within window size
            local tempLimit = (params.pps * (windowSize/1000000)) / rateGranularityInPackets
            -- Check if value is within range otherwise print error indication
            if tempLimit <= maxRateValue then
                portGfgPtr.rateLimit = tempLimit
            else
                command_data:handleCpssErrorDevPort(
                result, string.format("Rate limit out of range [%s] for pps = %s ", tempLimit, params.pps))
            end
        end


    else  --- flag no
        portGfgPtr.enableBc = false
        portGfgPtr.enableUcKnown = false
        portGfgPtr.enableUcUnk = false
        portGfgPtr.enableMc = false
        portGfgPtr.enableMcReg = false

        genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynSet", {
                         {"IN", "GT_U8", "devNum", devNum},
                         {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                         {"IN", "GT_BOOL", "enable", GT_FALSE}})
    end  --- end for enable check

    genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitSet",
        {
                     {"IN", "GT_U8", "devNum", devNum},
                     {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                     {"IN", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr", portGfgPtr}})

    if(is_sip_5(devNum)) then
        --the emulator not like default of 10M ... so set it in 1G mode
        -- it should be OK for all flavors (WM/emulator/HW)
        genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitSpeedGranularitySet",
            {
                {"IN", "GT_U8", "devNum", devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                {"IN", "CPSS_PORT_SPEED_ENT", "speedGranularity", "CPSS_PORT_SPEED_1000_E"}})
    end


   --command_data:initInterfaceDevPortRange()
   --command_data:iterateOverPorts(func, params)
   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()

   end -- func
--
--  stormControlEnableFunc
--        @description  Set Ingress Port Storm Rate Limit Enforcement
--
--  @param params   - parameters
--     - params["pps"]          - setup maximum of packets per second of traffic on a port
--     - params["kbps"]         - setup maximum of kilobits per second of traffic on a port
--     - params["broadcast"]    - To enable rate limiting on a physical port for Broadcast packets
--     - params["known"]        - To enable rate limiting on a physical port for Known Unicast packets
--     - params["unknown"]      - To enable rate limiting on a physical port for Unknown Unicast packets
--     - params["unregistered"] - To enable rate limiting on a physical port for Unregistered Multicast packets
--     - params["registered"]   - To enable rate limiting on a physical port for Registered Multicast packets
--     - params["tcp-syn"]      - To enable rate limiting on a physical port for TCP SYN packets
--
--
--
--        @return       true on success, otherwise false and error message
--
local function stormControlEnableFunc(params)
    params.command_data = Command_Data()
    return generic_port_range_func(stormControlEnable, params)
end



-- ************************************************************************
--
--  per_device__stormControlRateLimitMode
--        @description  storm control global configuration per device
--                      If enable == true
--                         set rate limit mode (byte based or packet based)
--                         set ingress rate limit time window size to 1024
--                      If enable == false
--                         set rate limit mode to default (byte based)
--                         set default values to all time window types
--
--        @param params - "byte" or "packet" :
--
--        @return       true on success, otherwise false and error message
--
local function per_device__stormControlRateLimitMode(command_data, devNum, params)

    local did_error, result, values
    local brgRateLimitPtr

    local enable = true

    if (params.flagNo == true) then
        enable = false
    end

    -- Get rate limit global configurations parameters (call cpss API cpssDxChBrgGenRateLimitGlobalCfgGet)
    did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgGet",
    {
        {"IN", "GT_U8", "devNum", devNum},
        {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr"}
    })


    if enable == true then
        -- Set all types of time window to constant value of 1024 (micro sec units)
        brgRateLimitPtr = values["brgRateLimitPtr"]
        local windowSize = 1024;
        brgRateLimitPtr.win10Mbps   = windowSize
        brgRateLimitPtr.win100Mbps  = windowSize
        brgRateLimitPtr.win1000Mbps = windowSize
        brgRateLimitPtr.win10Gbps   = windowSize
        brgRateLimitPtr.win100Gbps  = windowSize

        -- Set rate limit mode according to user's params
        brgRateLimitPtr.rMode = params.rateLimitMode

    else  --- flag no

        -- Set all types of time window to default values
        brgRateLimitPtr = values["brgRateLimitPtr"]
        local function convertHwValueToSwValue(hwValue)
            local granularity = 256
            return (hwValue+1) * granularity
        end

        brgRateLimitPtr.win10Mbps   = convertHwValueToSwValue(0xF41) -- 0xF41 is the HW final value , but need to convert to SW value here so the CPSS will write 0xF41 to HW
        brgRateLimitPtr.win100Mbps  = convertHwValueToSwValue(0x185)
        brgRateLimitPtr.win1000Mbps = convertHwValueToSwValue(0x26)
        brgRateLimitPtr.win10Gbps   = convertHwValueToSwValue(0x27)
        brgRateLimitPtr.win100Gbps  = convertHwValueToSwValue(0x27)

        -- Set default rate limit mode (byte based)
        brgRateLimitPtr.rMode = "CPSS_RATE_LIMIT_BYTE_BASED_E"

    end  --- end for enable check

    -- Set rate limit global configurations parameters (call cpss API cpssDxChBrgGenRateLimitGlobalCfgSet)
    genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgSet",
    {
        {"IN", "GT_U8", "devNum", devNum},
        {"IN", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr", brgRateLimitPtr}
    })

end -- func


--
--  stormControlRateLimitModeFunc
--        @description  Set rate limit mode (byte based or packet based)
--
--  @param params   - parameters
--     - params["device"]   - Set device number
--     - params["rate_limit_mode"] - Set rate limit mode (byte or packet)
--
--        @return       true on success, otherwise false and error message
--
local function stormControlRateLimitModeFunc(params)
    generic_all_device_func(per_device__stormControlRateLimitMode,params)
end


--------------------------------------------------------------------------------
-- command registration: storm-control
--------------------------------------------------------------------------------
CLI_addHelp("interface", "storm-control", "Set Ingress Port Storm Rate Limit Enforcement")
CLI_addCommand("interface", "storm-control", {
    func   = stormControlEnableFunc,
    help   = "Enabled Ingress Port Storm Rate Limit Enforcement for current port",
    params = {
        { type="named",
           -- { format = "level %percent", help = "Suppression level in percentage" },
            { format = "pps %GT_U32", help = "Maximum of packets per second of traffic on a port. Must be used when rate limit mode is packet based." },
            { format = "kbps %GT_U32", help = "Maximum of kilobits per second of traffic on a port. Must be used when rate limit mode is byte based." },
            { format = "broadcast", help = "To enable rate limiting on a physical port for Broadcast packets"},
            { format = "known", help = "To enable rate limiting on a physical port for Known Unicast packets"},
            { format = "unknown", help = "To enable rate limiting on a physical port for Unknown Unicast packets"},
            { format = "unregistered", help = "To enable rate limiting on a physical port for Unregistered Multicast packets"},
            { format = "registered", help = "To enable rate limiting on a physical port for Registered Multicast packets"},
            { format = "tcp-syn", help = "To enable rate limiting on a physical port for TCP SYN packets"},

            --alt = { limit = {"level", "kbps"}},

            -- Set rate limit to be mandatory. User must select kbps units or pps units
            alt = { rateLimitVal = {"kbps", "pps"}},
            mandatory = {"rateLimitVal"}
        }
    }
})


--------------------------------------------------------------------------------
-- command registration: no storm-control
--------------------------------------------------------------------------------
CLI_addHelp("interface", "no storm-control", "Disable Ingress Port Storm Rate Limit Enforcement")
CLI_addCommand("interface", "no storm-control", {
    func   = function(params)
    params.flagNo = true
    return stormControlEnableFunc(params) --need add storm enable
    end,
    help   = "Disable Ingress Port Storm Rate Limit Enforcement for current Port",
})



--------------------------------------------------------------------------------
-- command registration: storm-control rate-limit-mode
--------------------------------------------------------------------------------
CLI_addHelp("config", "storm-control rate-limit-mode", "Set rate limit mode (packet based or byte based)")
CLI_addCommand("config", "storm-control rate-limit-mode", {
    func=stormControlRateLimitModeFunc,
    help   = "Set rate limit mode (packet based or byte based) per device",
    params = {
        { type="values",
            { format = "%rateLimitMode",  name="rateLimitMode",  help="ingress rate limit mode"},

            -- Set rate limit mode to be mandatory. User must select byte mode or packet mode
            mandatory = {"rateLimitMode"}
        },
        { type="named",
            { format = "device %devID",name="devID", help="device number"}
        }
    }
})


--------------------------------------------------------------------------------
-- command registration: no storm-control rate-limit-mode
--------------------------------------------------------------------------------
CLI_addHelp("config", "no storm-control rate-limit-mode", "Set rate limit mode to default (byte based). Set time windows to default values")
CLI_addCommand("config", "no storm-control rate-limit-mode", {
    func   = function(params)
    params.flagNo = true
    return stormControlRateLimitModeFunc(params)
    end,
    help   = "Per device, set rate limit mode to default (byte based). Set default values to all time windows.",
    params = {
        { type="named",
            { format = "device %devID",name="devID", help="device number"},
        }
    }
})
