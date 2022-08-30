--********************************************************************************
--*              (c), Copyright 2018 Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlBtn.lua
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
local IDBG_PDL_BTN_SET_CMD_INFO_E = 0
local IDBG_PDL_BTN_SET_CMD_DEBUG_E = 1
--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_btn_get")
cmdLuaCLI_registerCfunction("wr_utils_pdl_btn_get_hw_status")
cmdLuaCLI_registerCfunction("wr_utils_pdl_btn_set")
cmdLuaCLI_registerCfunction("wr_utils_pdl_btn_get_validation")


--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_btn_get_info(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get(params["button-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_btn_get_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_btn_get_hw_status(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get_hw_status(params["button-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_btn_get_hw_status_all(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get_hw_status()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end



local function pdl_btn_set(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_set(IDBG_PDL_BTN_SET_CMD_INFO_E, params["button-id"],params["push-value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true 
end

local function pdl_btn_set_debug(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_set(IDBG_PDL_BTN_SET_CMD_DEBUG_E,params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
  return true
end


local function pdl_btn_get_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get_validation(params["button-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
  return true
end

local function pdl_btn_get_validation_all(params)
  local api_result=1
    api_result = wr_utils_pdl_btn_get_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end



--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "set",   "Set PDL parameters")
CLI_addHelp("pdl_test",    "show",  "Show PDL parameters")
CLI_addHelp("pdl_test",    "show button",  "Show button information")
CLI_addHelp("pdl_test",    "set button",  "Set button information")
CLI_addHelp("pdl_test",    "show button hw",  "Show button hw information")
CLI_addHelp("pdl_test",    "run",  "run validation")

--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["btnType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Button type",
    enum = {
        ["reset"] = { value=0, help="Reset button" },
        ["led"] = { value=1, help="Led mode button (change global led setting behaviour)" }
    }
}

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Button debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show button info", {
    func   = pdl_btn_get_info,
    help   = "Show button XML information (supported, interface info & push value) ",
--          Examples:
--          show button info reset
--    Btn Id   | Supported | Interface Type | Push Val |
--  ---------- | --------- | -------------- | -------- |
--  Reset      | +         | GPIO           | 0        |
--  
--  GPIO Device | Pin Number |             Type           | Init Value |
--  ----------- | ---------- | -------------------------- | ---------- |
--  255         | 15         | gpio-readable-pin          | N/A        |

--          show button info all
--    Btn Id   | Supported | Interface Type | Push Val |
--  ---------- | --------- | -------------- | -------- |
--  Reset      | +         | GPIO           | 0        |
--  LED        | -         | Not relevant   | 0        |  
 
    params = {
        { 
          type = "values",
          {format="%btnType", name="button-id", help="Button id"},
        },
        mandatory={ "button-id" }       
    }
})

CLI_addCommand("pdl_test", "show button info all", {
    func   = pdl_btn_get_info_all,
    help = "All button information",
})

CLI_addCommand("pdl_test", "show button hw", {
    func   = pdl_btn_get_hw_status,
    help   = "Show button hw status",
--          Examples:
--          show button hw status reset
--                Btn Id   |    Pressed    |
--              ---------- | ------------- |
--              Reset      | Pressed       |
--          show button hw status all
--                Btn Id   |    Pressed    |
--              ---------- | ------------- |
--              Reset      | Pressed       |
--              LED        | Not Relevant  |
    params = {
        {
          type = "values",
          {format="%btnType", name="button-id", help="Button id"},        
        },
        mandatory={ "button-id" },
    }
})

CLI_addCommand("pdl_test", "show button hw all", {
    func   = pdl_btn_get_hw_status_all,
    help = "All button hw status",
})

CLI_addCommand("pdl_test", "set button info", {
    func   = pdl_btn_set,
    help   =  "Change button push value",
--          Examples:
--          set button info reset 0
--          set button info reset 1
  params = {
        { 
          type = "values",
          {format="%btnType", name="button-id", help="set Button id"},
          {format="%integer", name="push-value", help="Value identified as button pushed"},
        },
        mandatory = { "btn-id", "push-val" },           
  }
})

CLI_addCommand("pdl_test", "set button debug", {
    func   = pdl_btn_set_debug,
    help   = "toggle button debug messages on/off",
--          Examples:
--          set button debug enable
  params = {
        {
          type = "values",
          {format = "%debugSetType", name = "set-id", help="Enable/Disable Debug messages"},      
        },
        mandatory = { "set-id" },
  }
})

CLI_addCommand("pdl_test", "run button validation", {
    func   = pdl_btn_get_validation,
    help   = "Run button validation - validate  all supported buttons HW status can be read",
--          Examples:
--          run button validation reset 
--            Btn Id   |     Info         |    Status      |
--          ---------- |   ---------      |  ----------    |
--          Reset      | Supported        |    pass        |

--          run button validation led 
--            Btn Id   |     Info         |    Status      |
--          ---------- |   ---------      |  ----------    |
--          LED        | Not Supported    | Not Supported  |

--          run button validation all 
--            Btn Id   |     Info         |    Status      |
--          ---------- |   ---------      |  ----------    |
--          Reset      | Supported        |    pass        |
--          LED        | Not Supported    |  Not Supported |
  params = {
        {
          type = "values",
          {format="%btnType", name="button-id", help="Button id"}      
        },
        mandatory = { "button-id" },
  }
})

CLI_addCommand("pdl_test", "run button validation all", {
    func   = pdl_btn_get_validation_all,
    help   = "All button validation",
})