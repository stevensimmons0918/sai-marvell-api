--********************************************************************************
--*              (c), Copyright 2018 Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mpd.lua
--*
--* DESCRIPTION:
--*       setting of mpd commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_mdix_oper")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_mdix_admin")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_autoneg_admin")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_ext_vct_params")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_cable_len")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_autoneg_remote_capabilities")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_link_partner_pause_capable")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_green_power_consumption")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_green_readiness")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_cable_len_no_range")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_phy_kind_and_media")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_eee_capability")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_vct_capability")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_internal_oper_status")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_dte_status")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_temperature")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_revision")
cmdLuaCLI_registerCfunction("wr_utils_mpd_get_autoneg_support")

cmdLuaCLI_registerCfunction("wr_utils_mpd_set_autoneg_enable") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_autoneg_disable")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_restart_autoneg")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_duplex_mode") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_speed") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_combo_media_type")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_mdix_mode") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_vct_test") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_reset_phy") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_phy_disable_oper") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_advertise_fc") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_present_notification") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_smi_access") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_xsmi_access") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_loop_back") 
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_check_link_up")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_power_modules")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_eee_advertize_capability")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_eee_master_enable")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_lpi_exit_time")
cmdLuaCLI_registerCfunction("wr_utils_mpd_set_lpi_enter_time")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function mpd_get_mdix_oper(params)
  local api_result=1
    api_result = wr_utils_mpd_get_mdix_oper(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_mdix_admin(params)
  local api_result=1
    api_result = wr_utils_mpd_get_mdix_admin(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_autoneg_admin(params)
  local api_result=1
    api_result = wr_utils_mpd_get_autoneg_admin(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_ext_vct_params(params)
  local api_result=1
    api_result = wr_utils_mpd_get_ext_vct_params(params["relIfindex"], params["vctTestType"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_cable_len(params)
  local api_result=1
    api_result = wr_utils_mpd_get_cable_len(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_autoneg_remote_capabilities(params)
  local api_result=1
    api_result = wr_utils_mpd_get_autoneg_remote_capabilities(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_link_partner_pause_capable(params)
  local api_result=1
    api_result = wr_utils_mpd_get_link_partner_pause_capable(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_green_power_consumption(params)
  local api_result=1
    api_result = wr_utils_mpd_get_green_power_consumption(params["relIfindex"], params["getMax"], params["energyDetect"], params["shortReach"], params["portUp"], params["portSpeed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_green_readiness(params)
  local api_result=1
    api_result = wr_utils_mpd_get_green_readiness(params["relIfindex"], params["readinessType"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_cable_len_no_range(params)
  local api_result=1
    api_result = wr_utils_mpd_get_cable_len_no_range(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_phy_kind_and_media(params)
  local api_result=1
    api_result = wr_utils_mpd_get_phy_kind_and_media(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_eee_capability(params)
  local api_result=1
    api_result = wr_utils_mpd_get_eee_capability(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_internal_oper_status(params)
  local api_result=1
    api_result = wr_utils_mpd_get_internal_oper_status(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_vct_capability(params)
  local api_result=1
    api_result = wr_utils_mpd_get_vct_capability(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_dte_status(params)
  local api_result=1
    api_result = wr_utils_mpd_get_dte_status(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_temperature(params)
  local api_result=1
    api_result = wr_utils_mpd_get_temperature(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_revision(params)
  local api_result=1
    api_result = wr_utils_mpd_get_revision(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_get_autoneg_support(params)
  local api_result=1
    api_result = wr_utils_mpd_get_autoneg_support(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_autoneg_enable(params)
  local api_result=1
    if (params["advertCapabilities2"] == nil) then
        api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"])
    else        
        if (params["advertCapabilities3"] == nil) then
            api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"])
        else
            if (params["advertCapabilities4"] == nil) then
                api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"])
            else
                if (params["advertCapabilities5"] == nil) then
                    api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"], params["advertCapabilities4"])
                else
                    if (params["advertCapabilities6"] == nil) then
                        api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"], params["advertCapabilities4"], params["advertCapabilities5"])
                    else
                        if (params["advertCapabilities7"] == nil) then
                            api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"], params["advertCapabilities4"], params["advertCapabilities5"], params["advertCapabilities6"])
                        else
                            if (params["advertCapabilities8"] == nil) then
                                api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"], params["advertCapabilities4"], params["advertCapabilities5"], params["advertCapabilities6"], params["advertCapabilities7"])
                            else
                                api_result = wr_utils_mpd_set_autoneg_enable(params["relIfindex"], params["masterSlave"], params["advertCapabilities1"], params["advertCapabilities2"], params["advertCapabilities3"], params["advertCapabilities4"], params["advertCapabilities5"], params["advertCapabilities6"], params["advertCapabilities7"], params["advertCapabilities8"])
                            end
                        end
                    end
                end
            end
        end     
    end
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_autoneg_disable(params)
  local api_result=1            
    api_result = wr_utils_mpd_set_autoneg_disable(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_restart_autoneg(params)
  local api_result=1            
    api_result = mpd_set_restart_autoneg(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_duplex_mode(params)
  local api_result=1
    api_result = wr_utils_mpd_set_duplex_mode(params["relIfindex"], params["mode"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_speed(params)
  local api_result=1
    api_result = wr_utils_mpd_set_speed(params["relIfindex"], params["speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_combo_media_type(params)
  local api_result=1
    api_result = wr_utils_mpd_set_combo_media_type(params["relIfindex"], params["fiberPresent"], params["comboMode"], params["speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_mdix_mode(params)
  local api_result=1
    api_result = wr_utils_mpd_set_mdix_mode(params["relIfindex"], params["mode"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_vct_test(params)
  local api_result=1
    api_result = wr_utils_mpd_set_vct_test(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_reset_phy(params)
  local api_result=1
    api_result = wr_utils_mpd_set_reset_phy(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_phy_disable_oper(params)
  local api_result=1
    api_result = wr_utils_mpd_set_phy_disable_oper(params["relIfindex"], params["forceLinkDown"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_advertise_fc(params)
  local api_result=1
    api_result = wr_utils_mpd_set_advertise_fc(params["relIfindex"], params["advertise"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_present_notification(params)
  local api_result=1
    api_result = wr_utils_mpd_set_present_notification(params["relIfindex"], params["fiberPresent"], params["fiberWa"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_smi_access(params)
  local api_result=1
    if (params["dataToWrite"] == nil) then
        api_result = wr_utils_mpd_set_smi_access(params["relIfindex"], params["type"], params["page"], params["address"])
    else        
        api_result = wr_utils_mpd_set_smi_access(params["relIfindex"], params["type"], params["page"], params["address"], params["dataToWrite"])
    end    
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_xsmi_access(params)
  local api_result=1
    if (params["dataToWrite"] == nil) then
        api_result = wr_utils_mpd_set_xsmi_access(params["relIfindex"], params["type"], params["device"], params["address"])
    else        
        api_result = wr_utils_mpd_set_xsmi_access(params["relIfindex"], params["type"], params["device"], params["address"], params["dataToWrite"])
    end    
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_loop_back(params)
  local api_result=1
    api_result = wr_utils_mpd_set_loop_back(params["relIfindex"], params["loop_back_mode"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_check_link_up(params)
  local api_result=1
    api_result = wr_utils_mpd_set_check_link_up(params["relIfindex"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_power_modules(params)
  local api_result=1
    api_result = wr_utils_mpd_set_power_modules(params["relIfindex"], params["energyDetetct"], params["shortReach"], params["resetPhy"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_eee_advertize_capability(params)
  local api_result=1
    api_result = wr_utils_mpd_set_eee_advertize_capability(params["relIfindex"], params["advEnable"], params["speed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_eee_master_enable(params)
  local api_result=1
    api_result = wr_utils_mpd_set_eee_master_enable(params["relIfindex"], params["masterEnable"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_lpi_exit_time(params)
  local api_result=1
    api_result = wr_utils_mpd_set_lpi_exit_time(params["relIfindex"], params["time"], params["eeeSpeed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

local function mpd_set_lpi_enter_time(params)
  local api_result=1
    api_result = wr_utils_mpd_set_lpi_enter_time(params["relIfindex"], params["time"], params["eeeSpeed"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end



--------------------------------------------
-- Help registration: 
--------------------------------------------


--------------------------------------------
-- Types registration: 
--------------------------------------------
CLI_type_dict["advertiseFC"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "advertise flow contorl or not\n",
    enum = {
        ["advertiseFc"] = { value=1, help="advertise flow contorl" },
        ["NotAdvertiseFc"] = { value=0, help="not advertise flow contorl" }
   }
} 

CLI_type_dict["duplexMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "half or full\n",
    enum = {
        ["half"] = { value="MPD_DUPLEX_ADMIN_MODE_HALF_E", help="half duplex" },
        ["full"] = { value="MPD_DUPLEX_ADMIN_MODE_FULL_E", help="full duplex" }
   }
} 

CLI_type_dict["masterSlave"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "master or slave preference\n",
    enum = {
        ["master"] = { value="MPD_AUTONEGPREFERENCE_MASTER_E", help="master preference" },
        ["slave"] = { value="MPD_AUTONEGPREFERENCE_SLAVE_E", help="slave preference" }
   }
}

CLI_type_dict["autoNegCapabilities"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "advertise capabilities\n",
    enum = {
        ["10MH"] = { value="MPD_AUTONEG_CAPABILITIES_TENHALF_CNS", help="10M half duplex" },
        ["10MF"] = { value="MPD_AUTONEG_CAPABILITIES_TENFULL_CNS", help="10M full duplex" },
        ["100MH"] = { value="MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS", help="100M half duplex" },
        ["100MF"] = { value="MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS", help="100M full duplex" },
        ["1GF"] = { value="MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS", help="1G full duplex" },
        ["2.5GF"] = { value="MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS", help="2.5G full duplex" },
        ["5GF"] = { value="MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS", help="5G full duplex" },
        ["10GF"] = { value="MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS", help="10G full duplex" }
   }
}  

CLI_type_dict["speedTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "speed\n",
    enum = {
        ["10M"] = { value="MPD_SPEED_10M_E", help="10M speed" },        
        ["100M"] = { value="MPD_SPEED_100M_E", help="100M speed" },
        ["1G"] = { value="MPD_SPEED_1000M_E", help="1G speed" },                
        ["2.5G"] = { value="MPD_SPEED_2500M_E", help="2.5G speed" },
        ["5G"] = { value="MPD_SPEED_5000M_E", help="5G speed" },
        ["10G"] = { value="MPD_SPEED_10000M_E", help="10G speed" },             
        ["20G"] = { value="MPD_SPEED_20000M_E", help="20G speed" }              
   }
}  

CLI_type_dict["mdixMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "mdi, mdix or auto\n",
    enum = {
        ["mdi"] = { value="MPD_MDI_MODE_MEDIA_E", help="mdi mode" },
        ["mdix"] = { value="MPD_MDIX_MODE_MEDIA_E", help="mdix mode" },
        ["auto"] = { value="MPD_AUTO_MODE_MEDIA_E", help="auto mode" }
   }
}

CLI_type_dict["forceLinkDown"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "force or unforce link down\n",
    enum = {
        ["forceLinkDown"] = { value=1, help="force link down" },
        ["unforceLinkDown"] = { value=0, help="unforce link down" }
   }
}  

CLI_type_dict["smiAccessType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "smi access read or write\n",
    enum = {
        ["read"] = { value=0, help="smi access read" },
        ["write"] = { value=1, help="smi access write" }
   }
}  

CLI_type_dict["xsmiAccessType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "xsmi access read or write\n",
    enum = {
        ["read"] = { value=0, help="xsmi access read" },
        ["write"] = { value=1, help="xsmi access write" }
   }
}  

CLI_type_dict["loopBackMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enable or disable loop back\n",
    enum = {
        ["enable"] = { value=1, help="enable loop back" },
        ["disable"] = { value=0, help="disable loop back" }
   }
} 

CLI_type_dict["fiberPresentTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "fiber present\n",
    enum = {
        ["FiberIsPresent"] = { value=1, help="fiber is present" },
        ["FiberIsNotPresent"] = { value=0, help="fiber is not present" }
   }
} 

CLI_type_dict["comboModeTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "combo mode\n",
    enum = {
        ["forceFiber"] = { value="MPD_COMBO_MODE_FORCE_FIBER_E", help="combo mode force fiber" },        
        ["forceCopper"] = { value="MPD_COMBO_MODE_FORCE_COPPER_E", help="combo mode force copper" },        
        ["preferFiber"] = { value="MPD_COMBO_MODE_PREFER_FIBER_E", help="combo mode prefer fiber" },                
        ["preferCopper"] = { value="MPD_COMBO_MODE_PREFER_COPPER_E", help="combo mode prefer copper" }              
   }
} 

CLI_type_dict["fiberWaTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "fiber workaround is needed or not\n",
    enum = {
        ["fiberWaIsNeeded"] = { value=1, help="fiber workaround is needed" },
        ["fiberWaIsNotNeeded"] = { value=0, help="fiber workaround is not needed" }
   }
} 

CLI_type_dict["vctTestTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "vct test type\n",
    enum = {
        ["cableChannel1"] = { value="MPD_VCT_TEST_TYPE_CABLECHANNEL1_E", help="vct test type cable channel 1" },        
        ["cableChannel2"] = { value="MPD_VCT_TEST_TYPE_CABLECHANNEL2_E", help="vct test type cable channel 2" },
        ["cableChannel3"] = { value="MPD_VCT_TEST_TYPE_CABLECHANNEL3_E", help="vct test type cable channel 3" },              
        ["cableChannel4"] = { value="MPD_VCT_TEST_TYPE_CABLECHANNEL4_E", help="vct test type cable channel 4" },
        ["cablePolarity1"] = { value="MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E", help="vct test type cable polarity 1" },
        ["cablePolarity2"] = { value="MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E", help="vct test type cable polarity 2" },               
        ["cablePolarity3"] = { value="MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E", help="vct test type cable polarity 3" },               
        ["cablePolarity4"] = { value="MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E", help="vct test type cable polarity 4" },
        ["cablePairSkew1"] = { value="MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E", help="vct test type cable pair skew 1" },
        ["cablePairSkew2"] = { value="MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E", help="vct test type cable pair skew 2" },
        ["cablePairSkew3"] = { value="MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E", help="vct test type cable pair skew 3" },
        ["cablePairSkew4"] = { value="MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E", help="vct test type cable pair skew 4" }                       
   }
}  

CLI_type_dict["greenSetTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "green settings\n",
    enum = {
		["greenNoSet"] = { value="MPD_GREEN_NO_SET_E", help="do nothing" },        
        ["greenEnable"] = { value="MPD_GREEN_ENABLE_E", help="enable green" },        
        ["greenDisable"] = { value="MPD_GREEN_DISABLE_E", help="disable green" }
   }
}

CLI_type_dict["greenReadinessTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Green readiness type\n",
    enum = {
        ["greenReadinessSr"] = { value="MPD_GREEN_READINESS_TYPE_SR_E", help="Sr type" },        
        ["greenReadinessEd"] = { value="MPD_GREEN_READINESS_TYPE_ED_E", help="Ed type" }                         
   }
}

CLI_type_dict["eeeSpeedTyp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "speed\n",
    enum = {
        ["10M"] = { value="MPD_EEE_SPEED_10M_E", help="10M speed" },        
        ["100M"] = { value="MPD_EEE_SPEED_100M_E", help="100M speed" },
		["1G"] = { value="MPD_EEE_SPEED_1G_E", help="1G speed" },
        ["10G"] = { value="MPD_EEE_SPEED_10G_E", help="10G speed" },               
        ["2.5G"] = { value="MPD_EEE_SPEED_2500M_E", help="2.5G speed" },
        ["5G"] = { value="MPD_EEE_SPEED_5G_E", help="5G speed" }             
   }
}

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("mpd_test", "mpd get mdix oper", {
    func   = mpd_get_mdix_oper,
    help   = "get mdix hw status",
--          Syntax: mpd get mdix oper <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get mdix admin", {
    func   = mpd_get_mdix_admin,
    help   = "get mdix hw admin",
--          Syntax: mpd get mdix admin <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get autoneg admin capabilities", {
    func   = mpd_get_autoneg_admin,
    help   = " mpd get autoneg admin capabilities <relIfindex>",
--          Syntax: mpd get autoneg admin capabilities <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get ext vct params", {
    func   = mpd_get_ext_vct_params,
    help   = "mpd get ext vct params <relIfindex> <vctTestType>",
--          Syntax: mpd get ext vct params <relIfindex> <vctTestType>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
          {format="%vctTestTyp", name="vctTestType", help="vct test type"},       
        },
        mandatory={ "relIfindex", "vctTestType" }       
    }
})

CLI_addCommand("mpd_test", "mpd get cable len", {
    func   = mpd_get_cable_len,
    help   = "mpd get cable len <relIfindex>",
--          Syntax: mpd get cable len <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get cable len no range", {
    func   = mpd_get_cable_len_no_range,
    help   = "mpd get cable len no range <relIfindex>",
--          Syntax: mpd get cable len no range <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get autoneg remote capabilities", {
    func   = mpd_get_autoneg_remote_capabilities,
    help   = " mpd get autoneg remote capabilities <relIfindex>",
--          Syntax: mpd get autoneg remote capabilities <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get link partner pause capable", {
    func   = mpd_get_link_partner_pause_capable,
    help   = "mpd get link partner pause capable <relIfindex>",
--          Syntax: mpd get link partner pause capable <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get green power consumption", {
    func   = mpd_get_green_power_consumption,
    help   = "mpd get green power consumption <relIfindex> <getMax> <energyDetect> <shortReach> <portUp> <portSpeed> ",
--          Syntax: mpd get green power consumption <relIfindex> <getMax> <energyDetect> <shortReach> <portUp> <portSpeed>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
		  {format="%bool", name="getMax", help="get max power consumption<true/false>"},
          {format="%greenSetTyp", name="energyDetect", help="energy detect enable/disable"},
          {format="%greenSetTyp", name="shortReach", help="short reach enable/disable"},
          {format="%bool", name="portUp", help="is power up"},
          {format="%speedTyp", name="portSpeed", help="speed "},
        },
        mandatory={ "relIfindex", "getMax", "energyDetect" ,"shortReach", "portUp" ,"portSpeed" }
    }
})

CLI_addCommand("mpd_test", "mpd get green readiness", {
    func   = mpd_get_green_readiness,
    help   = "mpd get green readiness <relIfindex> <readinessType>",
--          Syntax: mpd get green readiness <relIfindex> <readinessType>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
		  {format="%greenReadinessTyp", name="readinessType", help="readinessType"},
        },
        mandatory={ "relIfindex", "readinessType" }      
    }
})

CLI_addCommand("mpd_test", "mpd get phy kind and media", {
    func   = mpd_get_phy_kind_and_media,
    help   = "mpd get phy kind and media <relIfindex>",
--          Syntax: mpd get phy kind and media <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get eee capability", {
    func   = mpd_get_eee_capability,
    help   = "mpd get eee capability <relIfindex>",
--          Syntax: mpd get eee capability <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get vct capability", {
    func   = mpd_get_vct_capability,
    help   = "mpd get vct capability <relIfindex>",
--          Syntax: mpd get vct capability <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get internal oper status", {
    func   = mpd_get_internal_oper_status,
    help   = "mpd get internal oper status <relIfindex>",
--          Syntax: mpd get internal oper status <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get dte status", {
    func   = mpd_get_dte_status,
    help   = "mpd get dte status <relIfindex>",
--          Syntax: mpd get dte status <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get temperature", {
    func   = mpd_get_temperature,
    help   = "mpd get temperature <relIfindex>",
--          Syntax: mpd get temperature <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get revision", {
    func   = mpd_get_revision,
    help   = "mpd get revision <relIfindex>",
--          Syntax: mpd get revision <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd get autoneg support", {
    func   = mpd_get_autoneg_support,
    help   = "mpd get autoneg support <relIfindex>",
--          Syntax: mpd get autoneg support <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set autoneg enable", {
    func   = mpd_set_autoneg_enable,
    help   = "mpd set autoneg enable <relIfindex> <masterSlave> <advertCapabilities1> ... <advertCapabilities8>",
--          Syntax: mpd set autoneg enable <relIfindex> <masterSlave> <advertCapabilities1> ... <advertCapabilities8> 
    first_optional = "advertCapabilities2", -- advertCapabilities2 and all other after it are 'optional'
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},    
          {format="%masterSlave", name="masterSlave", help="master or slave preference"},                     
          {format="%autoNegCapabilities", name="advertCapabilities1", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities2", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities3", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities4", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities5", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities6", help="capabilities"},
          {format="%autoNegCapabilities", name="advertCapabilities7", help="capabilities"},       
          {format="%autoNegCapabilities", name="advertCapabilities8", help="capabilities"},       
        },
        mandatory={ "relIfindex", "masterSlave", "advertCapabilities1" }        
    }
})

CLI_addCommand("mpd_test", "mpd set autoneg disable", {
    func   = mpd_set_autoneg_disable,
    help   = "mpd set autoneg disable <relIfindex>",
--          Syntax: mpd set autoneg disable <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},              
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set restart autoneg", {
    func   = mpd_set_restart_autoneg,
    help   = "mpd set restart autoneg <relIfindex>",
--          Syntax: mpd set restart autoneg <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},              
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set duplex mode", {
    func   = mpd_set_duplex_mode,
    help   = "mpd set duplex mode <relIfindex> <mode>",
--          Syntax: mpd set duplex mode <relIfindex> <mode> 
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%duplexMode", name="mode", help="half or full"},       
        },
        mandatory={ "relIfindex", "mode" }      
    }
})

CLI_addCommand("mpd_test", "mpd set speed", {
    func   = mpd_set_speed,
    help   = "mpd set speed <relIfindex> <speed>",
--          Syntax: mpd set speed <relIfindex> <speed> 
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
          {format="%speedTyp", name="speed", help="speed"},       
        },
        mandatory={ "relIfindex", "speed" }     
    }
})

CLI_addCommand("mpd_test", "mpd set combo media type", {
    func   = mpd_set_combo_media_type,
    help   = "mpd set combo media type <relIfindex> <fiberPresent> <comboMode> <speed>",
--          Syntax: mpd set speed <relIfindex> <fiberPresent> <comboMode> <speed>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
          {format="%fiberPresentTyp", name="fiberPresent", help="fiber present"},
          {format="%comboModeTyp", name="comboMode", help="combo mode"},          
          {format="%speedTyp", name="speed", help="speed"},               
        },
        mandatory={ "relIfindex", "fiberPresent", "comboMode", "speed" }        
    }
})

CLI_addCommand("mpd_test", "mpd set mdix mode", {
    func   = mpd_set_mdix_mode,
    help   = "mpd set mdix mode <relIfindex> <mode>",
--          Syntax: mpd set mdix mode <relIfindex> <mode>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
          {format="%mdixMode", name="mode", help="mdi, mdix or auto"},        
        },
        mandatory={ "relIfindex", "mode" }      
    }
})

CLI_addCommand("mpd_test", "mpd set vct test", {
    func   = mpd_set_vct_test,
    help   = "mpd set vct test <relIfindex>",
--          Syntax: mpd set vct test <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set reset phy", {
    func   = mpd_set_reset_phy,
    help   = "mpd set reset phy <relIfindex>",
--          Syntax: mpd set reset phy <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set phy disable oper", {
    func   = mpd_set_phy_disable_oper,
    help   = "mpd set phy disable oper <relIfindex> <forceLinkDown>",
--          Syntax: mpd set phy disable oper <relIfindex> <forceLinkDown> 
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%forceLinkDown", name="forceLinkDown", help="force or unforce link down"},         
        },
        mandatory={ "relIfindex", "forceLinkDown" }     
    }
})

CLI_addCommand("mpd_test", "mpd set advertise fc", {
    func   = mpd_set_advertise_fc,
    help   = "mpd set advertise fc <relIfindex> <advertise>",
--          Syntax: mpd set advertise fc <relIfindex> <advertise> 
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%advertiseFC", name="advertise", help="true or false"},        
        },
        mandatory={ "relIfindex", "advertise" }     
    }
})

CLI_addCommand("mpd_test", "mpd set present notification", {
    func   = mpd_set_present_notification,
    help   = "mpd set present notification <relIfindex> <fiberPresent> <fiberWa>",
--          Syntax: mpd set present notification <relIfindex> <fiberPresent> <fiberWa>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
          {format="%fiberPresentTyp", name="fiberPresent", help="fiber present"},
          {format="%fiberWaTyp", name="fiberWa", help="fiber workaround"},                
        },
        mandatory={ "relIfindex", "fiberPresent", "fiberWa" }       
    }
})

CLI_addCommand("mpd_test", "mpd set smi access", {
    func   = mpd_set_smi_access,
    help   = "mpd set smi access <relIfindex> read <page> <address> or mpd set smi access <relIfindex> write <page> <address> <data>",
--          Syntax: mpd set smi access <relIfindex> <type> <page> <address> <data>
    first_optional = "dataToWrite", -- data is 'optional' - if entered means data that should be written otherwise parameter is not valid
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%smiAccessType", name="type", help="smi access read or write"},        
          {format="%integer", name="page", help="page number"},
          {format="%integer", name="address", help="register number"},
          {format="%integer", name="dataToWrite", help="data that should be written"},
        },
        mandatory={ "relIfindex", "type", "page", "address" }       
    }
})

CLI_addCommand("mpd_test", "mpd set xsmi access", {
    func   = mpd_set_xsmi_access,
    help   = "mpd set xsmi access <relIfindex> read <device> <address> or mpd set xsmi access <relIfindex> write <device> <address> <data>",
--          Syntax: mpd set xsmi access <relIfindex> <type> <device> <address> <data>
    first_optional = "dataToWrite", -- data is 'optional' - if entered means data that should be written otherwise parameter is not valid
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%xsmiAccessType", name="type", help="xsmi access read or write"},          
          {format="%integer", name="device", help="device number"},
          {format="%integer", name="address", help="register number"},
          {format="%integer", name="dataToWrite", help="data that should be written"},
        },
        mandatory={ "relIfindex", "type", "device", "address" }     
    }
})

CLI_addCommand("mpd_test", "mpd set loop back", {
    func   = mpd_set_loop_back,
    help   = "mpd set loop back <relIfindex> <loop_back_mode>",
--          Syntax: mpd set loop back <relIfindex> <loop_back_mode> 
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},          
          {format="%loopBackMode", name="loop_back_mode", help="enable or disable"},          
        },
        mandatory={ "relIfindex", "loop_back_mode" }        
    }
})

CLI_addCommand("mpd_test", "mpd set check link up", {
    func   = mpd_set_check_link_up,
    help   = "mpd set check link up <relIfindex>",
--          Syntax: mpd set check link up <relIfindex>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},                  
        },
        mandatory={ "relIfindex" }      
    }
})

CLI_addCommand("mpd_test", "mpd set power modules", {
    func   = mpd_set_power_modules,
    help   = "mpd set power modules <relIfindex> <energyDetetct> <shortReach> <resetPhy>",
--          Syntax: mpd set power modules <relIfindex> <energyDetetct> <shortReach> <resetPhy>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
		  {format="%greenSetTyp", name="energyDetetct", help="energyDetetct"}, 
		  {format="%greenSetTyp", name="shortReach", help="shortReach"}, 
		  {format="%bool", name="resetPhy", help="perform phy rese - true/false"}, 
        },
        mandatory={ "relIfindex", "energyDetetct", "shortReach", "resetPhy" }      
    }
})

CLI_addCommand("mpd_test", "mpd set eee advertize capability", {
    func   = mpd_set_eee_advertize_capability,
    help   = "mpd set eee advertize capability <relIfindex> <advEnable> <speed>",
--          Syntax: mpd set eee advertize capability <relIfindex> <advEnable> <speed>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
		  {format="%bool", name="advEnable", help="Advertise eee true/false"}, 
		  {format="%speedTyp", name="speed", help="speeds to advertise eee"}, 
        },
        mandatory={ "relIfindex", "advEnable", "speed" }      
    }
})

CLI_addCommand("mpd_test", "mpd set lpi exitTime", {
    func   = mpd_set_lpi_exit_time,
    help   = "mpd set lpi exitTime <relIfindex> <time> <eeeSpeed>",
--          Syntax: mpd set lpi exitTime <relIfindex> <time> <eeeSpeed>
    params = {
        { 
          type = "values",
          {format="%integer", name="relIfindex", help="relIfindex"},
		  {format="%integer", name="time", help="time in microseconds "},
		  {format="%eeeSpeedTyp", name="eeeSpeed", help="speed to configure"},
        },
        mandatory={ "relIfindex", "time", "eeeSpeed" }      
    }
})
