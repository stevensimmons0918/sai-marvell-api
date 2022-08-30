--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
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
--  serdes_tx_func
--        @description  configure specific parameters of serdes tx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_tx_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local serdesTxCfgPtr={}
  local ret
  serdesTxCfgPtr.txAmp=params.txAmp      --txAmp
  serdesTxCfgPtr.txAmpAdjEn=false        --txAmpAdjEn
  serdesTxCfgPtr.emph0=params.emph0      --emph0
  serdesTxCfgPtr.emph1=params.emph1      --emph1
  serdesTxCfgPtr.txAmpShft=false         --txAmpShft
  local laneNum=params.laneNum           --laneNum


  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssPxPortSerdesManualTxConfigSet",{
          {"IN","GT_SW_DEV_NUM","devNum",devNum},
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
    local portmgr, serdes

    if params.flagNo then
        if params.tx_atten~=nil then
            params.tx_atten=0
        end
        if params.tx_emph~=nil then
            params.tx_emph={}
            params.tx_emph[1]=0
            params.tx_emph[2]=0
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
            serdesTxCfgPtr = {}
            if serdes.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams
                serdesTxCfgPtr.type = serdes.serdesType
                txTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune
                txTuneAvago = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago

                serdesTxCfgPtr.txTune = {}
                serdesTxCfgPtr.txTune.avago = {}

                serdesTxCfgPtr.txTune = txTune
                serdesTxCfgPtr.txTune.avago = txTuneAvago

                if params.tx_atten ~= nil then
                   serdesTxCfgPtr.txTune.avago.atten = params.tx_atten
                end
                if params.tx_emph ~= nil then
                   serdesTxCfgPtr.txTune.avago.post = params.tx_emph[1]
                   serdesTxCfgPtr.txTune.avago.pre  = params.tx_emph[2]
                end
            else
                serdesTxCfgPtr = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams
                serdesTxCfgPtr.type = serdes.serdesType
                txTune = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune
                txTuneComphy = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy
                serdesTxCfgPtr.txTune = {}
                serdesTxCfgPtr.txTune.comphy = {}

                serdesTxCfgPtr.txTune = txTune
                serdesTxCfgPtr.txTune.comphy = txTuneComphy
            end

            result, values = myGenWrapper("cpssPxSamplePortManagerSerdesTxParamsSet",{
                                 {"IN","GT_U8","devNum",devNum},
                                 {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                 {"IN","GT_U32","laneNum",i},
                                 {"IN","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr",serdesTxCfgPtr}})
            if result == GT_NOT_INITIALIZED then
                print("Error : Port %d not initialized \n", portNum)
            elseif result == GT_BAD_STATE then
                print("Error : Port %d is not in reset state \n", portNum)
            elseif result~=GT_OK then
                print("Error at command: cpssPxSamplePortManagerSerdesTxParamsSet :%s", result)
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

    result, values = myGenWrapper("cpssPxPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if(values.enablePtr) then
        serdes_tx_port_mgr(params)
    else
        serdes_tx_func(params)
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
            { format="emph %type_txpost %type_txpre  ", name="tx_emph", multiplevalues=true,      help="Tx Emphasis"},
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
            { format="amplitude",     name="tx_atten", help="Tx Amplitude/Attenuation of the signal [0..31]"},
            { format="emph ", name="tx_emph", multiplevalues=true,      help="Tx Emphasis"},
            { format="laneNum %LaneNumberOrAll",name="lane_num",   help="Number of SERDES lane of port"    }
    }}}

)
