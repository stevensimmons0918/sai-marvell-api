--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_tx.lua
--*
--* DESCRIPTION:
--*     Configure specific parameters of serdes TX in HW
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  serdes_tx_func_bobk
--        @description  configure specific parameters of serdes tx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_tx_func_bobk(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local serdesTxCfgPtr={}
  local ret

  local serdes
  ret,serdes = cpssGenWrapper("cpssPortManagerLuaSerdesTypeGet",{
    { "IN",  "GT_SW_DEV_NUM",             "devNum",    devNum },
    { "OUT", "CPSS_PORT_SERDES_TYPE_ENT", "serdesType"        }
  })
  if ret~=0 then
      printLog ("Error in cpssPortManagerLuaSerdesTypeGet")
      setFailState()
  end

  serdesTxCfgPtr.type = serdes.serdesType
  serdesTxCfgPtr.txTune = {}
  serdesTxCfgPtr.txTune.avago = {}
  serdesTxCfgPtr.txTune.comphy = {}

  if serdesTxCfgPtr.type == "CPSS_PORT_SERDES_AVAGO_E" then
      if params.tx_atten ~=nil then
          serdesTxCfgPtr.txTune.avago.atten=params.tx_atten
      end
      if params.tx_emph ~=nil then
          serdesTxCfgPtr.txTune.avago.post=params.tx_emph[1]
          serdesTxCfgPtr.txTune.avago.pre=params.tx_emph[2]
          serdesTxCfgPtr.txTune.avago.pre2=params.tx_emph[3]
          serdesTxCfgPtr.txTune.avago.pre3=params.tx_emph[4]
      end
  else
      if params.tx_atten ~=nil then
          serdesTxCfgPtr.txTune.comphy.txAmp=params.tx_atten
      end
      if params.tx_txAmpAdjEn ~=nil then
          serdesTxCfgPtr.txTune.comphy.txAmpAdjEn=params.txAmpAdjEn
      end
      if params.tx_emph ~=nil then
          serdesTxCfgPtr.txTune.comphy.emph0=params.tx_emph[1]
          serdesTxCfgPtr.txTune.comphy.emph1=params.tx_emph[2]
      end
      if params.txAmpShft ~=nil then
          serdesTxCfgPtr.txTune.comphy.txAmpShft=params.txAmpShft
      end
  end

  local laneNum=params.laneNum           --laneNum


  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssDxChPortSerdesManualTxConfigSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_U32","laneNum",laneNum},
          {"IN","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr",serdesTxCfgPtr}})
        if(ret~=GT_OK) then
          print("Error at serdes tx command")
        end
      end


    end

  end




  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end
local function serdes_tx_port_mgr(params)
    local command_data = Command_Data()
    local devNum, portNum
    local GT_OK=0
    local GT_NOT_INITIALIZED=0x12
    local result, values
    local serdesTxCfgPtr
    local txTune = {}
    local txTuneAvago={}
    local txTuneComphy={}
    local iterator,i,j
    local portmgr
    local serdes
    local speedIdx
    local portMode
    local ifModeTmp, macNum
    local GT_NOT_SUPPORTED=0x10

    if params.flagNo then
        if params.tx_atten~=nil then
            params.tx_atten=0xFFFF
        end
        if params.tx_emph~=nil then
            params.tx_emph={}
            params.tx_emph[1]=0xFFFF
            params.tx_emph[2]=0xFFFF
            params.tx_emph[3]=0xFFFF
            params.tx_emph[4]=0xFFFF
        end
    end

    if params.tx_emph~=nil and params.tx_atten~=nil then
        if math.abs(params.tx_emph[1]) + math.abs(params.tx_emph[2]) + math.abs(params.tx_emph[3]) + math.abs(params.tx_emph[4]) + params.tx_atten > 26 then
           print(" Error : ABS(Pre3) + ABS(Pre2) + ABS(Pre) + Atten + ABS(Post) > 26")
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

        if result == GT_NOT_INITIALIZED then
            print("Error : Port %d not initialized \n", portNum)
        elseif result == GT_BAD_STATE then
            print("Error : Port %d is not in reset state \n", portNum)
        elseif result~=GT_OK then
            print("Error at command: cpssDxChPortManagerPortParamsGet :%s", result)
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
            serdesTxCfgPtr = {}
            serdesTxCfgPtr.type = serdes.serdesType
            if serdesTxCfgPtr.type == "CPSS_PORT_SERDES_AVAGO_E" then
                if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                    serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams
                    serdesTxCfgPtr.type = serdes.serdesType
                    txTune = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune
                    txTuneAvago = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune.avago
                else
                    serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams
                    serdesTxCfgPtr.type = serdes.serdesType
                    txTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune
                    txTuneAvago = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago
                end
                serdesTxCfgPtr.txTune = {}
                serdesTxCfgPtr.txTune.avago = {}
                serdesTxCfgPtr.txTune = txTune

                if txTuneAvago ~= nil then
                    serdesTxCfgPtr.txTune.avago = txTuneAvago
                end
                if params.tx_atten ~= nil then
                    serdesTxCfgPtr.txTune.avago.atten = params.tx_atten
                end
                if params.tx_emph ~= nil then
                    serdesTxCfgPtr.txTune.avago.post = params.tx_emph[1]
                    serdesTxCfgPtr.txTune.avago.pre  = params.tx_emph[2]
                    serdesTxCfgPtr.txTune.avago.pre2 = params.tx_emph[3]
                    serdesTxCfgPtr.txTune.avago.pre3 = params.tx_emph[4]
                end
            else
                if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                    serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].txParams
                    serdesTxCfgPtr.type = serdes.serdesType
                    txTune = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].txParams.txTune
                    txTuneComphy = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[0+i].txParams.txTune.comphy
                else
                    serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams
                    serdesTxCfgPtr.type = serdes.serdesType
                    txTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune
                    txTuneComphy = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy
                end
                serdesTxCfgPtr.txTune = {}
                serdesTxCfgPtr.txTune.comphy = {}
                serdesTxCfgPtr.txTune = txTune
                serdesTxCfgPtr.txTune.comphy = txTuneComphy
            end

            if portmgr.portParamsStcPtr.portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
                result, values = myGenWrapper("cpssDxChSamplePortManagerApOverrideTxLaneParamsSet",{
                                             {"IN", "GT_U8", "devNum", devnum},
                                             {"IN", "GT_PHYSICAL_PORT_NUM", "portNum",portNum},
                                             {"IN", "GT_U32","laneNum", i},
                                             {"IN", "CPSS_PM_AP_LANE_SERDES_SPEED", "laneSpeed", speedIdx},
                                             {"IN", "CPSS_PORT_SERDES_TX_CONFIG_STC","portParamsInputStcPtr", serdesTxCfgPtr}})
            else
                result, values = myGenWrapper("cpssDxChSamplePortManagerSerdesTxParamsSet",{
                                 {"IN","GT_U8","devNum",devNum},
                                 {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                 {"IN","GT_U32","laneNum",i},
                                 {"IN","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr",serdesTxCfgPtr}})
            end
            if result == GT_NOT_INITIALIZED then
                print("Error : Port %d not initialized \n", portNum)
            elseif result == GT_BAD_STATE then
                print("Error : Port %d is not in reset state \n", portNum)
            elseif result~=GT_OK then
                print("Error at command: cpssDxChSamplePortManagerSerdesTxParamsSet :%s", result)
            end
        end
    end
end

local function serdes_tx_main(params)
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
        serdes_tx_port_mgr(params)
    else
        serdes_tx_func_bobk(params)
    end
end

--------------------------------------------
-- command registration: serdes_tx
--------------------------------------------
CLI_addCommand("interface", "serdes tx", {
  func   = serdes_tx_main,
  help   = "Configure specific parameters of serdes TX in HW",
  params = {
  {   type="named",
            { format="amplitude %type_amp",     name="tx_atten", multiplevalues=true, help="Tx Amplitude/Attenuation of the signal [0..31]"},
            { format="emph %type_txpost %type_txpre %type_txpre2 %type_txpre3 ", name="tx_emph", multiplevalues=true,      help="Tx Emphasis"},
            { format="laneNum %LaneNumberOrAll",name="lane_num",   help="Number of SERDES lane of port"    }
         }}}
)

CLI_addCommand("interface", "no serdes tx", {
    func = function(params)
        params.flagNo=true
        return serdes_tx_main(params)
    end,
    help = "serdes tx parameters",
    params = {
    {   type="named",
            { format="amplitude",     name="tx_atten", multiplevalues=true, help="Tx Amplitude/Attenuation of the signal [0..31]"},
            { format="emph",          name="tx_emph", help="Tx Emphasis"},
            { format="laneNum %LaneNumberOrAll",name="lane_num",   help="Number of SERDES lane of port"    }
         }}}
)
