--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlSerdes.lua
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
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_get_b2b")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_get_front_panel")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_run_validation")
-- cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_get_cpss_reg")
-- cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_get_cpss_regs")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_set_debug")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_set_front_panel")
cmdLuaCLI_registerCfunction("wr_utils_pdl_pp_set_b2b")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_pp_get_info(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_get_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_pp_get_b2b(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_get_b2b(params["b2b"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_pp_get_b2b_all(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_get_b2b()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_pp_get_front_panel(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_get_front_panel(params["fp-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true         
end

local function pdl_pp_get_front_panel_all(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_get_front_panel()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true         
end

local function pdl_pp_run_validation_all(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_run_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


-- local function pdl_pp_get_cpss_reg(params)
--   local api_result=1
--     api_result = wr_utils_pdl_pp_get_cpss_reg(params["dev"], params["reg-addr"], params["reg-mask"])
--     if api_result ~= 0 then
--       printMsg ("command failed rc = "..api_result)
--     end
--     return true         
-- end
-- 
-- local function pdl_pp_get_cpss_reg_mask(params)
--   local api_result=1
--     api_result = wr_utils_pdl_pp_get_cpss_reg(params["dev"], params["reg-addr"], params["reg-mask"])
--     if api_result ~= 0 then
--       printMsg ("command failed rc = "..api_result)
--     end
--     return true         
-- end
-- 
-- local function pdl_pp_get_cpss_regs(params)
--   local api_result=1
--     api_result = wr_utils_pdl_pp_get_cpss_regs(params["dev"], params["start-reg-addr"], params["count"])
--     if api_result ~= 0 then
--       printMsg ("command failed rc = "..api_result)
--     end
--     return true         
-- end

local function pdl_pp_set_debug(params) 
  local api_result=1
    api_result = wr_utils_pdl_pp_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true     
end

local function pdl_pp_set_front_panel(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_set_front_panel(params["fp-id"], params["ordering"], params["max-speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true         
end

local function pdl_pp_set_b2b(params)
  local api_result=1
    api_result = wr_utils_pdl_pp_set_b2b(params["b2b"],
                                         params["max-speed"],
                                         params["interface-mode"],
                                         params["first-device"],
                                         params["first-port"],
                                         params["second-device"],
                                         params["second-port"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show packet-processor",  "Show Packet-Processor information")
CLI_addHelp("pdl_test",    "run packet-processor validation",  "Run Packet-Processor validation")

--------------------------------------------
-- Types registration: 
--------------------------------------------

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Packet-processor debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

CLI_type_dict["groupOrder"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Front-panel port ordering",
    enum = {
        ["RightDown"] = { value="RightDown", help = "Ports ordered in first-right-then-down manner" },
        ["DownRight"]  = { value="DownRight", help = "Ports ordered in first-down-then-right manner" },
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show packet-processor info", {
    func   = pdl_pp_get_info,
    help   = "show packet-processor XML information",
--      Examples:   
--      show packet-processor info
--      Packet processors   | Back-to-back Links | Front panels
--      -----------------   | ------------------ | ------------
--      2                   | 3                  | 3
    
  params = {}
})

CLI_addCommand("pdl_test", "show packet-processor b2b", {
    func   = pdl_pp_get_b2b,
    help   = "show back-to-back XML information",
--      Examples:
--      show packet-processor b2b 1
--      B2B LINK ID | FIRST DEVICE | FIRST PORT | SECOND DEVICE | SECOND PORT | MAX SPEED | INTERFACE MODE
--      ----------- | ------------ | ---------- | ------------- | ----------- | --------- | --------------
--      1           | 0            | 4          | 1             | 4           | 24G       | XHGS
--      show packet-processor b2b all
--      B2B LINK ID | FIRST DEVICE | FIRST PORT | SECOND DEVICE | SECOND PORT | MAX SPEED | INTERFACE MODE
--      ----------- | ------------ | ---------- | ------------- | ----------- | --------- | --------------
--      0           | 0            | 2          | 1             | 2           | 24G       | XHGS
--      1           | 0            | 4          | 1             | 4           | 24G       | XHGS
--      2           | 0            | 6          | 1             | 6           | 24G       | XHGS
      
  params = {
    { type = "values",
      {format="%integer", name="b2b", help="back-to-back link id" },
      mandatory={"b2b"}
    }
  }
})

CLI_addCommand("pdl_test", "show packet-processor b2b all", {
    func   = pdl_pp_get_b2b_all,
    help   = "show back-to-back XML information",
  params = {}
})

CLI_addCommand("pdl_test", "show packet-processor front-panel", {
    func   = pdl_pp_get_front_panel,
    help   = "show front-panel XML information",
--       Examples:
--       show packet-processor front-panel 1
--       FRONT PANEL ID | GROUP ORDERING | MAX SPEED | FIRST PORT NUMBER SHIFT | NUMBER OF PORTS IN GROUP
--       -------------- | -------------- | --------- | ----------------------- | ------------------------
--       1              | RightDown      | 1G        | 0                       | 40
--       show packet-processor front-panel all
--       FRONT PANEL ID | GROUP ORDERING | MAX SPEED | FIRST PORT NUMBER SHIFT | NUMBER OF PORTS IN GROUP
--       -------------- | -------------- | --------- | ----------------------- | ------------------------
--       1              | RightDown      | 1G        | 0                       | 40
--       2              | RightDown      | 5G        | 0                       | 8
--       3              | DownRight      | 10G       | 48                      | 4

  params = {
    { type = "values",
      {format="%integer", name="fp-id", help="front-panel id" },
      mandatory={"fp-id"}
    }
  }
})

CLI_addCommand("pdl_test", "show packet-processor front-panel all", {
    func   = pdl_pp_get_front_panel_all,
    help   = "show front-panel XML information",

  params = {}
})

-- CLI_addCommand("pdl_test", "show packet-processor cpss-reg", {
--     func   = pdl_pp_get_cpss_reg,
--     help   = "Usage Example: show packet-processor cpss-reg <dev> <reg-addr> [reg-mask]",
--   params = {
--     { type = "values",
--       {format="%integer", name="dev", help="device number" },
--       { format="register %GT_U32_hex", name="reg-addr", help="The register address to read from (hex)" },
--       { format="mask %GT_U32_hex", name="reg-mask", help="The mask of the data to read (hex)" },
--       mandatory={"dev", "reg-addr"}
--     }
--   }
-- })
-- 
-- CLI_addCommand("pdl_test", "show packet-processor cpss-regs", {
--     func   = pdl_pp_get_cpss_regs,
--     help   = "Usage Example: show packet-processor cpss-regs <dev> <start-reg-addr> <count>",
--   params = {
--     { type = "values",
--       {format="%integer", name="dev", help="device number" },
--       { format="register %GT_U32_hex", name="start-reg-addr", help="Address of first register address to read from (hex)" },
--       { format="%integer", name="count", help="Number of registers to read" },
--       mandatory={"dev", "start-reg-addr", "count"}
--     }
--   }
-- })

CLI_addCommand("pdl_test", "set packet-processor debug", {
    func   = pdl_pp_set_debug,
    help   = "toggle packet-processor debug messages on/off",
    
  params = {
    {
      type = "values",
      {format = "%debugSetType", name = "set-id", help="set-id"},
      mandatory = { "set-id" },

    }
  }
})


CLI_addCommand("pdl_test", "set packet-processor front-panel", {
    func   = pdl_pp_set_front_panel,
    help   = "set front-panel db information",
    params = {
    { type = "values",
      {format="%integer", name="fp-id", help="front-panel id" }
    },
    {
      type = "named",
      {format = "group-ordering %groupOrder", name = "ordering", help="group-ordering <order>"},
      {format = "max-speed %speedType", name = "max-speed", help="max-speed <speed-type>"}
    },
    mandatory = { "fp-id" },
  }
})


CLI_addCommand("pdl_test", "set packet-processor b2b", {
    func   = pdl_pp_set_b2b,
    help   = "set back-to-back db information",
  params = {
    { type = "values",
      {format="%integer", name="b2b", help="back-to-back link id" },
    },
    {
      type = "named",
      {format = "max-speed %speedType", name = "max-speed",  help="max-speed <speed-type>"},
      {format = "interface-mode %ifModeType", name = "interface-mode",  help="interface-mode <mode>"},
      {format = "first-device %integer", name = "first-device", help="first-device <number>" },
      {format = "first-port %integer", name = "first-port", help="first-port <number>" },
      {format = "second-device %integer", name = "second-device", help="second-device <number>" },
      {format = "second-port %integer", name = "second-port", help="second-port <number>" },
    },
    mandatory={ "b2b" },
  }
})


CLI_addCommand("pdl_test", "run packet-processor validation all", {
    func   = pdl_pp_run_validation_all,
    help   = "Run packet-processor validation - validate packet processor information can be retrieved from db",
--          Examples:
--          run packet-processor validation all
--   |  Packet processor Info  |
--   |-------------------------|
--   |Pass                     |

    params = {}
})
