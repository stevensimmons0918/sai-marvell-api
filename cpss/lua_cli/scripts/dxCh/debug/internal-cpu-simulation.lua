--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* internal-cpu-simulation.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for CM3 simulation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local pexMode
local superImageName = "super_image_harrier.bin"
local fwImageName = "firmware_cm3_falcon.bin"





local modeToStr =
{
"_100Base_FX",
"SGMII",
"_1000Base_X",
"SGMII2_5",
 "QSGMII",
 "_10GBase_KX4",
 "_10GBase_KX2",
 "_10GBase_KR",
 "_20GBase_KR2",
 "_40GBase_KR",
 "_100GBase_KR10",
 "HGL",
 "RHGL",
 "CHGL",
 "RXAUI",
 "_20GBase_KX4",
 "_10GBase_SR_LR",
 "_20GBase_SR_LR2",
 "_40GBase_SR_LR",
 "_12_1GBase_KR",
 "XLHGL_KR4",
 "HGL16G",
 "HGS",
 "HGS4",
 "_100GBase_SR10",
 "CHGL_LR12",
 "TCAM",
 "INTLKN_12Lanes_6_25G",
 "INTLKN_16Lanes_6_25G",
 "INTLKN_24Lanes_6_25G",
 "INTLKN_12Lanes_10_3125G",
 "INTLKN_16Lanes_10_3125G",
 "INTLKN_12Lanes_12_5G",
 "INTLKN_16Lanes_12_5G",
 "INTLKN_16Lanes_3_125G",
 "INTLKN_24Lanes_3_125G",
 "CHGL11_LR12",
 "INTLKN_4Lanes_3_125G",
 "INTLKN_8Lanes_3_125G",
 "INTLKN_4Lanes_6_25G",
 "INTLKN_8Lanes_6_25G",
 "_2_5GBase_QX",
 "_5GBase_DQX",
 "_5GBase_HX",
 "_12GBaseR",
 "_5_625GBaseR",
 "_48GBaseR",
 "_12GBase_SR",
 "_48GBase_SR",
 "_5GBaseR",
 "_22GBase_SR",
 "_24GBase_KR2",
 "_12_5GBase_KR",
 "_25GBase_KR2",
 "_50GBase_KR4",
 "_25GBase_KR",
 "_50GBase_KR2",
 "_100GBase_KR4",
 "_25GBase_SR",
 "_50GBase_SR2",
 "_100GBase_SR4",
 "_100GBase_MLG",
 "_107GBase_KR4",
 "_25GBase_KR_C",
 "_50GBase_KR2_C",
 "_40GBase_KR2",
 "_29_09GBase_SR4",
 "_40GBase_CR4",
 "_25GBase_CR",
 "_50GBase_CR2",
 "_100GBase_CR4",
 "_25GBase_KR_S",
 "_25GBase_CR_S",
 "_25GBase_CR_C",
 "_50GBase_CR2_C",
 "_50GBase_KR",
 "_100GBase_KR2",
 "_200GBase_KR4",
 "_200GBase_KR8",
 "_400GBase_KR8",
 "_102GBase_KR4",
 "_52_5GBase_KR2",
 "_40GBase_KR4",
 "_26_7GBase_KR",
 "_40GBase_SR_LR4",
 "_50GBase_CR",
 "_100GBase_CR2",
 "_200GBase_CR4",
 "_200GBase_CR8",
 "_400GBase_CR8",
 "_200GBase_SR_LR4",
 "_200GBase_SR_LR8",
 "_400GBase_SR_LR8",
 "_50GBase_SR_LR",
 "_100GBase_SR_LR2",
 "_20GBase_KR",
 "_106GBase_KR4",
 "_2_5G_SXGMII",
 "_5G_SXGMII",
 "_10G_SXGMII",
 "_5G_DXGMII",
 "_10G_DXGMII",
 "_20G_DXGMII",
 "_5G_QUSGMII",
 "_10G_QXGMII",
 "_20G_QXGMII",
 "_10G_OUSGMII",
 "_20G_OXGMII",
 "_42GBase_KR4",
 "_53GBase_KR2",
 "_424GBase_KR8",
 "_2500Base_X",
 "_212GBase_KR4",
 "_106GBase_KR2"
}

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function get_bit(value,offset,size)
  if(value==nil)then
   print("nil value")
   return 0
  else
   return (math.floor(value/2^offset)%2^size)
  end
end


local function load_micro_init(params)
  local rc, value

 --check that device work with memory access via BAR0,BAR2
 pexMode = myGenWrapper("cpssHwDriverGenWmInPexModeGet",{})
  if pexMode ==0 then
    print("\n usePexLogic flag is not set.\n ")
    return
  end

  if isSimulationUsed()==false or isGmUsed()==true then
   print("\n This option can be activated only on white model.\n ")
   return
  end

   if file_exists(superImageName)==false then
    print("\n File "..superImageName.." not found.")
   return
  end

  if file_exists(fwImageName)==false then
   print("\n File "..fwImageName.." not found.")
   testSupported=false
   return
  end

   print("Load super image \n ")
   rc, value = myGenWrapper("appDemoLoadSuperImageToDdr",{
            {"IN","GT_BOOL","runUboot",1}
        })
   if rc==0 then
   print("Success... \n ")
   else
   print("Fail... \n ")
   end
end



local function opc5_version_get(params)
  local rc, value

   rc, value = myGenWrapper("cm3BootChannelLuaWrapExecuteVersionGet",{
            {"OUT","GT_U32","superImagePtr"},
            {"OUT","GT_U32","fwPtr"},
            {"OUT","GT_U32","bootonPtr"}
        })

   print("rc = "..rc)
   if rc==0 then
    print("SuperImage version "..get_bit(value.superImagePtr,0,8).."."..get_bit(value.superImagePtr,8,8))
    print("FW version "..get_bit(value.fwPtr,0,8).."."..get_bit(value.fwPtr,8,8).."."..get_bit(value.fwPtr,16,8))
    print("Booton version "..get_bit(value.bootonPtr,0,8).."."..get_bit(value.bootonPtr,8,8).."."..get_bit(value.bootonPtr,16,8))
  end
end

local function opc6_port_status_get(params)
  local rc, value

   rc, value = myGenWrapper("cm3BootChannelLuaWrapExecutePortStatusGetCommand",{
            {"IN","GT_U32","portNum",params.portNum},
            {"OUT","GT_U32","bmpPtr"}
        })

   print("rc = "..rc)
   if rc==0 then
    print("Link up "..get_bit(value.bmpPtr,0,1))
    if get_bit(value.bmpPtr,0,1)==1 then
     print("Mode "..modeToStr[get_bit(value.bmpPtr,8,8)+1].." ("..get_bit(value.bmpPtr,8,8)..")")
    end
  end
end


local function opc4_run_config_file(params)
  local rc, value

   rc, value = myGenWrapper("cm3BootChannelChangeProfile",{
            {"IN","GT_U32","profileNum",params.index}
        })

   print("rc = "..rc)

end


local function loopback_enable(params)
  local rc, value

   rc, value = myGenWrapper("cm3BootChannelExecuteCliCommandsLb",{
            {"IN","GT_U32","portNum",params.portNum},
            {"IN","GT_BOOL","enable",params.enable},
        })

   print("rc = "..rc)

end

local function pve_enable(params)
  local rc, value

   rc, value = myGenWrapper("cm3BootChannelExecuteCliCommandsPve",{
            {"IN","GT_U32","portNum",params.portNum},
            {"IN","GT_BOOL","enable",params.enable},
        })

   print("rc = "..rc)

end

local function mi_state_get(params)
  local rc, value
  local stateToStr =
  {  "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E",
     "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E",
     "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E"
  }
   if params.enable == true then
   rc, value = myGenWrapper("cm3BootChannelProcessorInitializedGet",{
            {"OUT","GT_U32","statePtr"},
        })
   else
   rc, value = myGenWrapper("cm3BootChannelExecuteMiStatusGetCommand",{
            {"OUT","GT_U32","statePtr"},
        })
   end
   print("rc = "..rc)
   if rc ==0 then
    print("Micro init state is  "..stateToStr[value.statePtr])
   end


end


local function vlan_add(params)
  local rc, value
  local port,vlan

  port= params.portNum
  vlan = params.vlan

  print("Adding port "..port.." to VLAN "..vlan)
  if params.tag then
  print("Tag command CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E")
  else
  print("Tag command CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E")
  end

  rc = myGenWrapper("cm3BootChannelVlanAction",{
            {"IN","GT_U32","portNum",port},
            {"IN","GT_U32","vlan",vlan},
            {"IN","GT_BOOL","add",true},
            {"IN","GT_BOOL","tag",params.tag},
        })

  print("rc = "..rc)

end

local function vlan_remove(params)
  local rc, value
  local port,vlan

  port= params.portNum
  vlan = params.vlan

  print("Remove port "..port.." to VLAN "..vlan)


  rc = myGenWrapper("cm3BootChannelVlanAction",{
            {"IN","GT_U32","portNum",port},
            {"IN","GT_U32","vlan",vlan},
            {"IN","GT_BOOL","add",false},
            {"IN","GT_BOOL","tag",false},
        })

  print("rc = "..rc)

end






--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  load-micro-init
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  load-micro-init", {
   func = load_micro_init,
   help = "Load micro init simulation"
})

--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  opc5-version-get
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  opc5-version-get", {
   func = opc5_version_get,
   help = "Get version"
})

--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  opc6-port-status-get
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  opc6-port-status-get", {
   func = opc6_port_status_get,
   help = "Get port status",
   params={
        { type="named",
        { format="port %GT_U32",optional = false,name="portNum",help="The port number" }
        }
    }
})
--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  opc4-run-config-file
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  opc4-run-config-file", {
   func = opc4_run_config_file,
   help = "Run configuration file",
   params={
        { type="named",
        { format="index %GT_U32",optional = false,name="index",help="The index of configuration file" }
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  loopback-enable
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  loopback-enable", {
   func = loopback_enable,
   help = "Enable loopback on port",
   params={
        { type="named",
        { format="port %GT_U32",optional = false,name="portNum",help="The port number" },
        { format="enable %bool",optional = false,name="enable",help="enable/disable loopback" }
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  pve-enable
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  pve-enable", {
   func = pve_enable,
   help = "Enable pve on port (target==source)",
   params={
        { type="named",
        { format="port %GT_U32",optional = false,name="portNum",help="The port number" },
        { format="enable %bool",optional = false,name="enable",help="enable/disable loopback" }
        }
    }
})


--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  pve-enable
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  mi-state-get", {
   func = mi_state_get,
   help = "Get micro init state",
   params={
        { type="named",
        { format="direct-read %bool",optional = false,name="enable",help="read from cm3 ram or through opcode" }
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  vlan-add
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  vlan-add", {
   func = vlan_add,
   help = "Add port to VLAN ",
   params={
        { type="named",
        { format="port %GT_U32",optional = false,name="portNum",help="The port number" },
        { format="vlan %GT_U32",optional = false,name="vlan",help="The vlan number" },
        { format="tag %bool",optional = false,name="tag",help="if true then tag is CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E ,else CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E" }
        }
    }
})


--------------------------------------------------------------------------------
---- command registration: internal-cpu-simulation  vlan-remove
--------------------------------------------------------------------------------

CLI_addCommand("debug", "internal-cpu-simulation  vlan-remove", {
   func = vlan_remove,
   help = "Remove port from VLAN ",
   params={
        { type="named",
        { format="port %GT_U32",optional = false,name="portNum",help="The port number" },
        { format="vlan %GT_U32",optional = false,name="vlan",help="The vlan number" }
        }
    }
})



