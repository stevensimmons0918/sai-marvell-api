--********************************************************************************
--*              (c), Copyright 2018 Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlCpu.lua
--*
--* DESCRIPTION:
--*       setting of pdl commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_cpu_get")
cmdLuaCLI_registerCfunction("wr_utils_pdl_cpu_set_debug")
cmdLuaCLI_registerCfunction("wr_utils_pdl_cpu_set_type")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_cpu_get(params)
  local api_result=1
    api_result = wr_utils_pdl_cpu_get()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end  
    return true     
end

local function pdl_cpu_set_debug(params)
  local api_result=1
    api_result = wr_utils_pdl_cpu_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end  
    return true   
end

local function pdl_cpu_set_type(params)
  local api_result=1
    api_result = wr_utils_pdl_cpu_set_type(params["cpu-type"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end  
    return true   
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "set cpu",   "Set CPU Configuration")
CLI_addHelp("pdl_test",    "show cpu",  "Show CPU Information")

--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

CLI_type_dict["cpuType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "CPU types",
    enum = {
        ["xp-embedded"]    = { value="xp-embedded"   , help = "xp-embedded cpu type" },
        ["armada-MV78130"] = { value="armada-MV78130", help = "armada-MV78130 cpu type" },
        ["armada-MV78160"] = { value="armada-MV78160", help = "armada-MV78160 cpu type" },
        ["armada-MV78230"] = { value="armada-MV78230", help = "armada-MV78230 cpu type" },
        ["armada-MV78260"] = { value="armada-MV78260", help = "armada-MV78260 cpu type" },
        ["armada-MV78460"] = { value="armada-MV78460", help = "armada-MV78460 cpu type" },
        ["armada-88F6810"] = { value="armada-88F6810", help = "armada-88F6810 cpu type" },
        ["armada-88F6811"] = { value="armada-88F6811", help = "armada-88F6811 cpu type" },
        ["armada-88F6820"] = { value="armada-88F6820", help = "armada-88F6820 cpu type" },
        ["armada-88F6821"] = { value="armada-88F6821", help = "armada-88F6821 cpu type" },
        ["armada-88F6W21"] = { value="armada-88F6W21", help = "armada-88F6W21 cpu type" },
        ["armada-88F6828"] = { value="armada-88F6828", help = "armada-88F6828 cpu type" }
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show cpu info", {
    func   = pdl_cpu_get,
    help   = "show cpu type",
--      Examples:   
--      show cpu info
--      CPU Type  : armada-88F6820
--      CPU Family: A38X

})

CLI_addCommand("pdl_test", "set cpu debug", {
    func   = pdl_cpu_set_debug,
    help   = "toggle CPU debug messages on/off",    
  params = {
    {
      type = "values",
      {format = "%debugSetType", name = "set-id", help= "set-id" },
      mandatory = { "set-id" },
    }
  }
})

CLI_addCommand("pdl_test", "set cpu type", {
    func   = pdl_cpu_set_type,
    help   = "Set CPU type",    
--          Examples:
--          set cpu type armada-88F6810
--          set cpu type armada-MV78130
--
  params = {
    {
      type = "values",
      {format = "%cpuType", name = "cpu-type", help= "cpu-type" },
      mandatory = { "cpu-type" },
    }
  }
})
