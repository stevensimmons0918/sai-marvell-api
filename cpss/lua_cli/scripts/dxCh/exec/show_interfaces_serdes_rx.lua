--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_rx.lua
--*
--* DESCRIPTION:
--*       display the serdes rx configuration
--*
--*
--********************************************************************************



--constants



-- ************************************************************************
---
--  show_interfaces_serdes_rx_port_range
--        @description  shows statistics on traffic seen by the physical 
--                      interface with many ports
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--                                params["mac-address"]: mac-address, could 
--                                be irrelevant;
--                                params["ethernet"]: interface range, 
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be 
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be 
--                                irrelevant;
--                                params["skipped"]: skipping property, 
--                                could be irrelevant 
--        @param command_data   - command execution data object                     
--
--        @return        true on success, otherwise false and error message
--

local function show_interfaces_serdes_rx_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, footer_string1
    
    local ret, localSerdesRxPtr
    local ifMode
  
    -- Command specific variables initialization.
    if isSipVersion(devNum , "SIP_5_15") then
        header_string1 = 
            "\n" ..
            "Serdes Rx Config\n" ..
            "----------------\n" ..
            "Interface LaneNum  SqThresh   LowFreq    HighFreq   bandWidth    dcGain     \n" ..
            "--------- ------- ---------- ---------- ---------- ----------- ----------   \n"
    else
        header_string1 = 
            "\n" ..
            "Serdes Rx Config\n" ..
            "----------------\n" ..
            "Interface LaneNum  SqThresh   FFE Res    FFE Cap    Align 90 \n" ..
            "--------- ------- ---------- ---------- ---------- ----------\n"
            footer_string1 = "\n" 
    end 
    
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()     

            if portNum == "CPU" then portNum = 63 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                    '/' .. 
                    tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            -- Get ifMode
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
            ifMode=localSerdesRxPtr["ifModePtr"] 
			
            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end
            -- Get laneNum
            ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                    {"OUT","GT_U32","startSerdesPtr"},
                    {"OUT","GT_U32","numOfSerdesLanesPtr"}})
            local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
            local  numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end
			
            for i=0,numOfSerdesLanesPtr-1,1 do
                local laneNum = i
                ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U32","laneNum",laneNum},
                        {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})
				
                -- Traffic statistic getting.
                command_data:clearLocalStatus()

                command_data:updateStatus()    
					
                if(ret~=0) then
                    --command_data:addError("Error at Serdes Rx command")
                    print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
                else
                    if isSipVersion(devNum , "SIP_5_15") then
                        local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                        local sqlch = OutValues.sqlch
                        local lf = OutValues.LF
                        local hf = OutValues.HF
                        local bw = OutValues.BW
                        local dc = OutValues.DC


                        sqlch=to_string(sqlch)
                        lf=to_string(lf)
                        hf=to_string(hf)
                        bw=to_string(bw)
                        dc=to_string(dc)

                        laneNum=to_string(i)

                        command_data["result"] =
                                string.format("%-10s%-8s%-11s%-11s%-11s%-12s%-11s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(sqlch, 10),
                                alignLeftToCenterStr(lf, 10),
                                alignLeftToCenterStr(hf, 10),
                                alignLeftToCenterStr(bw, 10),
                                alignLeftToCenterStr(dc, 10))
                    else
                        local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["comphy"]
                        local sqlch=OutValues.sqlch
                        local ffeRes=OutValues.ffeRes
                        local ffeCap=OutValues.ffeCap
                        local align90=OutValues.align90

                        sqlch=to_string(sqlch)
                        ffeRes=to_string(ffeRes)
                        ffeCap=to_string(ffeCap)
                        align90=to_string(align90)
                        laneNum=to_string(i)

                        command_data["result"] =
                                string.format("%-10s%-8s%-11s%-11s%-11s%-11s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(sqlch, 10),
                                alignLeftToCenterStr(ffeRes, 10),
                                alignLeftToCenterStr(ffeCap, 10),
                                alignLeftToCenterStr(align90, 10))
                    end
                    command_data:addResultToResultArray()
                    command_data:updatePorts()
                end
            end
        end
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 1)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"],
            footer_string1,
            "There is no port information to show.\n")
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    end
    return command_data:getCommandExecutionResults()
end   



-- ************************************************************************
---
--  show_interfaces_serdes_rx_single_port
--        @description  shows interfaces_serdes_rx with single port 
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_serdes_rx_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    
    
    local ret, localSerdesRxPtr
    
    
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    if portNum == "CPU" then portNum = 63 end
        -- DevNum/PortNum string forming.
        devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                '/' .. 
                tostring(portNum), 9)
    local GT_OK = 0
    -- Command specific variables initialization.    

    command_data:clearLocalStatus()

	
    ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    local ifMode=localSerdesRxPtr["ifModePtr"] 
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Rx command")
    end
	
    ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
            {"OUT","GT_U32","startSerdesPtr"},
            {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
    local numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
    -- Traffic statistic getting.    
            

    -- Traffic statistic strings formatting and adding. 
    local laneNum=params.laneNum
    -- Display Serdes Rx 
    if(laneNum~=nil or numOfSerdesLanesPtr==1) then
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})

        if(ret~=GT_OK) then
            --command_data:addError("Error at Serdes Rx command")
            print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
        else

            if isSipVersion(devNum , "SIP_5_15") then
                local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                local sqlch = OutValues.sqlch
                local ffeRes = OutValues.LF
                local ffeCap = OutValues.HF
                local bandWidth = OutValues.BW
                local dcGain = OutValues.DC

                print("")
                print("Serdes RX Config")
                print("----------------")
                print("SQ Threshold:     ",sqlch)
                print("FFE Res:          ",ffeRes)
                print("FFE Cap:          ",ffeCap)
                print("bandWidth:        ",bandWidth)
                print("dcGain:           ",dcGain)
            else
                local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["comphy"]
                local sqlch=OutValues.sqlch
                local ffeRes=OutValues.ffeR
                local ffeCap=OutValues.ffeC
                local align90=OutValues.align90
			
            print("")
            print("Serdes RX Config")
            print("----------------")
            print("SQ Threshold:\t",sqlch)
            print("FFE Res:\t",ffeRes)
            print("FFE Cap:\t",ffeCap)
            print("Align 90:\t",align90)
			
            -- Resulting string formatting.
            command_data:setResultArrayToResultStr()      
            
            command_data:analyzeCommandExecution()
			   
                command_data:printCommandExecutionResults()
            end
        end


    else
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
        ifMode=localSerdesRxPtr["ifModePtr"] 
        if(ret~=GT_OK) then
             command_data:addError("Error at Serdes Rx command")
        end
		
        ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})
        startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
        numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
        if(ret~=GT_OK) then
            command_data:addError("Error at Serdes Rx command")
        end
        local header_string1

        if isSipVersion(devNum , "SIP_5_15") then
            header_string1 =
                    "\n" ..
                    "Serdes Rx Config\n" ..
                    "----------------\n" ..
                    "Interface LaneNum  SqThresh    LowFreq   HighFreq   bandWidth    dcGain     \n" ..
                    "--------- ------- ---------- ---------- ---------- ----------- ----------   \n"
        else
            header_string1 =
                    "\n" ..
                    "Serdes Rx Config\n" ..
                    "----------------\n" ..
                    "Interface LaneNum  SqThresh   FFE Res    FFE Cap    Align 90 \n" ..
                    "--------- ------- ---------- ---------- ---------- ----------\n"
        end
        local  footer_string1 = "\n"  
		
        for i=0,numOfSerdesLanesPtr-1,1 do
            local laneNum = i
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})
			
            -- Traffic statistic getting.
            command_data:clearLocalStatus()

            command_data:updateStatus()    
				
            if(ret~=GT_OK) then
                --command_data:addError("Error at Serdes Rx command")
                print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
            else
                if isSipVersion(devNum , "SIP_5_15") then
                    local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                    local sqlch = OutValues.sqlch
                    local lf = OutValues.LF
                    local hf = OutValues.HF
                    local bw = OutValues.BW
                    local dc = OutValues.DC


                    sqlch=to_string(sqlch)
                    lf=to_string(lf)
                    hf=to_string(hf)
                    bw=to_string(bw)
                    dc=to_string(dc)

                    laneNum=to_string(i)

                    command_data["result"] =
                            string.format("%-10s%-8s%-11s%-11s%-11s%-12s%-11s",
                            devnum_portnum_string,
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10),
                            alignLeftToCenterStr(lf, 10),
                            alignLeftToCenterStr(hf, 10),
                            alignLeftToCenterStr(bw, 10),
                            alignLeftToCenterStr(dc, 10))
                else
                    local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["comphy"]
                    local sqlch=OutValues.sqlch					--sqlch
                    local ffeRes=OutValues.ffeRes					--ffeRes
                    local ffeCap=OutValues.ffeCap					--ffeCap
                    local align90=OutValues.align90			   	        --align90
					
                    sqlch=to_string(sqlch)
                    ffeRes=to_string(ffeRes)
                    ffeCap=to_string(ffeCap)
                    align90=to_string(align90)
                    laneNum=to_string(i)
					
                    command_data["result"] =
                            string.format("%-10s%-8s%-11s%-11s%-11s%-11s",
                            devnum_portnum_string,
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10),
                            alignLeftToCenterStr(ffeRes, 10),
                            alignLeftToCenterStr(ffeCap, 10),
                            alignLeftToCenterStr(align90, 10))
                end
                command_data:addResultToResultArray()

						
                command_data:updatePorts()
            end

			
        end
        -- Resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 1)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
												footer_string1,
												"There is no port information to show.\n")
        command_data:analyzeCommandExecution()							
        command_data:printCommandExecutionResults()
    end
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  show_interfaces_serdes_rx
--        @description  show interfaces serdes rx
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_serdes_rx(params) 
    -- Common variables declaration
    local command_data = Command_Data() 
    
    local result, values
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)    
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()       
    
    if true == command_data:isOnePortInPortRange() then
        result, values = show_interfaces_serdes_rx_single_port(params, command_data)
    else
        result, values = show_interfaces_serdes_rx_port_range(params, command_data)
    end
    
    return result, values
end

local function show_interfaces_serdes_rx_ctle_dfe_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, footer_string1

    local ret, localSerdesRxPtr
    local ifMode

    -- Command specific variables initialization.
    if isSipVersion(devNum , "SIP_6") then
       if params.ctle~=nil then
        header_string1 =
            "\n" ..
            "Serdes Rx CTLE Config\n" ..
            "----------------\n" ..
            "Interface LaneNum  Gain-Shape1  Gain-Shape2   MinLF  MaxLf   MinLf   MaxHf   \n" ..
            "--------- ------- ------------ ------------ ------- ------- ------- -------  \n"
       else
        header_string1 =
            "\n" ..
            "Serdes Rx DFE Config\n" ..
            "--------------------\n" ..
            "Interface  LaneNo  Bflf   Bfhf  Pre1  MinPre1 MaxPre1  Pre2  MinPre2 MaxPre2  Post1 MinPost MaxPost Gain1  Gain2 \n" ..
            "---------- ------ ------ ---- ------- ------- ------ ------- -------- ----- ------- ------- ------ -----  -----\n"
       end
    end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            if portNum == "CPU" then portNum = 63 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                    '/' ..
                    tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            -- Get ifMode
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
            ifMode=localSerdesRxPtr["ifModePtr"]

            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end
            -- Get laneNum
            ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                    {"OUT","GT_U32","startSerdesPtr"},
                    {"OUT","GT_U32","numOfSerdesLanesPtr"}})
            local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
            local  numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end

            for i=0,numOfSerdesLanesPtr-1,1 do
                local laneNum = i
                ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U32","laneNum",laneNum},
                        {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})

                -- Traffic statistic getting.
                command_data:clearLocalStatus()

                command_data:updateStatus()

                if(ret~=0) then
                    --command_data:addError("Error at Serdes Rx command")
                    print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
                else
                    if isSipVersion(devNum , "SIP_6") then
                        local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                        if params.ctle~=nil then
                            local gain1 = OutValues.gainshape1
                            local gain2 = OutValues.gainshape2
                            local minLf = OutValues.minLf
                            local maxLf = OutValues.maxLf
                            local minHf = OutValues.minHf
                            local maxHf = OutValues.maxHf

                            gain1=to_string(gain1)
                            gain2=to_string(gain2)
                            minLf=to_string(minLf)
                            maxLf=to_string(maxLf)
                            minHf=to_string(minHf)
                            maxHf=to_string(maxHf)

                            laneNum=to_string(i)
                            command_data["result"] =
                                string.format("%-10s%-8s%-12s%-12s%-8s%-8s%-8s%-8s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(gain1, 9),
                                alignLeftToCenterStr(gain2, 13),
                                alignLeftToCenterStr(minLf, 8),
                                alignLeftToCenterStr(maxLf, 8),
                                alignLeftToCenterStr(minHf, 7),
                                alignLeftToCenterStr(maxHf, 8))


                        else
                            local bflf = OutValues.BFLF
                            local bfhf = OutValues.BFHF
                            local minPost = OutValues.minPost
                            local maxPost = OutValues.maxPost
                            local minPre1 = OutValues.minPre1
                            local maxPre1 = OutValues.maxPre1
                            local minPre2 = OutValues.minPre2
                            local maxPre2 = OutValues.maxPre2
                            local dfegain1 = OutValues.dfeGAIN
                            local dfegain2 = OutValues.dfeGAIN2
                            local pre1    = OutValues.pre1
                            local pre2    = OutValues.pre2
                            local post1   = OutValues.post1

                            bflf=to_string(bflf)
                            bfhf=to_string(bfhf)
                            minPost=to_string(minPost)
                            maxPost=to_string(maxPost)
                            minPre1=to_string(minPre1)
                            maxPre1=to_string(maxPre1)
                            minPre2=to_string(minPre2)
                            maxPre2=to_string(maxPre2)
                            dfegain1=to_string(dfegain1)
                            dfegain2=to_string(dfegain2)
                            pre1=to_string(pre1)
                            pre2=to_string(pre2)
                            post1=to_string(post1)

                            laneNum=to_string(i)
                            command_data["result"] =
                                string.format("%-10s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(bflf, 7),
                                alignLeftToCenterStr(bfhf, 7),
                                alignLeftToCenterStr(pre1, 7),
                                alignLeftToCenterStr(minPre1, 7),
                                alignLeftToCenterStr(maxPre1, 7),
                                alignLeftToCenterStr(pre2, 7),
                                alignLeftToCenterStr(minPre2, 7),
                                alignLeftToCenterStr(maxPre2, 7),
                                alignLeftToCenterStr(post1, 7),
                                alignLeftToCenterStr(minPost, 7),
                                alignLeftToCenterStr(maxPost, 7),
                                alignLeftToCenterStr(dfegain1, 7),
                                alignLeftToCenterStr(dfegain2, 7))

                        end
                    end
                    command_data:addResultToResultArray()
                    command_data:updatePorts()
                end
            end
        end
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 1)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"],
            footer_string1,
            "There is no port information to show.\n")
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    end
    return command_data:getCommandExecutionResults()
end

local function show_interfaces_serdes_rx_ctle_dfe_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string


    local ret, localSerdesRxPtr


    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    if portNum == "CPU" then portNum = 63 end
        -- DevNum/PortNum string forming.
        devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                '/' ..
                tostring(portNum), 9)
    local GT_OK = 0
    -- Command specific variables initialization.

    command_data:clearLocalStatus()


    ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    local ifMode=localSerdesRxPtr["ifModePtr"]
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Rx command")
    end

    ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
            {"OUT","GT_U32","startSerdesPtr"},
            {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
    local numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
    -- Traffic statistic getting.


    -- Traffic statistic strings formatting and adding.
    local laneNum=params.laneNum
    -- Display Serdes Rx
    if(laneNum~=nil or numOfSerdesLanesPtr==1) then
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})

        if(ret~=GT_OK) then
            --command_data:addError("Error at Serdes Rx command")
            print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
        else

            if isSipVersion(devNum , "SIP_6") then
                local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                if params.ctle~=nil then
                    local gain1 = OutValues.gainshape1
                    local gain2 = OutValues.gainshape2
                    local minLf = OutValues.minLf
                    local maxLf = OutValues.maxLf
                    local minHf = OutValues.minHf
                    local maxHf = OutValues.maxHf

                    print("")
                    print("Serdes RX CTLE Config")
                    print("----------------")
                    print("Gain Shape1:     ",gain1)
                    print("Gain Shape2:     ",gain2)
                    print("Min Lf:          ",minLf)
                    print("Max Lf:          ",maxLf)
                    print("Min Hf:          ",minHf)
                    print("Max Hf:          ",maxHf)
                else
                    local bflf = OutValues.BFLF
                    local bfhf = OutValues.BFHF
                    local minPost = OutValues.minPost
                    local maxPost = OutValues.maxPost
                    local minPre1 = OutValues.minPre1
                    local maxPre1 = OutValues.maxPre1
                    local minPre2 = OutValues.minPre2
                    local maxPre2 = OutValues.maxPre2
                    local dfegain1 = OutValues.dfeGAIN
                    local dfegain2 = OutValues.dfeGAIN2
                    local pre1    = OutValues.pre1
                    local pre2    = OutValues.pre2
                    local post1   = OutValues.post1

                    print("")
                    print("Serdes RX DFE Config")
                    print("----------------")
                    print("BFLF:            ",bflf)
                    print("BFHF:            ",bfhf)
                    print("Pre-cursor-1 :   ",pre1)
                    print("Min Pre1:        ",minPre1)
                    print("Max Pre1:        ",maxPre1)
                    print("Pre-cursor-2 :   ",pre2)
                    print("Min Pre2:        ",minPre2)
                    print("Max Pre2:        ",minPre2)
                    print("Post-cursor:     ",post1)
                    print("Min Post:        ",minPost)
                    print("Max Post:        ",maxPost)
                    print("Gain1:           ",dfegain1)
                    print("Gain2:           ",dfegain2)
                end

            -- Resulting string formatting.
            command_data:setResultArrayToResultStr()

            command_data:analyzeCommandExecution()

            command_data:printCommandExecutionResults()
            end
        end


    else
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
        ifMode=localSerdesRxPtr["ifModePtr"]
        if(ret~=GT_OK) then
             command_data:addError("Error at Serdes Rx command")
        end

        ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})
        startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
        numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
        if(ret~=GT_OK) then
            command_data:addError("Error at Serdes Rx command")
        end
        local header_string1

        if isSipVersion(devNum , "SIP_6") then
           if params.ctle~=nil then
               header_string1 =
                "\n" ..
                "Serdes Rx CTLE Config\n" ..
                "----------------\n" ..
                "Interface LaneNum  Gain-Shape1  Gain-Shape2   MinLF  MaxLf   MinLf   MaxHf   \n" ..
                "--------- ------- ------------ ------------ ------- ------- ------- -------  \n"
           else
               header_string1 =
                "\n" ..
                "Serdes Rx DFE Config\n" ..
                "--------------------\n" ..
                "Interface  LaneNo  Bflf   Bfhf  Pre1 MinPre1 MaxPre1  Pre2   MinPre2  MaxPre2  Post1 MinPost MaxPost Gain1  Gain2 \n" ..
                "---------- ------ ------ ---- ------- ------- ------ ------ --------- -------- ----- ------- ------- ------ ------\n"
           end
        end

        local  footer_string1 = "\n"

        for i=0,numOfSerdesLanesPtr-1,1 do
            local laneNum = i
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"OUT","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})

            -- Traffic statistic getting.
            command_data:clearLocalStatus()

            command_data:updateStatus()

            if(ret~=GT_OK) then
                --command_data:addError("Error at Serdes Rx command")
                print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
            else
                if isSipVersion(devNum , "SIP_6") then
                    local OutValues=localSerdesRxPtr["serdesRxCfgPtr"]["rxTune"]["avago"]
                        if params.ctle~=nil then
                            local gain1 = OutValues.gainshape1
                            local gain2 = OutValues.gainshape2
                            local minLf = OutValues.minLf
                            local maxLf = OutValues.maxLf
                            local minHf = OutValues.minHf
                            local maxHf = OutValues.maxHf

                            gain1=to_string(gain1)
                            gain2=to_string(gain2)
                            minLf=to_string(minLf)
                            maxLf=to_string(maxLf)
                            minHf=to_string(minHf)
                            maxHf=to_string(maxHf)

                            laneNum=to_string(i)

                            command_data["result"] =
                                string.format("%-10s%-8s%-13s%-9s%-8s%-8s%-8s%-8s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(gain1, 8),
                                alignLeftToCenterStr(gain2, 8),
                                alignLeftToCenterStr(minLf, 8),
                                alignLeftToCenterStr(maxLf, 8),
                                alignLeftToCenterStr(minHf, 8),
                                alignLeftToCenterStr(maxHf, 8))
                        else
                            local bflf = OutValues.BFLF
                            local bfhf = OutValues.BFHF
                            local minPost = OutValues.minPost
                            local maxPost = OutValues.maxPost
                            local minPre1 = OutValues.minPre1
                            local maxPre1 = OutValues.maxPre1
                            local minPre2 = OutValues.minPre2
                            local maxPre2 = OutValues.maxPre2
                            local dfegain1 = OutValues.dfeGAIN
                            local dfegain2 = OutValues.dfeGAIN2
                            local pre1    = OutValues.pre1
                            local pre2    = OutValues.pre2
                            local post1   = OutValues.post1

                            bflf=to_string(bflf)
                            bfhf=to_string(bfhf)
                            minPost=to_string(minPost)
                            maxPost=to_string(maxPost)
                            minPre1=to_string(minPre1)
                            maxPre1=to_string(maxPre1)
                            minPre2=to_string(minPre2)
                            maxPre2=to_string(maxPre2)
                            dfegain1=to_string(dfegain1)
                            dfegain2=to_string(dfegain2)
                            pre1=to_string(pre1)
                            pre2=to_string(pre2)
                            post1=to_string(post1)

                            laneNum=to_string(i)

                            command_data["result"] =
                                string.format("%-10s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s",
                                devnum_portnum_string,
                                alignLeftToCenterStr(laneNum, 7),
                                alignLeftToCenterStr(bflf, 7),
                                alignLeftToCenterStr(bfhf, 7),
                                alignLeftToCenterStr(pre1, 7),
                                alignLeftToCenterStr(minPre1, 7),
                                alignLeftToCenterStr(maxPre1, 7),
                                alignLeftToCenterStr(pre2, 7),
                                alignLeftToCenterStr(minPre2, 7),
                                alignLeftToCenterStr(maxPre2, 7),
                                alignLeftToCenterStr(post1, 7),
                                alignLeftToCenterStr(minPost, 7),
                                alignLeftToCenterStr(maxPost, 7),
                                alignLeftToCenterStr(dfegain1, 7),
                                alignLeftToCenterStr(dfegain2, 7))
                    end
                end
                command_data:addResultToResultArray()


                command_data:updatePorts()
            end


        end
        -- Resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 1)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"],
        footer_string1,
        "There is no port information to show.\n")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
    end
    return command_data:getCommandExecutionResults()
end

local function show_interfaces_serdes_rx_ctle_dfe(params)
    -- Common variables declaration
    local command_data = Command_Data()

    local result, values

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    if true == command_data:isOnePortInPortRange() then
        result, values = show_interfaces_serdes_rx_ctle_dfe_single_port(params, command_data)
    else
        result, values = show_interfaces_serdes_rx_ctle_dfe_port_range(params, command_data)
    end

    return result, values
end

local function show_interfaces_serdes_rx_main(params)
    local result, values
    local devNum, portNum

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled

    if params.ctle~=nil or params.dfe~=nil then
        if (values.enablePtr) then
            show_interfaces_serdes_rx_ctle_dfe(params)
        end
    else
        show_interfaces_serdes_rx(params)
    end
end
--------------------------------------------------------------------------------
-- command registration: show interfaces serdes rx
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes rx", {
    func = show_interfaces_serdes_rx_main,
    help = "Interface(s) serdes rx",
    params = {
        { type= "named", {format="lane_num %lane_number_type", name="laneNum", help="Serdes lane number"},
                         {format="ctle ", name="ctle", help="Serdes rx ctle parameters"},
                         {format="dfe ", name="dfe", help="Serdes rx dfe parameters"},
            "#all_interfaces",
            mandatory={"all_interfaces"}
        }
    }
})
