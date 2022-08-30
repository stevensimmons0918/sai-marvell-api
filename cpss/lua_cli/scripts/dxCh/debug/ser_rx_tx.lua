--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ser_rx_tx.lua
--*
--* DESCRIPTION:
--*       show ser statistics and SER counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************

local function serdesPrintResult(command_data,devNum,portNum,laneNum,state)

	local ret,OutValues
	local invert,val_serdes, val_pol
    local DFE_string, FFE_string, eyeO_string
    local coding, rxEncoding, dataPathConfig
    local cdrLock, pllLock, signalLock
    local tx_string, rx_string
    ret, val_serdes = myGenWrapper("cpssDxChPortSerdesAutoTuneResultsGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U32","laneNum",laneNum},
                        {"OUT","CPSS_PORT_SERDES_TUNE_STC","serdesTunePtr"}
                        })

    if ret == 0 then
        OutValues = val_serdes["serdesTunePtr"]
    else
        command_data:setFailPortStatus()
        command_data:addError("Port serdes result status cannot be read " ..
                              "device %d port %d.",
                              devNum, portNum)
    end

	ret,val_pol = myGenWrapper("cpssDxChPortSerdesPolarityGet",{
							{"IN","GT_U8","devNum",devNum},
							{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
							{"IN","GT_U32","laneNum",laneNum},
							{"OUT","GT_BOOL","invertTx"},
							{"OUT","GT_BOOL","invertRx"}
							})
	if ret ~= 0 then
		command_data:setFailPortStatus()
		command_data:addError("Port serdes polarity result status cannot be read " ..
							"device %d port %d.",
							devNum, portNum)
	else
	end
    if is_sip_6_10(devNum) then
        if OutValues.type == "CPSS_PORT_SERDES_COMPHY_C112G_E" then
            tx_string =   string.format("pre2 :%s  ",OutValues.txTune.comphy_C112G.pre2)
                        ..string.format("pre  :%s  ",OutValues.txTune.comphy_C112G.pre)
                        ..string.format("main :%s  ",OutValues.txTune.comphy_C112G.main)
                        ..string.format("post :%s  ",OutValues.txTune.comphy_C112G.post)

            rx_string =   string.format("current1Sel    :%s  ",OutValues.rxTune.comphy_C112G.current1Sel)
                        ..string.format("rl1Sel         :%s  ",OutValues.rxTune.comphy_C112G.rl1Sel)
                        ..string.format("rl1Extra       :%s  ",OutValues.rxTune.comphy_C112G.rl1Extra)
                        ..string.format("res1Sel        :%s\n",OutValues.rxTune.comphy_C112G.res1Sel)
                        ..string.format("cap1Sel        :%s  ",OutValues.rxTune.comphy_C112G.cap1Sel)
                        ..string.format("cl1Ctrl        :%s  ",OutValues.rxTune.comphy_C112G.cl1Ctrl)
                        ..string.format("enMidFreq      :%s  ",OutValues.rxTune.comphy_C112G.enMidFreq)
                        ..string.format("cs1Mid         :%s\n",OutValues.rxTune.comphy_C112G.cs1Mid)
                        ..string.format("rs1Mid         :%s  ",OutValues.rxTune.comphy_C112G.rs1Mid)
                        ..string.format("rfCtrl         :%s  ",OutValues.rxTune.comphy_C112G.rfCtrl)
                        ..string.format("rl1TiaSel      :%s  ",OutValues.rxTune.comphy_C112G.rl1TiaSel)
                        ..string.format("rl1TiaExtra    :%s\n",OutValues.rxTune.comphy_C112G.rl1TiaExtra)
                        ..string.format("hpfRSel1st     :%s  ",OutValues.rxTune.comphy_C112G.hpfRSel1st)
                        ..string.format("current1TiaSel :%s  ",OutValues.rxTune.comphy_C112G.current1TiaSel)
                        ..string.format("rl2Tune        :%s  ",OutValues.rxTune.comphy_C112G.rl2Tune)
                        ..string.format("rl2Sel         :%s\n",OutValues.rxTune.comphy_C112G.rl2Sel)
                        ..string.format("rs2Sel         :%s  ",OutValues.rxTune.comphy_C112G.rs2Sel)
                        ..string.format("current2Sel    :%s  ",OutValues.rxTune.comphy_C112G.current2Sel)
                        ..string.format("cap2Sel        :%s  ",OutValues.rxTune.comphy_C112G.cap2Sel)
                        ..string.format("hpfRsel2nd     :%s\n",OutValues.rxTune.comphy_C112G.hpfRsel2nd)
                        ..string.format("selmufi        :%s  ",OutValues.rxTune.comphy_C112G.selmufi)
                        ..string.format("selmuff        :%s  ",OutValues.rxTune.comphy_C112G.selmuff)
                        ..string.format("selmupi        :%s  ",OutValues.rxTune.comphy_C112G.selmupi)
                        ..string.format("selmupf        :%s\n",OutValues.rxTune.comphy_C112G.selmupf)
                        ..string.format("squelch        :%s  ",OutValues.rxTune.comphy_C112G.squelch)
                        ..string.format("align90AnaReg  :%s  ",OutValues.rxTune.comphy_C112G.align90AnaReg)
                        ..string.format("align90        :%s  ",OutValues.rxTune.comphy_C112G.align90)
                        ..string.format("sampler        :%s\n",OutValues.rxTune.comphy_C112G.sampler)
                        ..string.format("slewRateCtrl0  :%s  ",OutValues.rxTune.comphy_C112G.slewRateCtrl0)
                        ..string.format("slewRateCtrl1  :%s  ",OutValues.rxTune.comphy_C112G.slewRateCtrl1)
                        ..string.format("EO             :%s  ",OutValues.rxTune.comphy_C112G.EO)
        elseif OutValues.type == "CPSS_PORT_SERDES_COMPHY_C56G_E" then
            tx_string =   string.format("pre2 :%s  ",OutValues.txTune.comphy_C56G.pre2)
                        ..string.format("pre  :%s  ",OutValues.txTune.comphy_C56G.pre)
                        ..string.format("main :%s  ",OutValues.txTune.comphy_C56G.main)
                        ..string.format("post :%s  ",OutValues.txTune.comphy_C56G.post)
                        ..string.format("usr  :%s  ",tostring(OutValues.txTune.comphy_C56G.usr))

            rx_string =   string.format("cur1Sel   :%s   ",OutValues.rxTune.comphy_C56G.cur1Sel)
                        ..string.format("rl1Sel    :%s   ",OutValues.rxTune.comphy_C56G.rl1Sel)
                        ..string.format("rl1Extra  :%s   ",OutValues.rxTune.comphy_C56G.rl1Extra)
                        ..string.format("res1Sel   :%s   ",OutValues.rxTune.comphy_C56G.res1Sel)
                        ..string.format("cap1Sel   :%s\n",OutValues.rxTune.comphy_C56G.cap1Sel)
                        ..string.format("enMidFreq :%s  ",OutValues.rxTune.comphy_C56G.enMidfreq)
                        ..string.format("cs1Mid    :%s  ",OutValues.rxTune.comphy_C56G.cs1Mid)
                        ..string.format("rs1Mid    :%s  ",OutValues.rxTune.comphy_C56G.rs1Mid)
                        ..string.format("cur2Sel   :%s  ",OutValues.rxTune.comphy_C56G.cur2Sel)
                        ..string.format("rl2Sel    :%s\n",OutValues.rxTune.comphy_C56G.rl2Sel)
                        ..string.format("rl2TuneG  :%s  ",OutValues.rxTune.comphy_C56G.rl2TuneG)
                        ..string.format("res2Sel   :%s  ",OutValues.rxTune.comphy_C56G.res2Sel)
                        ..string.format("cap2Sel   :%s  ",OutValues.rxTune.comphy_C56G.cap2Sel)
                        ..string.format("selmufi   :%s  ",OutValues.rxTune.comphy_C56G.selmufi)
                        ..string.format("selmuff   :%s\n",OutValues.rxTune.comphy_C56G.selmuff)
                        ..string.format("selmupi   :%s  ",OutValues.rxTune.comphy_C56G.selmupi)
                        ..string.format("selmupf   :%s  ",OutValues.rxTune.comphy_C56G.selmupf)
                        ..string.format("squelch   :%s  ",OutValues.rxTune.comphy_C56G.squelch)
        elseif OutValues.type == "CPSS_PORT_SERDES_COMPHY_C28G_E" then
            tx_string =   string.format("pre  :%s  ",OutValues.txTune.comphy_C28G.pre)
                        ..string.format("peak :%s  ",OutValues.txTune.comphy_C28G.peak)
                        ..string.format("post :%s  ",OutValues.txTune.comphy_C28G.post)

            rx_string =   string.format("dataRate :%s  ",OutValues.rxTune.comphy_C28G.dataRate)
                        ..string.format("res1Sel  :%s  ",OutValues.rxTune.comphy_C28G.res1Sel)
                        ..string.format("res2Sel  :%s  ",OutValues.rxTune.comphy_C28G.res2Sel)
                        ..string.format("cap1Sel  :%s  ",OutValues.rxTune.comphy_C28G.cap1Sel)
                        ..string.format("cap2Sel  :%s  \n",OutValues.rxTune.comphy_C28G.cap2Sel)
                        ..string.format("selmufi  :%s  ",OutValues.rxTune.comphy_C28G.selmufi)
                        ..string.format("selmuff  :%s  ",OutValues.rxTune.comphy_C28G.selmuff)
                        ..string.format("selmupi  :%s  ",OutValues.rxTune.comphy_C28G.selmupi)
                        ..string.format("selmupf  :%s  \n",OutValues.rxTune.comphy_C28G.selmupf)
                        ..string.format("squelch  :%s  ",OutValues.rxTune.comphy_C28G.squelch)
                        ..string.format("align90  :%s  ",OutValues.rxTune.comphy_C28G.align90)
                        ..string.format("sampler  :%s  \n",OutValues.rxTune.comphy_C28G.sampler)
                        ..string.format("slewRateCtrl0 :%s  ",OutValues.rxTune.comphy_C28G.slewRateCtrl0)
                        ..string.format("slewRateCtrl1 :%s  ",OutValues.rxTune.comphy_C28G.slewRateCtrl1)
                        ..string.format("EO            :%s  \n",OutValues.rxTune.comphy_C28G.EO)
                        ..string.format("midpointLargeThresKLane    :%s  ",OutValues.rxTune.comphy_C28G.midpointLargeThresKLane)
                        ..string.format("midpointSmallThresKLane    :%s  \n",OutValues.rxTune.comphy_C28G.midpointSmallThresKLane)
                        ..string.format("midpointLargeThresCLane    :%s  ",OutValues.rxTune.comphy_C28G.midpointLargeThresCLane)
                        ..string.format("midpointSmallThresCLane    :%s  \n",OutValues.rxTune.comphy_C28G.midpointSmallThresCLane)
                        ..string.format("dfeResF0aHighThresInitLane :%s  ",OutValues.rxTune.comphy_C28G.dfeResF0aHighThresInitLane)
                        ..string.format("dfeResF0aHighThresEndLane  :%s \n",OutValues.rxTune.comphy_C28G.dfeResF0aHighThresEndLane)
        end

        if state == "ser_tx" then
	    command_data["result"] = string.format("Interface %s/%s:",tostring(devNum),tostring(portNum))
                                     ..string.format("  Lane %-3s: ",laneNum).."\n"
			 .."---------------------------------Tx--------------------------------------- \n"
			 ..tx_string.."\n"
			 .."-------------------------------------------------------------------------- \n"
         elseif state == "ser_rx" then
	    command_data["result"] = string.format("Interface %s/%s:",tostring(devNum),tostring(portNum))
                                     ..string.format("  Lane %-3s: ",laneNum).."\n"
			 .."---------------------------------RX--------------------------------------- \n"
			 ..rx_string.."\n"
			 .."-------------------------------------------------------------------------- \n"
         else
            command_data["result"] = string.format("  Lane %-3s: ",laneNum).."\n"
			 .."-------------------------------------------------------------------------- \n"
			 ..string.format("RX Polarity: %-6s                 TX Polarity: %-6s  ",to_string(val_pol.invertRx), to_string(val_pol.invertTx)).. " \n"
			 .."---------------------------------Tx--------------------------------------- \n"
			 ..tx_string.."\n"
			 .."---------------------------------RX--------------------------------------- \n"
			 ..rx_string.."\n"
			 .."-------------------------------------------------------------------------- \n"
         end
    elseif is_sip_6(devNum) then
        DFE_string = string.format("%s",OutValues.rxTune.avago.DFE[0])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[1])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[2])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[3])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[4])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[5])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[6])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[7])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[8])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[9])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[10])..","
        ..string.format("%s",OutValues.rxTune.avago.DFE[11])

        FFE_string = string.format("%s",OutValues.rxTune.avago.pre1)..","
        ..string.format("%s",OutValues.rxTune.avago.pre2)..","
        ..string.format("%s",OutValues.rxTune.avago.post1)

        if OutValues.rxTune.avago.EO ~= 0 then
            eyeO_string = string.format("%s",OutValues.rxTune.avago.EO)
        else
            eyeO_string = string.format("%s",OutValues.rxTune.avago.pam4EyesArr[0])..","
				..string.format("%s",OutValues.rxTune.avago.pam4EyesArr[1])..","
				..string.format("%s",OutValues.rxTune.avago.pam4EyesArr[2])..","
				..string.format("%s",OutValues.rxTune.avago.pam4EyesArr[3])..","
				..string.format("%s",OutValues.rxTune.avago.pam4EyesArr[4])..","
				..string.format("%s",OutValues.rxTune.avago.pam4EyesArr[5])
        end
	command_data:updateStatus()

	if state ~= "ser_rx" and state ~= "ser_tx" then

        ret, cdrLock = myGenWrapper("cpssDxChPortSerdesCDRLockStatusGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","cdrLockPtr"}
                })
	    if ret ~= 0 then
		    command_data:setFailPortStatus()
		    command_data:addError("Error in cal of api cpssDxChPortSerdesCDRLockStatusGet" ..
			    				"device %d port %d.", devNum, portNum)
	    else
	    end
        ret, pllLock = myGenWrapper("cpssDxChPortSerdesRxPllLockGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","lockPtr"}
                })
	    if ret ~= 0 then
		    command_data:setFailPortStatus()
		    command_data:addError("Error in cal of api cpssDxChPortSerdesRxPllLockGet" ..
			    				"device %d port %d.", devNum, portNum)
	    else
	    end
        ret, signalLock = myGenWrapper("cpssDxChPortSerdesLaneSignalDetectGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","portGroupId",0},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","GT_BOOL","signalStatePtr"}
                })
	    if ret ~= 0 then
		    command_data:setFailPortStatus()
		    command_data:addError("Error in cal of api cpssDxChPortSerdesLaneSignalDetectGet" ..
			    				"device %d port %d.", devNum, portNum)
	    else
	    end
        ret, dataPathConfig = myGenWrapper("cpssDxChPortSerdesRxDatapathConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC","rxDatapathConfigPtr"}
                })
	    if ret ~= 0 then
		    command_data:setFailPortStatus()
		    command_data:addError("Error in cal of api cpssDxChPortSerdesRxDatapathConfigGet" ..
			    				"device %d port %d.", devNum, portNum)
	    else
	    end
        ret, coding = myGenWrapper("cpssDxChPortSerdesEncodingTypeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","MV_HWS_SERDES_ENCODING_TYPE","txEncodingPtr"},
                {"OUT","MV_HWS_SERDES_ENCODING_TYPE","rxEncodingPtr"}
                })
	    if ret ~= 0 then
		    command_data:setFailPortStatus()
		    command_data:addError("Error in cal of api cpssDxChPortSerdesEncodingTypeGet" ..
			    				"device %d port %d.", devNum, portNum)
	    else
	    end

        if coding.rxEncodingPtr == "SERDES_ENCODING_PAM4" then
            rxEncoding = "PAM4"
        elseif  coding.rxEncodingPtr == "SERDES_ENCODING_NRZ" then
            rxEncoding = "NRZ"
        else
            rxEncoding = "N/A"
        end

            command_data["result"] = string.format("  Lane %-3s: ",laneNum).."   ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               LF: %-3s    HF:  %-3s    BW:  %-3s    SQ:%-3s    DC: %-3s    GS1: %-3s    GS2: %-3s",
                     OutValues.rxTune.avago.LF, OutValues.rxTune.avago.HF, OutValues.rxTune.avago.BW, OutValues.rxTune.avago.sqlch,
                     OutValues.rxTune.avago.DC, OutValues.rxTune.avago.gainshape1, OutValues.rxTune.avago.gainshape2).."\n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               DFE: %-30s  FFE:  %-8s ",DFE_string,FFE_string).. "\n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               EO: %-17s ",eyeO_string).. "\n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               Attn: %-3s   Post: %-3s   Pre1: %-3s   Pre2: %-3s   Pre3: %-3s",
                     OutValues.txTune.avago.atten, OutValues.txTune.avago.post, OutValues.txTune.avago.pre, OutValues.txTune.avago.pre2, OutValues.txTune.avago.pre3).."\n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               RX Polarity: %-6s                 TX Polarity: %-6s  ",to_string(val_pol.invertRx), to_string(val_pol.invertTx)).. " \n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               Rx Coding: %-4s   Gray: %-4s      Pre-Coder: %-4s   Swizzle:  %-4s ",rxEncoding, 
                     to_string(dataPathConfig.rxDatapathConfigPtr.grayEnable),
                     to_string(dataPathConfig.rxDatapathConfigPtr.precodeEnable),
                     to_string(dataPathConfig.rxDatapathConfigPtr.swizzleEnable)).. " \n"
			 .."               ------------------------------------------------------------------------------------------ \n"
			 ..string.format("               PLL Lock: %-4s          Signal Lock: %-4s           CDR Lock: %-4s ",to_string(pllLock.lockPtr),
                     to_string(signalLock.signalStatePtr),
                     to_string(cdrLock.cdrLockPtr)).. " \n"
			 .."               ------------------------------------------------------------------------------------------ \n"
	else
		if state == "ser_rx" then
            command_data["result"] =  string.format("%s/%-8s",tostring(devNum),tostring(portNum)).."|  "
				..string.format("%-3s",laneNum).." | "
				..string.format("%-3s",OutValues.rxTune.avago.sqlch).." | "
				..string.format("%-3s",OutValues.rxTune.avago.LF).." | "
				..string.format("%-3s",OutValues.rxTune.avago.HF).." | "
				..string.format("%-3s",OutValues.rxTune.avago.BW).." | "
				..string.format("%-3s",OutValues.rxTune.avago.DC).." | "
				..string.format("%-3s",OutValues.rxTune.avago.gainshape1).." | "
				..string.format("%-3s",OutValues.rxTune.avago.gainshape2).." | "
				..string.format("%-30s",DFE_string).."| "
				..string.format("%-8s",FFE_string).."| "
				..string.format("%-17s",eyeO_string).." | "
				..string.format("%-9s",to_string(val_pol.invertRx))
		else
			command_data["result"] =  string.format("%s/%-8s",tostring(devNum),tostring(portNum)).."|  "
				..string.format("%-3s",laneNum).." | "
				..string.format("%-4s",OutValues.txTune.avago.atten).." | "
				..string.format("%-4s",OutValues.txTune.avago.pre).." | "
				..string.format("%-4s",OutValues.txTune.avago.pre2).." | "
				..string.format("%-4s",OutValues.txTune.avago.pre3).." | "
				..string.format("%-4s",OutValues.txTune.avago.post).." | "
				..string.format("%-9s",to_string(val_pol.invertTx))
		end
	end
    end
	command_data:addResultToResultArray()
    return 0
end

-- ************************************************************************

local function serdesPrintPort(command_data,devNum,portNum,state)

    local ret
    local numOfSerdesLanesPtr
	local laneNum

	ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
	if( ret == 0 ) then
		for i=0,numOfSerdesLanesPtr-1,1 do
			laneNum = i
			ret = serdesPrintResult(command_data,devNum,portNum,laneNum,state)
			if ret ~= 0 then
				command_data:setFailPortAndLocalStatus()
				command_data:addError("Error Printing data: device %d " ..
									  "port %d: lane %d: %s", devNum, portNum, laneNum,
									  returnCodes[ret])
			else
			end
		end
	end

    return ret
end

function serdesCommonDevPortLoop(params)

    local portNum,devNum, devPort
    local all_ports
    local ret=0
    local result
    local state=""
    local header_string=""
    local footer_string=""

    local command_data = Command_Data()
    setGlobal("ifType", "ethernet")
    setGlobal("ifRange", params["devPort"])

    command_data:initInterfaceDevPortRange()
    command_data:clearResultArray()
    state=params.state

    if state == "ser_rx" then
        if is_sip_6(devNum) and not is_sip_6_10(devNum) then
            header_string =
           "\n" ..
           "Interface | Lane | SQ  | LF  | HF  | BW  | DC  | GS1 | GS2 | DFE                           | FFE     | EO                | Polarity \n" ..
           "----------+------+-----+-----+-----+-----+-----+-----+-----+-------------------------------+---------+-------------------+----------\n"
            footer_string = "\n"
        end
		command_data:enablePausedPrinting()
		command_data:setEmergencyPrintingHeaderAndFooter(header_string,
														 footer_string)

		if true == command_data["status"] then
			all_ports = getGlobal("ifRange")--get table of ports
			for iterator, devNum, portNum in command_data:getPortIterator() do
				ret=serdesPrintPort(command_data,devNum,portNum,state)
			end
		end

		command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"], footer_string)
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

	elseif state == "ser_tx" then
        if is_sip_6(devNum) and not is_sip_6_10(devNum) then
		header_string =
           "\n" ..
           "Interface | Lane | Attn | Pre1 | Pre2 | Pre3 | Post | Polarity \n" ..
           "----------+------+------+------+------+------+------+----------\n"
        footer_string = "\n"
        end
		command_data:enablePausedPrinting()
		command_data:setEmergencyPrintingHeaderAndFooter(header_string,
														 footer_string)

		if true == command_data["status"] then
			all_ports = getGlobal("ifRange")--get table of ports
			for iterator, devNum, portNum in command_data:getPortIterator() do
				ret=serdesPrintPort(command_data,devNum,portNum,state)
			end
		end

		command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"], footer_string)
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

	elseif state == "ser_only" then
		if true == command_data["status"] then
			all_ports = getGlobal("ifRange")--get table of ports
			for iterator, devNum, portNum in command_data:getPortIterator() do
				command_data["result"] = string.format("Interface %s/%s:",tostring(devNum),tostring(portNum)).."\n"
				command_data:addResultToResultArray()
				ret=serdesPrintPort(command_data,devNum,portNum,state)
			end
		end
		command_data:setResultArrayToResultStr()
		command_data:setResultStr(header_string, command_data["result"], footer_string)
		command_data:analyzeCommandExecution()
		command_data:printCommandExecutionResults()
	elseif state == "ser_both" then
		if true == command_data["status"] then
            command_data["result"] = string.format("Either RX or TX params can be shown at once.").."\n"
            command_data:addResultToResultArray()
		end
		command_data:setResultArrayToResultStr()
		command_data:setResultStr(header_string, command_data["result"], footer_string)
		command_data:analyzeCommandExecution()
		command_data:printCommandExecutionResults()
	else
    end

    return ret
end

local function serShowHandler(params)

    if params["rx"] ~= nil and params["tx"] ~= nil then
        params.state="ser_both"
    elseif params["rx"] ~= nil then
		params.state="ser_rx"
	elseif params["tx"] ~= nil then
		params.state="ser_tx"
	else
		params.state="ser_only"
	end
    return serdesCommonDevPortLoop(params)
end

--------------------------------------------------------------------------------
-- command registration: fec show
--------------------------------------------------------------------------------

CLI_addHelp("debug", "link serdes", "Link Serdes info")
CLI_addHelp("debug", "link serdes show", "Show Serdes RX and TX statistics")
CLI_addCommand("debug", "link serdes show interface", {
  func   = serShowHandler,
  help   = "Show Serdes RX and TX statistics",
  params={
        {   type="named",
			{ format=" ethernet %port-range", name="devPort", help="Ethernet interface to show" },
			{ format=" rx", name="rx", help="serdes Rx parameters" },
			{ format=" tx", name="tx", help="serdes Tx parameters" },
            mandatory={"devPort"}
        }
   }
})

--------------------------------------------------------------------------------
