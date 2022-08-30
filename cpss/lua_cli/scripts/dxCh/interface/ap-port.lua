--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ap-port.lua
--*
--* DESCRIPTION:
--*     Enable/Disable AP process on port
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  ap_func
--        @description  enable/disable ap process on port
--
--
--
--        @return       error message if fails
--
local function ap_func(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParamsPtr={}
  local fecSupported, fecRequested
  local speed, ifMode

  if params.advertisement~=nil then
      if params.flagNo then
          enable = false
      else
          speed = params.advertisement[1]
          ifMode = params.advertisement[2]
          fecSupported = params.advertisement[3]
          fecRequested = params.advertisement[4]
          if params.flow_control~=nil then
              apParamsPtr.fcPause=GT_TRUE
              apParamsPtr.fcAsmDir=fcToVal(params.flow_control)
          else
              apParamsPtr.fcPause=GT_FALSE
              apParamsPtr.fcAsmDir=0
          end

          if params.nonce_match~=nil then
               apParamsPtr.noneceDisable=GT_TRUE           --noneceDisable
          else
               apParamsPtr.noneceDisable=GT_FALSE           --noneceDisable
          end
          if params.lane_num~=nil then
              apParamsPtr.laneNum=params.lane_num
          else
              apParamsPtr.laneNum=0         --laneNum
          end

          if params.ic_profile~=nil then
              apParamsPtr.interconnectProfile=profileToVal(params.ic_profile)
          else
              apParamsPtr.interconnectProfile=0         --default profile
          end
      end
  end

  if fecSupported ~= "CPSS_DXCH_PORT_FEC_MODE_DISABLED_E" then
      apParamsPtr.fecSupported = true
  else
      apParamsPtr.fecSupported = false
  end

  if fecRequested ~= "CPSS_DXCH_PORT_FEC_MODE_DISABLED_E" then
      apParamsPtr.fecRequired = true
  else
      apParamsPtr.fecRequired = false
  end


  apParamsPtr.modesAdvertiseArr={}
  apParamsPtr.fecAbilityArr={}
  apParamsPtr.fecRequestedArr={}

  apParamsPtr.modesAdvertiseArr[0]={}          --mode1 speed1
  apParamsPtr.modesAdvertiseArr[0].ifMode=ifMode
  apParamsPtr.modesAdvertiseArr[0].speed=speed
  apParamsPtr.fecAbilityArr[0]=fecSupported
  apParamsPtr.fecRequestedArr[0]=fecRequested

  enable = command_data:getTrueIfFlagNoIsNil(params)

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                                    command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssDxChPortApPortConfigSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_BOOL","apEnable",enable},
          {"IN","CPSS_DXCH_PORT_AP_PARAMS_STC","apParamsPtr",apParamsPtr}})
        if(ret~=GT_OK) then
          print("Error at ap command")
        end
      end
    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function ap_func_port_mgr(params)
    local result, values
    local command_data = Command_Data()
    local devNum, portNum
    local event = {}
    local iterator
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local GT_BAD_STATE = 0x7
    local nonceDisable, fcPause, fcAsmDir, negotiationLaneNum, interconnectProfile, skipRes
    local ifMode, speed, fecSupported, fecRequested
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    if params.flagNo then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            if params.advertisement~=nil then
                event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
                result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                              {{ "IN", "GT_U8", "devNum", devNum},
                               { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                               { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

                if result~=GT_OK and result~=GT_NOT_INITIALIZED then
                    print("Error at command: cpssDxChPortManagerEventSet :%d", result)
                end
            else
                result, values = myGenWrapper("cpssDxChPortManagerPortParamsGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})
                if(result~=GT_OK) then
                    print("Error at command: cpssDxChPortManagerPortParamsGet :%d", result)
                end

                if params.nonce_match~=nil then
                    nonceDisable =false
                else
                    nonceDisable = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["nonceDisable"]
                end

                if params.flow_control~=nil then
                    fcPause=false
                else
                    fcPause=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["fcPause"]
                    fcAsmDir=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["fcAsmDir"]
                end
                if params.lane_num~=nil then
                    negotiationLaneNum=0
                else
                    negotiationLaneNum=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["negotiationLaneNum"]
                end

                if params.ic_profile~=nil then
                    interconnectProfile =0
                else
                    interconnectProfile = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["interconnectProfile"]
                end

                if params.skipRes~=nil then
                    skipRes =false
                else
                    skipRes = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["skipRes"]
                end

                if params.nonce_match~= nil or params.flow_control~=nil or params.lane_num~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApAttributeParamsSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "GT_BOOL", "nonceDisable", nonceDisable},
                           { "IN", "GT_BOOL", "fcPause", fcPause},
                           { "IN", "CPSS_PORT_AP_FLOW_CONTROL_ENT", "fcAsmDir", fcAsmDir},
                           { "IN", "GT_U32", "negotiationLaneNum", negotiationLaneNum}})

                   if result == GT_NOT_INITIALIZED then
                       print("Error : Port not initialized \n")
                   elseif result == GT_BAD_STATE then
                       print("Error : Port is not in reset state \n")
                   elseif result~=GT_OK then
                       print("Error at command: cpssDxChSamplePortManagerApAttributeParamsSet:%s", result)
                   end
               elseif  params.ic_profile~= nil then
                   result, values = myGenWrapper("cpssDxChSamplePortManagerApOverrideInterconnectProfileSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT", "interconnectProfile", interconnectProfile}})
                   if result == GT_NOT_INITIALIZED then
                       print("Error : Port not initialized \n")
                   elseif result == GT_BAD_STATE then
                       print("Error : Port is not in reset state \n")
                   elseif result~=GT_OK then
                       print("Error at command: cpssDxChSamplePortManagerApOverrideInterconnectProfileSet:%s", result)
                   end
               elseif params.skipRes~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApSkipResSet",
                           {{ "IN", "GT_U8"  , "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                            { "IN", "GT_BOOL", "skipRes", skipRes}})
                    if result == GT_NOT_INITIALIZED then
                        print("Error : Port not initialized \n")
                    elseif result == GT_BAD_STATE then
                        print("Error : Port is not in reset state \n")
                    elseif result~=GT_OK then
                        print("Error at command: cpssDxChSamplePortManagerApSkipResSet:%s", result)
                    end
                else
                    event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
                    result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                              {{ "IN", "GT_U8", "devNum", devNum},
                               { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                               { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

                    if result~=GT_OK and result~=GT_NOT_INITIALIZED then
                        print("Error at command: cpssDxChPortManagerEventSet :%d", result)
                    end
                end
            end
        end
    else
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            if params.advertisement~=nil then
                event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
                result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                             {{ "IN", "GT_U8", "devNum", devNum},
                              { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                              { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

                if result~=GT_OK and result~=GT_NOT_INITIALIZED then
                    print("Error at command: cpssDxChPortManagerEventSet :%d", result)
                    command_data:analyzeCommandExecution()
                    command_data:printCommandExecutionResults()
                    return command_data:getCommandExecutionResults()
                end

                speed = params.advertisement[1]
                ifMode = params.advertisement[2]
                fecSupported = fecToVal(params.advertisement[3])
                fecRequested = fecToVal(params.advertisement[4])

                result, values = myGenWrapper("cpssDxChSamplePortManagerApMandatoryParamsSet",
                      {{ "IN", "GT_U8"  , "devNum", devNum},
                       { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                       { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode", ifMode},
                       { "IN", "CPSS_PORT_SPEED_ENT", "speed", speed},
                       { "IN", "CPSS_PORT_FEC_MODE_ENT", "fecSupported", fecSupported},
                       { "IN", "CPSS_PORT_FEC_MODE_ENT", "fecRequested", fecRequested}})

               if result == GT_NOT_INITIALIZED then
                   print("Error : Port not initialized \n")
               elseif result == GT_BAD_STATE then
                   print("Error : Port is not in reset state \n")
               elseif result~=GT_OK then
                   print("Error at command: cpssDxChSamplePortManagerApMandatoryParamsSet:%s", result)
               end

                if params.nonce_match~=nil then
                    nonceDisable =true
                else
                    nonceDisable =false
                end

                if params.flow_control~=nil then
                    fcPause=true
                    fcAsmDir=fcToVal(params.flow_control)
                else
                    fcPause=false
                    fcAsmDir=0
                end
                if params.lane_num~=nil then
                    negotiationLaneNum=params.lane_num
                else
                    negotiationLaneNum=0
                end

                result, values = myGenWrapper("cpssDxChSamplePortManagerApAttributeParamsSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "GT_BOOL", "nonceDisable", nonceDisable},
                           { "IN", "GT_BOOL", "fcPause", fcPause},
                           { "IN", "CPSS_PORT_AP_FLOW_CONTROL_ENT", "fcAsmDir", fcAsmDir},
                           { "IN", "GT_U32", "negotiationLaneNum", negotiationLaneNum}})

               if result == GT_NOT_INITIALIZED then
                    print("Error : Port not initialized \n")
               elseif result == GT_BAD_STATE then
                    print("Error : Port is not in reset state \n")
               elseif result~=GT_OK then
                    print("Error at command: cpssDxChSamplePortManagerApAttributeParamsSet :%s", result)
               end

               if params.skipRes~=nil then
                    skipRes =true
               else
                    skipRes =false
               end

               if params.skipRes~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApSkipResSet",
                           {{ "IN", "GT_U8"  , "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                            { "IN", "GT_BOOL", "skipRes", skipRes}})
                    if result == GT_NOT_INITIALIZED then
                        print("Error : Port not initialized \n")
                    elseif result == GT_BAD_STATE then
                        print("Error : Port is not in reset state \n")
                    elseif result~=GT_OK then
                        print("Error at command: cpssDxChSamplePortManagerApSkipResSet:%s", result)
                    end
                end

                if params.ic_profile~=nil then
                    interconnectProfile =profileToVal(params.ic_profile)
                else
                    interconnectProfile =0
                end

                if params.ic_profile~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApOverrideInterconnectProfileSet",
                           {{ "IN", "GT_U8"  , "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                            { "IN", "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT", "interconnectProfile", interconnectProfile}})
                    if result == GT_NOT_INITIALIZED then
                        print("Error : Port not initialized \n")
                    elseif result == GT_BAD_STATE then
                        print("Error : Port is not in reset state \n")
                    elseif result~=GT_OK then
                        print("Error at command: cpssDxChSamplePortManagerApOverrideInterconnectProfileSet:%s", result)
                    end
                end

            else
                result, values = myGenWrapper("cpssDxChPortManagerPortParamsGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})
                if(result~=GT_OK) then
                    print("Error at command: cpssDxChPortManagerPortParamsGet :%d", result)
                end

                if params.nonce_match~=nil then
                    nonceDisable =true
                else
                    nonceDisable = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["nonceDisable"]
                end

                if params.flow_control~=nil then
                    fcPause=true
                    fcAsmDir=fcToVal(params.flow_control)
                else
                    fcPause=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["fcPause"]
                    fcAsmDir=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["fcAsmDir"]
                end
                if params.lane_num~=nil then
                    negotiationLaneNum=params.lane_num
                else
                    negotiationLaneNum=values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["negotiationLaneNum"]
                end

                if params.ic_profile~=nil then
                    interconnectProfile =profileToVal(params.ic_profile)
                else
                    interconnectProfile = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["interconnectProfile"]
                end

                if params.skipRes~=nil then
                    skipRes =true
                else
                    skipRes = values["portParamsStcPtr"]["portParamsType"]["apPort"]["apAttrs"]["skipRes"]
                end

                result, values = myGenWrapper("cpssDxChSamplePortManagerApAttributeParamsSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "GT_BOOL", "nonceDisable", nonceDisable},
                           { "IN", "GT_BOOL", "fcPause", fcPause},
                           { "IN", "CPSS_PORT_AP_FLOW_CONTROL_ENT", "fcAsmDir", fcAsmDir},
                           { "IN", "GT_U32", "negotiationLaneNum", negotiationLaneNum}})

               if result == GT_NOT_INITIALIZED then
                   print("Error : Port not initialized \n")
               elseif result == GT_BAD_STATE then
                   print("Error : Port is not in reset state \n")
               elseif result~=GT_OK then
                   print("Error at command: cpssDxChSamplePortManagerApAttributeParamsSet:%s", result)
               end

                if params.ic_profile~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApOverrideInterconnectProfileSet",
                           {{ "IN", "GT_U8"  , "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                            { "IN", "CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT", "interconnectProfile", interconnectProfile}})
                    if result == GT_NOT_INITIALIZED then
                        print("Error : Port not initialized \n")
                    elseif result == GT_BAD_STATE then
                        print("Error : Port is not in reset state \n")
                    elseif result~=GT_OK then
                        print("Error at command: cpssDxChSamplePortManagerApOverrideInterconnectProfileSet:%s", result)
                    end
                end

                if params.skipRes~=nil then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerApSkipResSet",
                           {{ "IN", "GT_U8"  , "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                            { "IN", "GT_BOOL", "skipRes", skipRes}})
                    if result == GT_NOT_INITIALIZED then
                        print("Error : Port not initialized \n")
                    elseif result == GT_BAD_STATE then
                        print("Error : Port is not in reset state \n")
                    elseif result~=GT_OK then
                        print("Error at command: cpssDxChSamplePortManagerApSkipResSet:%s", result)
                    end
                end
            end
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function ap_func_main(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    command_data:initInterfaceRangeIterator()
    command_data:initAllInterfacesPortIterator(params)
    command_data:initInterfaceDeviceRange()

    devNum, portNum = command_data:getFirstPort()
    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                      {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                       { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if (values.enablePtr) then
        ap_func_port_mgr(params)
    else
        ap_func(params)
    end
end

--------------------------------------------
-- command registration: 802.3ap
--------------------------------------------
CLI_addCommand("interface", "802.3ap", {
  func   = ap_func_main,
  help   = "Enable AP process on port",
  params = {
  {   type="named",
            { format="advertisement %ap_port_speed %ap_port_interface_mode %port_fec_mode %port_fec_mode1 ", name="advertisement", multiplevalues=true, help = "802.3ap advertised modes"},
            { format="nonce-match", name="nonce_match", help="Enable 802.3ap nonce-match"},
            { format="flow-control %ap_flow_control", name="flow_control", help="Enable 802.3ap flow-control"},
            { format="lane-num %GT_U32", name="lane_num", help="Enable 802.3ap lane-number negotiation"},
            { format="interconnect profile %ic_profile", name="ic_profile", help="Set Interconnect profile"},
            { format="skipRes", name="skipRes", help="Skip 802.3ap resolution"}
         }}}
)

CLI_addCommand("interface", "no 802.3ap", {
  func = function(params)
    params.flagNo = true
    return ap_func_main(params)
    end,
    params = {
    {   type="named",
            { format="advertisement ", name="advertisement", help = "802.3ap advertised modes"},
            { format="nonce-match", name="nonce_match", help="Enable 802.3ap nonce-match"},
            { format="flow-control ", name="flow_control", help="Enable 802.3ap flow-control"},
            { format="lane-num ", name="lane_num", help="Enable 802.3ap lane-number negotiation"},
            { format="interconnect profile ", name="ic_profile", help="Set Interconnect profile"},
            { format="skipRes", name="skipRes", help="Skip 802.3ap resolution"}
    }},
  help = "Disable AP process on port"
})
