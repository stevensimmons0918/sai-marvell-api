--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* log.lua
--*
--* DESCRIPTION:
--*       enable/disable CPSS log
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

-- ************************************************************************
---
--  log_func
--        @description  enable the CPSS log
--
--        @param params The parameters
--
--        @return       true if there was no error otherwise false
--
local function enable_func(params)
    if (params.flagNo) then
        myGenWrapper("cpssLogEnableSet", {
                    { "IN", "GT_BOOL", "enable", false }
                })
    else
        myGenWrapper("cpssLogEnableSet", {
                    { "IN", "GT_BOOL", "enable", true }
                })
    end
end

local function lib_configuration_func(params)
    local status, err, result
    local defaultType = "CPSS_LOG_TYPE_ALL_E"    --default value
    local enable
    local currLib = params.libName

    if (params.typeName ~= nil) then
        defaultType = params.typeName
    end

    myGenWrapper("cpssLogEnableSet", {
                { "IN", "GT_BOOL", "enable", true }
            })

    if (params.flagNo) then
        enable = false
    else
        enable = true
    end

    result = myGenWrapper("cpssLogLibEnableSet", {
                { "IN", "CPSS_LOG_LIB_ENT", "lib", params.libName },
                { "IN", "CPSS_LOG_TYPE_ENT", "type", defaultType },
                { "IN", "GT_BOOL", "enable", enable }
            })

    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


local function format_func(params)
    local defaultType = "CPSS_LOG_API_FORMAT_NO_PARAMS_E"    --default value

    if (params.flagNo) then
        myGenWrapper("cpssLogApiFormatSet", {
                    { "IN", "CPSS_LOG_API_FORMAT_ENT", "format", defaultType }
                })
    else
        myGenWrapper("cpssLogApiFormatSet", {
                    { "IN", "CPSS_LOG_API_FORMAT_ENT", "enable", params.formatName }
                })
    end
end

--
-- note this must be 1:1 with the enum of CPSS_LOG_LIB_ENT in CPSS H file !!!
--
local cpssLogLibs =
{
 "CPSS_LOG_LIB_APP_DRIVER_CALL_E"
,"CPSS_LOG_LIB_BRIDGE_E"
,"CPSS_LOG_LIB_CNC_E"
,"CPSS_LOG_LIB_CONFIG_E"
,"CPSS_LOG_LIB_COS_E"
,"CPSS_LOG_LIB_HW_INIT_E"
,"CPSS_LOG_LIB_CSCD_E"
,"CPSS_LOG_LIB_CUT_THROUGH_E"
,"CPSS_LOG_LIB_DIAG_E"
,"CPSS_LOG_LIB_FABRIC_E"
,"CPSS_LOG_LIB_IP_E"
,"CPSS_LOG_LIB_IPFIX_E"
,"CPSS_LOG_LIB_IP_LPM_E"
,"CPSS_LOG_LIB_L2_MLL_E"
,"CPSS_LOG_LIB_LOGICAL_TARGET_E"
,"CPSS_LOG_LIB_LPM_E"
,"CPSS_LOG_LIB_MIRROR_E"
,"CPSS_LOG_LIB_MULTI_PORT_GROUP_E"
,"CPSS_LOG_LIB_NETWORK_IF_E"
,"CPSS_LOG_LIB_NST_E"
,"CPSS_LOG_LIB_OAM_E"
,"CPSS_LOG_LIB_PCL_E"
,"CPSS_LOG_LIB_PHY_E"
,"CPSS_LOG_LIB_POLICER_E"
,"CPSS_LOG_LIB_PORT_E"
,"CPSS_LOG_LIB_PROTECTION_E"
,"CPSS_LOG_LIB_PTP_E"
,"CPSS_LOG_LIB_SYSTEM_RECOVERY_E"
,"CPSS_LOG_LIB_TCAM_E"
,"CPSS_LOG_LIB_TM_GLUE_E"
,"CPSS_LOG_LIB_TRUNK_E"
,"CPSS_LOG_LIB_TTI_E"
,"CPSS_LOG_LIB_TUNNEL_E"
,"CPSS_LOG_LIB_VNT_E"
,"CPSS_LOG_LIB_RESOURCE_MANAGER_E"
,"CPSS_LOG_LIB_VERSION_E"
,"CPSS_LOG_LIB_TM_E"
,"CPSS_LOG_LIB_SMI_E"
,"CPSS_LOG_LIB_INIT_E"
,"CPSS_LOG_LIB_DRAGONITE_E"
,"CPSS_LOG_LIB_VIRTUAL_TCAM_E"
,"CPSS_LOG_LIB_INGRESS_E"
,"CPSS_LOG_LIB_EGRESS_E"
,"CPSS_LOG_LIB_LATENCY_MONITORING_E"
,"CPSS_LOG_LIB_TAM_E"
,"CPSS_LOG_LIB_EXACT_MATCH_E"
,"CPSS_LOG_LIB_PHA_E"
,"CPSS_LOG_LIB_PACKET_ANALYZER_E"
,"CPSS_LOG_LIB_FLOW_MANAGER_E"
,"CPSS_LOG_LIB_BRIDGE_FDB_MANAGER_E"
,"CPSS_LOG_LIB_I2C_E"
,"CPSS_LOG_LIB_PPU_E"
,"CPSS_LOG_LIB_EXACT_MATCH_MANAGER_E"
,"CPSS_LOG_LIB_MAC_SEC_E"
,"CPSS_LOG_LIB_PTP_MANAGER_E"
,"CPSS_LOG_LIB_HSR_PRP_E"
,"CPSS_LOG_LIB_STREAM_E"
,"CPSS_LOG_LIB_IPFIX_MANAGER_E"
,"CPSS_LOG_LIB_ALL_E"
}

local function convert_lib(n)
    --LUA array is 1 based , cpss enum is 0 based  (so use 'n+1')
    local libString = cpssLogLibs[n+1]

    if libString == nil then
        return "CPSS_LOG_LIB_ALL_E"
    end

    return libString
end


local function convert_type(n)
    if n==0 then
        return "CPSS_LOG_TYPE_INFO_E"
    elseif n==1 then
        return "CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E"
    elseif n==2 then
        return "CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E"
    elseif n==3 then
        return "CPSS_LOG_TYPE_DRIVER_E"
    elseif n==4 then
        return "CPSS_LOG_TYPE_ERROR_E"
    elseif n==5 then
        return "CPSS_LOG_TYPE_ALL_E"
    end
end


local function show_status_func(params)
    local result, index, values, j, x, y, z

    result = myGenWrapper("cpssLogEnableGet", {})
    if (result ~= 0) then
        print ("Global status:  enabled")
    else
        print ("Global status:  disabled")
    end

    result = myGenWrapper("cpssLogApiFormatGet", {})
    if (result == 0) then
        print ("Format:         no-params")
    elseif (result == 1) then
        print ("Format:         all-params")
    elseif (result == 2) then
        print ("Format:         non-zero-params")
    end

    result = myGenWrapper("cpssLogStackSizeGet", {})
    print("stack size:     " .. result .. "\n")

    print ("Lib enabled status:")
    x = "Lib                         info       entry-level  non-entry-level     driver         error\n" ..
        "----------------          --------     -----------  ---------------    --------       --------"
    print (x)

    i = 0
    while (true) do
        x = convert_lib(i)
        if (x == "CPSS_LOG_LIB_ALL_E") then
            break
        end

        x = string.sub(x, 14, -3)
        y = string.len(x)

        if (y < 26) then
            for z = 1, (26 - y), 1 do
                x = x .. " "
            end
        end

        for j = 0, 4, 1 do
            result, values = myGenWrapper("cpssLogLibEnableGet", {
                        { "IN", "CPSS_LOG_LIB_ENT", "lib", convert_lib(i) },
                        { "IN", "CPSS_LOG_TYPE_ENT", "type", convert_type(j) },
                        { "OUT", "GT_BOOL", "enablePtr" }
                    })

            if (values["enablePtr"]) then
                x = x .. "enabled "
            else
                x = x .. "disabled"
            end

            x = x .. "       "
        end

        print(x)

        i = i + 1
    end
end


local function stack_size_func(params)
    local result, status, err, size

    if (params.flagNo) then
        size = tonumber(params.stackSize)
    else
        size = 1
    end

    result = myGenWrapper("cpssLogStackSizeSet", {
                { "IN", "GT_U32", "size", size }
            })
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


--------------------------------------------------------------------
-- command registration: trace cpss-log/no trace cpss-log
--------------------------------------------------------------------
CLI_addHelp("debug", "cpss-log", "enable CPSS LOG")
CLI_addCommand("debug", "trace cpss-log", {
    func   = enable_func,
    help   = "enable CPSS LOG",
    params = {}
})

CLI_addCommand("debug", "no trace cpss-log", {
    func   = function(params)
                 params.flagNo = true
                 return enable_func(params)
             end,
    help   = "disable CPSS log",
    params = {}
})

----------------------------------------------------------------------
-- command registration: trace cpss-log lib /no trace cpss-log lib
----------------------------------------------------------------------
CLI_addCommand("debug", "trace cpss-log lib", {
    func   = lib_configuration_func,
    help   = "enable logging of specific info for specific or all libs",
    params = {
        {
            type = "values",
            {
                format = "%log_lib",
                name   = "libName"
            }
        },
        {
            type = "named",
            {
                format = "log-type %log_type",
                name   = "typeName"
            },
            mandatory = { "libName" }
        }
    }
})

CLI_addCommand("debug", "no trace cpss-log lib", {
    func   = function(params)
                 params.flagNo = true
                 return lib_configuration_func(params)
             end,
    help   = "disable logging of specific info for specific or all libs",
    params = {
        {
            type = "values",
            {
                format = "%log_lib",
                name   = "libName"
            }
        },
        {
            type = "named",
            {
                format = "log-type %log_type",
                name   = "typeName"
            },
            mandatory = { "libName" }
        }
    }
})

-------------------------------------------------------------------------
-- command registration: trace cpss-log format /no trace cpss-log format
-------------------------------------------------------------------------
CLI_addCommand("debug", "trace cpss-log format", {
    func   = format_func,
    help   = "configure format of CPSS log",
    params = {
        {
            type = "named",
            {
                format = "%log_format",
                name   = "formatName"
            }
        }
    }
})

CLI_addCommand("debug", "no trace cpss-log format", {
    func   = function(params)
                 params.flagNo = true
                 return format_func(params)
             end,
    help   = "set CPSS log format - no params",
    params = {
        {
            type = "named",
            {
                format = "%log_format",
                name   = "formatName"
            }
        }
    }
})

--------------------------------------------------------------------
-- command registration: trace cpss-log/no trace cpss-log
--------------------------------------------------------------------
CLI_addHelp("debug", "show cpss-log status", "Show the CPSS log status")
CLI_addCommand("debug", "show cpss-log status", {
    func   = show_status_func,
    help   = "show the CPSS log status including per-lib configuration",
    params = {}
})

--------------------------------------------------------------------
-- command registration: trace cpss-log stack size
--------------------------------------------------------------------
CLI_addHelp("debug", "trace cpss-log stack size", "Set the CPSS log stack size")
CLI_addCommand("debug", "trace cpss-log stack size", {
    func   = function(params)
                 params.flagNo = true
                 return stack_size_func(params)
             end,
    help   = "Set the CPSS log stack size",
    params = {
        {
            type = "named",
            {
                format = "%GT_U32",
                name   = "stackSize"
            }
        }
    }
})
