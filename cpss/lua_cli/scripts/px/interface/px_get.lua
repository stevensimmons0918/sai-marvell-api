--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* get.lua
--*
--* DESCRIPTION:
--*    GET command.
--*
--*
--*
--*
--********************************************************************************

--includes


--constants

local function get_func(params)
    local command_data = Command_Data()
    local devNum, portNum
    local GT_OK=0
    local GT_NOT_INITIALIZED=0x12
    local result, values
    local iterator
    local portmgr
    local ifMode, speed, loopback, fec
    local fecSupported, fecRequested
    local nonceDisable, fcPause, fcAsmDir, negotiationLaneNum
    local unMaskEventsMode, apLaneParams
    local portType, serdesType
    local trainMode, adaptRxTrainSupp, edgeDetectSupported
    local etOverride_minLf, etOverride_maxLf, calibrationMode
    local sqlch, DC, LF, HF, BW
    local atten, post, pre

    for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()
        result, portmgr =  myGenWrapper("cpssPxPortManagerPortParamsGet",{
                                      { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                                      { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                      {"OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})
        if result == GT_NOT_INITIALIZED then
            print("Port not initialized ", portNum)
        elseif result == GT_BAD_STATE then
            print("Error : Port %d is not in reset state \n", portNum)
        elseif result~=GT_OK then
            print("Error at command: cpssDxChPortManagerPortParamsGet :%s", result)
        end

        portType = portmgr.portParamsStcPtr.portType

        result,values = cpssGenWrapper("cpssPortManagerLuaSerdesTypeGet",{
          { "IN",  "GT_SW_DEV_NUM",             "devNum",    devNum },
          { "OUT", "CPSS_PORT_SERDES_TYPE_ENT", "serdesType"        }
        })

        print("\n----------------------------------------------------------------------------")
        print("Port type            : ",tostring(portType))
        print("\n----------------------------------------------------------------------------")

        if portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
            ifMode = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode
            speed = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].speed
            fecSupported = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported
            fecRequested = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested
            nonceDisable = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.nonceDisable
            fcPause = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.fcPause
            fcAsmDir = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.fcAsmDir
            negotiationLaneNum = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.negotiationLaneNum
            laneNum=getLaneNum(ifMode)

            if params.get_all ~=nil then
                print("Speed            : ",tostring(speed))
                print("Ifmode           : ",tostring(ifMode))
                print("Fec Supported    : ",tostring(fecSupported))
                print("Fec Requested    : ",tostring(fecRequested))
                print("nonce-match      : ",nonceDisable)
                if fcPause then
                    print("flow-control     :",tostring(fcAsmDir))
                else
                    print("flow-control     :",tostring(fcPause))
                end

                print("LaneNum          : ",negotiationLaneNum)
            else
                if params.speed~=nil then
                    print("Speed            :",tostring(speed))
                end
                if params.ifmode~=nil then
                    print("Ifmode           :",tostring(ifMode))
                end
                if params.fec~=nil then
                    print("Fec Supported    :",tostring(fecSupported))
                    print("Fec Requested    :",tostring(fecRequested))
                end
                if params.ap~=nil then
                    print("nonce-match      :",tostring(nonceDisable))
                    if fcPause== true then
                        print("flow-control :",tostring(fcAsmDir))
                    else
                        print("flow-control :",tostring(fcPause))
                    end
                    print("LaneNum          : ",tostring(negotiationLaneNum))
                end
            end
        else
            ifMode = portmgr.portParamsStcPtr.portParamsType.regPort.ifMode
            speed = portmgr.portParamsStcPtr.portParamsType.regPort.speed
            fec = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.fecMode
            trainMode =  portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.trainMode
            adaptRxTrainSupp = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.adaptRxTrainSupp
            edgeDetectSupported = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.edgeDetectSupported
            etOverride_minLf = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.etOverride.minLF
            etOverride_maxLf = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.etOverride.maxLF
            calibrationMode = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.calibrationMode.calibrationType

            loopback = {}
            loopback = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.loopback
            unMaskEventsMode = portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.unMaskEventsMode
            laneNum=getLaneNum(ifMode)
            if params.get_all~=nil then
                print("Speed                : ",tostring(speed))
                print("Ifmode               : ",tostring(ifMode))
                print("--------------------------------------------------------------------------------")
                print("Port Attributes")
                print("--------------------------------------------------------------------------------")
                print("Fec                  : ",tostring(fec))
                print("Loopback             : ",tostring(loopback.loopbackType))
                if loopback.loopbackType == "CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E" then
                    print("Serdes LoopBack Mode : ",tostring(portmgr.portParamsStcPtr.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode))
                end
                print("events               : ",tostring(unMaskEventsMode))
                print("--------------------------------------------------------------------------------")
                print("Serdes Attributes ")
                print("--------------------------------------------------------------------------------")
                print("Train Mode           : ",tostring(trainMode))
                print("Adaptive Rx          : ",tostring(adaptRxTrainSupp))
                print("Edge Detect          : ",tostring(edgeDetectSupported))
                print("Eye Threshold")
                print("    min LF           :",etOverride_minLf)
                print("    max LF           :",etOverride_maxLf)
                print("calibration Mode     : ", tostring(calibrationMode))
                print("--------------------------------------------------------------------------------")
                print("Serdes Params")
                print("--------------------------------------------------------------------------------")
                print("Serdes Type : ",values.serdesType)
                for i=0,laneNum-1,1 do
                    print("laneNum              :", i)
                    if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                       sqlch = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.sqlch
                       DC = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.DC
                       LF = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.LF
                       HF = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.HF
                       BW = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.BW
                       atten = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.atten
                       pre = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.pre
                       post = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.post
                       print("-----------------------------Tx---------------------------------")
                       print(string.format("atten:%d\t post:%d\t pre:%d\n", atten, post, pre))
                       print("-----------------------------Rx---------------------------------")
                       print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW   :%d\t", sqlch, DC, LF, HF, BW))
                    end
                end
            else
                if params.speed~=nil then
                    print("Speed            :",tostring(speed))
                end
                if params.ifmode~=nil then
                    print("Ifmode           :",tostring(ifMode))
                end
                if params.fec~=nil then
                    print("Fec              :",tostring(fec))
                end
                if params.loopback~=nil then
                    print("Loopback  : ",tostring(loopback.loopbackType))
                end
                if params.event~=nil then
                    print("events    :",tostring(unMaskEventsMode))
                end
                if params.serdes~=nil then
                    print("--------------------------------------------------------------------------------")
                    print("Serdes Attributes ")
                    print("--------------------------------------------------------------------------------")
                    print("Train Mode           : ",tostring(trainMode))
                    print("Adaptive Rx          : ",tostring(adaptRxTrainSupp))
                    print("Edge Detect          : ",tostring(edgeDetectSupported))
                    print("Eye Threshold")
                    print("    min LF           :",etOverride_minLf)
                    print("    max LF           :",etOverride_maxLf)
                    print("calibration Mode     : ", tostring(calibrationMode))
                end
                if params.serdes_rx~=nil then
                    if params.serdes_rx ~="all" then
                        laneNum=params.serdes_rx
                        j = laneNum
                    else
                        laneNum=getLaneNum(ifMode)
                        laneNum=laneNum-1
                        j = 0
                    end
                    for i=j,laneNum,1 do
                        print("-----------------------------Rx---------------------------------")
                        print("laneNum              :", i)
                        if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                           sqlch = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.sqlch
                           DC = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.DC
                           LF = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.LF
                           HF = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.HF
                           BW = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago.BW
                           print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW   :%d\t", sqlch, DC, LF, HF, BW))
                       end
                   end
                end
                if params.serdes_tx~=nil then
                    if params.serdes_tx ~="all" then
                        laneNum=params.serdes_tx
                        j = laneNum
                    else
                        laneNum=getLaneNum(ifMode)
                        laneNum=laneNum-1
                        j = 0
                    end
                    for i=j,laneNum,1 do
                        print("-----------------------------Tx---------------------------------")
                        print("laneNum              :", i)
                        if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                           atten = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.atten
                           pre = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.pre
                           post = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago.post
                        end
                        print(string.format("atten:%d\t post:%d\t pre:%d\n", atten, post, pre))
                   end
                end
            end
        end
    end
end

local function get_func_main(params)
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
        get_func(params)
    end
end
-------------------------------------------
-- command registration: serdes_tx
--------------------------------------------
CLI_addCommand("interface", "get", {
  func   = get_func_main,
  help   = "Get configured parameters in port manager DB",
  params = {
  {   type="named",
            { format="all", name="get_all", help="Display all configured port parameters"},
            { format="speed", name="speed", help="Speed"},
            { format="ifmode", name="ifmode", help="Interface Mode"},
            { format="fec", name="fec", help="forward error correction mode"},
            { format="serdes rx %LaneNumberOrAll", name="serdes_rx", help="serdes rx paramerters"},
            { format="serdes tx %LaneNumberOrAll", name="serdes_tx", help="serdes tx parameters"},
            { format="loopback ", name="loopback", help="loopback mode"},
            { format="events", name="event", help="port manager event unmask"},
            { format="802.3ap", name="ap", help="802.3ap parameters"},
            { format="serdes", name="serdes", help="Serdes general parameters"}
  }}}
)

