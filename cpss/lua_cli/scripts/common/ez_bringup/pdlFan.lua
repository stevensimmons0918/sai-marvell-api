--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlFan.lua
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
local IDBG_PDL_FAN_SET_CMD_STATE_E = 0
local IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_E = 1
local IDBG_PDL_FAN_SET_CMD_DEBUG_E = 2
local IDBG_PDL_FAN_SET_CMD_ROTATION_DIRECTION_E = 3
local IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_METHOD_E = 4
local IDBG_PDL_FAN_SET_CMD_FAULT_E = 5
local IDBG_PDL_FAN_SET_CMD_INTILAIZE_E = 6
local IDBG_PDL_FAN_SET_CMD_PULSE_PER_ROTATION_E = 7
local IDBG_PDL_FAN_SET_CMD_THRESHOLD_E = 8
--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_get_hw_status")
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_set")
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_get_controller_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_get_fan_controller_hw")
cmdLuaCLI_registerCfunction("wr_utils_pdl_fan_run_controller_validation")


--------------------------------------------
-- Local functions: 
--------------------------------------------
-- set functions
local function pdl_fan_set_state(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_STATE_E,params["controller-id"],params["fan-id"],params["state"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_set_state_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_STATE_E, params["state"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_set_dc_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_E, params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_debug(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_DEBUG_E, params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_controller_rotation_direction(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_ROTATION_DIRECTION_E, params["controller-id"], params["rotation-direction"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_controller_duty_cycle_method(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_METHOD_E, params["controller-id"], params["duty-cycle-method"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_controller_dc(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_E, params["controller-id"], params["speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_set_controller_fault(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_FAULT_E, params["controller-id"], params["fault"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_controller_intilaize(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_INTILAIZE_E, params["controller-id"], 0)
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_pulse_per_rotation(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_PULSE_PER_ROTATION_E, params["controller-id"], params["fan-number"], params["pulse-per-rotation"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_set_threshold(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_set(IDBG_PDL_FAN_SET_CMD_THRESHOLD_E, params["controller-id"], params["fan-number"], params["threshold"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


-- show functions
local function pdl_fan_show_info(params)       
  local api_result=1
    if (params["fan-number"] == nil) then
        api_result = wr_utils_pdl_fan_get_info(params["controller-id"])
    else
        api_result = wr_utils_pdl_fan_get_info(params["controller-id"], params["fan-number"]);
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_show_info_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_show_hw_status(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_hw_status(params["controller-id"], params["fan-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_show_hw_status_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_hw_status()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_show_controller_info(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_controller_info(params["controller-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_show_controller_info_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_controller_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_show_fan_controller_hw(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_fan_controller_hw(params["controller-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_fan_show_controller_hw_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_get_fan_controller_hw()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_run_controller_validation(params)       
  local api_result=1
    if (params["fan-number"] == nil) then
        api_result = wr_utils_pdl_fan_run_controller_validation(params["controller-id"])
    else
        api_result = wr_utils_pdl_fan_run_controller_validation(params["controller-id"], params["fan-number"]);
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_fan_run_controller_validation_all(params)       
  local api_result=1
    api_result = wr_utils_pdl_fan_run_controller_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "set fan",   "Set fan configuration")
CLI_addHelp("pdl_test",    "set fan hw",   "Set fan hw configuration")
CLI_addHelp("pdl_test",    "set fan-controller",   "Set fan-controller configuration")
CLI_addHelp("pdl_test",    "set fan-controller hw",   "Set fan-controller hw configuration")
CLI_addHelp("pdl_test",    "show fan",  "Show fan information")
CLI_addHelp("pdl_test",    "show fan hw",  "Show Fan hw information")
CLI_addHelp("pdl_test",    "show fan-controller",  "Show fan-controller information ")
CLI_addHelp("pdl_test",    "show fan-controller hw",  "Show fan-controller hw information")
CLI_addHelp("pdl_test",    "run fan-controller validation",  "Run fan-controller validation")

--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["stateType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Controller State",
    enum = {
        ["normal"] = { value=0, help="Notmal operation" },
        ["shutdown"] =  { value=1, help="Shutdown operation" }      
    }
}

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Fan debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

CLI_type_dict["rotationDirectionType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Controller Rotation Direction",
    enum = {
        ["right"] = { value=0, help="Right Direcction" },
        ["left"] =  { value=1, help="Left Direction" }      
    }
}


CLI_type_dict["dutyCycleType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Controller Duty cycle",
    enum = {
        ["hw"] = { value=0, help="Hw Method" },
        ["software"] =  { value=1, help="Software Method" }      
    }
}

CLI_type_dict["faultType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Controller Fault",
    enum = {
        ["clear"] = { value=0, help="Clear Fault" },
        ["normal"] =  { value=1, help="Normal Fault" }      
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------

CLI_addCommand("pdl_test", "set fan duty-cycle all", {
    func   = pdl_fan_set_dc_all,
    help   = "Set all fan duty cycle",  
    params = {
    {
      type = "values",
      {format="%integer", name="value", help="Value: each fan might have a different value range/granularity i.e Tc654[0..16] and Adt7476[0..255]"},
    },
    mandatory = { "val-id" },   
  } 
})


CLI_addCommand("pdl_test", "set fan-controller hw duty-cycle-speed", {
    func   = pdl_fan_set_controller_dc,
    help   = "Set fan-controller duty cycle speed",
--          Examples:
--          set fan-controller hw duty-cycle 0 100
--          set fan-controller hw duty-cycle 1 50
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Fan Controller ID"},
      {format="%integer", name="speed", help="Value: each fan might have a different speed range/granularity i.e Tc654[0..16] and Adt7476[0..255]"},
    },
    mandatory = { "controller-id", "speed" },
  }
})



CLI_addCommand("pdl_test", "set fan hw state", {
    func   = pdl_fan_set_state,
    help   = "Set fan hw state <normal | shutdown>",
--          Examples:
--          set fan hw state 0 1 normal
--          set fan hw state 0 1 shutdown
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%integer", name="fan-id", help="Fan ID"},
      {format="%stateType", name="state", help="Fan state"},
    },
    mandatory = { "controller-id", "fan-id", "state" },
  }
})

CLI_addCommand("pdl_test", "set fan hw state all", {
    func   = pdl_fan_set_state_all,
    help   = "Set all fan state",   
    params = {
    {
      type = "values",
      {format="%stateType", name="state", help="Fan state"},
    },
    mandatory = { "val-id" },
  } 
})

CLI_addCommand("pdl_test", "set fan hw pulse-per-rotation", {
    func   = pdl_fan_set_pulse_per_rotation,
    help   = "Set fan pulse per rotation",
--          Examples:
--          set fan hw pulse-per-rotation 0 1 1
--          set fan hw pulse-per-rotation 1 1 2
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%integer", name="fan-number", help="Fan Number"},
      {format="%integer", name="pulse-per-rotation", help=" Pulse Per Rotation"},
    },
    mandatory = { "controller-id", "fan-number", "pulse-per-rotation" },
  }
})


CLI_addCommand("pdl_test", "set fan hw threshold", {
    func   = pdl_fan_set_threshold,
    help   = "Set fan threshold",
--          Examples:
--          set fan hw threshold 0 1 1
--          set fan hw threshold 1 1 2
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%integer", name="fan-number", help="Fan Number"},
      {format="%integer", name="treshold", help="Threshold"},
    },
    mandatory = { "controller-id", "fan-number", "threshold" },
  }
})


CLI_addCommand("pdl_test", "set fan debug", {
    func   = pdl_fan_set_debug,
    help   = "toggle fan debug messages on/off", 
--          Examples:
--          set fan debug enable
    params = {
    {
      type = "values",
      {format = "%debugSetType", name = "set-id", help="Enable/Disable Debug messages"},
    },
    mandatory = { "set-id" },
  }
})


CLI_addCommand("pdl_test", "set fan-controller hw rotation-direction", {
    func   = pdl_fan_set_controller_rotation_direction,
    help   = "Set fan-controller rotation direction",
--          Examples:
--          set fan-controller hw 0 right       
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%rotationDirectionType", name="rotation-direction", help="Rotation-direction"},
    },
    mandatory = { "controller-id", "rotation-direction" },
  }
})


CLI_addCommand("pdl_test", "set fan-controller hw duty-cycle-method", {
    func   = pdl_fan_set_controller_duty_cycle_method,
    help   = "Set fan-controller duty cycle method",
--          Examples:
--          set fan-controller hw duty-cycle-method 0 software
--          set fan-controller hw duty-cycle-method 1 hw
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%dutyCycleType", name="duty-cycle-method", help="Duty-cycle"},
    },
    mandatory = { "controller-id", "duty-cycle-method" },
  }
})


CLI_addCommand("pdl_test", "set fan-controller hw fault", {
    func   = pdl_fan_set_controller_fault,
    help   = "Set fan-controller fault",
--          Examples:
--          set fan-controller hw fault 0 clear
--          set fan-controller hw fault 1 normal
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
      {format="%faultType", name="fault", help="Fault"},
    },
    mandatory = { "controller-id", "fault" },
  }
})


CLI_addCommand("pdl_test", "set fan-controller hw intilaize", {
    func   = pdl_fan_set_controller_intilaize,
    help   = "Set fan-controller intilaize",
--          Examples:
--          set fan-controller hw intilaize 0 
--          set fan-controller hw intilaize 1
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID"},
    },
    mandatory = { "controller-id" },
  }
})



CLI_addCommand("pdl_test", "show fan info", {
    func   = pdl_fan_show_info,
    help   = "Show fan XML information",
    first_optional = "fan-number", 

--          Examples:
--          show fan info 1 1
--       Ctrl ID   | Fan Number |   Fan ID   |    Role    |
--      ---------- | ---------- | ---------- | ---------- |
--      1          | 1          | 4          | Backup     |
    params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID" },
      {format="%integer",optional = true, name="fan-number", help="Fan Number"},  
    },
    mandatory = {"controller-id"},
  }
})

 

CLI_addCommand("pdl_test", "show fan info all", {
    func   = pdl_fan_show_info_all,
    help = "All fan information",
--          Examples:
--          show fan info all
--      Ctrl ID   | Fan Number |   Fan ID   |    Role    |
--     ---------- | ---------- | ---------- | ---------- |
--     0          | 2          | 1          | Normal     |
--     0          | 3          | 2          | Normal     |
--     0          | 4          | 3          | Normal     |
--     0          | 5          | 4          | Normal     |
})

CLI_addCommand("pdl_test", "show fan hw status", {
    func   = pdl_fan_show_hw_status,
    help   = "Show fan hw status",
--          Examples:
--          show fan hw status 0 1
--              Ctrl ID | Fan Id   |  State   | Duty Cycle |   RPM   |
--              ------- | ------   | -------  | ---------- | ------- |
--              0       | 1        | OK       | 0%         | 1000    |
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID" },
      {format="%integer", name="fan-id", help="Fan ID"},  
    },
    mandatory = { "controller-id" , "fan-id"}
  }
})



CLI_addCommand("pdl_test", "show fan hw status all", {
    func   = pdl_fan_show_hw_status_all,
    help = "All fan hw status",
--          Examples:
--          show fan hw status all
--     Ctrl ID | Fan Id   |  State   | Duty Cycle |   RPM   |
--     ------- | ------   | -------  | ---------- | ------- |
--     0       | 2        | OK       | 0%         | 1000    |
--     0       | 3        | OK       | 0%         | 1000    |
--     0       | 4        | OK       | 0%         | 1000    |
--     0       | 5        | OK       | 0%         | 1000    |

})


CLI_addCommand("pdl_test", "show fan-controller info", {
    func   = pdl_fan_show_controller_info,
    help   = "Show fan-controller info",
--          Examples: 
--          show fan-controller info 0
--    fan-controller-number    |  fan-number list          |  capabilities             |  i2c-interfaceId          |  fan-Hw                   |
--   ----------------------    | -----------------         | --------------            | -----------------         | --------                  |
--   0                         | 1 , 2 , 3 ,               |                           | 22                        | EMC2305                   |
--                             |                           | stateControl              |                           |                           |
--                             |                           | stateInfo                 |                           |                           |
--                             |                           | dutyCycleControl          |                           |                           |
--                             |                           | dutyCycleInfo             |                           |                           |
--                             |                           | rpmInfo                   |                           |                           |
                    
--   I2C Bus-Id   | Address | Access Size |  Offset  |    Mask    | Mux Count |
     ------------ | ------- | ----------- | -------- | ---------- | --------- |
--   0            | 0x2f    | two-bytes   | 0x0030   | 0x000000ff | 2         |

--   Mux Num | Type |       Attribute      |
--   ------- | ---- | -------------------- |
--   #1      | GPIO |                      |
--           |      | Device: : 255        |
--           |      | Pin:    : 50         |
--           |      | Value:  : 0          |
--           |      | -------------------- |
--   #2      | GPIO |                      |
--           |      | Device: : 255        |
--           |      | Pin:    : 52         |
--           |      | Value:  : 0          |
--           |      | -------------------- |


  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID" }, 
    },
    mandatory = { "controller-id" }
  }
})


CLI_addCommand("pdl_test", "show fan-controller info all", {
    func   = pdl_fan_show_controller_info_all,
    help = "All fan controller info",
--          Examples:
--          show fan-controller info all
--       fan-controller-number    |  fan-number list          |  capabilities             |  i2c-interfaceId          |  fan-Hw                   |
--      ----------------------    | -----------------         | --------------            | -----------------         | --------                  |
--      0                         | 2 , 3 , 4 , 5 ,           |                           | 22                        | EMC2305                   |
--                                |                           | stateControl              |                           |                           |
--                                |                           | stateInfo                 |                           |                           |
--                                |                           | dutyCycleControl          |                           |                           |
--                                |                           | dutyCycleInfo             |                           |                           |
--                                |                           | rpmInfo                   |                           |                           |

})


CLI_addCommand("pdl_test", "show fan-controller hw", {
    func   = pdl_fan_show_fan_controller_hw,
    help   = "Show fan-controller hw information",
--          Examples:   
--          show fan-controller hw 0
--      Ctrl ID          |  Rotation Direction  |          RPM         |
--  -------------------- | -------------------- | -------------------- |
--         0             | Not Supported        | 1000                 |

  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID" },
    },
    mandatory = {"controller-id"}
  }
})


CLI_addCommand("pdl_test", "show fan-controller hw all", {
    func   = pdl_fan_show_controller_hw_all,
    help   = "Show all fan-controller hw information", 
--          Examples:   
--          show fan-controller hw all
--          Ctrl ID       |  Rotation Direction  |          RPM         |
--   -------------------- | -------------------- | -------------------- |
--   0                    | Not Supported        | 1000                 |

})


CLI_addCommand("pdl_test", "run fan-controller validation", {
    func   = pdl_fan_run_controller_validation,
    help   = "Run fan-controller validation - validate fan can be accessed using all supported APIs",
    first_optional = "fan-number",
--          Examples:
--          run fan-controller validation 0 1
--   Ctrl ID  | Fan Number |  Fan ID  |       ID <-> Number       |            Type           |            Init           |          Rotation         |      DutyCycleMethod      |
--   -------  | ---------- | -------- | ------------------------- | ------------------------- | ------------------------- | ------------------------- | ------------------------- |
--   0        | 1          | 1        | Pass                      | Pass                      | Pass                      | Not Supported             | Not Supported             |

--    Ctrl ID | Fan Number |  Fan ID  |         DutyCycle         |            Rpm            |           Fault           |            Role           |
--   -------- | ---------- | -------- | ------------------------- | ------------------------- | ------------------------- | ------------------------- |
--   0        | 1          | 1        | Pass                      | Pass                      | Not Supported             | Pass                      |
--
--    Ctrl ID | Fan Number |  Fan ID  |         StateInfo         |        stateControl       |           Pulse           |         Threshold         |
--   -------- | ---------- | -------- | ------------------------- | ------------------------- | ------------------------- | ------------------------- |
--   0        | 1          | 1        | Pass                      | Pass                      |                           |                           |

                
  params = {
    {
      type = "values",
      {format="%integer", name="controller-id", help="Controller ID" },
      {format="%integer",optional = true, name="fan-number", help="Fan Number"},  
    },
    mandatory = { "controller-id" }
  }
})

CLI_addCommand("pdl_test", "run fan-controller validation all", {
    func   = pdl_fan_run_controller_validation_all,
    help = "Run all fan-controller validation - validate all fan can be accessed using all supported APIs",
})