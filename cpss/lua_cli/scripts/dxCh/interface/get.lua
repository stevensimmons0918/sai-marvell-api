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
    local result, values, portmap, macNum
    local iterator, i, j
    local portmgr
    local ifMode, speed, loopback, fec
    local fecSupported, fecRequested
    local nonceDisable, fcPause, fcAsmDir, negotiationLaneNum, interconnectProfile
    local unMaskEventsMode, apLaneParams
    local portType, serdesType
    local atten, post, pre, pre2, pre3
    local sqlch, DC, LF, HF, BW, gainshape1, gainshape2
    local shortChannelEn, dfeGAIN, dfeGAIN2, BFLF
    local BFHF, minLf, maxLf, minHf, maxHf, minPre1, maxPre1
    local minPre2, maxPre2, minPost, maxPost, pre1, pre2, post1, rx_termination
    local txParams
    local rxParams
    local trainMode, adaptRxTrainSupp, edgeDetectSupported
    local etOverride_minLf, etOverride_maxLf, calibrationMode
    local cold_envelope, hot_envelope
    local ifModeTmp, speedIdx, portMode
    local current1Sel, rl1Sel, rl1Extra, res1Sel, cap1Sel, cl1Ctrl, enMidFreq, cs1Mid
    local rs1Mid, rfCtrl, rl1TiaSel, rl1TiaExtra, hpfRSel1st, current1TiaSel, rl2Tune
    local rl2Sel, rs2Sel, current2Sel, cap2Sel, hpfRsel2nd, selmufi, selmuff, selmupi
    local selmupf, squelch, align90AnaReg, align90, sampler, slewRateCtrl0, slewRateCtrl1, EO
    local midpointLargeThresKLane, midpointSmallThresKLane
    local midpointLargeThresCLane, midpointSmallThresCLane
    local dfeResF0aHighThresInitLane
    local dfeResF0aHighThresEndLane, dataRate, usr

    for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()
        result, portmgr =  myGenWrapper("cpssDxChPortManagerPortParamsGet",{
                                      { "IN",  "GT_U8", "devNum", devNum },
                                      { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                                      {"OUT", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr"}})
        if result == GT_NOT_INITIALIZED then
            print(string.format("Port %d not initialized \n", portNum))
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

        print("--------------------------------------------------------------------------------")
        print("Port type            : ",tostring(portType))
        print("--------------------------------------------------------------------------------")

        if portType == "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E" then
            ifMode = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode
            speed = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].speed
            fecSupported = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported
            fecRequested = portmgr.portParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested
            nonceDisable = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.nonceDisable
            fcPause = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.fcPause
            fcAsmDir = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.fcAsmDir
            negotiationLaneNum = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.negotiationLaneNum
            interconnectProfile = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.interconnectProfile
            laneNum=getLaneNum(ifMode)

            result, ifModeTmp = myGenWrapper("prvCpssCommonPortIfModeToHwsTranslate",{
                             { "IN",  "GT_U8", "devNum", devNum },
                             { "IN",  "CPSS_PORT_INTERFACE_MODE_ENT", "cpssIfMode", ifMode},
                             { "IN",  "CPSS_PORT_SPEED_ENT", "cpssSpeed", speed},
                             { "OUT", "MV_HWS_PORT_STANDARD", "hwsIfModePtr"}})
            result, portmap = myGenWrapper("cpssDxChPortPhysicalPortMapGet", {
                                {"IN", "GT_U8","devNum",devNum},
                                {"IN", "GT_U32","firstPhysicalPortNumber",portNum},
                                {"IN", "GT_U32", "portMapArraySize", 1 },
                                {"OUT","CPSS_DXCH_PORT_MAP_STC","portMap"}  })
            if result == GT_OK then
                if portmap.portMap.mappingType == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
                    macNum = portmap.portMap.interfaceNum
                else
                    return "Error in cpssDxChPortPhysicalPortMapGet " .. result
                end
            else
                return "Error in cpssDxChPortPhysicalPortMapGet " .. result
            end

            result, portMode = myGenWrapper("hwsPortModeParamsGetToBuffer", {
                             { "IN", "GT_U8",  "devNum", devNum },
                             { "IN", "GT_U32", "portGroup", 0},
                             { "IN", "GT_U32", "portNum", macNum},
                             { "IN", "MV_HWS_PORT_STANDARD", "portMode", ifModeTmp.hwsIfModePtr},
                             { "OUT", "MV_HWS_PORT_INIT_PARAMS", "portParamsBuffer"}})

           speedIdx = getSpeedIdx(portMode.portParamsBuffer.serdesSpeed)
            if params.get_all ~=nil then
                print("Speed                : ",tostring(speed))
                print("Ifmode               : ",tostring(ifMode))
                print("Fec Supported        : ",tostring(fecSupported))
                print("Fec Requested        : ",tostring(fecRequested))
                print("nonce-match          : ",nonceDisable)
                if fcPause then
                    print("flow-control         : ",tostring(fcAsmDir))
                else
                    print("flow-control         : ",tostring(fcPause))
                end

                print("LaneNum              : ",negotiationLaneNum)
                print("Interconnect profile : ",tostring(interconnectProfile))
                print("--------------------------------------------------------------------------------")
                print("Serdes Params")
                print("--------------------------------------------------------------------------------")
                print("Serdes Type          : ",values.serdesType)

                for i=0,laneNum-1,1 do
                    --TX--
                    print("laneNum:", i)
                    if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                        txParams = {}
                        txParams= portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i]
                        atten=txParams.txParams.txTune.avago.atten
                        post=txParams.txParams.txTune.avago.post
                        pre=txParams.txParams.txTune.avago.pre
                        pre2=txParams.txParams.txTune.avago.pre2
                        pre3=txParams.txParams.txTune.avago.pre3
                        --RX--
                        rxParams= portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i]
                        sqlch= rxParams.rxParams.rxTune.avago.sqlch
                        DC=rxParams.rxParams.rxTune.avago.DC
                        LF=rxParams.rxParams.rxTune.avago.LF
                        HF=rxParams.rxParams.rxTune.avago.HF
                        BW=rxParams.rxParams.rxTune.avago.BW
                        gainshape1=rxParams.rxParams.rxTune.avago.gainshape1
                        gainshape2=rxParams.rxParams.rxTune.avago.gainshape2
                        print("-----------------------------------------Tx------------------------------------------------")
                        print(string.format("atten:%d\t post:%d\t pre:%d\t pre2:%d\t pre3:%d\t\n", atten, post, pre, pre2, pre3))
                        print("-----------------------------------------Rx------------------------------------------------")
                        print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW   :%d\t", sqlch, DC, LF, HF, BW))
                        if is_sip_6(devNum) then
                            shortChannelEn=rxParams.rxParams.rxTune.avago.shortChannelEn
                            dfeGAIN=rxParams.rxParams.rxTune.avago.dfeGAIN
                            dfeGAIN2=rxParams.rxParams.rxTune.avago.dfeGAIN2
                            BFLF=rxParams.rxParams.rxTune.avago.BFLF
                            BFHF=rxParams.rxParams.rxTune.avago.BFHF
                            minLf=rxParams.rxParams.rxTune.avago.minLf
                            maxLf=rxParams.rxParams.rxTune.avago.maxLf
                            minHf=rxParams.rxParams.rxTune.avago.minHf
                            maxHf=rxParams.rxParams.rxTune.avago.maxHf
                            minPre1=rxParams.rxParams.rxTune.avago.minPre1
                            maxPre1=rxParams.rxParams.rxTune.avago.maxPre1
                            minPre2=rxParams.rxParams.rxTune.avago.minPre2
                            maxPre2=rxParams.rxParams.rxTune.avago.maxPre2
                            minPost=rxParams.rxParams.rxTune.avago.minPost
                            maxPost=rxParams.rxParams.rxTune.avago.maxPost
                            pre1=rxParams.rxParams.rxTune.avago.pre1
                            pre2=rxParams.rxParams.rxTune.avago.pre2
                            post1=rxParams.rxParams.rxTune.avago.post1
                            rx_termination=rxParams.rxParams.rxTune.avago.termination
                            cold_envelope=rxParams.rxParams.rxTune.avago.coldEnvelope
                            hot_envelope=rxParams.rxParams.rxTune.avago.hotEnvelope
                            print(string.format("gainshape1:%d\t gainshape2:%d\t shortChannelEn:%d\t dfeGAIN   :%d\t", gainshape1, gainshape2, shortChannelEn, dfeGAIN))
                            print(string.format("dfeGAIN2  :%d\t BFLF      :%d\t BFHF      :%d\t minLf         :%d\t", dfeGAIN2, BFLF, BFHF, minLf))
                            print(string.format("maxLf     :%d\t minHf     :%d\t maxHf     :%d\t minPre1       :%d\t", maxLf, minHf, maxHf, minPre1))
                            print(string.format("maxPre1   :%d\t minPre2   :%d\t maxPre2   :%d\t minPost       :%d\t", maxPre1, minPre2, maxPre2, minPost))
                            print(string.format("maxPost   :%d\t pre1      :%d\t pre2      :%d\t post1         :%d\t", maxPost, pre1, pre2, post1))
                            print(string.format("rx termination: %s", rx_termination))
                            print(string.format("cold envelope: %s", cold_envelope))
                            print(string.format("hot envelope: %s", hot_envelope))
                        end
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
                    print("IC profile       : ",tostring(interconnectProfile))
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

                    print("Serdes Params")
                    print("----------------------------------Tx-------------------------------------------------------------")
                    print("Serdes Type : ",values.serdesType)
                    for i=j,laneNum,1 do
                        print("laneNum:", i)
                        if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                            txParams= portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune.avago
                            atten=txParams.atten
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            pre3=txParams.pre3
                        print(string.format("atten:%d\t post:%d\t pre:%d\t pre2:%d\t pre3:%d\t", atten, post, pre, pre2, pre3))
                        elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C112G_E" then
                            txParams = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune.comphy_C112G
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            main=txParams.main
                            print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t \n", main, post, pre, pre2))
                        elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C56G_E" then
                            txParams = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune.comphy_C56G
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            main=txParams.main
                            usr=txParams.usr
                            print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t usr:%s\t\n", main, post, pre, pre2, tostring(usr)))
                        elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C28G_E" then
                            txParams = portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].txParams.txTune.comphy_C28G
                            post=txParams.post
                            pre=txParams.pre
                            peak=txParams.peak
                            print(string.format("peak:%d\t post:%d\t pre:%d\t \n", peak, post, pre))
                        end
                    end
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
                    print("Serdes Params")
                    print("---------------------------------Rx--------------------------------------------------------------")
                    print("Serdes Type : ",values.serdesType)
                    for i=j,laneNum,1 do
                        print("laneNum:", i)
                        if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                            rxParams= portmgr.portParamsStcPtr.portParamsType.apPort.apAttrs.overrideLaneParams[speedIdx*8 + i].rxParams.rxTune.avago
                            sqlch= rxParams.sqlch
                            DC=rxParams.DC
                            LF=rxParams.LF
                            HF=rxParams.HF
                            BW=rxParams.BW
                            gainshape1=rxParams.gainshape1
                            gainshape2=rxParams.gainshape2
                            print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW   :%d\t", sqlch, DC, LF, HF, BW))
                            if is_sip_6(devNum) then
                                shortChannelEn=rxParams.shortChannelEn
                                dfeGAIN=rxParams.dfeGAIN
                                dfeGAIN2=rxParams.dfeGAIN2
                                BFLF=rxParams.BFLF
                                BFHF=rxParams.BFHF
                                minLf=rxParams.minLf
                                maxLf=rxParams.maxLf
                                minHf=rxParams.minHf
                                maxHf=rxParams.maxHf
                                minPre1=rxParams.minPre1
                                maxPre1=rxParams.maxPre1
                                minPre2=rxParams.minPre2
                                maxPre2=rxParams.maxPre2
                                minPost=rxParams.minPost
                                maxPost=rxParams.maxPost
                                pre1=rxParams.pre1
                                pre2=rxParams.pre2
                                post1=rxParams.post1
                                rx_termination=rxParams.termination
                                cold_envelope=rxParams.coldEnvelope
                                hot_envelope=rxParams.hotEnvelope
                                print(string.format("gainshape1:%d\t gainshape2:%d\t shortChannelEn:%d\t dfeGAIN   :%d\t", gainshape1, gainshape2, shortChannelEn, dfeGAIN))
                                print(string.format("dfeGAIN2  :%d\t BFLF      :%d\t BFHF      :%d\t minLf         :%d\t", dfeGAIN2, BFLF, BFHF, minLf))
                                print(string.format("maxLf     :%d\t minHf     :%d\t maxHf     :%d\t minPre1       :%d\t", maxLf, minHf, maxHf, minPre1))
                                print(string.format("maxPre1   :%d\t minPre2   :%d\t maxPre2   :%d\t minPost       :%d\t", maxPre1, minPre2, maxPre2, minPost))
                                print(string.format("maxPost   :%d\t pre1      :%d\t pre2      :%d\t post1         :%d\t", maxPost, pre1, pre2, post1))
                                print(string.format("rx termination: %s", rx_termination))
                                print(string.format("cold envelope: %s", cold_envelope))
                                print(string.format("hot envelope: %s", hot_envelope))
                            end
                        end
                    end
                end
            print("--------------------------------------------------------------------------------")
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
                if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                    for i=0,laneNum-1,1 do
                        print("laneNum:", i)
                        txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago
                        rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago
                        atten=txParams.atten
                        post=txParams.post
                        pre=txParams.pre
                        pre2=txParams.pre2
                        pre3=txParams.pre3
                    --RX--
                        sqlch= rxParams.sqlch
                        DC=rxParams.DC
                        LF=rxParams.LF
                        HF=rxParams.HF
                        BW=rxParams.BW
                        gainshape1=rxParams.gainshape1
                        gainshape2=rxParams.gainshape2
                        print("----------------------------------------Tx----------------------------------------")
                        print(string.format("atten:%d\t post:%d\t pre:%d\t pre2:%d\t pre3:%d\t\n", atten, post, pre, pre2, pre3))
                        print("----------------------------------------Rx----------------------------------------")
                        print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW  :%d\t", sqlch, DC, LF, HF, BW))
                        if is_sip_6(devNum) then
                            shortChannelEn=rxParams.shortChannelEn
                            dfeGAIN=rxParams.dfeGAIN
                            dfeGAIN2=rxParams.dfeGAIN2
                            BFLF=rxParams.BFLF
                            BFHF=rxParams.BFHF
                            minLf=rxParams.minLf
                            maxLf=rxParams.maxLf
                            minHf=rxParams.minHf
                            maxHf=rxParams.maxHf
                            minPre1=rxParams.minPre1
                            maxPre1=rxParams.maxPre1
                            minPre2=rxParams.minPre2
                            maxPre2=rxParams.maxPre2
                            minPost=rxParams.minPost
                            maxPost=rxParams.maxPost
                            pre1=rxParams.pre1
                            pre2=rxParams.pre2
                            post1=rxParams.post1
                            rx_termination=rxParams.termination
                            cold_envelope=rxParams.coldEnvelope
                            hot_envelope=rxParams.hotEnvelope
                            print(string.format("gainshape1:%d\t gainshape2:%d\t shortChannelEn:%d\t dfeGAIN   :%d\t", gainshape1, gainshape2, shortChannelEn, dfeGAIN))
                            print(string.format("dfeGAIN2  :%d\t BFLF      :%d\t BFHF      :%d\t minLf         :%d\t", dfeGAIN2, BFLF, BFHF, minLf))
                            print(string.format("maxLf     :%d\t minHf     :%d\t maxHf     :%d\t minPre1       :%d\t", maxLf, minHf, maxHf, minPre1))
                            print(string.format("maxPre1   :%d\t minPre2   :%d\t maxPre2   :%d\t minPost       :%d\t", maxPre1, minPre2, maxPre2, minPost))
                            print(string.format("maxPost   :%d\t pre1      :%d\t pre2      :%d\t post1         :%d\t", maxPost, pre1, pre2, post1))
                            print(string.format("rx termination: %s", rx_termination))
                            print(string.format("cold envelope: %s", cold_envelope))
                            print(string.format("hot envelope: %s", hot_envelope))
                        end
                    end
                elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C112G_E" then
                    for i=0,laneNum-1,1 do
                        print("laneNum:", i)
                        txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C112G
                        post=txParams.post
                        pre=txParams.pre
                        pre2=txParams.pre2
                        main=txParams.main
                        print("----------------------------------------Tx----------------------------------------")
                        print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t \n", main, post, pre, pre2))
                        print("----------------------------------------Rx----------------------------------------")
                        rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C112G
                        current1Sel    = rxParams.current1Sel
                        rl1Sel         = rxParams.rl1Sel
                        rl1Extra       = rxParams.rl1Extra
                        res1Sel        = rxParams.res1Sel
                        cap1Sel        = rxParams.cap1Sel
                        cl1Ctrl        = rxParams.cl1Ctrl
                        enMidFreq      = rxParams.enMidFreq
                        cs1Mid         = rxParams.cs1Mid
                        rs1Mid         = rxParams.rs1Mid
                        rfCtrl         = rxParams.rfCtrl
                        rl1TiaSel      = rxParams.rl1TiaSel
                        rl1TiaExtra    = rxParams.rl1TiaExtra
                        hpfRSel1st     = rxParams.hpfRSel1st
                        current1TiaSel = rxParams.current1TiaSel
                        rl2Tune        = rxParams.rl2Tune
                        rl2Sel         = rxParams.rl2Sel
                        rs2Sel         = rxParams.rs2Sel
                        current2Sel    = rxParams.current2Sel
                        cap2Sel        = rxParams.cap2Sel
                        hpfRsel2nd     = rxParams.hpfRsel2nd
                        selmufi        = rxParams.selmufi
                        selmuff        = rxParams.selmuff
                        selmupi        = rxParams.selmupi
                        selmupf        = rxParams.selmupf
                        squelch        = rxParams.squelch
                        align90AnaReg  = rxParams.align90AnaReg
                        align90        = rxParams.align90
                        sampler        = rxParams.sampler
                        slewRateCtrl0  = rxParams.slewRateCtrl0
                        slewRateCtrl1  = rxParams.slewRateCtrl1
                        EO             = rxParams.EO
                        print(string.format("current1Sel :%d\t rl1Sel            :%d\t rl1Extra  :%d\t res1Sel     :%d\t", current1Sel, rl1Sel, rl1Extra, res1Sel  ))
                        print(string.format("cap1Sel     :%d\t cl1Ctrl           :%d\t enMidFreq :%d\t cs1Mid      :%d\t", cap1Sel, cl1Ctrl, enMidFreq ,cs1Mid))
                        print(string.format("rs1Mid      :%d\t rfCtrl            :%d\t rl1TiaSel :%d\t rl1TiaExtra :%d\t", rs1Mid, rfCtrl, rl1TiaSel, rl1TiaExtra))
                        print(string.format("hpfRSel1st  :%d\t current1TiaSel    :%d\t rl2Tune   :%d\t rl2Sel      :%d\t", hpfRSel1st, current1TiaSel, rl2Tune, rl2Sel))
                        print(string.format("rs2Sel      :%d\t current2Sel       :%d\t cap2Sel   :%d\t hpfRsel2nd  :%d\t", rs2Sel, current2Sel, cap2Sel, hpfRsel2nd))
                        print(string.format("selmufi     :%d\t selmuff           :%d\t selmupi   :%d\t selmupf     :%d\t", selmufi, selmuff, selmupi, selmupf))
                        print(string.format("squelch     :%d\t align90AnaReg     :%d\t align90   :%d\t sampler     :%d\t", squelch, align90AnaReg, align90, sampler))
                        print(string.format("slewRateCtrl0   :%d\t slewRateCtrl1 :%d\t EO        :%d\t", slewRateCtrl0, slewRateCtrl1, EO))
                    end
                elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C56G_E" then
                    for i=0,laneNum-1,1 do
                        print("laneNum:", i)
                        txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C56G
                        post=txParams.post
                        pre=txParams.pre
                        pre2=txParams.pre2
                        main=txParams.main
                        usr=txParams.usr
                        print("----------------------------------------Tx----------------------------------------")
                        print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t usr:%s\t\n", main, post, pre, pre2, tostring(usr)))
                        print("----------------------------------------Rx----------------------------------------")
                        rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C56G
                        current1Sel    = rxParams.cur1Sel
                        rl1Sel         = rxParams.rl1Sel
                        rl1Extra       = rxParams.rl1Extra
                        res1Sel        = rxParams.res1Sel
                        cap1Sel        = rxParams.cap1Sel
                        enMidFreq      = rxParams.enMidfreq
                        cs1Mid         = rxParams.cs1Mid
                        rs1Mid         = rxParams.rs1Mid
                        cur2Sel        = rxParams.cur2Sel
                        rl2Sel         = rxParams.rl2Sel
                        rl2TuneG       = rxParams.rl2TuneG
                        res2Sel        = rxParams.res2Sel
                        cap2Sel        = rxParams.cap2Sel
                        selmufi        = rxParams.selmufi
                        selmuff        = rxParams.selmuff
                        selmupi        = rxParams.selmupi
                        selmupf        = rxParams.selmupf
                        squelch        = rxParams.squelch
                        print(string.format("current1Sel :%d\t rl1Sel    :%d\t rl1Extra :%d\t res1Sel     :%d\t", current1Sel, rl1Sel, rl1Extra, res1Sel ))
                        print(string.format("cap1Sel     :%d\t enMidFreq :%d\t cs1Mid   :%d\t rs1Mid      :%d\t", cap1Sel, enMidFreq ,cs1Mid, rs1Mid))
                        print(string.format("current2Sel :%d\t rl2Sel    :%d\t rl2TuneG :%d\t res2Sel :%d\t cap2Sel :%d\t", cur2Sel, rl2Sel,rl2TuneG, res2Sel, cap2Sel))
                        print(string.format("selmufi     :%d\t selmuff   :%d\t selmupi   :%d\t selmupf     :%d\t",selmufi, selmuff, selmupi, selmupf))
                        print(string.format("squelch     :%d\t \t", squelch))
                    end
                elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C28G_E" then
                    for i=0,laneNum-1,1 do
                        print("laneNum:", i)
                        txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C28G
                        post=txParams.post
                        pre=txParams.pre
                        peak=txParams.peak
                        print("----------------------------------------Tx----------------------------------------")
                        print(string.format("peak:%d\t post:%d\t pre:%d\t \n", peak, post, pre))
                        print("----------------------------------------Rx----------------------------------------")
                        rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C28G
                        dataRate = rxParams.dataRate
                        res1Sel  = rxParams.res1Sel
                        res2Sel   = rxParams.res2Sel
                        cap1Sel  = rxParams.cap1Sel
                        cap2Sel  = rxParams.cap2Sel
                        selmufi  = rxParams.selmufi
                        selmuff  = rxParams.selmuff
                        selmupi  = rxParams.selmupi
                        selmupf  = rxParams.selmupf
                        midpointLargeThresKLane = rxParams.midpointLargeThresKLane
                        midpointSmallThresKLane = rxParams.midpointSmallThresKLane
                        midpointLargeThresCLane = rxParams.midpointLargeThresCLane
                        midpointSmallThresCLane = rxParams.midpointSmallThresCLane
                        dfeResF0aHighThresInitLane = rxParams.dfeResF0aHighThresInitLane
                        dfeResF0aHighThresEndLane  = rxParams.dfeResF0aHighThresEndLane
                        squelch = rxParams.squelch
                        align90 = rxParams.align90
                        sampler = rxParams.sampler
                        slewRateCtrl0 = rxParams.slewRateCtrl0
                        slewRateCtrl1 = rxParams.slewRateCtrl1
                        EO = rxParams.EO
                        print(string.format("dataRate    :%d\t res1Sel :%d\t res2Sel :%d\t cap1Sel :%d\t cap2Sel :%d\t", dataRate, res1Sel, res2Sel, cap1Sel, cap2Sel ))
                        print(string.format("selmufi     :%d\t selmuff   :%d\t selmupi   :%d\t selmupf     :%d\t",selmufi, selmuff, selmupi, selmupf))
                        print(string.format("midpointLargeThresKLane     :%d\t midpointSmallThresKLane :%d \t", midpointLargeThresKLane, midpointSmallThresKLane))
                        print(string.format("midpointLargeThresCLane     :%d\t midpointSmallThresCLane :%d \t", midpointLargeThresCLane, midpointSmallThresCLane))
                        print(string.format("dfeResF0aHighThresInitLane  :%d\t dfeResF0aHighThresEndLane :%d \t", dfeResF0aHighThresInitLane, dfeResF0aHighThresEndLane))
                        print(string.format("squelch       :%d\t align90       :%d \t sampler :%d \t ", squelch, align90, sampler))
                        print(string.format("slewRateCtrl0 :%d\t slewRateCtrl1 :%d \t EO      :%d \t ", slewRateCtrl0, slewRateCtrl1, EO))
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
                    print("Loopback  :",tostring(loopback.loopbackType))
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
                if params.serdes_tx~=nil then
                    if params.serdes_tx ~="all" then
                        laneNum=params.serdes_tx
                        j = laneNum
                    else
                        laneNum=getLaneNum(ifMode)
                        laneNum=laneNum-1
                        j = 0
                    end
                    print("Serdes Params")
                    print("-----------------------------------Tx----------------------------------------------")
                    if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.avago
                            atten=txParams.atten
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            pre3=txParams.pre3
                            print(string.format("atten    :%d\t post:%d\t pre:%d\t pre2:%d\t pre3:%d\t", atten, post, pre, pre2, pre3))
                            if params.serdes_tx~="all" then
                                break
                            end
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C112G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C112G
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            main=txParams.main
                            print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t \n", main, post, pre, pre2))
                            if params.serdes_tx~="all" then
                                break
                            end
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C56G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C56G
                            post=txParams.post
                            pre=txParams.pre
                            pre2=txParams.pre2
                            main=txParams.main
                            usr=txParams.usr
                            print(string.format("main:%d\t post:%d\t pre:%d\t pre2:%d\t usr:%s\t\n", main, post, pre, pre2, tostring(usr)))
                            if params.serdes_tx~="all" then
                                break
                            end
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C28G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            txParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].txParams.txTune.comphy_C28G
                            post=txParams.post
                            pre=txParams.pre
                            peak=txParams.peak
                            print(string.format("peak:%d\t post:%d\t pre:%d\t \n", peak, post, pre))
                            if params.serdes_tx~="all" then
                                break
                            end
                        end
                    end
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
                    print("Serdes Params")
                    print("-------------------------------Rx-------------------------------------------------")
                    if values.serdesType == "CPSS_PORT_SERDES_AVAGO_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.avago
                            sqlch= rxParams.sqlch
                            DC=rxParams.DC
                            LF=rxParams.LF
                            HF=rxParams.HF
                            BW=rxParams.BW
                            print(string.format("sqlch     :%d\t DC        :%d\t LF            :%d\t HF     :%d\t BW   :%d\t", sqlch, DC, LF, HF, BW))
                            if is_sip_6(devNum) then
                                gainshape1=rxParams.gainshape1
                                gainshape2=rxParams.gainshape2
                                shortChannelEn=rxParams.shortChannelEn
                                dfeGAIN=rxParams.dfeGAIN
                                dfeGAIN2=rxParams.dfeGAIN2
                                BFLF=rxParams.BFLF
                                BFHF=rxParams.BFHF
                                minLf=rxParams.minLf
                                maxLf=rxParams.maxLf
                                minHf=rxParams.minHf
                                maxHf=rxParams.maxHf
                                minPre1=rxParams.minPre1
                                maxPre1=rxParams.maxPre1
                                minPre2=rxParams.minPre2
                                maxPre2=rxParams.maxPre2
                                minPost=rxParams.minPost
                                maxPost=rxParams.maxPost
                                pre1=rxParams.pre1
                                pre2=rxParams.pre2
                                post1=rxParams.post1
                                rx_termination=rxParams.termination
                                cold_envelope=rxParams.coldEnvelope
                                hot_envelope=rxParams.hotEnvelope
                                print(string.format("gainshape1:%d\t gainshape2:%d\t shortChannelEn:%d\t dfeGAIN   :%d\t", gainshape1, gainshape2, shortChannelEn, dfeGAIN))
                                print(string.format("dfeGAIN2  :%d\t BFLF      :%d\t BFHF      :%d\t minLf         :%d\t", dfeGAIN2, BFLF, BFHF, minLf))
                                print(string.format("maxLf     :%d\t minHf     :%d\t maxHf     :%d\t minPre1       :%d\t", maxLf, minHf, maxHf, minPre1))
                                print(string.format("maxPre1   :%d\t minPre2   :%d\t maxPre2   :%d\t minPost       :%d\t", maxPre1, minPre2, maxPre2, minPost))
                                print(string.format("maxPost   :%d\t pre1      :%d\t pre2      :%d\t post1         :%d\t", maxPost, pre1, pre2, post1))
                                print(string.format("rx termination: %s", rx_termination))
                                print(string.format("cold envelope: %s", cold_envelope))
                                print(string.format("hot envelope: %s",  hot_envelope))
                            end
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C112G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C112G
                            current1Sel    = rxParams.current1Sel
                            rl1Sel         = rxParams.rl1Sel
                            rl1Extra       = rxParams.rl1Extra
                            res1Sel        = rxParams.res1Sel
                            cap1Sel        = rxParams.cap1Sel
                            cl1Ctrl        = rxParams.cl1Ctrl
                            enMidFreq      = rxParams.enMidFreq
                            cs1Mid         = rxParams.cs1Mid
                            rs1Mid         = rxParams.rs1Mid
                            rfCtrl         = rxParams.rfCtrl
                            rl1TiaSel      = rxParams.rl1TiaSel
                            rl1TiaExtra    = rxParams.rl1TiaExtra
                            hpfRSel1st     = rxParams.hpfRSel1st
                            current1TiaSel = rxParams.current1TiaSel
                            rl2Tune        = rxParams.rl2Tune
                            rl2Sel         = rxParams.rl2Sel
                            rs2Sel         = rxParams.rs2Sel
                            current2Sel    = rxParams.current2Sel
                            cap2Sel        = rxParams.cap2Sel
                            hpfRsel2nd     = rxParams.hpfRsel2nd
                            selmufi        = rxParams.selmufi
                            selmuff        = rxParams.selmuff
                            selmupi        = rxParams.selmupi
                            selmupf        = rxParams.selmupf
                            squelch        = rxParams.squelch
                            align90AnaReg  = rxParams.align90AnaReg
                            align90        = rxParams.align90
                            sampler        = rxParams.sampler
                            slewRateCtrl0  = rxParams.slewRateCtrl0
                            slewRateCtrl1  = rxParams.slewRateCtrl1
                            EO             = rxParams.EO
                            print(string.format("current1Sel :%d\t rl1Sel            :%d\t rl1Extra  :%d\t res1Sel     :%d\t", current1Sel, rl1Sel, rl1Extra, res1Sel  ))
                            print(string.format("cap1Sel     :%d\t cl1Ctrl           :%d\t enMidFreq :%d\t cs1Mid      :%d\t", cap1Sel, cl1Ctrl, enMidFreq ,cs1Mid))
                            print(string.format("rs1Mid      :%d\t rfCtrl            :%d\t rl1TiaSel :%d\t rl1TiaExtra :%d\t", rs1Mid, rfCtrl, rl1TiaSel, rl1TiaExtra))
                            print(string.format("hpfRSel1st  :%d\t current1TiaSel    :%d\t rl2Tune   :%d\t rl2Sel      :%d\t", hpfRSel1st, current1TiaSel, rl2Tune, rl2Sel))
                            print(string.format("rs2Sel      :%d\t current2Sel       :%d\t cap2Sel   :%d\t hpfRsel2nd  :%d\t", rs2Sel, current2Sel, cap2Sel, hpfRsel2nd))
                            print(string.format("selmufi      :%d\t selmuff           :%d\t selmupi   :%d\t selmupf     :%d\t", selmufi, selmuff, selmupi, selmupf))
                            print(string.format("squelch     :%d\t align90AnaReg     :%d\t align90   :%d\t sampler     :%d\t", squelch, align90AnaReg, align90, sampler))
                            print(string.format("slewRateCtrl0   :%d\t slewRateCtrl1 :%d\t EO        :%d\t", slewRateCtrl0, slewRateCtrl1, EO))
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C56G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C56G
                            current1Sel    = rxParams.cur1Sel
                            rl1Sel         = rxParams.rl1Sel
                            rl1Extra       = rxParams.rl1Extra
                            res1Sel        = rxParams.res1Sel
                            cap1Sel        = rxParams.cap1Sel
                            enMidFreq      = rxParams.enMidfreq
                            cs1Mid         = rxParams.cs1Mid
                            rs1Mid         = rxParams.rs1Mid
                            cur2Sel        = rxParams.cur2Sel
                            rl2Sel         = rxParams.rl2Sel
                            rl2TuneG       = rxParams.rl2TuneG
                            res2Sel        = rxParams.res2Sel
                            cap2Sel        = rxParams.cap2Sel
                            selmufi        = rxParams.selmufi
                            selmuff        = rxParams.selmuff
                            selmupi        = rxParams.selmupi
                            selmupf        = rxParams.selmupf
                            squelch        = rxParams.squelch
                            print(string.format("current1Sel :%d\t rl1Sel    :%d\t rl1Extra :%d\t res1Sel     :%d\t", current1Sel, rl1Sel, rl1Extra, res1Sel ))
                            print(string.format("cap1Sel     :%d\t enMidFreq :%d\t cs1Mid   :%d\t rs1Mid      :%d\t", cap1Sel, enMidFreq ,cs1Mid, rs1Mid))
                            print(string.format("current2Sel :%d\t rl2Sel    :%d\t rl2TuneG :%d\t res2Sel :%d\t cap2Sel :%d\t", cur2Sel, rl2Sel,rl2TuneG, res2Sel, cap2Sel))
                            print(string.format("selmufi     :%d\t selmuff   :%d\t selmupi   :%d\t selmupf     :%d\t",selmufi, selmuff, selmupi, selmupf))
                            print(string.format("squelch     :%d\t \t", squelch))
                        end
                    elseif values.serdesType == "CPSS_PORT_SERDES_COMPHY_C28G_E" then
                        for i=j,laneNum,1 do
                            print("laneNum:", i)
                            rxParams = portmgr.portParamsStcPtr.portParamsType.regPort.laneParams[i].rxParams.rxTune.comphy_C28G
                            dataRate = rxParams.dataRate
                            res1Sel  = rxParams.res1Sel
                            res2Sel   = rxParams.res2Sel
                            cap1Sel  = rxParams.cap1Sel
                            cap2Sel  = rxParams.cap2Sel
                            selmufi  = rxParams.selmufi
                            selmuff  = rxParams.selmuff
                            selmupi  = rxParams.selmupi
                            selmupf  = rxParams.selmupf
                            midpointLargeThresKLane = rxParams.midpointLargeThresKLane
                            midpointSmallThresKLane = rxParams.midpointSmallThresKLane
                            midpointLargeThresCLane = rxParams.midpointLargeThresCLane
                            midpointSmallThresCLane = rxParams.midpointSmallThresCLane
                            dfeResF0aHighThresInitLane = rxParams.dfeResF0aHighThresInitLane
                            dfeResF0aHighThresEndLane  = rxParams.dfeResF0aHighThresEndLane
                            squelch = rxParams.squelch
                            align90 = rxParams.align90
                            sampler = rxParams.sampler
                            slewRateCtrl0 = rxParams.slewRateCtrl0
                            slewRateCtrl1 = rxParams.slewRateCtrl1
                            EO = rxParams.EO
                            print(string.format("dataRate       :%d\t res1Sel :%d\t res2Sel :%d\t cap1Sel :%d\t cap2Sel :%d\t", dataRate, res1Sel, res2Sel, cap1Sel, cap2Sel ))
                            print(string.format("selmufi        :%d\t selmuff :%d\t selmupi :%d\t selmupf :%d\t",selmufi, selmuff, selmupi, selmupf))
                            print(string.format("squelch        :%d\t align90 :%d \t sampler :%d \t ", squelch, align90, sampler))
                            print(string.format("slewRateCtrl0  :%d\t slewRateCtrl1 :%d \t EO      :%d \t ", slewRateCtrl0, slewRateCtrl1, EO))
                            print(string.format("midpointLargeThresKLane    :%d\t midpointSmallThresKLane   :%d \t", midpointLargeThresKLane, midpointSmallThresKLane))
                            print(string.format("midpointLargeThresCLane    :%d\t midpointSmallThresCLane   :%d \t", midpointLargeThresCLane, midpointSmallThresCLane))
                            print(string.format("dfeResF0aHighThresInitLane :%d\t dfeResF0aHighThresEndLane :%d \t", dfeResF0aHighThresInitLane, dfeResF0aHighThresEndLane))
                        end
                    end
                end
            print("--------------------------------------------------------------------------------")
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

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
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
            { format="serdes", name="serdes", help="Serdes general parameters"},
            { format="802.3ap", name="ap", help="802.3ap parameters"}
  }}}
)

