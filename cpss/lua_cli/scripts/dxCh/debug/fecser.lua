--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fecser.lua
--*
--* DESCRIPTION:
--*       show fec statistics and SER counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--********************************************************************************
-- function to get current time

--********************************************************************************

__ser_start_time    	= {}
__ser_read_time     	= {}
__ser_error_counter 	= {}

-- function to set SER test start time
-- ************************************************************************
local function set_start_time(portNum)
    local ret
    ret, __ser_start_time[portNum] = get_time()
    __ser_read_time[portNum] =  __ser_start_time[portNum]
    return ret
end

-- ************************************************************************
local function get_start_time(portNum)
    return __ser_start_time[portNum]
end

-- function to set SER test read time
-- ************************************************************************
local function set_read_time(portNum)
    local ret
	ret, __ser_read_time[portNum] = get_time()
    return ret
end

-- ************************************************************************
local function get_read_time(portNum)
	return __ser_read_time[portNum]
end

local function set_error_cntr(portNum, laneNum, err)
    if __ser_error_counter[(8 * portNum) + laneNum ] == nil then
       __ser_error_counter[(8 * portNum) + laneNum ] = 0
    end
    __ser_error_counter[(8 * portNum) + laneNum ] =  __ser_error_counter[(8 * portNum) + laneNum ] + err
    return true
end

-- ************************************************************************
local function clear_all(devNum, portNum)
	local ret, numOfSerdesLanesPtr, laneNum, i
    __ser_start_time[portNum]    	= 0
    __ser_read_time[portNum]     	= 0

	ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
	if( ret == 0 ) then
		for i=0,numOfSerdesLanesPtr-1,1 do
			laneNum = i
			__ser_error_counter[(8 * portNum) + laneNum ] 	= 0
		end
	end
    return true
end

-- ************************************************************************
local function get_error_cntr(portNum,laneNum)
    return __ser_error_counter[(8 * portNum) + laneNum ]
end

local function getPortFecType(devNum,portNum)
    local fecType

	ret,fecType = myGenWrapper("cpssDxChPortFecModeGet",
                    {
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_DXCH_PORT_FEC_MODE_ENT","modePtr"}
                    })

    return fecType.modePtr
end

local function fecSerClear(devNum,portNum)
    local ret,val_status, val_counter_rs
    local fecType

    clear_all(devNum,portNum)
    set_start_time(portNum)

    fecType=getPortFecType(devNum, portNum)
-- to clear the counters
    if  "CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E"==fecType or "CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E"==fecType then
	ret,val_status = myGenWrapper("cpssDxChRsFecStatusGet",{
			{"IN","GT_U8","devNum",devNum},
			{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
			{"OUT","CPSS_DXCH_RSFEC_STATUS_STC","rsfecStatusPtr"}
			})
	if ret ~= 0 then
		command_data:setFailPortStatus()
		command_data:addError("Port RsFec status cannot be read " ..
							"device %d port %d.",
							devNum, portNum)
        else
	    ret,val_counter_rs = myGenWrapper("cpssDxChRsFecCounterGet",{
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"OUT","CPSS_RSFEC_COUNTERS_STC","rsfecCountersPtr"}
				})
	    if ret ~= 0 then
		command_data:setFailPortStatus()
		command_data:addError("Port RsFec status cannot be read " ..
							"device %d port %d.",
							devNum, portNum)
	    end
	end
    end
    return 0
end


local function getLaneFecSer(start_time, read_time, laneSpeed, symbolErr)
    local ser, speedbits, elapsed_time, bitcnt

    if start_time > read_time then
        print("Error in reading time")
        return 1
    end
    elapsed_time = read_time - start_time
    ser = (10 * symbolErr)/ (laneSpeed * elapsed_time * 1000000)
    return ser
end

local function GetLaneSpeedToVLMultiplier(laneSpeed)
    if  laneSpeed == "_26_5625G_PAM4" or
        laneSpeed == "_28_125G_PAM4"  or
        laneSpeed == "_26_5625G_PAM4_SR_LR"  then
        return "PAM4"
    else
        return "NRZ"
    end
end

-- ************************************************************************

local function fecSerPrintResult(command_data,devNum,portNum,laneNum,fecType,uncorrected
							,corrected,symbolErr,singlePort,readTimePrev)


    local start_time = get_start_time(portNum)
    local read_time  = get_read_time(portNum)
    local totalerror = get_error_cntr(portNum, laneNum)
	local fecTypeStr
    local ser, totalSer
	local port_speed, port_speed_string
	local lane_speed, laneNo_per_port = 0

	ret, values = cpssPerPortParamGet("cpssDxChPortSpeedGet",
                                      devNum, portNum, "speed",
                                      "CPSS_PORT_SPEED_ENT")
	if 0 == ret then
		port_speed = values["speed"]
		port_speed_string,port_speed  = speedStrGet(port_speed)
	elseif 4 == result and portNum == 63 then
		-- skip CPU port
		return 0
	elseif 0x10 == result then
		command_data:setFailPortStatus()
		command_data:addWarning("Port interface mode getting is " ..
								"not allowed in device %d port %d.",
								devNum, portNum)
		return 0
	else
		return 0
	end

	command_data:updateStatus()
	if fecType == "CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E" then
		fecTypeStr =  string.format("RS_FEC544")
	elseif fecType == "CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E" then
		fecTypeStr =  string.format("RS_FEC   ")
	elseif fecType == "CPSS_DXCH_PORT_FEC_MODE_ENABLED_E" then
		fecTypeStr =  string.format("FC_FEC   ")
	else
		fecTypeStr =  string.format("FEC_DIS  ")
		corrected = "N/A"
		uncorrected = "N/A"
		symbolErr = "N/A"
		totalerror = "N/A"
		ser = "N/A"
		totalSer  = "N/A"
		if singlePort == "ser" then
			command_data["result"] = string.format("  Lane %-3s: ",laneNum).."  "
				 ..string.format(" Start time  :%-15s"," N/A").." "
				 ..string.format(" Total errors :%-12s ",totalerror).. " "
				 ..string.format("Total SER :%-28s ",totalSer).. "\n"
				 .."               "
				 ..string.format("Time Period :%-15s "," N/A").. " "
				 ..string.format("Errors       :%-12s ",symbolErr).. " "
				 ..string.format("SER       :%-28s \n",ser)
		else
			command_data["result"] =  string.format("%s/%-8s",tostring(devNum),tostring(portNum)).."|  "
				..string.format("%-3s",laneNum).." | "
				..fecTypeStr.." | "
				..string.format("%-11s",corrected).."  | "
				..string.format("%-11s",uncorrected).."  | "
				..string.format("%-15s",symbolErr).." | "
				..string.format("%-25s ",ser).."|"
		end
	end

	if "CPSS_DXCH_PORT_FEC_MODE_DISABLED_E" ~= fecType then
		ret,laneNo_per_port=GetNuberSerdesOfPort(devNum,portNum)
		lane_speed = port_speed / laneNo_per_port

		ser      = getLaneFecSer(readTimePrev,read_time,lane_speed,symbolErr)
		totalSer = getLaneFecSer(start_time,read_time,lane_speed,totalerror)
		if singlePort == "ser" then
			command_data["result"] = string.format("  Lane %-3s: ",laneNum).."  "
				 ..string.format(" Start time  :%-15s",start_time).." "
				 ..string.format(" Total errors :%-12s ",totalerror).. " "
				 ..string.format("Total SER :%-28e ",totalSer).. "\n"
				 .."               "
				 ..string.format("Time Period :%-15s ",(read_time-readTimePrev)).. " "
				 ..string.format("Errors       :%-12s ",symbolErr).. " "
				 ..string.format("SER       :%-28e \n",ser)
		else
			if laneNum == 0 then
				command_data["result"] =  string.format("%s/%-8s",tostring(devNum),tostring(portNum)).."|  "
					..string.format("%-3s",laneNum).." | "
					..fecTypeStr.." | "
					..string.format("%-11s",corrected).."  | "
					..string.format("%-11s",uncorrected).."  | "
					..string.format("%-15s",symbolErr).." | "
					..string.format("%-25e ",ser).."|"
			else
				command_data["result"] =  string.format("%s/%-8s",tostring(devNum),tostring(portNum)).."|  "
					..string.format("%-3s",laneNum).." | "
					..fecTypeStr.." | "
					..string.format("%-11s","  |        ").."  | "
					..string.format("%-11s","  |        ").."  | "
					..string.format("%-15s",symbolErr).." | "
					..string.format("%-25e ",ser).."|"
			end
		end
	end
	command_data:addResultToResultArray()
    return 0
end

-- ************************************************************************

local function fecPrintPort(command_data,devNum,portNum,singlePort)

    local ret,fecType
    local val_status, val_counter_rs, val_counter_fc, uncorrected, corrected, symbolErr
    local readTimePrev
	local laneNum, laneSpeed, multiplier

    fecType=getPortFecType(devNum, portNum)

	if  "CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E"==fecType or "CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E"==fecType then
		ret,val_status = myGenWrapper("cpssDxChRsFecStatusGet",{
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"OUT","CPSS_DXCH_RSFEC_STATUS_STC","rsfecStatusPtr"}
				})
		if ret ~= 0 then
			command_data:setFailPortStatus()
			command_data:addError("Port RsFec status cannot be read " ..
								"device %d port %d.",
								devNum, portNum)
		else
			ret,val_counter_rs = myGenWrapper("cpssDxChRsFecCounterGet",{
					{"IN","GT_U8","devNum",devNum},
					{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
					{"OUT","CPSS_RSFEC_COUNTERS_STC","rsfecCountersPtr"}
					})

			if ret == 0 then
				uncorrected = val_counter_rs.rsfecCountersPtr.uncorrectedFecCodeword
				corrected = val_counter_rs.rsfecCountersPtr.correctedFecCodeword
				readTimePrev = get_read_time(portNum)
				set_read_time(portNum)

				ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
				if( ret == 0 ) then
					for i=0,numOfSerdesLanesPtr-1,1 do
						laneNum = i
                        laneSpeed = GetPortLaneSpeed(devNum, portNum, laneNum)
                        encoding = GetLaneSpeedToVLMultiplier(laneSpeed)
                        if encoding == "PAM4" then
						    symbolErr = val_status.rsfecStatusPtr.symbolError[laneNum*2]
                                     + (val_status.rsfecStatusPtr.symbolError[(laneNum*2)+1])
                        elseif encoding == "NRZ" then
						    symbolErr = val_status.rsfecStatusPtr.symbolError[laneNum]
                        else
                        end
						set_error_cntr(portNum,laneNum,symbolErr)
						ret = fecSerPrintResult(command_data,devNum,portNum,laneNum,fecType,uncorrected
													,corrected,symbolErr,singlePort,readTimePrev)
						if ret ~= 0 then
							command_data:setFailPortAndLocalStatus()
							command_data:addError("Error Printing data: device %d " ..
												  "port %d: lane %d: %s", devNum, portNum, laneNum,
												  returnCodes[ret])
						else
						end
					end
				end
			else
				command_data:setFailPortStatus()
				command_data:addError("Port RsFec counters cannot be read " ..
									"device %d port %d.",
									devNum, portNum)
			end
		end
	elseif "CPSS_DXCH_PORT_FEC_MODE_ENABLED_E"==fecType then
		--[[
		ret,val_counter_fc = myGenWrapper("cpssDxChFcFecCounterGet",{
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"OUT","CPSS_FCFEC_COUNTERS_STC","fcfecCountersPtr"}
				})
		--]]
		uncorrected = 0
		corrected = 0
		symbolErr = 0

		readTimePrev = get_read_time(portNum)
		set_read_time(portNum)
		ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
		if( ret == 0 ) then
			for i=0,numOfSerdesLanesPtr-1,1 do
				laneNum = i
				--Fec error not working for FC_FEC
				--symbolErr = val_status.rsfecStatusPtr.symbolError[laneNum*2] + (val_status.rsfecStatusPtr.symbolError[(laneNum*2)+1] * 65536)
				set_error_cntr(portNum,laneNum,symbolErr)

				ret = fecSerPrintResult(command_data,devNum,portNum,laneNum,fecType,uncorrected
											,corrected,symbolErr,singlePort,readTimePrev)
				if ret ~= 0 then
					command_data:setFailPortAndLocalStatus()
					command_data:addError("Error Printing data: device %d " ..
										  "port %d: lane %d: %s", devNum, portNum, laneNum,
										  returnCodes[ret])
				else
				end
			end
		end
	else
		uncorrected = 0
		corrected = 0
		symbolErr = 0
		readTimePrev = get_read_time(portNum)
		set_read_time(portNum)
		ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
		if( ret == 0 ) then
			for i=0,numOfSerdesLanesPtr-1,1 do
				laneNum = i
				set_error_cntr(portNum,laneNum,symbolErr)
				ret = fecSerPrintResult(command_data,devNum,portNum,laneNum,fecType,uncorrected
											,corrected,symbolErr,singlePort,readTimePrev)
				if ret ~= 0 then
					command_data:setFailPortAndLocalStatus()
					command_data:addError("Error Printing data: device %d " ..
										  "port %d: lane %d: %s", devNum, portNum, laneNum,
										  returnCodes[ret])
				else
				end
			end
		end
	end

    return ret
end

function fecCommonDevPortLoop(params)

    local portNum,devNum, devPort
    local all_ports, dev_ports

    local ret=0
    local result
    local state=""
    local numOfSerdesLanesPtr=0
    local header_string=""
    local footer_string=""

    local command_data = Command_Data()
    local firstPort = 0, singlePort
    local dev
    setGlobal("ifType", "ethernet")
    setGlobal("ifRange", params["devPort"])

    command_data:initInterfaceDevPortRange()
    command_data:clearResultArray()

    state=params.state

    if params["ser"] ~= nil then
        dev,firstPort = command_data:getFirstPort()
        singlePort = true
    else
        singlePort = false
    end


    if state == "show" and params["ser"] == nil then
        header_string =
           "\n" ..
           "Interface | Lane | Code      | Corrected    | Uncorrected  | Symbol Errors  | SER                       |\n" ..
           "----------+------+-----------+--------------+--------------+----------------+---------------------------+\n"
        footer_string = "\n"

		command_data:enablePausedPrinting()
		command_data:setEmergencyPrintingHeaderAndFooter(header_string,
														 footer_string)

		if true == command_data["status"] then
			all_ports = getGlobal("ifRange")--get table of ports
			for iterator, devNum, portNum in command_data:getPortIterator() do
				ret=fecPrintPort(command_data,devNum,portNum,laneNum, params["ser"])
			end
		end

		command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"], footer_string)
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

	elseif state == "show" and params["ser"] ~= nil then

		if true == command_data["status"] then
			all_ports = getGlobal("ifRange")--get table of ports
			for iterator, devNum, portNum in command_data:getPortIterator() do
				command_data["result"] = string.format("Interface %s/%s:",tostring(devNum),tostring(portNum)).."\n"
				command_data:addResultToResultArray()
				ret=fecPrintPort(command_data,devNum,portNum, params["ser"])
			end
		end

		command_data:setResultArrayToResultStr()
		command_data:setResultStr(header_string, command_data["result"], footer_string)
		command_data:analyzeCommandExecution()
		command_data:printCommandExecutionResults()

	elseif state == "clear" then
        for iterator, devNum, portNum in command_data:getPortIterator() do
			ret = fecSerClear(devNum,portNum)
        end
		command_data:addResultToResultArray()
		command_data:setResultArrayToResultStr()
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
	else

    end

    return ret
end

local function fecShowHandler(params)
    params.state="show"
    return fecCommonDevPortLoop(params)
end

local function fecClearHandler(params)
    params.state="clear"
    return fecCommonDevPortLoop(params)
end



--------------------------------------------------------------------------------
-- command registration: fec show
--------------------------------------------------------------------------------

CLI_addHelp("debug", "link fec", "Link FEC and SER info")
CLI_addHelp("debug", "link fec clear", "Clear FEC Status and SER counters")
CLI_addCommand("debug", "link fec clear interface", {
    func=fecClearHandler,
    help="Clear FEC Status and SER counters",
    params={
        {   type="named",
            { format=" ethernet %port-range", name="devPort", help="Ethernet interface to show" },
            mandatory={"devPort"}
        }
    }
})


CLI_addHelp("debug", "link fec show", "Show FEC Status and SER counters")
CLI_addCommand("debug", "link fec show interface", {
  func   = fecShowHandler,
  help   = "Show FEC Status and SER counters",
  params={
        {   type="named",
			{ format=" ethernet %port-range", name="devPort", help="Ethernet interface to show" },
			{ format=" ser", name="ser", help="ser per port for all Lanes" },
            mandatory={"devPort"}
        }
   }
})
--------------------------------------------------------------------------------

