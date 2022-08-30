--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlInterface.lua
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
local PDL_INTERFACE_TYPE_I2C_E = 1
local PDL_INTERFACE_TYPE_SMI_E = 2
local PDL_INTERFACE_TYPE_XSMI_E = 3
local PDL_INTERFACE_TYPE_GPIO_E = 4

local PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E = 0
local PDL_INTERFACE_GPIO_OFFSET_TYPE_E = 4
local PDL_INTERFACE_GPIO_OFFSET_BLINKING_E = 8
local PDL_INTERFACE_GPIO_OFFSET_INPUT_E = 16

local PDL_INTERFACE_GPIO_SET_MASK_DEVICE = 1
local PDL_INTERFACE_GPIO_SET_MASK_PIN_NUMBER = 2
local PDL_INTERFACE_GPIO_SET_MASK_TYPE = 4
local PDL_INTERFACE_GPIO_SET_MASK_VALUE = 8

local PDL_INTERFACE_I2C_SET_MASK_BUS_ID = 1
local PDL_INTERFACE_I2C_SET_MASK_ADDRESS = 2
local PDL_INTERFACE_I2C_SET_MASK_ACCESS_SIZE = 4
local PDL_INTERFACE_I2C_SET_MASK_OFFSET = 8
local PDL_INTERFACE_I2C_SET_MASK_MASK = 16
local PDL_INTERFACE_I2C_SET_MASK_VALUE = 32

local PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_DEVICE = 1
local PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_INTERFACE_ID  = 2
local PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_ADDRESS = 4   

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_gpio_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_i2c_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_mux_gpio_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_run_validation")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_mux_i2c_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_smi_xsmi_set_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_get_hw")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_set_hw")
cmdLuaCLI_registerCfunction("wr_utils_pdl_interface_set_debug")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_interface_info_get(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_get_info(params["interface-type"],params["interface-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_hw_get(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_get_hw(params["interface-type"],params["interface-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_hw_smi_xsmi_get(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_get_hw(params["interface-id"], params["smi-dev"], params["smi-address"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

-- set functions
local function pdl_interface_set_debug(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_hw_set_i2c(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_set_hw("i2c",params["interface-id"],params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_hw_set_gpio(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_set_hw("gpio",params["interface-id"], params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_hw_set_smi(params)
  local api_result=1
    api_result = wr_utils_pdl_interface_set_hw(params["interface-id"], params["smi-dev"], params["smi-address"],params["mask"],params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end


local function pdl_interface_info_set_gpio(params)
  local api_result=1
  local param_mask=0
    if (nil ~= params.dev_id) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_DEVICE;
    end
    if (nil ~= params.pin_id) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_PIN_NUMBER;
    end
    if (nil ~= params.type_id) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_TYPE;
    end 
    
    api_result = wr_utils_pdl_interface_gpio_set_info(params["interface-id"],param_mask, params["dev_id"],params["pin_id"],params["type_id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_info_set_i2c(params)
  local api_result=1
  local param_mask=0
    if (nil ~= params.i2c_bus) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_BUS_ID;
    end
    if (nil ~= params.i2c_address) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_ADDRESS;
    end
    if (nil ~= params.i2c_access) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_ACCESS_SIZE;
    end    
    if (nil ~= params.i2c_offset) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_OFFSET;
    end    
    if (nil ~= params.i2c_mask) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_MASK;
    end     
    api_result = wr_utils_pdl_interface_i2c_set_info(params["interface-id"],param_mask, params["i2c_bus"],params["i2c_address"],params["i2c_access"],
                                                      params["i2c_offset"], params["i2c_mask"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_info_set_mux_gpio(params)
  local api_result=1
  local param_mask=0
    if (nil ~= params.dev_id) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_DEVICE;
    end
    if (nil ~= params.pin_id) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_PIN_NUMBER;
    end
    if (nil ~= params.value) then
        param_mask = param_mask + PDL_INTERFACE_GPIO_SET_MASK_VALUE;
    end 
    
    api_result = wr_utils_pdl_interface_mux_gpio_set_info(params["interface-id"],params["mux-id"], param_mask, params["dev_id"],params["pin_id"],params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_info_set_mux_i2c(params)
  local api_result=1
  local param_mask=0
    if (nil ~= params.i2c_address) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_ADDRESS;
    end
    if (nil ~= params.i2c_offset) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_OFFSET;
    end
    if (nil ~= params.i2c_mask) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_MASK;
    end 
    if (nil ~= params.value) then
        param_mask = param_mask + PDL_INTERFACE_I2C_SET_MASK_VALUE;
    end 
    
    api_result = wr_utils_pdl_interface_mux_i2c_set_info(params["interface-id"],params["mux-id"], param_mask, params["i2c_address"],params["i2c_offset"],params["i2c_mask"], params["value"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_info_set_smi(params)
  local api_result=1
  local param_mask=0
    if (nil ~= params.smi_dev) then
        param_mask = param_mask + PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_DEVICE;
    end
    if (nil ~= params.smi_int_id) then
        param_mask = param_mask + PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_INTERFACE_ID;
    end
    if (nil ~= params.smi_address) then
        param_mask = param_mask + PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_ADDRESS;
    end
    
    api_result = wr_utils_pdl_interface_smi_xsmi_set_info(params["interface-id"], param_mask, params["smi_dev"],params["smi_int_id"],params["smi_address"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function pdl_interface_run_validation(params)
    local api_result=1
    if (nil == params.is_verbose) then
        params.is_verbose = false
    end
    api_result = wr_utils_pdl_interface_run_validation(params["interface-type"], params["interface-id"], params["is_verbose"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show interface",  "Show interface information")
CLI_addHelp("pdl_test",    "set interface",  "Set interface information/HW")
CLI_addHelp("pdl_test",    "set interface info",  "Set interface information")
CLI_addHelp("pdl_test",    "set interface info mux",  "Change i2c gpio/i2c mux information db")
CLI_addHelp("pdl_test",    "set interface hw",  "Set interface HW value")


--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["interfaceHwGetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface Type",
    enum = {
        ["gpio"] = { value="gpio", help = "gpio interface" },
        ["i2c"]  = { value="i2c",  help = "i2c interface" },
    }
}

CLI_type_dict["debugSetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface debug messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable debug messages" },
        ["disable"] = { value=0, help = "Disable debug messages" }
    }
}

CLI_type_dict["interfaceInfoType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface type",
    enum = {
        ["gpio"]        = { value="gpio", help = "Gpio interface" },
        ["i2c"]         = { value="i2c",  help = "I2c interface" },
        ["smi/xsmi"]    = { value="smi",  help = "Smi/xsmi interface" },
    }
}

CLI_type_dict["interfaceInfoTypeStr"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface type",
    enum = {
        ["gpio"] = { value="gpio",  help = "Gpio interface" },
        ["i2c"]  = { value="i2c" ,  help = "Xsmi interface" },
        ["smi"]  = { value="smi" ,  help = "I2c interface" },
        ["xsmi"] = { value="xsmi",  help = "Smi interface" }
    }
}

CLI_type_dict["gpioPinType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Gpio pin type",
    enum = {
        ["gpio-readable-pin"]               = { value="gpio-readable-pin",              help = "Gpio read pin" },
        ["gpio-writable-pin"]               = { value="gpio-writable-pin",              help = "Gpio write pin" },
        ["gpio-readable-writable-pin"]      = { value="gpio-readable-writable-pin",     help = "Gpio read & write pin" }
    }
}

CLI_type_dict["i2cAccessType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "I2c access size",
    enum = {
        ["one-byte"] =      { value="one-byte",     help = "1 byte"  },
        ["two-bytes"] =     { value="two-bytes",    help = "2 bytes" },
        ["four-bytes"] =    { value="four-bytes",   help = "4 bytes" }
    }
}

CLI_type_dict["gpioVal"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    help = "Gpio value [0..1]"
}

CLI_type_dict["smiXsmiDevorPage"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help = "Smi/xsmi device_id [0..255]"
}
CLI_type_dict["smiXsmiIntId"] = {
    checker = CLI_check_param_number,
    min=0,
    max=3,
    help = "Smi/xsmi interface_id [0..3]"
}
CLI_type_dict["smiXsmiAddress"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    help = "Smi/xsmi address [0..31]"
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------

CLI_addCommand("pdl_test", "show interface info", {
    func   = pdl_interface_info_get,
    help   = "Show interface XML information",   
--          Examples:
--          show interface info gpio 0
--              GPIO Device | Pin Number |             Type           | Init Value |
--              ----------- | ---------- | -------------------------- | ---------- |
--              255         | 15         | gpio-readable-pin          | N/A        |
--          
--          show interface info i2c 0
--              I2C Bus-Id | Address | Access Size |  Offset  |    Mask    | Mux Count |
--              ---------- | ------- | ----------- | -------- | ---------- | --------- |
--              0          | 0x4a    | four-bytes  | 0x0000   | 0x0000ffff | 0         |


  params = {
    {
      type = "values",
      {format = "%interfaceInfoType", name = "interface-type", help = "Interface type"},
      {format = "%integer", name = "interface-id", help = "Interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
    },
    mandatory = { "interface-type", "interface-id"},    
  }
})

CLI_addCommand("pdl_test", "show interface hw", {
    func   = pdl_interface_hw_get,
    help   = "Show interface HW value (read actual value from HW for given interface-id)",  
--          Examples:
--          show interface hw gpio 0
--              HW Value: 0
--          show interface hw i2c 0
--              HW Value: 13
  params = {
    {
      type = "values",
     {format = "%interfaceHwGetType", name = "interface-type", help = "Interface type"},
     {format = "%integer", name = "interface-id",help = "Interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
    },
    mandatory = { "interface-type", "interface-id"},        
  }
})

CLI_addCommand("pdl_test", "show interface hw smi/xsmi", {
    func   = pdl_interface_hw_smi_xsmi_get,
    help   = "Show Smi/xsmi interface HW value (read actual value from HW for given interface-id)",     
--          Examples:
--          show interface hw smi/xsmi 0 0 16
--              HW Value: 0 
  params = {
    {
      type = "values",
     {format = "%integer", name = "interface-id",help = "Interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
     {format="%smiXsmiDevorPage", name="smi-dev", help="Smi/xsmi device id" },
     {format="%smiXsmiAddress", name="smi-address", help="Smi/xsmi address" },       
    },
    mandatory = { "interface-id", "smi-dev", "smi-address"},            
  }
})

CLI_addCommand("pdl_test", "set interface debug", {
    func   = pdl_interface_set_debug,
    help   = "toggle interface debug messages on/off",
--          Examples:
--          set interface debug enable
  params = {
    {
      type = "values",
      {format = "%debugSetType", name = "set-id", help="Enable/Disable Debug messages"},
    },
    mandatory = { "set-id" },   
  }
})

CLI_addCommand("pdl_test", "set interface hw i2c", {
    func   = pdl_interface_hw_set_i2c,
    help   = "Set i2c interface hw value (write value to HW for given interface-id)",
--          Examples:
--          set interface hw i2c 0 0x10
  params = {
    {
      type = "values",
     {format = "%integer", name = "interface-id",help = "I2c interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
     {format = "%integer", name = "value",help = "Value"}
    },
    mandatory = { "interface-id", "value"},     
  }
})

CLI_addCommand("pdl_test", "set interface hw gpio", {
    func   = pdl_interface_hw_set_gpio,
    help   = "Set gpio interface hw value (write value to HW for given interface-id)",  
--          Examples:
--          set interface hw gpio 0 1
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id",help = "Gpio interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
      {format = "%gpioVal", name = "value", help = "Value"},
    },
    mandatory = { "interface-id", "value"},         
  }
})

CLI_addCommand("pdl_test", "set interface hw smi/xsmi", {
    func   = pdl_interface_hw_set_smi,
    help   = "Set smi/xsmi interface hw value (write value to HW for given interface-id)",  
--          Examples:
--          set interface hw smi/xsmi 10 0 10 0xf 0x6
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id", help = "Smi/xsmi interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
      {format="%smiXsmiDevorPage", name="smi-dev", help="Smi/xsmi device id or page" },
      {format="%smiXsmiAddress", name="smi-addr", help="Smi/xsmi address" },      
      {format="%integer", name="mask", help="mask" },     
      {format="%integer", name="value", help="value" },   
    },
    mandatory = { "interface-id", "smi-dev", "smi-address", "mask", "value"},           
  }
})

CLI_addCommand("pdl_test", "set interface info gpio", {
    func   = pdl_interface_info_set_gpio,
    help   =  "Change gpio interface information db",
--          Examples:
--          set interface info gpio 0 dev-id 1 pin-id 5 pin-type gpio-writable-pin
--          set interface info gpio 0 pin-id 5
  params = {
    {type = "values",
      {format = "%integer", name = "interface-id", help = "Gpio interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
    },
    {type = "named",
      {format="dev-id %integer", name="dev_id", help="Device id" },
      {format="pin-id %integer", name="pin_id", help="Pin number" },
      {format="pin-type %gpioPinType", name="pin_type", help="Gpio pin type" },   
    },
    mandatory = { "interface-id"},              
  }
})

CLI_addCommand("pdl_test", "set interface info i2c", {
    func   = pdl_interface_info_set_i2c,
    help   =  "Change i2c interface information db. Muxes should be changed using set interface info mux command",  
--          Examples:
--          set interface info i2c 0 i2c-access two-bytes
--          set interface info i2c 0 i2c-offset 0x10 i2c-mask 0x1
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id", help = "I2c interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
    },
    {type = "named",
      {format="bus-id %integer", name="i2c_bus", help="Bus id" },
      {format="i2c-address %integer", name="i2c_address", help="I2c address" },
      {format="i2c-access %i2cAccessType", name="i2c_access", help="I2c access type" },   
      {format="i2c-offset %integer", name="i2c_offset", help="Offset value" },
      {format="i2c-mask %integer", name="i2c_mask", help="Mask value" },
    },
    mandatory = { "interface-id"},  
  }
})

CLI_addCommand("pdl_test", "set interface info mux i2c", {
    func   = pdl_interface_info_set_mux_i2c,
    help   =  "Change interface i2c mux information db",        
--          Examples:
--          set interface info mux i2c 6 1 i2c-address 0x40
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id", help = "I2c Interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
      {format = "%integer", name = "mux-id", help = "Mux id: running number that should be retrieved via show interface info i2c command"}
    },
    {
      type = "named",
      {format="i2c-addr %integer", name="i2c_address", help="I2c address" },
      {format="i2c-offset %integer", name="i2c_offset", help="Offset value" },
      {format="i2c-mask %integer", name="i2c_mask", help="Mask value" },
      {format="value %integer", name="value", help="Mux Value" },
    },
    mandatory = { "interface-id", "mux-id"},    
  }
})

CLI_addCommand("pdl_test", "set interface info mux gpio", {
    func   = pdl_interface_info_set_mux_gpio,
    help   =  "Change interface gpio mux information db",       
--          Examples:
--          set interface info mux gpio 6 1 pin-id 5
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id", help = "Gpio Interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
      {format = "%integer", name = "mux-id", help = "Mux id: running number that should be retrieved via show interface info i2c command"}
    },
    {type = "named",
      {format="dev-id %integer", name="dev_id", help="Device id" },
      {format="pin-id %integer", name="pin_id", help="Pin number" },
      {format="value %gpioVal", name="value", help="Pin value" },     
    },
    mandatory = { "interface-id", "mux-id"},    
  }
})

CLI_addCommand("pdl_test", "set interface info smi/xsmi", {
    func   = pdl_interface_info_set_smi,
    help   =  "Change smi/xsmi interface information db",   
--          Examples:
--          set interface info smi/xsmi 0 dev-id 10
  params = {
    {
      type = "values",
      {format = "%integer", name = "interface-id", help = "Smi/xsmi interface id: should be retrieved via show <peripheral> info command (i.e show sensor info all)"},
    },
    {type = "named",
      {format="dev-id %smiXsmiDevorPage", name="smi_dev", help="Smi/xsmi device id" },
      {format="smi-int-id %smiXsmiIntId", name="smi_int_id", help="Smi/xsmi interface id" },
      {format="smi-address %smiXsmiAddress", name="smi_address", help="Smi/xsmi address" },   
    },
    mandatory = { "interface-id"},      
  }
})


CLI_addCommand("pdl_test", "run interface validation", {
    func   = pdl_interface_run_validation,
    help   = "Validate all configured interfaces can be accessed by reading their HW values",   
--          Examples:
--          run interface validation i2c all
--              [id 00]: [SUCCESS]
--              [id 01]: [SUCCESS]
--              [id 02]: [SUCCESS]
--              [id 03]: [SUCCESS]
--              [id 04]: [SUCCESS]
--              [id 05]: [SUCCESS]
  params = {
    {
      type = "values",
      {format = "%interfaceInfoTypeStr", name = "interface-type", help = "Interface type"},
      {format = "%string", name = "interface-id", help = "Interface id or all"},
    },
    { type = "named",
      {format="verbose %debugSetType", name="is_verbose", help="print verbose errors" },
    },
    mandatory = { "interface-type", "interface-id" },   
  }
})
