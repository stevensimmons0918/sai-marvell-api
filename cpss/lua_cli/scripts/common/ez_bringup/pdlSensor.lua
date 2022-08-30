--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlSensor.lua
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
local PDL_SENSOR_UTILS_CMD_GET_INFO_E = 0
local PDL_SENSOR_UTILS_CMD_GET_TEMP_E = 1
local PDL_SENSOR_UTILS_CMD_GET_NUM_E = 2

local PDL_SENSOR_UTILS_CMD_SET_INFO_E = 0
local PDL_SENSOR_UTILS_CMD_SET_DEBUG_E = 1

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_sensor_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_sensor_set")
cmdLuaCLI_registerCfunction("wr_utils_pdl_sensor_get_temperature")
cmdLuaCLI_registerCfunction("wr_utils_pdl_sensor_get_count")
cmdLuaCLI_registerCfunction("wr_utils_pdl_sensor_run_validation")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_sensor_get_info(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_get_info(params["sens-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_sensor_get_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_get_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_sensor_get_temperature(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_get_temperature(params["sens-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_sensor_get_temperature_all(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_get_temperature()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_sensor_get_num(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_get_count()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true         
end

local function pdl_sensor_set_debug(params) 
  local api_result=1
    api_result = wr_utils_pdl_sensor_set(PDL_SENSOR_UTILS_CMD_SET_DEBUG_E,params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true     
end


local function pdl_sensor_run_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_run_validation(params["sensor-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_sensor_run_all_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_sensor_run_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show sensor",  "Show Sensor information")
CLI_addHelp("pdl_test",    "run sensor validation",   "Run sensor validation")

--------------------------------------------
-- Types registration: 
--------------------------------------------

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Sensor debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}
--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show sensor info", {
    func   = pdl_sensor_get_info,
    help   = "show sensor XML information", 
--          Examples:       
--      show sensor info 1
--      Sensor ID |   Type   |     Description      | Interface ID    | Interface Type  |
--      --------- | -------- | -------------------- | ------------    | --------------  |
--      1         | BOARD    | LM75 (U8)            | 1               | i2c             |
    
  params = {
    { type = "values",
      {format="%integer", name="sens-id", help="sensor id" },
      mandatory={"sens-id"}
    }
  }
})

CLI_addCommand("pdl_test", "show sensor info all", {
    func   = pdl_sensor_get_info_all,
--          Examples:       
--          show sensor info all 
--      Sensor ID |   Type   |     Description      | Interface ID    | Interface Type  |
--      --------- | -------- | -------------------- | ------------    | --------------  |
--      0         | BOARD    | LM75 (U6)            | 0               | i2c             |
--      1         | BOARD    | LM75 (U8)            | 1               | i2c             |

})


CLI_addCommand("pdl_test", "show sensor temperature", {
    func   = pdl_sensor_get_temperature,
    help   = "read HW for sensor temperature", 
--          Examples:       
--          show sensor temperature 1   
--      Sensor ID |                        Temperature                           |
--      --------- | ------------------------------------------------------------ |
--      1         | 0                                                            |
    
  params = {
    { type = "values",
      {format="%integer", name="sens-id", help="sensor id" },
      mandatory={"sens-id"}
    }
  }
})

CLI_addCommand("pdl_test", "show sensor temperature all", {
    func   = pdl_sensor_get_temperature_all,
--          Examples:
--          show sensor temperature all
--      Sensor ID |                        Temperature                           |
--      --------- | ------------------------------------------------------------ |
--      0         | 0                                                            |
--      1         | 0                                                            |
--      2         | Sensor Temperature is unreadable (not a BOARD type sensor)   |
--      3         | Sensor Temperature is unreadable (not a BOARD type sensor)   |
--      4         | Sensor Temperature is unreadable (not a BOARD type sensor)   |
--      5         | Sensor Temperature is unreadable (not a BOARD type sensor)   |
--      6         | Sensor Temperature is unreadable (not a BOARD type sensor)   |
--      7         | Sensor Temperature is unreadable (not a BOARD type sensor)   |

})


CLI_addCommand("pdl_test", "show sensor number", {
    func   = pdl_sensor_get_num,
    help   = "get number of sensors in system", 
--          Examples:       
--          show sensor number
--          Number of sensors: 8

})


CLI_addCommand("pdl_test", "set sensor debug", {
    func   = pdl_sensor_set_debug,
    help   = "toggle sensor debug messages on/off",    
  params = {
    {
      type = "values",
      {
        format = "%debugSetType", 
        name = "set-id", 
         help="set-id"
      },
      mandatory = { "set-id" },

    }
  }
})


CLI_addCommand("pdl_test", "run sensor validation", {
    func   = pdl_sensor_run_validation,
    help   = "Run sensor validation - validate all sensor temperature can be read from HW",
    
--       Examples:  run sensor validation 0
--       Sensor Number |    Sensor Type   |   Sensor Count   |    Sensor Info   |   Temperature    |
--       ------------- | ---------------- | ---------------- | ---------------- | ---------------- |
--       0             | BOARD            | Pass             | Pass             | Pass             |
        
                
    params = {
    {
      type = "values",
      {format="%integer", name="sensor-number", help="sensor-id"}, 
    },
    mandatory = {"sensor-number"},
  }
})



CLI_addCommand("pdl_test", "run sensor validation all", {
    func   = pdl_sensor_run_all_validation,
    help   = "Run all sensor validation",
        
--       Examples:      
--       run sensor validation all
--       Sensor Number |    Sensor Type   |   Sensor Count   |    Sensor Info   |   Temperature    |
--       ------------- | ---------------- | ---------------- | ---------------- | ---------------- |
--       0             | BOARD            | Pass             | Pass             | Pass             |
--       1             | BOARD            | Pass             | Pass             | Pass             |
--       2             | CPU              | Pass             | Pass             | PDL_ERROR        |
--       3             | MAC              | Pass             | Pass             | PDL_ERROR        |
--       4             | MAC              | Pass             | Pass             | PDL_ERROR        |
--       5             | PHY              | Pass             | Pass             | PDL_ERROR        |
--       6             | PHY              | Pass             | Pass             | PDL_ERROR        |
--       7             | PHY              | Pass             | Pass             | PDL_ERROR        |
        
})



