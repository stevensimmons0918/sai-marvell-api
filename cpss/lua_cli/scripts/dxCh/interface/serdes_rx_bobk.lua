--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_rx.lua
--*
--* DESCRIPTION:
--*     Configure specific parameters of serdes RX in HW
--*
--*
--*
--*
--********************************************************************************

--includes


--constants

-- ************************************************************************
---
--  serdes_rx_func_bobk
--        @description  configure specific parameters of serdes rx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_rx_func_bobk(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local numOfSerdesLanesPtr=0
  local devnum_portnum_string
  local ret
  local serdesRxCfgPtr={}

  local serdes
  ret,serdes = cpssGenWrapper("cpssPortManagerLuaSerdesTypeGet",{
    { "IN",  "GT_SW_DEV_NUM",             "devNum",    devNum },
    { "OUT", "CPSS_PORT_SERDES_TYPE_ENT", "serdesType"        }
  })
  if ret~=0 then
      printLog ("Error in cpssPortManagerLuaSerdesTypeGet")
      setFailState()
  end

  serdesRxCfgPtr.type = serdes.serdesType
  serdesRxCfgPtr.rxTune = {}

  if serdesRxCfgPtr.type == "CPSS_PORT_SERDES_AVAGO_E" then
      serdesRxCfgPtr.rxTune.avago = {}
      if params.sqlch ~= nil then
          serdesRxCfgPtr.rxTune.avago.sqlch = params.sqlch
      end
      if params.dc_gain ~= nil then
          serdesRxCfgPtr.rxTune.avago.DC = params.dc_gain
      end
      if params.frequency ~= nil then
          serdesRxCfgPtr.rxTune.avago.LF = params.frequency[1]
          serdesRxCfgPtr.rxTune.avago.HF = params.frequency[2]
      end
      if params.band_width ~= nil then
          serdesRxCfgPtr.rxTune.avago.BW = params.band_width
      end
  else
      serdesRxCfgPtr.rxTune.comphy = {}
      if params.sqlch ~= nil then
          serdesRxCfgPtr.rxTune.comphy.sqlch  = params.sqlch
      end
      if params.dc_gain ~= nil then
          serdesRxCfgPtr.rxTune.comphy.dcGain = params.dc_gain
      end
      if params.ffe_res ~= nil then
          serdesRxCfgPtr.rxTune.comphy.ffeR   = params.ffe_res
      end
      if params.ffe_cap ~= nil then
          serdesRxCfgPtr.rxTune.comphy.ffeC   = params.ffe_cap
      end
      if params.band_width ~= nil then
          serdesRxCfgPtr.rxTune.comphy.bandWidth = params.band_width
      end
  end

  local laneNum=params.laneNum                     --laneNum
  --serdesRxCfgPtr.align90=0                         --align90

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do

            command_data:clearPortStatus()
            command_data:clearLocalStatus()

      if(nil~=devNum) then
                devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
        if laneNum == "all" then
          ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
          if( ret == 0 and numOfSerdesLanesPtr > 0) then
            for i=0,numOfSerdesLanesPtr-1,1 do
              laneNum=i
              ret = myGenWrapper("cpssDxChPortSerdesManualRxConfigSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
              if(ret~=GT_OK) then
                print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
              end
            end  -- for i=0,numOfSerdesLanesPtr-1,1
          else -- if( ret == 0 and numOfSerdesLanesPtr > 0)
                        print(devnum_portnum_string.." There is no port serdes\n")
          end -- if( ret == 0 and numOfSerdesLanesPtr > 0)

        else
          ret = myGenWrapper("cpssDxChPortSerdesManualRxConfigSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","GT_U32","laneNum",laneNum},
            {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
          if(ret~=GT_OK) then
            print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
          end
        end  -- if laneNum == "all"
      end -- if(nil~=devNum)
    end -- for iterator, devNum, portNum
  end -- if true == command_data["status"] then

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local function serdes_rx_port_mgr(params)
    local command_data = Command_Data()
    local devNum, portNum
    local GT_OK=0
    local GT_NOT_INITIALIZED=0x12
    local GT_NOT_SUPPORTED=0x10
    local result, values,portmgr
    local serdesRxCfgPtr
    local rxTune = {}
    local rxTuneAvago={}
    local rxTuneComphy={}
    local iterator,i,j=0
    local laneNum =0
    local ifMode, speed
    local serdes
    local laneSpeed
    local portType, portMode, speedIdx
    local ifModeTmp, macNum

    if params.flagNo then
        if params.sqlch ~= nil then
            params.sqlch = 0xFFFF
        end
        if params.dc_gain ~= nil then
            params.dc_gain = 0xFFFF
        end
        if params.frequency ~= nil then
            params.frequency={}
            params.frequency[1] = 0xFFFF
            params.frequency[2] = 0xFFFF
        end
        if params.bw ~= nil then
            params.bw = 0xFFFF
        end
        if  is_sip_6(devNum) then
        -- CTLE params --
            if params.gain1 ~= nil then
                params.gain1 = 0xFFFF
            end
            if params.gain2 ~= nil then
                params.gain2 = 0xFFFF
            end
            if params.minLf ~= nil then
                params.minLf = 0xFFFF
            end
            if params.maxLf ~= nil then
                params.maxLf = 0xFFFF
            end
            if params.minHf ~= nil then
                params.minHf = 0xFFFF
            end
            if params.maxHf ~= nil then
                params.maxHf = 0xFFFF
            end
            -- DFE params --
            if params.bflf ~= nil then
                params.bflf = 0xFFFF
            end
            if params.bfhf ~= nil then
                params.bfhf =0xFFFF
            end
            if params.minPre1 ~= nil then
                params.minPre1 = 0xFFFF
            end
            if params.maxPre1 ~= nil then
                params.maxPre1 = 0xFFFF
            end
            if params.minPre2 ~= nil then
                params.minPre2 = 0xFFFF
            end
            if params.maxPre2 ~= nil then
                params.maxPre2 = 0xFFFF
            end
            if params.minPost ~= nil then
                params.minPost = 0xFFFF
            end
            if params.maxPost ~= nil then
                params.maxPost = 0xFFFF
            end
            if params.Pre1 ~= nil then
                params.Pre1 = 0xFFFF
            end
            if params.Pre2 ~= nil then
                params.Pre2 = 0xFFFF
            end
            if params.Post ~= nil then
                params.Post = 0xFFFF
            end
            if params.dfe_gain1 ~= nil then
                params.dfe_gain1 = 0xFFFF
            end
            if params.dfe_gain2 ~= nil then
                params.dfe_gain2 = 0xFFFF
            end
            if params.short_channel~= nil then
                params.short_channel = 0xFFFF
            end
            if params.termination~=nil then
               params.termination = 0xFFFF
            end
            if params.cold_envelope~=nil then
               params.cold_envelope = 0xFFFF
            end
            if params.hot_envelope~=nil then
               params.hot_envelope = 0xFFFF
            end
        end

    end

    for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        result,serdes = cpssGenWrapper("cpssPortManagerLuaSerdesTypeGet",{
                                  { "IN",  "GT_SW_DEV_NUM",             "devNum",    devNum },
                                  { "OUT", "CPSS_PORT_SERDES_TYPE_ENT", "serdesType"        }
                                  })
        if result~=0 then
            printLog ("Error in cpssPortManagerLuaSerdesTypeGet")
        end

        result, portmgr =  myGenWrapper("cpssDxChPortManagerPortParamsGet",{
                                       { "IN",  "GT_U8", "devNum", devNum },
                                       { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                       {"OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})

        if result ~= GT_OK then
            return "Error in cpssDxChPortManagerPortParamsGet " .. result
        end
        if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
            ifMode = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode
            laneNum = getLaneNum(ifMode)
            speed = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].speed

            result, values = myGenWrapper("cpssDxChPortPhysicalPortMapGet", {
                                {"IN", "GT_U8","devNum",devNum},
                                {"IN", "GT_U32","firstPhysicalPortNumber",portNum},
                                {"IN", "GT_U32", "portMapArraySize", 1 },
                                {"OUT","CPSS_DXCH_PORT_MAP_STC","portMap"}  })

            if result == GT_OK then
                if values.portMap.mappingType == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
                    macNum = values.portMap.interfaceNum
                else
                    return "Error in cpssDxChPortPhysicalPortMapGet " .. result
                end
            else
                return "Error in cpssDxChPortPhysicalPortMapGet " .. result
            end

            result, ifModeTmp = myGenWrapper("prvCpssCommonPortIfModeToHwsTranslate",{
                             { "IN",  "GT_U8", "devNum", devNum },
                             { "IN",  "CPSS_PORT_INTERFACE_MODE_ENT", "cpssIfMode", ifMode},
                             { "IN",  "CPSS_PORT_SPEED_ENT", "cpssSpeed", speed},
                             { "OUT", "MV_HWS_PORT_STANDARD", "hwsIfModePtr"}})

            if result ~= GT_OK then
                return "Error in prvCpssCommonPortIfModeToHwsTranslate " .. result
            end

            result, portMode = myGenWrapper("hwsPortModeParamsGetToBuffer", {
                             { "IN", "GT_U8",  "devNum", devNum },
                             { "IN", "GT_U32", "portGroup", 0},
                             { "IN", "GT_U32", "portNum", macNum},
                             { "IN", "MV_HWS_PORT_STANDARD", "portMode", ifModeTmp.hwsIfModePtr},
                             { "OUT", "MV_HWS_PORT_INIT_PARAMS", "portParamsBuffer"}})

	   speedIdx = getSpeedIdx(portMode.portParamsBuffer.serdesSpeed)

            if  speedIdx == "CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E" then
                print("Not supported: %s", speedIdx)
		return GT_NOT_SUPPORTED
            end
        else
            ifMode = portmgr.portParamsStcPtr.portParamsType.regPort.ifMode
            laneNum = getLaneNum(ifMode)
            speed = portmgr.portParamsStcPtr.portParamsType.regPort.speed
        end
        laneNum=laneNum-1
        j = 0

        if params.lane_num~=nil then
            if params.lane_num~="all" then
                laneNum = params.lane_num
                j=laneNum
            end
        end
        for i=j,laneNum,1 do
            serdesRxCfgPtr = {}
            serdesRxCfgPtr.type = serdes.serdesType
            if serdesRxCfgPtr.type == "CPSS_PORT_SERDES_AVAGO_E" then
                if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                    serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].rxParams
                    serdesRxCfgPtr.type = serdes.serdesType
                    rxTune = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].rxParams.rxTune
                    rxTuneAvago = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].rxParams.rxTune.avago
                else
                    serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams
                    serdesRxCfgPtr.type = serdes.serdesType
                    rxTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune
                    rxTuneAvago = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago
                end

                serdesRxCfgPtr.rxTune = {}
                serdesRxCfgPtr.rxTune.avago = {}
                serdesRxCfgPtr.rxTune = rxTune
                if rxTuneAvago ~= nil then
                    serdesRxCfgPtr.rxTune.avago = rxTuneAvago
                end
                if params.sqlch ~= nil then
                    serdesRxCfgPtr.rxTune.avago.sqlch = params.sqlch
                end
                if params.dc_gain ~= nil then
                    serdesRxCfgPtr.rxTune.avago.DC = params.dc_gain
                end
                if params.frequency ~= nil then
                    serdesRxCfgPtr.rxTune.avago.LF = params.frequency[1]
                    serdesRxCfgPtr.rxTune.avago.HF = params.frequency[2]
                end
                if params.bw ~= nil then
                    serdesRxCfgPtr.rxTune.avago.BW= params.bw
                end
                if  is_sip_6(devNum) then
                -- CTLE params --
                    if params.gain1 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.gainshape1= params.gain1
                    end
                    if params.gain2 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.gainshape2= params.gain2
                    end
                    if params.minLf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.minLf= params.minLf
                    end
                    if params.maxLf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.maxLf= params.maxLf
                    end
                    if params.minHf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.minHf= params.minHf
                    end
                    if params.maxHf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.maxHf= params.maxHf
                    end
                    -- DFE params --
                    if params.bflf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.BFLF= params.bflf
                    end
                    if params.bfhf ~= nil then
                        serdesRxCfgPtr.rxTune.avago.BFHF= params.bfhf
                    end
                    if params.minPre1 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.minPre1= params.minPre1
                    end
                    if params.maxPre1 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.maxPre1= params.maxPre1
                    end
                    if params.minPre2 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.minPre2= params.minPre2
                    end
                    if params.maxPre2 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.maxPre2= params.maxPre2
                    end
                    if params.minPost ~= nil then
                        serdesRxCfgPtr.rxTune.avago.minPost= params.minPost
                    end
                    if params.maxPost ~= nil then
                        serdesRxCfgPtr.rxTune.avago.maxPost= params.maxPost
                    end
                    if params.Pre1 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.pre1= params.Pre1
                    end
                    if params.Pre2 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.pre2= params.Pre2
                    end
                    if params.Post ~= nil then
                        serdesRxCfgPtr.rxTune.avago.post1= params.Post
                    end
                    if params.dfe_gain1 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.dfeGAIN= params.dfe_gain1
                    end
                    if params.dfe_gain2 ~= nil then
                        serdesRxCfgPtr.rxTune.avago.dfeGAIN2= params.dfe_gain2
                    end
                    if params.short_channel~= nil then
                        serdesRxCfgPtr.rxTune.avago.shortChannelEn=params.short_channel
                    end
                    if params.termination~=nil then
                        serdesRxCfgPtr.rxTune.avago.termination = params.termination
                    end
                    if params.cold_envelope~=nil then
                        serdesRxCfgPtr.rxTune.avago.coldEnvelope = params.cold_envelope
                    end
                    if params.hot_envelope~=nil then
                        serdesRxCfgPtr.rxTune.avago.hotEnvelope = params.hot_envelope
                    end
                end
            else
                if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                    serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].rxParams
                    serdesRxCfgPtr.type = serdes.serdesType
                    rxTune = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].rxParams.rxTune
                    rxTuneComphy = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].rxParams.rxTune.comphy
                else
                    serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams
                    serdesRxCfgPtr.type = serdes.serdesType
                    rxTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune
                    rxTuneComphy = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy
                end
                serdesRxCfgPtr.rxTune = {}
                serdesRxCfgPtr.rxTune.comphy = {}
                serdesRxCfgPtr.rxTune = rxTune
                serdesRxCfgPtr.rxTune.comphy = rxTuneComphy
            end
            if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                result, values = myGenWrapper("cpssDxChSamplePortManagerApOverrideRxLaneParamsSet",{
                                             {"IN", "GT_U8", "devNum", devNum},
                                             {"IN", "GT_PHYSICAL_PORT_NUM", "portNum",portNum},
                                             {"IN", "GT_U32","laneNum", i},
                                             {"IN", "CPSS_PM_AP_LANE_SERDES_SPEED", "laneSpeed", speedIdx},
                                             {"IN", "CPSS_PORT_SERDES_RX_CONFIG_STC","portParamsInputStcPtr", serdesRxCfgPtr}})
            else
                result, values = myGenWrapper("cpssDxChSamplePortManagerSerdesRxParamsSet",{
                                 {"IN","GT_U8","devNum",devNum},
                                 {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                 {"IN","GT_U32","laneNum",i},
                                 {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr", serdesRxCfgPtr}})
            end
            if result == GT_NOT_INITIALIZED then
                print("Error :- Port %d not initialized \n", portNum)
            elseif result == GT_BAD_STATE then
                print("Error : Port %d is not in reset state \n", portNum)
            elseif result~=GT_OK then
                print("Error at command: %s", result)
            end
        end
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function serdes_rx_main(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    devNum, portNum = command_data:getFirstPort()

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if (values.enablePtr) then
        serdes_rx_port_mgr(params)
    else
        serdes_rx_func_bobk(params)
    end
end

--------------------------------------------
-- command registration: serdes_rx
--------------------------------------------

CLI_addCommand("interface", "serdes rx", {
  func   = serdes_rx_main,
  help   = "serdes rx parameters",
  params = {
    {   type="named",
         { format="squelch-level %type_sqlch",               name="sqlch",         help="Threshold that trips the Squelch detector"},
         { format="frequency %type_ffeRes   %type_ffeCap", name="frequency",multiplevalues=true,        help="Mainly controls the low frequency gain"    },
         { format="dc_gain %type_dcGain",                 name="dc_gain",        help="DC-Gain value"                           },
         { format="bandwidth %type_bandWidth",            name="bw",        help="CTLE bandwidth"                           },
         { format="short-channel %short_channel",   name="short_channel",        help="Enable/Disable short channel"},
         { format="termination %type_rxtermination",name="termination",        help="Set Rx termination"},
         { format="cold_envelope %type_rxenvelope",name="cold_envelope",       help="Set cold signal envelope"},
         { format="hot_envelope %type_rxenvelope",name="hot_envelope",        help="Set hot signal envelope"},
         { format="lane_num %LaneNumberOrAll",             name="lane_num",       help="Number of SERDES lane of port or all"    }
               }}}
)

CLI_addCommand("interface", "no serdes rx", {
   func   = function(params)
       params.flagNo = true
       return serdes_rx_main(params)
   end,
   help   = "serdes rx parameters",
   params = {
   {   type="named",
         { format="squelch-level",               name="sqlch",         help="Threshold that trips the Squelch detector"},
         { format="frequency",                   name="frequency",     help="Mainly controls the low frequency gain"    },
         { format="dc_gain",                     name="dc_gain",       help="DC-Gain value"                           },
         { format="bandwidth",                   name="bw",            help="CTLE bandwidth"                           },
         { format="short-channel ",              name="short_channel", help="Enable/Disable short channel"},
         { format="termination ",                name="termination",   help="Set Rx termination"},
         { format="cold_envelope ",              name="cold_envelope", help="Set cold signal envelope"},
         { format="hot_envelope",                name="hot_envelope",  help="Set hot signal envelope"},
         { format="lane_num %LaneNumberOrAll",   name="lane_num",     help="Number of SERDES lane of port or all"    }
               }}}
)
--------------------------------------------
-- command registration: serdes rx ctle
--------------------------------------------

CLI_addCommand("interface", "serdes rx ctle", {
  func   = serdes_rx_main,
  help   = "serdes rx parameters",
  params = {
    {   type="named",
         { format="gain-shape-1 %type_gainShape1",name="gain1",          help="CTLE gain shape1  "                      },
         { format="gain-shape-2 %type_gainShape2",name="gain2",          help="CTLE gain shape2 "                       },
         { format="minLf %type_minLf",            name="minLf",          help="CTLE min LF "                            },
         { format="maxLf %type_maxLf",            name="maxLf",          help="CTLE max LF "                            },
         { format="minHf %type_minHf",            name="minHf",          help="CTLE min HF "                            },
         { format="maxHf %type_maxHf",            name="maxHf",          help="CTLE max HF "                            },
         { format="lane_num %LaneNumberOrAll",    name="lane_num",       help="Number of SERDES lane of port or all"    }
               }}}
)

CLI_addCommand("interface", "no serdes rx ctle", {
    func   = function(params)
      params.flagNo = true
      return serdes_rx_main(params)
    end,
    help   = "serdes rx parameters",
    params = {
    {   type="named",
         { format="gain-shape-1",      name="gain1",          help="CTLE gain shape1  "                      },
         { format="gain-shape-2",      name="gain2",          help="CTLE gain shape2 "                       },
         { format="minLf",             name="minLf",          help="CTLE min LF "                            },
         { format="maxLf",             name="maxLf",          help="CTLE max LF "                            },
         { format="minHf",             name="minHf",          help="CTLE min HF "                            },
         { format="maxHf",             name="maxHf",          help="CTLE max HF "                            },
         { format="lane_num %LaneNumberOrAll", name="lane_num",       help="Number of SERDES lane of port or all"    }
   }}}
)

--------------------------------------------
-- command registration: serdes rx dfe
--------------------------------------------
CLI_addCommand("interface", "serdes rx dfe", {
  func   = serdes_rx_main,
  help   = "serdes rx parameters",
  params = {
    {   type="named",
         { format="bfLf %type_BfLf",           name="bflf",           help="DFE BfLf "                               },
         { format="bfHf %type_BfHf",           name="bfhf",           help="DFE BfHf "                               },
         { format="min-pre1 %type_minPre1",    name="minPre1",        help="DFE min Pre1 "                           },
         { format="max-pre1 %type_maxPre1",    name="maxPre1",        help="DFE max Pre1 "                           },
         { format="min-pre2 %type_minPre2",    name="minPre2",        help="DFE min Pre2 "                           },
         { format="max-pre2 %type_maxPre2",    name="maxPre2",        help="DFE max Pre2 "                           },
         { format="min-post %type_minPost",    name="minPost",        help="DFE min Post-cursor "                    },
         { format="max-post %type_maxPost",    name="maxPost",        help="DFE max Post-cursor "                    },
         { format="pre1 %type_pre1",           name="Pre1",           help="DFE Pre-cursor1 "                        },
         { format="pre2 %type_pre2",           name="Pre2",           help="DFE Pre-cursor2 "                        },
         { format="post %type_post1",          name="Post",           help="DFE Post-cursor"                         },
         { format="gain1 %type_gain1",         name="dfe_gain1",      help="DFE gain1 "                              },
         { format="gain2 %type_gain2",         name="dfe_gain2",      help="DFE gain2 "                              },
         { format="lane_num %LaneNumberOrAll", name="lane_num",       help="Number of SERDES lane of port or all"    }
   }}}
)

CLI_addCommand("interface", "no serdes rx dfe", {
    func   = function(params)
      params.flagNo = true
      return serdes_rx_main(params)
    end,
    help   = "serdes rx parameters",
    params = {
      {   type="named",
         { format="bfLf ",           name="bflf",           help="DFE BfLf "                               },
         { format="bfHf ",           name="bfhf",           help="DFE BfHf "                               },
         { format="min-pre1 ",       name="minPre1",        help="DFE min Pre1 "                           },
         { format="max-pre1 ",       name="maxPre1",        help="DFE max Pre1 "                           },
         { format="min-pre2 ",       name="minPre2",        help="DFE min Pre2 "                           },
         { format="max-pre2 ",       name="maxPre2",        help="DFE max Pre2 "                           },
         { format="min-post ",       name="minPost",        help="DFE min Post-cursor "                    },
         { format="max-post ",       name="maxPost",        help="DFE max Post-cursor "                    },
         { format="pre1 ",           name="Pre1",           help="DFE Pre-cursor1 "                        },
         { format="pre2 ",           name="Pre2",           help="DFE Pre-cursor2 "                        },
         { format="post ",           name="Post",           help="DFE Post-cursor"                         },
         { format="gain1",           name="dfe_gain1",      help="DFE gain1 "                              },
         { format="gain2 ",          name="dfe_gain2",      help="DFE gain2 "                              },
         { format="lane_num %LaneNumberOrAll", name="lane_num",       help="Number of SERDES lane of port or all"    }
   }}}
)
