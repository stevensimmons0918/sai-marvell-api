--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlPHY.lua
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
cmdLuaCLI_registerCfunction("wr_pdl_phy_show_info")
cmdLuaCLI_registerCfunction("wr_pdl_phy_show_count")
cmdLuaCLI_registerCfunction("wr_pdl_phy_set_debug")
cmdLuaCLI_registerCfunction("wr_pdl_phy_run_validation")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_phy_get_info(params)
  local api_result=1
    api_result = wr_pdl_phy_show_info(params["phy-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_phy_get_info_all(params)
  local api_result=1
    api_result = wr_pdl_phy_show_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_phy_get_count(params)
  local api_result=1
    api_result = wr_pdl_phy_show_count()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_phy_set_debug(params)
  local api_result=1
    api_result = wr_pdl_phy_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_phy_run_validation(params)
  local api_result=1
    api_result = wr_pdl_phy_run_validation(params["phy-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_phy_run_all_validation(params)
  local api_result=1
    api_result = wr_pdl_phy_run_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show phy",   "Show PHY information")
CLI_addHelp("pdl_test",    "show phy info",   "Show PHY information")
CLI_addHelp("pdl_test",    "show phy number", "Show number of PHYs")
CLI_addHelp("pdl_test",    "set phy debug",   "Set PHY debug")
CLI_addHelp("pdl_test",    "run phy validation",   "Run PHY validation")


--------------------------------------------
-- Types registration: 
--------------------------------------------

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PHY debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------

CLI_addCommand("pdl_test", "show phy info all", {
    func   = pdl_phy_get_info_all,
    help   = "show phy xml information",
    params = {}

--      Examples:   
--      show phy info all
--       PHY ID    | PHY Type        | Download Type | Connected Ports (dev/port)
--       --------- | --------------- | ------------- | --------------------------
--       0         | alaska-88E1680L | Flash         |  1/29  1/31  1/25  1/27
--                 |                 |               |  1/28  1/30  1/24  1/26
--       1         | alaska-88E1680L | Flash         |  1/21  1/23  1/17  1/19
--                 |                 |               |  1/20  1/22  1/16  1/18
--       2         | alaska-88E1680L | Flash         |  1/13  1/15  0/13  0/15
--                 |                 |               |  1/12  1/14  0/12  0/14
--       3         | alaska-88E1680L | Flash         |  0/29  0/31  0/25  0/27
--                 |                 |               |  0/28  0/30  0/24  0/26
--       4         | alaska-88E1680L | Flash         |  0/21  0/23  0/17  0/19
--                 |                 |               |  0/20  0/22  0/16  0/18
--       5         | alaska-88E2180  | Flash         |  0/09  0/11  1/09  1/11
--                 |                 |               |  0/08  0/10  1/08  1/10
--       6         | alaska-88E33X0  | Flash         |  0/00
--       7         | alaska-88E33X0  | Flash         |  0/01


})


CLI_addCommand("pdl_test", "show phy info", {
    func   = pdl_phy_get_info,
    help   = "show phy xml information",
    
--      Examples: 
--      show phy info 0
--      PHY ID    | PHY Type        | Download Type | Connected Ports (dev/port)
--      --------- | --------------- | ------------- | --------------------------
--      0         | alaska-88E1680L | Flash         |  1/29  1/31  1/25  1/27
--                |                 |               |  1/28  1/30  1/24  1/26
         
    params = {
    { 
        type = "values",
        {format="%integer", name="phy-id", help="phy id" },
        mandatory={"phy-id"}
      }
    }
})


CLI_addCommand("pdl_test", "show phy number", {
    func   = pdl_phy_get_count,
    help   = "get number of phys in system",

--      Examples:
--      show phy number
--      Number of phys: 8
    
    params = {}
})


CLI_addCommand("pdl_test", "set phy debug", {
    func   = pdl_phy_set_debug,
    help   = "toggle phy debug messages on/off",
    params = {
    { 
      type = "values",
      {format = "%debugSetType", name = "set-id", help="set-id"},
      mandatory = { "set-id" },
    }}
})


CLI_addCommand("pdl_test", "run phy validation", {
    func   = pdl_phy_run_validation,
    help   = "Run phy validation - validate all phy information can be retrieved from XML",
    
--      Examples:   
--      run phy validation 1
--         PHY ID   |     PHY TYPE     |       INFO       |
--       ---------- | ---------------- | ---------------- |
--       1          | alaska-88E1680L  | Pass             |
--      

    params = {
    {
      type = "values",
      {format="%integer", name="phy-number", help="phy-id"}, 
    },
    mandatory = {"phy-number"},
  }
})



CLI_addCommand("pdl_test", "run phy validation all", {
    func   = pdl_phy_run_all_validation,
    help   = "Run phy validation - validate all phy information can be retrieved from db",
        
--      Examples:       
--      run phy validation all
--         PHY ID   |     PHY TYPE     |       INFO       |
--       ---------- | ---------------- | ---------------- |
--       0          | alaska-88E1680L  | Pass             |
--       1          | alaska-88E1680L  | Pass             |
--       2          | alaska-88E1680L  | Pass             |
--       3          | alaska-88E1680L  | Pass             |
--       4          | alaska-88E1680L  | Pass             |
--       5          | alaska-88E2180   | Pass             |
--       6          | alaska-88E33X0   | Pass             |
--       7          | alaska-88E33X0   | Pass             |
        
})
