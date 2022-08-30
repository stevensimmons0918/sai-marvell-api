--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlPower.lua
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
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_power_get_port_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_power_get_hw_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_power_run_validation")


--------------------------------------------
-- Local functions: 
--------------------------------------------
-- show functions

local function pdl_power_show_info(params)
  local api_result=1
    api_result = wr_utils_pdl_power_get_port_info(params["power-type"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_power_show_hw_info(params)
  local api_result=1
    api_result = wr_utils_pdl_power_get_hw_info(params["power-type"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_power_run_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_power_run_validation(params["power-type"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show power",  "Show power information")
CLI_addHelp("pdl_test",    "show power hw",  "Show power hw information")

--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["powerType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Power types or all",
    enum = {
        ["ps"] = { value="ps", help="ps power type" },
        ["rps"] =  { value="rps", help="rps power type" },  
        ["all"] =  { value="all", help="all power type" }       
    }
}


--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show power info", {
    func   = pdl_power_show_info,
    help   = "Show power related XML information",
    
--          Examples:
--      show power info ps
--      power-id |  active-info  | interface-type |  interface-id |
--      -------- | ------------- | -------------- | ------------- |
--      ps       | 1             | i2c            | 23            |
--
--  I2C Bus-Id | Address | Access Size |  Offset  |    Mask    | Mux Count |
--  ---------- | ------- | ----------- | -------- | ---------- | --------- |
--  0          | 0x41    | two-bytes   | 0x0009   | 0x00000001 | 0         |


--      show power info all
--      power-id |  active-info  | interface-type |  interface-id |
--      -------- | ------------- | -------------- | ------------- |
--      ps       | 1             | i2c            | 23            |
--      rps      | Not supported | Not supported  | Not supported |


    params = {
    {
      type = "values",
      {format="%powerType", name="power-type", help="Power type" }, 
    },
    mandatory = {"power-type"},
  }
})


CLI_addCommand("pdl_test", "show power hw", {
    func   = pdl_power_show_hw_info,
    help   = "Show power supply hw information",
    
--          Examples:
--      show power hw ps
--      power-id |  power-status  |   rps-status   |
--      -------- | -------------- | -------------- |
--      ps       | Active         | ready          |        

--      show power hw all
--      power-id |  power-status  |   rps-status   |
--      -------- | -------------- | -------------- |
--      all      | Active         | ready          |
--      rps      | Not supported  | Not supported  |


    params = {
    {
      type = "values",
      {format="%powerType", name="power-type", help="Power type" }, 
    },
    mandatory = {"power-type"},
  }
})


CLI_addCommand("pdl_test", "run power validation", {
    func   = pdl_power_run_validation,
    help   = "Run power validation - verify  all power supply status can be read",
    
--          Examples:   
--      run power validation ps
--      power-id |      Info      |     status     |
--      -------- | -------------- | -------------- |
--      ps       | Pass           | Pass           |
        
--      run power validation all
--      power-id |      Info      |     status     |
--      -------- | -------------- | -------------- |
--      ps       | Pass           | Pass           |
--      rps      | Not supported  | Not supported  |


    params = {
    {
      type = "values",
      {format="%powerType", name="power-type", help="Power type" }, 
    },
    mandatory = {"power-type"},
  }
})


