--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlLed.lua
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
local PDL_LED_TYPE_DEVICE_SYSTEM_E = 0
local PDL_LED_TYPE_DEVICE_RPS_E = 1
local PDL_LED_TYPE_DEVICE_FAN_E = 2
local PDL_LED_TYPE_DEVICE_POE_E = 3
local PDL_LED_TYPE_DEVICE_PORT_LED_STATE_E = 4
local PDL_LED_TYPE_DEVICE_STACK_UNIT1_E = 5
local PDL_LED_TYPE_DEVICE_STACK_UNIT2_E = 6
local PDL_LED_TYPE_DEVICE_STACK_UNIT3_E = 7
local PDL_LED_TYPE_DEVICE_STACK_UNIT4_E = 8
local PDL_LED_TYPE_DEVICE_STACK_UNIT5_E = 9
local PDL_LED_TYPE_DEVICE_STACK_UNIT6_E = 10
local PDL_LED_TYPE_DEVICE_STACK_UNIT7_E = 11
local PDL_LED_TYPE_DEVICE_STACK_UNIT8_E = 12
local PDL_LED_TYPE_DEVICE_STACK_MASTER_E = 13

local PDL_LED_TYPE_PORT_OOB_LEFT_E = 15
local PDL_LED_TYPE_PORT_OOB_RIGHT_E = 16
local PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E = 17
local PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E = 18

local PDL_LED_COLOR_ANY_E = 0
local PDL_LED_COLOR_GREEN_E = 1
local PDL_LED_COLOR_AMBER_E = 2

local PDL_LED_STATE_OFF_E = 0
local PDL_LED_STATE_SOLID_E = 1
local PDL_LED_STATE_LOWRATEBLINKING_E = 2
local PDL_LED_STATE_NORMALRATEBLINKING_E = 3
local PDL_LED_STATE_HIGHRATEBLINKING_E = 4
local PDL_LED_STATE_INIT_E = 5

local PDL_LED_MGMT_TYPE_CPU_E = 0
local PDL_LED_MGMT_TYPE_PHY_E = 1
    
--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_system_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_port_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_system_set_hw")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_system_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_port_set_hw")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_port_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_set_debug")
cmdLuaCLI_registerCfunction("wr_utils_pdl_led_run_validation")
--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_led_show_system_info(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_system_get_info(params["led-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_show_system_info_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_system_get_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_show_port_info(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_port_get_info(params["ledPort-deviceId"],
                                                params["ledPort-portId"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_led_system_set_hw(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_system_set_hw(params["led-id"],
                                                params["led-state"],
                                                params["led-color"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_system_set_hw_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_system_set_hw(params["led-state"],
                                                params["led-color"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_system_set_info(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_system_set_info(params["led-id"],
                                                  params["led-state"],
                                                  params["led-color"],
                                                  params["led-value"],
                                                  params["led-smiDev"],
                                                  params["led-smiReg"],
                                                  params["led-smiMask"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_port_set_hw(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_port_set_hw(params["ledPort-deviceId"],
                                              params["ledPort-portId"],
                                              params["led-id"],
                                              params["led-state"],
                                              params["led-color"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_port_set_hw_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_port_set_hw(params["led-state"],
                                                params["led-color"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_port_set_info(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_port_set_info(params["led-id"],
                                                params["ledPort-deviceId"],
                                                params["ledPort-portId"],   
                                                params["led-state"],
                                                params["led-color"],
                                                params["led-value"],
                                                params["led-smiDev"],
                                                params["led-smiReg"],
                                                params["led-smiMask"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_led_set_debug(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_led_run_device_validation(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_run_validation(params["led-id"], params["time-sec"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_led_run_all_device_validation(params)       
  local api_result=1
    api_result = wr_utils_pdl_led_run_validation(params["time-sec"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["ledSystemIdType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "system LED id",
    enum = {
        ["system"] = { value=PDL_LED_TYPE_DEVICE_SYSTEM_E, help="Device System LED" },
        ["rps"] = { value=PDL_LED_TYPE_DEVICE_RPS_E, help="Device RPS LED" },
        ["fan"] = { value=PDL_LED_TYPE_DEVICE_FAN_E, help="Device Fan LED" },
        ["poe"] = { value=PDL_LED_TYPE_DEVICE_POE_E, help="Device PoE LED" },
        ["port-led-state"] = { value=PDL_LED_TYPE_DEVICE_PORT_LED_STATE_E, help="Device Port LED state LED" },  
        ["unit1"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT1_E, help="Device Stack Unit1 LED" },
        ["unit2"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT2_E, help="Device Stack Unit2 LED" },
        ["unit3"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT3_E, help="Device Stack Unit3 LED" },
        ["unit4"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT4_E, help="Device Stack Unit4 LED" },
        ["unit5"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT5_E, help="Device Stack Unit5 LED" },
        ["unit6"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT6_E, help="Device Stack Unit6 LED" },
        ["unit7"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT7_E, help="Device Stack Unit7 LED" },
        ["unit8"] = { value=PDL_LED_TYPE_DEVICE_STACK_UNIT8_E, help="Device Stack Unit8 LED" },
        ["master"] = { value=PDL_LED_TYPE_DEVICE_STACK_MASTER_E, help="Device Stack Master LED" }, 
    }
}

CLI_type_dict["ledPortIdType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "port LED id",
    enum = {
        ["oob-left"] = { value=PDL_LED_TYPE_PORT_OOB_LEFT_E, help="OOB port Left LED" },
        ["oob-right"] = { value=PDL_LED_TYPE_PORT_OOB_RIGHT_E, help="OOB port Right LED" },
        ["port-left"] = { value=PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E, help="Port Left LED" },    
        ["port-right"] = { value=PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E, help="Port Right LED" },
    }
}

CLI_type_dict["ledStateType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED State",
    enum = {
        ["off"]    = { value=PDL_LED_STATE_OFF_E, help="LED Off" },
        ["solid"]     = { value=PDL_LED_STATE_SOLID_E, help="LED Solid" },
        ["low"]    = { value=PDL_LED_STATE_LOWRATEBLINKING_E, help="LED Low Rate Blinking" },
        ["normal"] = { value=PDL_LED_STATE_NORMALRATEBLINKING_E, help="LED Normal Rate Blinking" },
        ["high"]   = { value=PDL_LED_STATE_HIGHRATEBLINKING_E, help="LED High Rate Blinking" },
        ["init"]   = { value=PDL_LED_STATE_INIT_E, help="LED State Init" },
    }
}

CLI_type_dict["ledColorType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED Color",
    enum = {
        ["any"]   = { value=PDL_LED_COLOR_ANY_E, help="Any Color" },
        ["green"] = { value=PDL_LED_COLOR_GREEN_E, help="Green Color" },
        ["amber"] = { value=PDL_LED_COLOR_AMBER_E, help="Amber Color" },
    }
}

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

CLI_type_dict["ledmgmtType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED Managmet Type",
    enum = {
        ["cpu"] = { value=PDL_LED_MGMT_TYPE_CPU_E, help = "CPU Managment" },
        ["phy"] = { value=PDL_LED_MGMT_TYPE_PHY_E, help = "PHY Managment" }
    }
}

CLI_type_dict["smi_mask_range"] = {
    checker = CLI_check_param_number,
    min=1,
    max=65535,
    help = "SMI mask value [1..65535]"
}
--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "set led",   "Set LED Configuration/HW")
CLI_addHelp("pdl_test",    "set led hw",   "Set LED HW values")
CLI_addHelp("pdl_test",    "set led info",   "Set LED Configuration/HW")
CLI_addHelp("pdl_test",    "show led",  "Show LED Information")

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show led info system", {
    func   = pdl_led_show_system_info,
    help   = "show system led XML information",
--      Examples:    
--      show led info system fan
--        Led ID   | Interface Type  |        State         | Color    | Interface ID |        Value       |
--      ---------- | --------------  | -------------------- | -----    | ------------ | ------------------ |
--      fan        | I2C             | off                  | any      | 73           | 0                  |
--      fan        | I2C             | solid                | green    | 73           | 2                  |
--      fan        | I2C             | normalRateblinking   | amber    | 73           | 3                  |
--      fan        | I2C             | initialize           | any      | 73           | 4                  |

  params = {
    { type = "named",
      {format="%ledSystemIdType", name="led-id"},
      mandatory={"led-id"}
    }
  }
})

CLI_addCommand("pdl_test", "show led info system all", {
    func   = pdl_led_show_system_info_all
--       Examples:  
--       show led info system all
--         Led ID   | Interface Type  |        State         | Color    | Interface ID |        Value       |
--       ---------- | --------------  | -------------------- | -----    | ------------ | ------------------ |
--       system     | I2C             | off                  | any      | 72           | 0                  |
--       system     | I2C             | solid                | green    | 72           | 2                  |
--       system     | I2C             | normalRateblinking   | green    | 72           | 4                  |
--       system     | I2C             | normalRateblinking   | amber    | 72           | 3                  |
--       system     | I2C             | initialize           | any      | 72           | 4                  |
--       fan        | I2C             | off                  | any      | 73           | 0                  |
--       fan        | I2C             | solid                | green    | 73           | 2                  |
--       fan        | I2C             | normalRateblinking   | amber    | 73           | 3                  |
--       fan        | I2C             | initialize           | any      | 73           | 4                  |
--       unit1      | GPIO            | Not Relevant         | green    | 2            | 0                  |
--       unit2      | GPIO            | Not Relevant         | green    | 3            | 0                  |
--       unit3      | GPIO            | Not Relevant         | green    | 4            | 0                  |
--       unit4      | GPIO            | Not Relevant         | green    | 5            | 0                  |
--       master     | GPIO            | Not Relevant         | green    | 1            | 0                  |
        
})

CLI_addCommand("pdl_test", "show led info port", {
    func   = pdl_led_show_port_info,
    help   = "show port led XML information",  
--      Examples:
--      show led info port 0 0
--        Led ID   | Interface Type  |        State         | Color    | Interface ID |        Value       |
--      ---------- | --------------  | -------------------- | -----    | ------------ | ------------------ |
--      port_left  | XSMI            | off                  | any      | 147          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf020 |
--                 |                 |                      |          |              | value     : 0x0000 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_left  | XSMI            | solid                | green    | 147          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf020 |
--                 |                 |                      |          |              | value     : 0x00b8 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_left  | XSMI            | normalRateblinking   | green    | 147          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf020 |
--                 |                 |                      |          |              | value     : 0x1700 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_left  | XSMI            | initialize           | any      | 147          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf020 |
--                 |                 |                      |          |              | value     : 0x0138 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_right | XSMI            | off                  | any      | 148          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf021 |
--                 |                 |                      |          |              | value     : 0x0000 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_right | XSMI            | solid                | green    | 148          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf021 |
--                 |                 |                      |          |              | value     : 0x00b8 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_right | XSMI            | normalRateblinking   | green    | 148          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf021 |
--                 |                 |                      |          |              | value     : 0x1700 |
--                 |                 |                      |          |              | mask      : 0xffff |
--      port_right | XSMI            | initialize           | any      | 148          | devOrPage :     31 |
--                 |                 |                      |          |              | regAddress: 0xf021 |
--                 |                 |                      |          |              | value     : 0x0058 |
--                 |                 |                      |          |              | mask      : 0xffff |
        
  params = {
    { type = "values",
      {format="%integer", name="ledPort-deviceId", help="LED Device-ID, for LED Port type only"},
    },
    { type = "values",
      {format="%integer", name="ledPort-portId", help="LED Port-ID, for LED Port type only"},
    }         
  }
})

CLI_addCommand("pdl_test", "set led hw system", {
    func   = pdl_led_system_set_hw,
    help   = "set system led hw state & color",
  params = {
    { type = "values",
      {format="%ledSystemIdType", name="led-id", help="led-id"},
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
    },        
  }
})

CLI_addCommand("pdl_test", "set led hw system all", {
    func   = pdl_led_system_set_hw_all,
  params = {
    { type = "values",
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
    }                                                
  } 
})

CLI_addCommand("pdl_test", "set led info system", {
    func   = pdl_led_system_set_info,
    help   = "set system led db information",
  params = {
    { type = "values",
      {format="%ledSystemIdType", name="led-id", help="led-id"},
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
      {format="%integer", name="led-value", help="led value"},
      mandatory={"led-id", "led-state", "led-color", "led-value"}  
    },
    { type = "named",
      {format="smiDevOrPage %integer", name="led-smiDev", help="LED SMI devOrPage" },
      {format="smiReg %integer", name="led-smiReg", help="LED SMI registerAddress" },
      {format="smiMask %integer", name="led-smiMask", help="LED SMI mask" },      
    }   
  }
})

CLI_addCommand("pdl_test", "set led hw port", {
    func   = pdl_led_port_set_hw,
    help   = "set port led HW state & color",
  params = {
    { type = "values",
      {format="%integer", name="ledPort-deviceId", help="LED Device-ID"},
      {format="%integer", name="ledPort-portId", help="LED Port-ID"},
      {format="%ledPortIdType", name="led-id", help="led-id"},
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
    },        
  }
})

CLI_addCommand("pdl_test", "set led hw port all", {
    func   = pdl_led_port_set_hw_all,
  params = {
    { type = "values",
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
    }
  } 
})

CLI_addCommand("pdl_test", "set led info port", {
    func   = pdl_led_port_set_info,
    help   = "set port led db information",
  params = {
    { type = "values",
      {format="%ledPortIdType", name="led-id", help="led-id"},
      {format="%integer", name="ledPort-deviceId", help="LED Device-ID"},
      {format="%integer", name="ledPort-portId", help="LED Port-ID"},     
      {format="%ledStateType", name="led-state", help="led-state"},
      {format="%ledColorType", name="led-color", help="led-color"},
      {format="%integer", name="led-value", help="LED value"},
      mandatory={"led-id", "led-state", "led-color", "led-value"}  
    },
    { type = "named",
      {format="smiDevOrPage %integer", name="led-smiDev", help="LED SMI devOrPage" },
      {format="smiReg %integer", name="led-smiReg", help="LED SMI registerAddress" },
      {format="smiMask %smi_mask_range", name="led-smiMask", help="LED SMI mask" },   
    }   
  }
})

CLI_addCommand("pdl_test", "set led debug", {
    func   = pdl_led_set_debug,
    help   = "toggle Led debug messages on/off",    
  params = {
    {
      type = "values",
      {format = "%debugSetType", name = "set-id", help = "set-id"},
      mandatory = { "set-id" },

    }
  }
})


CLI_addCommand("pdl_test", "run led validate device", {
    func   = pdl_led_run_device_validation,
    help   = "Run system led validation - validate all existing led states & colors can be set",
--          Examples:   
--          run led validate device fan 1
--          Led ID      |      Info       |                               Hw                              |
--      --------------- | --------------- | --------------------------------------------------------------|
--                      |                 |      Color      |        State         |        Hw info       |
--                      |                 | --------------- | -------------------- | -------------------- |
--      fan             | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | any             | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | any             | normalRateblinking   | PDL_NOT_SUPPORTED    |
--                      |                 | any             | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | green           | normalRateblinking   | PDL_NOT_SUPPORTED    |
--                      |                 | green           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |

    

    params = {
    {
      type = "values",
       {format="%ledSystemIdType", name="led-id", help="Led-id"},
       {format="%integer", name="time-sec", help="Time-sec"},
    },
    mandatory = {"led-id", "time-sec"},
  }
})

CLI_addCommand("pdl_test", "run led validate device all", {
    func   = pdl_led_run_all_device_validation,
    help   = "Run all led devices validation",
--          Examples:   
--          run led validate device all 1
--          Led ID      |      Info       |                               Hw                              |
--      --------------- | --------------- | --------------------------------------------------------------|
--                      |                 |      Color      |        State         |        Hw info       |
--                      |                 | --------------- | -------------------- | -------------------- |
--      system          | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | any             | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | any             | normalRateblinking   | PDL_NOT_SUPPORTED    |
--                      |                 | any             | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      rps             | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      fan             | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | any             | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | any             | normalRateblinking   | PDL_NOT_SUPPORTED    |
--                      |                 | any             | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | green           | normalRateblinking   | PDL_NOT_SUPPORTED    |
--                      |                 | green           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | lowRateblinking      | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_NOT_SUPPORTED    |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      poe             | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      port_led_state  | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit1           | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | Pass                 |
--                      |                 | any             | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | any             | normalRateblinking   | Pass                 |
--                      |                 | any             | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | Pass                 |
--                      |                 | amber           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit2           | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | Pass                 |
--                      |                 | any             | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | any             | normalRateblinking   | Pass                 |
--                      |                 | any             | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | Pass                 |
--                      |                 | amber           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit3           | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | Pass                 |
--                      |                 | any             | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | any             | normalRateblinking   | Pass                 |
--                      |                 | any             | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | Pass                 |
--                      |                 | amber           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit4           | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | Pass                 |
--                      |                 | any             | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | any             | normalRateblinking   | Pass                 |
--                      |                 | any             | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | Pass                 |
--                      |                 | amber           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit5           | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit6           | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit7           | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      unit8           | Not Supported   | Not Supported   | Not Supported        | Not Supported        |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |
--      master          | Pass            |                 |                      |                      |
--                      |                 | any             | off                  | Pass                 |
--                      |                 | any             | solid                | Pass                 |
--                      |                 | any             | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | any             | normalRateblinking   | Pass                 |
--                      |                 | any             | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | any             | initialize           | Pass                 |
--                      |                 | green           | off                  | Pass                 |
--                      |                 | green           | solid                | Pass                 |
--                      |                 | green           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | green           | normalRateblinking   | Pass                 |
--                      |                 | green           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | green           | initialize           | Pass                 |
--                      |                 | amber           | off                  | Pass                 |
--                      |                 | amber           | solid                | Pass                 |
--                      |                 | amber           | lowRateblinking      | PDL_BAD_VALUE        |
--                      |                 | amber           | normalRateblinking   | Pass                 |
--                      |                 | amber           | highRateblinking     | PDL_BAD_VALUE        |
--                      |                 | amber           | initialize           | Pass                 |
--      --------------- | --------------- | --------------- | -------------------- | -------------------- |

    params = {
    {
      type = "values",
       {format="%integer", name="time-sec",  help="Time-sec"},
    },
    mandatory = {"time-sec"},
  }
})