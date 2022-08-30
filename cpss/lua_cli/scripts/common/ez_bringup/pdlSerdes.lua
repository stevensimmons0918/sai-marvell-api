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
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_get_polarity")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_get_fine_tune")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_set_debug")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_run_validation")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_run_validation_dev_all")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_set_tx_fine_tune")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_set_rx_fine_tune")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_set_polarity")
cmdLuaCLI_registerCfunction("wr_utils_pdl_serdes_apply_config")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_serdes_get_info(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_get_info(params["dev"], params["port"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_serdes_get_polarity(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_get_polarity(params["dev"], params["lane"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_serdes_get_fine_tune(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_get_fine_tune(params["dev"], params["lane"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true         
end

local function pdl_serdes_set_debug(params) 
  local api_result=1
    api_result = wr_utils_pdl_serdes_set_debug(params["set-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true     
end

local function pdl_serdes_run_validation(params) 
    local api_result=1
    if (nil == params.is_verbose) then
        params.is_verbose = false;
    end
    api_result = wr_utils_pdl_serdes_run_validation(params["dev"], params["lane"], params["is_verbose"])
    if api_result ~= 0 then
        printMsg ("command failed rc = "..api_result)
    end
    return true     
end

local function pdl_serdes_run_validation_dev_all(params) 
    local api_result=1
    if (nil == params.is_verbose) then
        params.is_verbose = false;
    end
    api_result = wr_utils_pdl_serdes_run_validation_dev_all(params["lane"], params["is_verbose"])
    if api_result ~= 0 then
        printMsg ("command failed rc = "..api_result)
    end
    return true     
end

local function pdl_serdes_set_polarity(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_set_polarity(params["dev"], params["lane"], params["tx_swap"], params["rx_swap"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_serdes_set_tx_fine_tune(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_set_tx_fine_tune(params["dev"], params["lane"], params["mode"], params["connector"], params["txAmp"], params["txAmpAdjEn"], params["txAmpShftEn"], params["emph0"], params["emph1"], params["txEmphEn"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_serdes_set_rx_fine_tune(params)
  local api_result=1
    api_result = wr_utils_pdl_serdes_set_rx_fine_tune(params["dev"], params["lane"], params["mode"], params["connector"], params["sqlch"], params["ffeR"], params["ffeC"], params["align90"], params["dcGain"], params["bandWidth"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_serdes_apply_config(params)
  local api_result=1
    if (nil == params.verbose) then
        params.verbose = false;
    end
    api_result = wr_utils_pdl_serdes_apply_config(params["dev"], params["port"], params["speed"], params["verbose"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show serdes",  "Show Serdes information")

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

CLI_type_dict["portConnectorType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Port connector types",
    enum = {
        ["SFPPlus"]     = { value="SFPPlus"     , help = "SFPPlus port connector type" },
        ["DAC"]         = { value="DAC"         , help = "DAC port connector type" },
        ["RJ45"]  = { value="RJ45"  , help = "RJ45 port connector type" }
    }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show serdes info", {
    func   = pdl_serdes_get_info,
    help   = "show Serdes mapping XML information",
--          Examples: 
--          show serdes info 1 1
--       DEVICE | PORT | ABSOLUTE NUMBER | RELATIVE NUMBER
--       ------ | ---- | --------------- | ---------------
--       1      | 1    | 1               | 0
        
  params = {
    { type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%string", name="port", help="port number or all" },
      mandatory={"dev", "port"}
    }
  }
})

CLI_addCommand("pdl_test", "show serdes polarity", {
    func   = pdl_serdes_get_polarity,
    help   = "show Serdes polarity XML information",

  params = {
    { type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%string", name="lane", help="absolute lane number or all" },
      mandatory={"dev", "lane"}
    }
  }
})

CLI_addCommand("pdl_test", "show serdes fine-tune", {
    func   = pdl_serdes_get_fine_tune,
    help   = "show Serdes rx&tx fine-tune XML information",
--          Examples: 
--          show serdes fine-tune 1 0
--       DEVICE | LANE | INTERFACE MODE | CONNECTOR TYPE | Tx Attributes               | Rx Attributes
--       ------ | ---- | -------------- | -------------- | --------------------------- | --------------
--       1      | 0    | SR-LR          | SFPPlus        | Tx Amp              :    12 |
--              |      |                |                | Tx Amp Adj Enable   :  true |
--              |      |                |                | emph0               :     8 |
--              |      |                |                | emph1               :     0 |
--              |      |                |                | Tx Amp Shift Enable :  true |
--              |      |                |                | Tx Emph Enable      :  true |
--              |      |                |                | Slew Rate           :     0 |
--       ------   ----   --------------   --------------   ---------------------------   --------------
        
  params = {
    { type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%string", name="lane", help="absolute lane number or all" },
      mandatory={"dev", "lane"}
    }
  }
})

CLI_addCommand("pdl_test", "set serdes debug", {
    func   = pdl_serdes_set_debug,
    help   = "toggle serdes debug messages on/off",    
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

CLI_addCommand("pdl_test", "run serdes validation", {
    func   = pdl_serdes_run_validation,
    help   = "Run serdes validation - validate all serdes information can be retrieved from XML",    
    params = {
    { type = "values",
      {format="%integer", name="dev", help="device number or all" },
      {format="%string", name="lane", help="absolute lane number or all" },
    },
    { type = "named",
      {format="verbose %debugSetType", name="is_verbose", help="print verbose errors" },
    },
    mandatory={"dev", "lane"}
  }
})

CLI_addCommand("pdl_test", "run serdes validation all", {
    func   = pdl_serdes_run_validation_dev_all,
    help   = "run all serdes validation",    
    params = {
    {
      type = "values",
      {format="%string", name="lane", help="absolute lane number or all" }
    },
    {
      type = "named",
      {format = "verbose %debugSetType", name = "is_verbose", help="enable/disable"}
    },
    mandatory = { "lane" },
  }
})

CLI_addCommand("pdl_test", "set serdes fine-tune tx", {
    func   = pdl_serdes_set_tx_fine_tune,
    help   = "change serdes tx-fine_tune db information",
    params = {
    {
      type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%integer", name="lane", help="absolute lane number" },
      {format="%ifModeType", name="mode", help="interface mode" },
      {format="%portConnectorType", name="connector", help="connector type" },
    },
    {
      type = "named",
      {format = "txAmp %integer", name = "txAmp", help="txAmp <number>" },
      {format = "txAmpAdjEn %bool", name = "txAmpAdjEn", help="txAmpAdjEn <true/false>" },
      {format = "txAmpShftEn %bool", name = "txAmpShftEn", help="txAmpShftEn <true/false>" },
      {format = "emph0 %integer", name = "emph0", help="emph0 <number>" },
      {format = "emph1 %integer", name = "emph1", help="emph1 <number>" },
      {format = "txEmphEn %bool", name = "txEmphEn", help="txEmphEn <true/false>" }
    },
    mandatory = { "dev", "lane", "mode", "connector" },
  }
})

CLI_addCommand("pdl_test", "set serdes fine-tune rx", {
    func   = pdl_serdes_set_rx_fine_tune,
    help   = "change serdes rx-fine-tune db information",
    params = {
    {
      type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%integer", name="lane", help="absolute lane number" },
      {format="%ifModeType", name="mode", help="interface mode" },
      {format="%portConnectorType", name="connector", help="connector type" },
    },
    {
      type = "named",
      {format = "sqlch %integer", name = "sqlch", help="sqlch <number>" },
      {format = "ffeR %integer", name = "ffeR", help="ffeR <number>" },
      {format = "ffeC %integer", name = "ffeC", help="ffeC <number>" },
      {format = "align90 %integer", name = "align90", help="align90 <number>" },
      {format = "dcGain %integer", name = "dcGain", help="dcGain <number>" },
      {format = "bandWidth %integer", name = "bandWidth", help="bandWidth <number>" }
    },
    mandatory = { "dev", "lane", "mode", "connector" },
  }
})

CLI_addCommand("pdl_test", "set serdes polarity", {
    func   = pdl_serdes_set_polarity,
    help   = "change serdes polarity db information",
    params = {
    {
      type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%integer", name="lane", help="absolute lane number" }
    },
    {
      type = "named",
      {format = "tx_swap %bool", name = "tx_swap", help="tx_swap <true/false>" },
      {format = "rx_swap %bool", name = "rx_swap", help="rx_swap <true/false>" }
    },
    mandatory = { "dev", "lane", "mode", "connector" },
  }
})

CLI_addCommand("pdl_test", "apply serdes configuration", {
    func   = pdl_serdes_apply_config,
    help   = "Apply currently configured serdes parameters to HW",
    params = {
    {
      type = "values",
      {format="%integer", name="dev", help="device number" },
      {format="%integer", name="port", help="port number" },
      {format="%speedType", name="speed", help="port speed" }
    },
    {
      type = "named",
      {format = "verbose %debugSetType", name = "verbose", help="enable/disable"}
    },
    mandatory = { "dev", "port", "speed" },
  }
})
