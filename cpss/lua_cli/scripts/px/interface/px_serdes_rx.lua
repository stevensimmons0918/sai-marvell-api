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
local function serdes_rx_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local numOfSerdesLanesPtr=0
  local devnum_portnum_string
  local ret
  local serdesRxCfgPtr={}

  serdesRxCfgPtr.sqlch=params.sqlch              --sqlch
  serdesRxCfgPtr.ffeRes=params.ffeRes              --ffeRes
  serdesRxCfgPtr.ffeCap=params.ffeCap              --ffeCap
  serdesRxCfgPtr.dcGain=params.dcGain              --dcGain
  serdesRxCfgPtr.bandWidth=params.bandWidth        --bandWidth

  local laneNum=params.laneNum                     --laneNum

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
          numOfSerdesLanesPtr=16
          if(numOfSerdesLanesPtr > 0) then
            for i=0,numOfSerdesLanesPtr-1,1 do
              laneNum=i
              ret = myGenWrapper("cpssPxPortSerdesManualRxConfigSet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
              if(ret~=GT_OK) then
                print(devnum_portnum_string.." Error cpssPxPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
              end
            end  -- for i=0,numOfSerdesLanesPtr-1,1
          else -- if( ret == 0 and numOfSerdesLanesPtr > 0)
                        print(devnum_portnum_string.." There is no port serdes\n")
          end -- if( ret == 0 and numOfSerdesLanesPtr > 0)

        else
          ret = myGenWrapper("cpssPxPortSerdesManualRxConfigSet",{
            {"IN","GT_SW_DEV_NUM","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","GT_U32","laneNum",laneNum},
            {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
          if(ret~=GT_OK) then
            print(devnum_portnum_string.." Error cpssPxPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
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
    local result, values,portmgr
    local serdesRxCfgPtr
    local rxTune = {}
    local rxTuneAvago={}
    local rxTuneComphy={}
    local iterator,i,j
    local ifMode
    local laneNum =0
    local ifMode, speed
    local serdes

    if params.flagNo then
        if params.sqlch ~= nil then
            params.sqlch = 0
        end
        if params.dc_gain ~= nil then
            params.dc_gain = 0
        end
        if params.frequency ~= nil then
            params.frequency = {}
            params.frequency[1] = 0
            params.frequency[2] = 0
        end
        if params.bw ~= nil then
            params.bw = 0
        end
        if params.short_channel~= nil then
            params.short_channel=0
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

        result, portmgr =  myGenWrapper("cpssPxPortManagerPortParamsGet",{
                                      { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                                      { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                      {"OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Port %d not initialized \n", portNum)
        elseif result == GT_BAD_STATE then
            print("Error : Port %d is not in reset state \n", portNum)
        elseif result~=GT_OK then
            print("Error at command: cpssPxPortManagerPortParamsGet :%s", result)
        end

        if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
            ifMode = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode
            speed = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].speed
        else
            ifMode = portmgr.portParamsStcPtr.portParamsType.regPort.ifMode
            speed = portmgr.portParamsStcPtr.portParamsType.regPort.speed
        end

        laneNum = getLaneNum(ifMode)
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
            if serdes.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams
                serdesRxCfgPtr.type = serdes.serdesType
                rxTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune
                rxTuneAvago = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago

                serdesRxCfgPtr.rxTune = {}
                serdesRxCfgPtr.rxTune.avago = {}
                serdesRxCfgPtr.rxTune = rxTune
                serdesRxCfgPtr.rxTune.avago = rxTuneAvago

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
                if params.short_channel~= nil then
                    serdesRxCfgPtr.rxTune.avago.shortChannelEn=1
                end
            else
                serdesRxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams
                serdesRxCfgPtr.type = serdes.serdesType
                rxTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune
                rxTuneComphy = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy
                serdesRxCfgPtr.rxTune = {}
                serdesRxCfgPtr.rxTune.comphy = {}
                serdesRxCfgPtr.rxTune = rxTune
                serdesRxCfgPtr.rxTune.comphy = rxTuneComphy
            end

            result, values = myGenWrapper("cpssPxSamplePortManagerSerdesRxParamsSet",{
                                 {"IN","GT_U8","devNum",devNum},
                                 {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                 {"IN","GT_U32","laneNum",i},
                                 {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr", serdesRxCfgPtr}})
            if result == GT_NOT_INITIALIZED then
                print("Error : Port %d not initialized \n", portNum)
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

    result, values = myGenWrapper("cpssPxPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if(values.enablePtr) then
        serdes_rx_port_mgr(params)
    else
        serdes_rx_func(params)
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
         { format="short-channel ",                 name="short_channel",        help="Enable/Disable short channel"},
         { format="lane_num %LaneNumberOrAll",             name="lane_num",       help="Number of SERDES lane of port or all"    }
               }}}
)

CLI_addCommand("interface", "no serdes rx", {
    func = function(params)
        params.flagNo=true
        return serdes_rx_main(params)
    end,
    help = "serdes rx parameters",
    params = {
    {   type="named",
         { format="squelch-level ",               name="sqlch",         help="Threshold that trips the Squelch detector"},
         { format="frequency ",                   name="frequency",     help="Mainly controls the low frequency gain"    },
         { format="dc_gain",                      name="dc_gain",       help="DC-Gain value"                           },
         { format="bandwidth",                    name="bw",            help="CTLE bandwidth"                           },
         { format="short-channel ",               name="short_channel", help="Enable/Disable short channel"},
         { format="lane_num %LaneNumberOrAll",    name="lane_num",      help="Number of SERDES lane of port or all"    }
               }}}
)
