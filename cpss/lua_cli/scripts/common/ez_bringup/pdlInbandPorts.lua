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
-- commented out function not present in cpss at least from baseline CPSS_4.2_730 (see LUA test logs).
--cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_get_phy_post_init_info")

cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_get_count")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_front_panel_port_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_front_panel_group_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_packet_processor_port_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_packet_processor_port_phy_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_packet_processor_port_phy_script_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_packet_processor_port_gbic_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_set_info_phy_id")
cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_set_info_phy_position")
cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_set_info_interface_mode")
cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_validate_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_inband_validate_info_all_dev") 
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_run_fromt_panel_validation")
cmdLuaCLI_registerCfunction("wr_utils_pdl_network_run_packet_processor_port_validation")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_inband_validate_info(params)
  local api_result=1
    if (nil == params.is_verbose) then
        params.is_verbose = 0;
    end
    api_result = wr_utils_pdl_inband_validate_info(params["dev"], params["port"], params["is_verbose"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_inband_validate_info_all_dev(params)
  local api_result=1
    if (nil == params.is_verbose) then
        params.is_verbose = 0;
    end
    api_result = wr_utils_pdl_inband_validate_info_all_dev(params["port"], params["is_verbose"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_run_front_panel_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_network_run_fromt_panel_validation(params["group-number"], params["port"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_run_front_panel_validation_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_run_fromt_panel_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_run_packet_processor_port_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_network_run_packet_processor_port_validation(params["dev-number"], params["port"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_run_packet_processor_port_validation_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_run_packet_processor_port_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_hw_packet_processor_port_gbic_info(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info(params["pp-device-number"], params["pp-port-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



local function pdl_network_get_hw_packet_processor_port_gbic_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end




local function pdl_network_get_packet_processor_port_gbic_info(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_packet_processor_port_gbic_info(params["pp-device-number"], params["pp-port-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



local function pdl_network_get_packet_processor_port_gbic_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_packet_processor_port_gbic_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



local function pdl_network_get_hw_packet_processor_port_gbic_info(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info(params["pp-device-number"], params["pp-port-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



local function pdl_network_get_hw_packet_processor_port_gbic_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_hw_packet_processor_port_gbic_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_inband_get_count(params)
  local api_result=1
    api_result = wr_utils_pdl_inband_get_count(params["dev"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_front_panel_port_info(params)
  local api_result=1
    if (params["port-number"] == nil) then
        api_result = wr_utils_pdl_network_get_front_panel_port_info(params["group-number"])
    else 
        api_result = wr_utils_pdl_network_get_front_panel_port_info(params["group-number"], params["port-number"])
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_front_panel_port_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_front_panel_port_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_front_panel_group_info(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_front_panel_group_info(params["group-number"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_front_panel_group_info_all(params)
   local api_result=1
    api_result = wr_utils_pdl_network_get_front_panel_group_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true  
end


local function pdl_network_get_packet_processor_port_info(params)
  local api_result=1
    if (params["pp-port-number"] == nil) then
        api_result = wr_utils_pdl_network_get_packet_processor_port_info(params["pp-device-number"])
    else 
        api_result = wr_utils_pdl_network_get_packet_processor_port_info(params["pp-device-number"], params["pp-port-number"])
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end



local function pdl_network_get_packet_processor_port_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_packet_processor_port_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_packet_processor_port_phy_info(params)
  local api_result=1
    if (params["pp-port-number"] == nil) then
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_info(params["pp-device-number"])
    else 
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_info(params["pp-device-number"], params["pp-port-number"])
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_packet_processor_port_phy_info_all(params)
  local api_result=1
    api_result = wr_utils_pdl_network_get_packet_processor_port_phy_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_packet_processor_port_phy_init_list_info(params)
  local api_result=1
    if (params["speed"] == nil) then
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info(params["pp-device-number"], params["pp-port-number"], params["post-or-transceiver"]) 
    else 
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_init_list_info(params["pp-device-number"], params["pp-port-number"], params["post-or-transceiver"], params["speed"])
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_network_get_packet_processor_port_phy_script_info(params)
  local api_result=1
    if (params["speed"] == nil) then
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_script_info(params["pp-device-number"], params["pp-port-number"], params["post-or-transceiver"])
    else 
        api_result = wr_utils_pdl_network_get_packet_processor_port_phy_script_info(params["pp-device-number"], params["pp-port-number"], params["post-or-transceiver"], params["speed"])
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_inband_set_info_phy_id(params)
  local api_result=1
    api_result = wr_utils_pdl_inband_set_info_phy_id(params["dev"], params["port"],params["phy-id"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_inband_set_info_phy_position(params)
  local api_result=1
    api_result = wr_utils_pdl_inband_set_info_phy_position(params["dev"], params["port"],params["phy-pos"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end


local function pdl_inband_set_info_interface_mode(params)
  local api_result=1
    api_result = wr_utils_pdl_inband_set_info_interface_mode(params["dev"], params["port"],params["trans"],params["speed"],params["mode"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show network-port",  "Show network ports information")
CLI_addHelp("pdl_test",    "show network-port info front-panel-port",  "Show network port info front-panel-port information")
CLI_addHelp("pdl_test",    "show network-port info front-panel-group",  "Show network port info front-panel-group information")
CLI_addHelp("pdl_test",    "show network-port info packet-processor-port",  "Show network port info packet-processor-port information")
CLI_addHelp("pdl_test",    "show network-port info packet-processor-port-phy",  "Show network port info packet-processor-port-phy information")

--------------------------------------------
-- Types registration: 
--------------------------------------------

CLI_type_dict["boolType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Verbose messages",
    enum = {
        ["enable"]  = { value=1, help = "Enable verbose massages" },
        ["disable"] = { value=0, help = "Disable verbose messages" }
    }
}

CLI_type_dict["transceiverType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Transceiver types",
    enum = {
        ["fiber"]  = { value="fiber", help = "Fiber transceiver type" },
        ["copper"] = { value="copper", help = "Copper transceiver type" },
        ["combo"]  = { value="combo", help = "Combo transceiver type" }
    }
}

CLI_type_dict["speedType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Speed types",
    enum = {
        ["10M"]  = { value="10M" , help = "10M  speed type" },
        ["100M"] = { value="100M", help = "100M speed type" },
        ["1G"]   = { value="1G"  , help = "1G   speed type" },
        ["2.5G"] = { value="2.5G", help = "2.5G speed type" },
        ["5G"]   = { value="5G"  , help = "5G   speed type" },
        ["10G"]  = { value="10G" , help = "10G  speed type" },
        ["20G"]  = { value="20G" , help = "20G  speed type" },
        ["24G"]  = { value="24G" , help = "24G  speed type" },
        ["25G"]  = { value="25G" , help = "25G  speed type" },
        ["40G"]  = { value="40G" , help = "40G  speed type" },
        ["50G"]  = { value="50G" , help = "50G  speed type" },
        ["100G"] = { value="100G", help = "100G speed type" }
    }
}

CLI_type_dict["ifModeType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Interface mode types",
    enum = {
        ["SGMII"]       = { value="SGMII"       , help = "SGMII      interface mode type" },
        ["XGMII"]       = { value="XGMII"       , help = "XGMII      interface mode type" },
        ["1000BASE_X"]  = { value="1000BASE_X"  , help = "1000BASE_X interface mode type" },
        ["QX"]          = { value="QX"          , help = "QX         interface mode type" },
        ["HX"]          = { value="HX"          , help = "HX         interface mode type" },
        ["RXAUI"]       = { value="RXAUI"       , help = "RXAUI      interface mode type" },
        ["100BASE_FX"]  = { value="100BASE_FX"  , help = "100BASE_FX interface mode type" },
        ["KR"]          = { value="KR"          , help = "KR         interface mode type" },
        ["SR-LR"]       = { value="SR-LR"       , help = "SR-LR      interface mode type" },
        ["QSGMII"]      = { value="QSGMII"      , help = "QSGMII     interface mode type" },
        ["MII"]         = { value="MII"         , help = "MII        interface mode type" },
        ["XHGS"]        = { value="XHGS"        , help = "XHGS       interface mode type" },
        ["KR2"]         = { value="KR2"         , help = "KR2        interface mode type" }
    }
}


CLI_type_dict["postOrTransceiver"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "choose transceiver types or 'post-init'",
    enum = {
        ["fiber"]  = { value="fiber", help = "Fiber transceiver type" },
        ["copper"] = { value="copper", help = "Copper transceiver type" },
        ["combo"]  = { value="combo", help = "Combo transceiver type" },
        ["post-init"]  = { value="post-init", help = "post init type" },
        ["all"]  = { value="all", help = "all exists init list" }
    }
}


--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "run network-port validation", {
    func   = pdl_inband_validate_info,
    help   = "Run network port validation - verify XML mapping and actual values configured by cpssDxChPortPhysicalPortMapSet are identical",
--  Example:
--  run network-port validation all 1
--  Validating port 0/01 mapping -  FAILED
--  Validating port 0/01 supported configuration -  FAILED
--  Validating port 0/01 current configuration -  FAILED
--  Validating port 1/01 mapping -  FAILED
--  Validating port 1/01 supported configuration -  FAILED
--  Validating port 1/01 current configuration -  FAILED

  params = {
    { type = "values",
      {format="%integer", name="dev", help="device number or all" },
      {format="%string", name="port", help="port number or all" },
      mandatory={"dev", "port"}
    },
    { type = "named",
      {format="verbose %boolType", name="is_verbose", help="print verbose errors" },
    }
  }
})

CLI_addCommand("pdl_test", "run network-port validation all", {
    func   = pdl_inband_validate_info_all_dev,
    help   = "Run network port validation - verify XML mapping and actual values configured by cpssDxChPortPhysicalPortMapSet are identical",

  params = {
    { type = "values",
      {format="%string", name="port", help="port number or all" },
      mandatory={"port"}
    },
    { type = "named",
      {format="verbose %boolType", name="is_verbose", help="print verbose errors" },
    }
  }
})



CLI_addCommand("pdl_test", "run network-port validation front-panel", {
    func   = pdl_network_run_front_panel_validation,
    help   = "Run port front panel validation - verify port front panel information can be read from db",
--          Examples:
--      run network-port validation front-pannel 2 48
--      Group Number | Port Number | Group Info | Port Info |
--      ------------ | ----------- | ---------- | --------- |
--      2            | 48          | Pass       | Pass      |
--      run network-port validation front-pannel 3 all
--      Group Number | Port Number | Group Info | Port Info |
--      ------------ | ----------- | --------=- | --------- |
--      3            | 1           | Pass       | Pass      |
--      3            | 2           | Pass       | Pass      |
--      3            | 3           | Pass       | Pass      |
--      3            | 4           | Pass       | Pass      |
--      run network-port validation front-panel all
--      Group Number | Port Number | Group Info | Port Info |
--      ------------ | ----------- | ---------- | --------- |
--      1            | 1           | Pass       | Pass      |
--      1            | 2           | Pass       | Pass      |
--      1            | 3           | Pass       | Pass      |
--      1            | 4           | Pass       | Pass      |
--      1            | 5           | Pass       | Pass      |
--      1            | 6           | Pass       | Pass      |
--      1            | 7           | Pass       | Pass      |
--      1            | 8           | Pass       | Pass      |
--      1            | 9           | Pass       | Pass      |
--      1            | 10          | Pass       | Pass      |
--      1            | 11          | Pass       | Pass      |
--      1            | 12          | Pass       | Pass      |
--      1            | 13          | Pass       | Pass      |
--      1            | 14          | Pass       | Pass      |
--      1            | 15          | Pass       | Pass      |
--      1            | 16          | Pass       | Pass      |
--      1            | 17          | Pass       | Pass      |
--      1            | 18          | Pass       | Pass      |
--      1            | 19          | Pass       | Pass      |
--      1            | 20          | Pass       | Pass      |
--      1            | 25          | Pass       | Pass      |
--      1            | 26          | Pass       | Pass      |
--      1            | 27          | Pass       | Pass      |
--      1            | 28          | Pass       | Pass      |
--      1            | 29          | Pass       | Pass      |
--      1            | 30          | Pass       | Pass      |
--      1            | 31          | Pass       | Pass      |
--      1            | 32          | Pass       | Pass      |
--      1            | 33          | Pass       | Pass      |
--      1            | 34          | Pass       | Pass      |
--      1            | 35          | Pass       | Pass      |
--      1            | 36          | Pass       | Pass      |
--      1            | 37          | Pass       | Pass      |
--      1            | 38          | Pass       | Pass      |
--      1            | 39          | Pass       | Pass      |
--      1            | 40          | Pass       | Pass      |
--      1            | 41          | Pass       | Pass      |
--      1            | 42          | Pass       | Pass      |
--      1            | 43          | Pass       | Pass      |
--      1            | 44          | Pass       | Pass      |
--      2            | 21          | Pass       | Pass      |
--      2            | 22          | Pass       | Pass      |
--      2            | 23          | Pass       | Pass      |
--      2            | 24          | Pass       | Pass      |
--      2            | 45          | Pass       | Pass      |
--      2            | 46          | Pass       | Pass      |
--      2            | 47          | Pass       | Pass      |
--      2            | 48          | Pass       | Pass      |
--      3            | 1           | Pass       | Pass      |
--      3            | 2           | Pass       | Pass      |
--      3            | 3           | Pass       | Pass      |
--      3            | 4           | Pass       | Pass      |


    params = {
    { type = "values",                  
      {format="%integer", name="group-number", help="group number" },
      {format="%string", name="port", help="port number or all" },
      mandatory = {"group-number", "port" }
    }
  }
})

  
  CLI_addCommand("pdl_test", "run network-port validation front-panel all", {
    func   = pdl_network_run_front_panel_validation_all,
    help   = "Run port front panel validation - verify port front panel information can be read from db",

})


CLI_addCommand("pdl_test", "run network-port validation packet-processor-port", {
    func   = pdl_network_run_packet_processor_port_validation,
    help   = "Run packet-processor validation - verify front panel port information can be read from db",
--   Examples:
--   run network-port validation packet-processor-port 0 0
--   pp-dev-number | pp-port-number | connected info |   laser       | tx enale/disable |   memory      |
--   ------------- | -------------- | -------------- | ----------    | ---------------- | ----------    |
--   0             | 0              | Pass           | Pass          | Pass             |               |


--   run network-port validation packet-processor-port 1 all
--   pp-dev-number | pp-port-number | connected info |    laser      | tx enale/disable |    memory     |
--   ------------- | -------------- | -------------- | ------------- | ---------------- | ------------- |
--   1             | 29             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 31             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 25             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 27             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 21             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 23             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 17             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 19             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 13             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 15             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 28             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 30             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 24             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 26             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 20             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 22             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 16             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 18             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 12             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 14             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 9              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 11             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 8              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 10             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--   1             | 0              | Pass           | Pass          | Pass             |               |
--   1             | 1              | Pass           | Pass          | Pass             |               |



    params = {
    { type = "values",                  
      {format="%integer", name="dev-number", help="device number" },
      {format="%string", name="port", help="port number or all" },
      mandatory = {"dev-number", "port" }
    }
  }
})


  
CLI_addCommand("pdl_test", "run network-port validation packet-processor-port all", {
    func   = pdl_network_run_packet_processor_port_validation_all,
    help   = "Run packet-processor port validation - verify all port sfp related HW values can be read",
--    Examples:
--    run network-port validation packet-processor-port all
--    pp-dev-number | pp-port-number | connected info |    laser      | tx enale/disable |    memory     |
--    ------------- | -------------- | -------------- | ------------- | ---------------- | ------------- |
--    1             | 29             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 31             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 25             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 27             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 21             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 23             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 17             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 19             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 13             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 15             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 13             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 15             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 29             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 31             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 25             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 27             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 21             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 23             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 17             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 19             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 28             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 30             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 24             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 26             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 20             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 22             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 16             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 18             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 12             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 14             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 12             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 14             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 28             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 30             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 24             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 26             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 20             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 22             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 16             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 18             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 9              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 11             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 9              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 11             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 8              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 10             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 8              | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    1             | 10             | PDL_NOT_FOUND  | PDL_NOT_FOUND | Not Supported    |               |
--    0             | 1              | Pass           | Pass          | Pass             |               |
--    0             | 0              | Pass           | Pass          | Pass             |               |
--    1             | 0              | Pass           | Pass          | Pass             |               |
--    1             | 1              | Pass           | Pass          | Pass             |               |


})

CLI_addCommand("pdl_test", "show network-port count", {
    func   = pdl_inband_get_count,
    help   = "get number of network-ports in system",
--          Examples:
--          show network-port count 1
--          Number of ports: 26

  params = {
    { type = "values",
      {format="%integer", name="dev", help="device number" },
      mandatory={"dev"}
    }
  }
})

CLI_addCommand("pdl_test", "show network-port info packet-processor-port-phy-init-list", {
    func   = pdl_network_get_packet_processor_port_phy_init_list_info,
    help   = "show network-port phy-init-list existence table for different transceivers & speeds",
    first_optional = "speed",  
--   Examples:
--   show network-port info packet-processor-port-phy-init-list 0 0 all
--   pp-dev-number | pp-port-number | Transceiver type | Speed | phy-init-list |
--   ------------- | -------------- | ---------------- | ----- | ------------- |
--   0             | 0              |                  |       |               |
--                 |                | fiber            | 10G   | Exist         |
--                 |                | copper           | 2.5G  | Exist         |
--                 |                | copper           | 5G    | Exist         |
--                 |                | copper           | 10G   | Exist         |
--   show network-port info packet-processor-port-phy-init-list 0 0 fiber 10G
--   pp-dev-number | pp-port-number | phy-init-list |
--   ------------- | -------------- | ------------- |
--   0             | 0              | Exist         |
--   show network-port info packet-processor-port-phy-init-list 1 1 post-init
--   pp-dev-number | pp-port-number | phy-post-init |
--   ------------- | -------------- | ------------- |
--   1             | 1              | No exist      |




  params = {
    { type = "values",                  
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%integer", name="pp-port-number", help="port number" },
      {format="%postOrTransceiver", name="post-or-transceiver", help="post-init or transceiver type" },
      {format="%speedType", name="speed", help="network-port speed" },
      mandatory = { "pp-device-number", "pp-port-number", "post-or-transceiver"}
    }
  }
})


CLI_addCommand("pdl_test", "show network-port info packet-processor-port-phy-script", {
    func   = pdl_network_get_packet_processor_port_phy_script_info,
    help   = "show network-port phy-script (init/post-init) list",
    first_optional = "speed",
--          Examples:
--     show network-port info packet-processor-port-phy-script 0 0 fiber 10G
--     pp-dev-number | pp-port-number |  devOrPage  |  registerAddress  |   value   |     mask     |
--     ------------- | -------------- | ----------- | ----------------- | --------- | ------------ |
--     0             | 0              | 30          | 0x8301            | 0x324     | 0xFFFF       |


  params = {
    { type = "values",                  
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%integer", name="pp-port-number", help="port number" },
      {format="%postOrTransceiver", name="post-or-transceiver", help="post-init or transceiver type" },
      {format="%speedType", name="speed", help="network-port speed" },
      mandatory = { "pp-device-number", "pp-port-number", "post-or-transceiver"}
    }
  }
})



CLI_addCommand("pdl_test", "show network-port info packet-processor-port-gbic", {
    func   = pdl_network_get_packet_processor_port_gbic_info,
    help   = "show network-port gbic XML information",
    
--      Examples:
--      show network-port info packet-processor-port-gbic 0 0
--      pp-dev-number | pp-port-number |    connected info     |          laser        |       tx enable       |       tx disable      |
--      ------------- | -------------- | --------------------- | --------------------- | --------------------- | --------------------- |
--      0             | 0              | interface-i2c id:5    | interface-i2c id:6    | interface-i2c id:7    | interface-i2c id:8    |

--      show network-port info packet-processor-port-gbic all
--      pp-dev-number | pp-port-number |    connected info     |          laser        |       tx enable       |       tx disable      |
--      ------------- | -------------- | --------------------- | --------------------- | --------------------- | --------------------- |
--      1             | 29             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 31             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 25             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 27             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 21             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 23             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 17             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 19             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 13             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 15             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 13             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 15             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 29             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 31             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 25             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 27             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 21             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 23             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 17             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 19             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 28             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 30             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 24             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 26             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 20             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 22             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 16             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 18             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 12             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 14             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 12             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 14             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 28             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 30             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 24             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 26             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 20             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 22             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 16             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 18             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 9              | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 11             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 9              | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 11             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 8              | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 10             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 8              | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      1             | 10             | Not Supported         | Not Supported         | Not Supported         | Not Supported         |
--      0             | 1              | interface-i2c id:0    | interface-i2c id:1    | interface-i2c id:2    | interface-i2c id:3    |
--      0             | 0              | interface-i2c id:5    | interface-i2c id:6    | interface-i2c id:7    | interface-i2c id:8    |
--      1             | 0              | interface-i2c id:10   | interface-i2c id:11   | interface-i2c id:12   | interface-i2c id:13   |
--      1             | 1              | interface-i2c id:15   | interface-i2c id:16   | interface-i2c id:17   | interface-i2c id:18   |

  params = {
    { type = "values",
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%string", name="pp-port-number", help="port number | all" },
      mandatory = {"pp-device-number", "pp-port-number"}
    }
  }
})



CLI_addCommand("pdl_test", "show network-port info packet-processor-port-gbic all", {
    func   = pdl_network_get_packet_processor_port_gbic_info_all,
    help   = "show network-port gbic XML information",
   
})


CLI_addCommand("pdl_test", "show network-port hw packet-processor-port-gbic", {
    func   = pdl_network_get_hw_packet_processor_port_gbic_info,
    help   = "show network-port gbic hw status",
    
--          Examples:
--   show network-port hw packet-processor-port-gbic 1 1
--   pp-dev-number | pp-port-number | connected info |   laser       | tx enale/disable |   memory      |
--   ------------- | -------------- | -------------- | ----------    | ---------------- | ----------    |
--   1             | 1              | True           | True          | Enable           |               |
--   show network-port hw packet-processor-port-gbic all
--   pp-dev-number | pp-port-number | connected info |    laser      | tx enale/disable |    memory     |
--   ------------- | -------------- | -------------- | ------------- | ---------------- | ------------- |
--   1             | 29             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 31             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 25             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 27             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 21             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 23             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 17             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 19             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 13             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 15             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 13             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 15             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 29             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 31             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 25             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 27             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 21             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 23             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 17             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 19             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 28             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 30             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 24             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 26             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 20             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 22             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 16             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 18             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 12             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 14             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 12             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 14             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 28             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 30             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 24             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 26             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 20             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 22             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 16             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 18             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 9              | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 11             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 9              | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 11             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 8              | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 10             | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 8              | Not Supported  | Not Supported | Not Supported    |               |
--   1             | 10             | Not Supported  | Not Supported | Not Supported    |               |
--   0             | 1              | True           | True          | Enable           |               |
--   0             | 0              | True           | True          | Enable           |               |
--   1             | 0              | True           | True          | Enable           |               |
--   1             | 1              | True           | True          | Enable           |               |
  params = {
    { type = "values",
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%string", name="pp-port-number", help="port number | all" },
      mandatory = {"pp-device-number", "pp-port-number"}
    }
  }
})



CLI_addCommand("pdl_test", "show network-port hw packet-processor-port-gbic all", {
    func   = pdl_network_get_hw_packet_processor_port_gbic_info_all,
    help   = "show network-port gbic hw status",  
})


CLI_addCommand("pdl_test", "show network-port info front-panel-port", {
    func   = pdl_network_get_front_panel_port_info,
    help   = "show network-port front-panel to presetera device & port mappings",
    first_optional = "port-number",
--          Examples:
--      show network-port info front-panel-port 1 11
--      Group Number | Group Port Number | pp-dev-number | pp-port-number |
--      ------------ | ----------------- | ------------- | -------------- |
--      1            | 11                | 0             | 13             |
--
--      show network-port info front-panel-port 3
--      Group Number | Group Port Number | pp-dev-number | pp-port-number |
--      ------------ | ----------------- | ------------- | -------------- |
--      3            | 1                 | 0             | 1              |
--      3            | 2                 | 0             | 0              |
--      3            | 3                 | 1             | 0              |
--      3            | 4                 | 1             | 1              |
--      show network-port info front-panel-port all
--      Group Number | Group Port Number | pp-dev-number | pp-port-number |
--      ------------ | ----------------- | ------------- | -------------- |
--      1            | 1                 | 1             | 29             |
--      1            | 2                 | 1             | 31             |
--      1            | 3                 | 1             | 25             |
--      1            | 4                 | 1             | 27             |
--      1            | 5                 | 1             | 21             |
--      1            | 6                 | 1             | 23             |
--      1            | 7                 | 1             | 17             |
--      1            | 8                 | 1             | 19             |
--      1            | 9                 | 1             | 13             |
--      1            | 10                | 1             | 15             |
--      1            | 11                | 0             | 13             |
--      1            | 12                | 0             | 15             |
--      1            | 13                | 0             | 29             |
--      1            | 14                | 0             | 31             |
--      1            | 15                | 0             | 25             |
--      1            | 16                | 0             | 27             |
--      1            | 17                | 0             | 21             |
--      1            | 18                | 0             | 23             |
--      1            | 19                | 0             | 17             |
--      1            | 20                | 0             | 19             |
--      1            | 25                | 1             | 28             |
--      1            | 26                | 1             | 30             |
--      1            | 27                | 1             | 24             |
--      1            | 28                | 1             | 26             |
--      1            | 29                | 1             | 20             |
--      1            | 30                | 1             | 22             |
--      1            | 31                | 1             | 16             |
--      1            | 32                | 1             | 18             |
--      1            | 33                | 1             | 12             |
--      1            | 34                | 1             | 14             |
--      1            | 35                | 0             | 12             |
--      1            | 36                | 0             | 14             |
--      1            | 37                | 0             | 28             |
--      1            | 38                | 0             | 30             |
--      1            | 39                | 0             | 24             |
--      1            | 40                | 0             | 26             |
--      1            | 41                | 0             | 20             |
--      1            | 42                | 0             | 22             |
--      1            | 43                | 0             | 16             |
--      1            | 44                | 0             | 18             |
--      2            | 21                | 0             | 9              |
--      2            | 22                | 0             | 11             |
--      2            | 23                | 1             | 9              |
--      2            | 24                | 1             | 11             |
--      2            | 45                | 0             | 8              |
--      2            | 46                | 0             | 10             |
--      2            | 47                | 1             | 8              |
--      2            | 48                | 1             | 10             |
--      3            | 1                 | 0             | 1              |
--      3            | 2                 | 0             | 0              |
--      3            | 3                 | 1             | 0              |
--      3            | 4                 | 1             | 1              |
 

  params = {
    { type = "values",                  
      {format="%integer", name="group-number", help="group number" },
      {format="%integer", optional = true, name="port-number", help="port number" },
      mandatory = { "group-number" }
    }
  }
})


CLI_addCommand("pdl_test", "show network-port info front-panel-port all", {
    func   = pdl_network_get_front_panel_port_info_all,
    help   = "show network-port front-panel to device & port mappings",
})


CLI_addCommand("pdl_test", "show network-port info front-panel-group", {
    func   = pdl_network_get_front_panel_group_info,
    help   = "show network-port front-panel-group XML information",
--      Examples:
--      show network-port info front-panel-group 3
--      Group Number |  Ordering  |    Speed   | Port List |
--      ------------ | ---------- | ---------- | --------- |
--      3            | DownRight  | 10G        | 1         |
--      3            | DownRight  | 10G        | 2         |
--      3            | DownRight  | 10G        | 3         |
--      3            | DownRight  | 10G        | 4         |
--      show network-port info front-panel-group all
--      Group Number |  Ordering  |   Speed   | Port List |
--      ------------ | ---------- | --------- | --------- |
--      1            | RightDown  | 1G        | 1         |
--      1            | RightDown  | 1G        | 2         |
--      1            | RightDown  | 1G        | 3         |
--      1            | RightDown  | 1G        | 4         |
--      1            | RightDown  | 1G        | 5         |
--      1            | RightDown  | 1G        | 6         |
--      1            | RightDown  | 1G        | 7         |
--      1            | RightDown  | 1G        | 8         |
--      1            | RightDown  | 1G        | 9         |
--      1            | RightDown  | 1G        | 10        |
--      1            | RightDown  | 1G        | 11        |
--      1            | RightDown  | 1G        | 12        |
--      1            | RightDown  | 1G        | 13        |
--      1            | RightDown  | 1G        | 14        |
--      1            | RightDown  | 1G        | 15        |
--      1            | RightDown  | 1G        | 16        |
--      1            | RightDown  | 1G        | 17        |
--      1            | RightDown  | 1G        | 18        |
--      1            | RightDown  | 1G        | 19        |
--      1            | RightDown  | 1G        | 20        |
--      1            | RightDown  | 1G        | 25        |
--      1            | RightDown  | 1G        | 26        |
--      1            | RightDown  | 1G        | 27        |
--      1            | RightDown  | 1G        | 28        |
--      1            | RightDown  | 1G        | 29        |
--      1            | RightDown  | 1G        | 30        |
--      1            | RightDown  | 1G        | 31        |
--      1            | RightDown  | 1G        | 32        |
--      1            | RightDown  | 1G        | 33        |
--      1            | RightDown  | 1G        | 34        |
--      1            | RightDown  | 1G        | 35        |
--      1            | RightDown  | 1G        | 36        |
--      1            | RightDown  | 1G        | 37        |
--      1            | RightDown  | 1G        | 38        |
--      1            | RightDown  | 1G        | 39        |
--      1            | RightDown  | 1G        | 40        |
--      1            | RightDown  | 1G        | 41        |
--      1            | RightDown  | 1G        | 42        |
--      1            | RightDown  | 1G        | 43        |
--      1            | RightDown  | 1G        | 44        |
--      2            | RightDown  | 5G        | 21        |
--      2            | RightDown  | 5G        | 22        |
--      2            | RightDown  | 5G        | 23        |
--      2            | RightDown  | 5G        | 24        |
--      2            | RightDown  | 5G        | 45        |
--      2            | RightDown  | 5G        | 46        |
--      2            | RightDown  | 5G        | 47        |
--      2            | RightDown  | 5G        | 48        |
--      3            | DownRight  | 10G       | 1         |
--      3            | DownRight  | 10G       | 2         |
--      3            | DownRight  | 10G       | 3         |
--      3            | DownRight  | 10G       | 4         |


  params = {
    { type = "values",                  
      {format="%integer", name="group-number", help="group number" },
      mandatory = { "group-number" }
    }
  }
})


CLI_addCommand("pdl_test", "show network-port info front-panel-group all", {
    func   = pdl_network_get_front_panel_group_info_all,
    help   = "show all front-panel-group info",

})



CLI_addCommand("pdl_test", "show network-port info packet-processor-port", {
    func   = pdl_network_get_packet_processor_port_info,
    help   = "show network-port XML information",
    first_optional = "pp-port-number",
--  Examples:
--  show network-port info packet-processor-port 1 1
--  pp-dev-number | pp-port-number | transceiver type | speed/mode list | serdes list |
--  ------------- | -------------- | ---------------- | --------------- |------------ |
--  1             | 30             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  show network-port info packet-processor-port all
--  pp-dev-number | pp-port-number | transceiver type | speed/mode list | serdes list |
--  ------------- | -------------- | ---------------- | --------------- |------------ |
--  1             | 29             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 31             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 25             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 27             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 21             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 23             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 17             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 19             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 13             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 15             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 13             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 15             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 29             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 31             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 25             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 27             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 21             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 23             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 17             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 19             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 28             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 30             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 24             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 26             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 20             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 22             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 16             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 18             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 12             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  1             | 14             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 12             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 14             | copper           |                 |12,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 28             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 30             | copper           |                 |28,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 24             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 26             | copper           |                 |24,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 20             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 22             | copper           |                 |20,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 16             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 18             | copper           |                 |16,          |
--                |                |                  | QSGMII, 100M    |             |
--                |                |                  | QSGMII, 10M     |             |
--                |                |                  | QSGMII, 1G      |             |
--  0             | 9              | copper           |                 |9,           |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  0             | 11             | copper           |                 |11,          |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  1             | 9              | copper           |                 |9,           |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  1             | 11             | copper           |                 |11,          |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  0             | 8              | copper           |                 |8,           |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  0             | 10             | copper           |                 |10,          |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  1             | 8              | copper           |                 |8,           |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  1             | 10             | copper           |                 |10,          |
--                |                |                  | SGMII, 100M     |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--  0             | 1              | combo            |                 |1,           |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | SGMII, 1G       |             |
--  0             | 0              | combo            |                 |0,           |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | SGMII, 1G       |             |
--                |                |                  | SGMII, 2.5G     |             |
--                |                |                  | KR, 5G          |             |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | SGMII, 1G       |             |
--  1             | 0              | fiber            |                 |0,           |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | 1000BASE-X, 1G  |             |
--  1             | 1              | fiber            |                 |1,           |
--                |                |                  | SR-LR, 10G      |             |
--                |                |                  | 1000BASE-X, 1G  |             |
   


  params = {
    { type = "values",                  
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%integer", optional = true, name="pp-port-number", help="packet-processor port number" },
      mandatory = { "pp-device-number" }
    }
  }
})


CLI_addCommand("pdl_test", "show network-port info packet-processor-port all", {
    func   = pdl_network_get_packet_processor_port_info_all,
    help   = "show network-port XML information",
 
})



CLI_addCommand("pdl_test", "show network-port info packet-processor-port-phy", {
    func   = pdl_network_get_packet_processor_port_phy_info,
    help   = "show network-port phy XML information",
    first_optional = "pp-port-number",
--  Examples:
--  show network-port info packet-processor-port-phy 1 10
--  pp-dev-number | pp-port-number |      phy type      | phy number | phy position | phy interface type | phy address |
--  ------------- | -------------- | ------------------ | ---------- | ------------ | ------------------ | ----------- |
--  1             | 10             | alaska-88E2180     | 5          | 6            | Xsmi               | 6           |
--  show network-port info packet-processor-port-phy all
--  pp-dev-number | pp-port-number |      phy type      | phy number | phy position | phy interface type | phy address |
--  ------------- | -------------- | ------------------ | ---------- | ------------ | ------------------ | ----------- |
--  1             | 29             | alaska-88E1680L    | 0          | 1            | smi                | 1           |
--  1             | 31             | alaska-88E1680L    | 0          | 3            | smi                | 3           |
--  1             | 25             | alaska-88E1680L    | 0          | 5            | smi                | 5           |
--  1             | 27             | alaska-88E1680L    | 0          | 7            | smi                | 7           |
--  1             | 21             | alaska-88E1680L    | 1          | 1            | smi                | 9           |
--  1             | 23             | alaska-88E1680L    | 1          | 3            | smi                | 11          |
--  1             | 17             | alaska-88E1680L    | 1          | 5            | smi                | 13          |
--  1             | 19             | alaska-88E1680L    | 1          | 7            | smi                | 15          |
--  1             | 13             | alaska-88E1680L    | 2          | 1            | smi                | 1           |
--  1             | 15             | alaska-88E1680L    | 2          | 3            | smi                | 3           |
--  0             | 13             | alaska-88E1680L    | 2          | 5            | smi                | 5           |
--  0             | 15             | alaska-88E1680L    | 2          | 7            | smi                | 7           |
--  0             | 29             | alaska-88E1680L    | 3          | 1            | smi                | 1           |
--  0             | 31             | alaska-88E1680L    | 3          | 3            | smi                | 3           |
--  0             | 25             | alaska-88E1680L    | 3          | 5            | smi                | 5           |
--  0             | 27             | alaska-88E1680L    | 3          | 7            | smi                | 7           |
--  0             | 21             | alaska-88E1680L    | 4          | 1            | smi                | 9           |
--  0             | 23             | alaska-88E1680L    | 4          | 3            | smi                | 11          |
--  0             | 17             | alaska-88E1680L    | 4          | 5            | smi                | 13          |
--  0             | 19             | alaska-88E1680L    | 4          | 7            | smi                | 15          |
--  1             | 28             | alaska-88E1680L    | 0          | 0            | smi                | 0           |
--  1             | 30             | alaska-88E1680L    | 0          | 2            | smi                | 2           |
--  1             | 24             | alaska-88E1680L    | 0          | 4            | smi                | 4           |
--  1             | 26             | alaska-88E1680L    | 0          | 6            | smi                | 6           |
--  1             | 20             | alaska-88E1680L    | 1          | 0            | smi                | 8           |
--  1             | 22             | alaska-88E1680L    | 1          | 2            | smi                | 10          |
--  1             | 16             | alaska-88E1680L    | 1          | 4            | smi                | 12          |
--  1             | 18             | alaska-88E1680L    | 1          | 6            | smi                | 14          |
--  1             | 12             | alaska-88E1680L    | 2          | 0            | smi                | 0           |
--  1             | 14             | alaska-88E1680L    | 2          | 2            | smi                | 2           |
--  0             | 12             | alaska-88E1680L    | 2          | 4            | smi                | 4           |
--  0             | 14             | alaska-88E1680L    | 2          | 6            | smi                | 6           |
--  0             | 28             | alaska-88E1680L    | 3          | 0            | smi                | 0           |
--  0             | 30             | alaska-88E1680L    | 3          | 2            | smi                | 2           |
--  0             | 24             | alaska-88E1680L    | 3          | 4            | smi                | 4           |
--  0             | 26             | alaska-88E1680L    | 3          | 6            | smi                | 6           |
--  0             | 20             | alaska-88E1680L    | 4          | 0            | smi                | 8           |
--  0             | 22             | alaska-88E1680L    | 4          | 2            | smi                | 10          |
--  0             | 16             | alaska-88E1680L    | 4          | 4            | smi                | 12          |
--  0             | 18             | alaska-88E1680L    | 4          | 6            | smi                | 14          |
--  0             | 9              | alaska-88E2180     | 5          | 1            | Xsmi               | 1           |
--  0             | 11             | alaska-88E2180     | 5          | 3            | Xsmi               | 3           |
--  1             | 9              | alaska-88E2180     | 5          | 5            | Xsmi               | 5           |
--  1             | 11             | alaska-88E2180     | 5          | 7            | Xsmi               | 7           |
--  0             | 8              | alaska-88E2180     | 5          | 0            | Xsmi               | 0           |
--  0             | 10             | alaska-88E2180     | 5          | 2            | Xsmi               | 2           |
--  1             | 8              | alaska-88E2180     | 5          | 4            | Xsmi               | 4           |
--  1             | 10             | alaska-88E2180     | 5          | 6            | Xsmi               | 6           |
--  0             | 1              | alaska-88E33X0     | 7          | 0            | Xsmi               | 0           |
--  0             | 0              | alaska-88E33X0     | 6          | 0            | Xsmi               | 1           |
--  1             | 0              | Doesn't exist      | 0          | 0            | Not Found          | 0           |
--  1             | 1              | Doesn't exist      | 0          | 0            | Not Found          | 0           |


  params = {
    { type = "values",                  
      {format="%integer", name="pp-device-number", help="device number" },
      {format="%integer", optional = true, name="pp-port-number", help="packet-processor port number" },
      mandatory = { "pp-device-number" }
    }
  }
})


CLI_addCommand("pdl_test", "show network-port info packet-processor-port-phy all", {
    func   = pdl_network_get_packet_processor_port_phy_info_all,
    help   = "show network-port phy XML information",
  
})
  
    
    
  CLI_addCommand("pdl_test", "set network-port info phy number", {
      func   = pdl_inband_set_info_phy_id,
      help   = "change phy-id for network-port in db",
    params = {
      { type = "values",
        {format="%integer", name="dev", help="device number" },
        {format="%integer", name="port", help="port number" },
        {format="%integer", name="phy-id", help="phy number" },
        mandatory={"dev", "port", "phy-id"}
      }
    }
 })
  
  CLI_addCommand("pdl_test", "set network-port info phy position", {
      func   = pdl_inband_set_info_phy_position,
      help   = "change phy position for network-port in db",
    params = {
      { type = "values",
        {format="%integer", name="dev", help="device number" },
        {format="%integer", name="port", help="port number" },
        {format="%integer", name="phy-pos", help="position in phy" },
        mandatory={"dev", "port", "phy-pos"}
      }
    }
 })
  
  CLI_addCommand("pdl_test", "set network-port info interface mode", {
      func   = pdl_inband_set_info_interface_mode,
      help   = "change speed & interface mode for network-port in db",
    params = {
      { type = "values",
        {format="%integer", name="dev", help="device number" },
        {format="%integer", name="port", help="port number" },
        {format="%transceiverType", name="trans", help="transceiver parameter"},
        {format="%speedType", name="speed", help="speed parameter"},
        {format="%ifModeType", name="mode", help="mode parameter"},
        mandatory={"dev", "port", "trans", "speed", "mode"}
      }
    }
 })
