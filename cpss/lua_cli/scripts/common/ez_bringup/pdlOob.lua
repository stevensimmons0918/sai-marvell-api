    --********************************************************************************
--*              (c), Copyright 2018 Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlOob.lua
--*
--* DESCRIPTION:
--*       setting of pdl commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------
-- Parameters registration: 
--------------------------------------------

--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "OOB debug messages",
    enum = {
        ["enable"]  = { value=1, help="Enable Out-Of-Band debug messages" },
        ["disable"] = { value=0, help="Disable Out-Of-Band debug messages" }
    }
}

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_debug")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_info_get")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_count_get")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_eth_id")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_cpu_port")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_max_speed")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_phy_id")
cmdLuaCLI_registerCfunction("wr_utils_pdl_oob_set_phy_position")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_run_oob_validation")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_oob_set_debug(params)       
  local api_result=1
    api_result = wr_utils_pdl_oob_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_get_info(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_info_get(params["oob-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_get_num(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_count_get()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_set_eth_id(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_set_eth_id(params["oob-id"], params["eth-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_set_cpu_port(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_set_cpu_port(params["oob-id"], params["cpu-port"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_set_max_speed(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_set_max_speed(params["oob-id"], params["speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_oob_set_phy_id(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_set_phy_id(params["oob-id"], params["phy-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_oob_set_phy_position(params)
  local api_result=1
    api_result = wr_utils_pdl_oob_set_phy_position(params["oob-id"], params["phy-pos"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_network_run_oob_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_network_run_oob_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end
--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show oob",  "Show Out-Of-Band port Information")
CLI_addHelp("pdl_test",    "set oob",  "Show Out-Of-Band Configuration")

CLI_type_dict["speedType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Speed types",
    enum = {
        ["10M"]  = { value="10M" , help = "10M  speed type" },
        ["100M"] = { value="100M", help = "100M speed type" },
        ["1G"]   = { value="1G"  , help = "1G   speed type" },
        ["2.5G"] = { value="2.5G", help = "2.5G speed type" },
        ["5G"]   = { value="5G"  , help = "5G   speed type" },
        ["10G"]  = { value="10G" , help = "10G  speed type" },
        ["20G"]  = { value="20G" , help = "20G  speed type" },
        ["24G"]  = { value="24G" , help = "24G  speed type" },
        ["25G"]  = { value="25G" , help = "25G  speed type" },
        ["40G"]  = { value="40G" , help = "40G  speed type" },
        ["50G"]  = { value="50G" , help = "50G  speed type" },
        ["100G"] = { value="100G", help = "100G speed type" }
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "set oob debug", {
    func   = pdl_oob_set_debug,
    help   = "toggle OOB debug messages on/off",    
  params = {
    {
      type = "values",
      {
        format="%debugSetType", 
        name="set-id", 
        help="Value"
      },

      mandatory = { "set-id" }
    }
  }
})

CLI_addCommand("pdl_test", "show oob info",
{
    func   = pdl_oob_get_info,
    help   = "show OOB XML information",
    params =
    {
        {
            type = "values",
            {
                format="%string",
                name="oob-id",
                help="oob port number | all"
            },
            mandatory={"oob-id"}
        }
    }
})

CLI_addCommand("pdl_test", "show oob count", {
    func   = pdl_oob_get_num,
    help   = "get number of OOB ports in system",
--      Examples:
--      show oob count
--      Number of Oob ports: 0

})

CLI_addCommand("pdl_test", "set oob ethernet-id", {
    func   = pdl_oob_set_eth_id,
    help   = "change OOB port linux ethernet id",    
    params = {
        { type = "values",
            {format="%integer", name="oob-id", help="oob port id"},
            {format="%integer", name="eth-id", help="linux eth-id of the port"},
            mandatory={"oob-id", "eth-id"}
        }
    }
})

CLI_addCommand("pdl_test", "set oob cpu-port", {
    func   = pdl_oob_set_cpu_port,
    help   = "change number of cpu port that's connected to OOB port",    
    params = {
        { type = "values",
            {format="%integer", name="oob-id", help="oob port id"},
            {format="%integer", name="cpu-port", help="cpu port oob connected to"},
            mandatory={"oob-id", "cpu-port"}
        }
    }
})

CLI_addCommand("pdl_test", "set oob max-speed", {
    func   = pdl_oob_set_max_speed,
    help   = "change maximum speed supported by OOB port",    
    params = {
        { type = "values",
            {format="%integer", name="oob-id", help="oob port id"},
            {format="%speedType", name="speed", help="Value"},
            mandatory={"oob-id", "speed"}
        }
    }
})

CLI_addCommand("pdl_test", "set oob phy number", {
    func   = pdl_oob_set_phy_id,
    help   = "set phy number for OOB port",
  params = {
    { type = "values",
            {format="%integer", name="oob-id", help="oob port id"},
            {format="%integer", name="phy-id", help="phy number" },
            mandatory={"oob-id", "phy-id"}
    }
  }
})

CLI_addCommand("pdl_test", "set oob phy position", {
    func   = pdl_oob_set_phy_position,
    help   = "set phy position for OOB port",
  params = {
    { type = "values",
            {format="%integer", name="oob-id", help="oob port id"},
            {format="%integer", name="phy-pos", help="position in phy" },
            mandatory={"oob-id", "phy-pos"}
    }
  }
})



CLI_addCommand("pdl_test", "run oob validation all", {
    func   = pdl_network_run_oob_validation,
    help   = "Run OOB validation - validate all OOB information can be retrieved from XML",
--      Examples:
--      run oob validation all
--        oob Id   |         oob Info          |
--      ---------- | ------------------------- |
--                 | Not supported             |
    
})

  
