--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_tx.lua
--*
--* DESCRIPTION:
--*       display the serdes tx configuration
--*
--*
--********************************************************************************



--constants



-- ************************************************************************
---
--  show_interfaces_serdes_tx_port_range
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

local function show_interfaces_serdes_tx_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, footer_string1
    local ret, localSerdesTxPtr
    
    -- Command specific variables initialization.   
    if isSipVersion(devNum , "SIP_5_15") then
          header_string1 =
            "\n" ..
            "Serdes TX Config\n" ..
            "----------------\n" ..
            "Interface LaneNum    Atten    post       pre       pre2       pre3\n" ..
            "--------- ------- ---------- -------- ---------- --------- ----------\n"
    else
          header_string1 = 
            "\n" ..
            "Serdes TX Config\n" ..
            "----------------\n" ..
            "Interface LaneNum    Amp     AmpAdjEn   emph0      emph1    TxAmpShift\n" ..
            "--------- ------- ---------- -------- ---------- ---------- ----------\n"
    end
    footer_string1 = "\n"  
    
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
            
            ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
      local ifMode=localSerdesTxPtr["ifModePtr"] 
      
      if(ret~=GT_OK) then
          command_data:addError("Error at Serdes Tx command")
      end
    
      ret, localSerdesTxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
        {"OUT","GT_U32","startSerdesPtr"},
        {"OUT","GT_U32","numOfSerdesLanesPtr"}})
      local startSerdesPtr=localSerdesTxPtr["startSerdesPtr"]
      local numOfSerdesLanesPtr=localSerdesTxPtr["numOfSerdesLanesPtr"]

      if(ret~=GT_OK) then
          command_data:addError("Error at Serdes Tx command")
      end
    
      for i=0,numOfSerdesLanesPtr-1,1 do
        local laneNum=i
        ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortSerdesManualTxConfigGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_U32","laneNum",laneNum},
          {"OUT","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr"}})
        
        -- Traffic statistic getting.
        command_data:clearLocalStatus()

        command_data:updateStatus()    
          
        if(ret~=GT_OK) then
          command_data:addError("Error at Serdes Tx command")
        end
        if isSipVersion(devNum , "SIP_5_15") then
            local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["avago"]
            local atten=OutValues.atten
            local post=OutValues.post
            local pre=OutValues.pre
            local pre2=OutValues.pre2
            local pre3=OutValues.pre3

            atten=to_string(atten)
            post=to_string(post)
            pre=to_string(pre)
            pre2=to_string(pre2)
            pre3=to_string(pre3)

            laneNum=to_string(i)

            command_data["result"] =
               string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                  devnum_portnum_string,
                  alignLeftToCenterStr(laneNum, 7),
                  alignLeftToCenterStr(atten, 10),
                  alignLeftToCenterStr(post, 10),
                  alignLeftToCenterStr(pre, 10),
                  alignLeftToCenterStr(pre2, 10),
                  alignLeftToCenterStr(pre3, 10))
        else
            local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["comphy"]
            local txAmp=OutValues.txAmp
            local txAmpAdjEn=OutValues.txAmpAdjEn
            local emph0=OutValues.emph0
            local emph1=OutValues.emph1
            local txAmpShft=OutValues.txAmpShft
            txAmp=to_string(txAmp)
            txAmpAdjEn=to_string(txAmpAdjEn)
            emph0=to_string(emph0)
            emph1=to_string(emph1)
            txAmpShft=to_string(txAmpShft)
            laneNum=to_string(i)

            command_data["result"] =
               string.format("%-10s%-8s%-11s%-9s%-11s%-11s%-11s",
                  devnum_portnum_string,
                  alignLeftToCenterStr(laneNum, 7),
                  alignLeftToCenterStr(txAmp, 10), 
                  alignLeftToCenterStr(txAmpAdjEn, 8),
                  alignLeftToCenterStr(emph0, 10),
                  alignLeftToCenterStr(emph1, 10),
                  alignLeftToCenterStr(txAmpShft, 10))
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
--  show_interfaces_serdes_tx_single_port
--        @description  shows interfaces_serdes_tx with single port 
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
local function show_interfaces_serdes_tx_single_port(params, command_data)
  -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    
    local ret, localSerdesTxPtr
  
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

    -- Traffic statistic getting.    
            

  ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
  local ifMode=localSerdesTxPtr["ifModePtr"] 
  if(ret~=GT_OK) then
      command_data:addError("Error at Serdes Tx command")
  end
  ret, localSerdesTxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
      {"OUT","GT_U32","startSerdesPtr"},
      {"OUT","GT_U32","numOfSerdesLanesPtr"}})
  if(ret~=GT_OK) then
      command_data:addError("Error at Serdes Tx command")
  end
  local startSerdesPtr=localSerdesTxPtr["startSerdesPtr"]
  local numOfSerdesLanesPtr=localSerdesTxPtr["numOfSerdesLanesPtr"]    
    -- Traffic statistic strings formatting and adding. 
  local laneNum=params.laneNum
  -- Display Serdes Tx.
  if(laneNum~=nil or numOfSerdesLanesPtr==1) then
    ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortSerdesManualTxConfigGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_U32","laneNum",laneNum},
        {"OUT","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr"}})
    if(ret~=GT_OK) then
      command_data:addError("Error at Serdes Tx command")
    end

    print("")
    print("Serdes TX Config")
    print("----------------")

    if isSipVersion(devNum , "SIP_5_15") then
        local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["avago"]
        local atten = OutValues.atten
        local post = OutValues.post
        local pre = OutValues.pre
        local pre2 = OutValues.pre2
        local pre3 = OutValues.pre3
        print("Tx Attenuation:              ",atten)
        print("Post Cursor:                 ",post)
        print("Pre  Cursor:                 ",pre)
        print("Pre  Cursor2:                ",pre2)
        print("Pre  Cursor3:                ",pre3)
    else
        local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["comphy"]
        local txAmp = OutValues.txAmp
        local txAmpAdjEn = OutValues.txAmpAdjEn
        local emph0 = OutValues.emph0
        local emph1 = OutValues.emph1
        local txAmpShft = OutValues.txAmpShft
        print("Amplitude:\t\t\t",txAmp)
        print("Amplitude Adjust Enable:\t",txAmpAdjEn)
        print("Emphasis for Gen0 bit rates:\t",emph0)
        print("Emphasis for Gen1 bit rates:\t",emph1)
        print("Transmitter Amplitude Shift:\t",txAmpShft)
    end

    -- Resulting string formatting.
    command_data:setResultArrayToResultStr()      
      
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
  else
    ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    ifMode=localSerdesTxPtr["ifModePtr"] 
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Tx command")
    end
    ret, localSerdesTxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
        {"OUT","GT_U32","startSerdesPtr"},
        {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Tx command")
    end
    startSerdesPtr=localSerdesTxPtr["startSerdesPtr"]
    numOfSerdesLanesPtr=localSerdesTxPtr["numOfSerdesLanesPtr"]

    local header_string1
    if isSipVersion(devNum , "SIP_5_15") then
       header_string1 = 
        "\n" ..
        "Serdes TX Config\n" ..
        "----------------\n" ..
        "Interface LaneNum    Atten      Post      pre      pre2     pre3\n" ..
        "--------- ------- ---------- -------- ---------- -------- -----------\n"
    else
        header_string1 = 
        "\n" ..
        "Serdes TX Config\n" ..
        "----------------\n" ..
        "Interface LaneNum    Amp     AmpAdjEn   emph0      emph1    TxAmpShift\n" ..
        "--------- ------- ---------- -------- ---------- ---------- ----------\n"
   end
    local footer_string1 = "\n"  
    
    for i=0,numOfSerdesLanesPtr-1,1 do
	  local laneNum = i
      ret, localSerdesTxPtr = myGenWrapper("cpssDxChPortSerdesManualTxConfigGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_U32","laneNum",laneNum},
        {"OUT","CPSS_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr"}})
      
            -- Traffic statistic getting.
            command_data:clearLocalStatus()

            command_data:updateStatus()    
        
      if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Tx command")
      end

      if isSipVersion(devNum , "SIP_5_15") then
          local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["avago"]
          local atten=OutValues.atten
          local post=OutValues.post
          local pre=OutValues.pre
          local pre2=OutValues.pre2
          local pre3=OutValues.pre3
          atten=to_string(atten)
          post=to_string(post)
          pre=to_string(pre)
          pre2=to_string(pre2)
          pre3=to_string(pre3)
          laneNum=to_string(laneNum)
          command_data["result"] =
               string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                  devnum_portnum_string,
                  alignLeftToCenterStr(laneNum, 7),
                  alignLeftToCenterStr(atten, 10),
                  alignLeftToCenterStr(post, 10),
                  alignLeftToCenterStr(pre, 10),
                  alignLeftToCenterStr(pre2, 10),
                  alignLeftToCenterStr(pre3, 10))
      else
          local OutValues=localSerdesTxPtr["serdesTxCfgPtr"]["txTune"]["comphy"]
          local txAmp=OutValues.txAmp
          local txAmpAdjEn=OutValues.txAmpAdjEn
          local emph0=OutValues.emph0
          local emph1=OutValues.emph1
          local txAmpShft=OutValues.txAmpShft
          txAmp=to_string(txAmp)
          txAmpAdjEn=to_string(txAmpAdjEn)
          emph0=to_string(emph0)
          emph1=to_string(emph1)
          txAmpShft=to_string(txAmpShft)
          laneNum=to_string(i)

          command_data["result"] =
               string.format("%-10s%-8s%-11s%-9s%-11s%-11s%-11s",
                  devnum_portnum_string,
                  alignLeftToCenterStr(laneNum, 7),
                  alignLeftToCenterStr(txAmp, 10),
                  alignLeftToCenterStr(txAmpAdjEn, 8),
                  alignLeftToCenterStr(emph0, 10),
                  alignLeftToCenterStr(emph1, 10),
                  alignLeftToCenterStr(txAmpShft, 10))
      end
      command_data:addResultToResultArray()
      command_data:updatePorts()
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
--  show_interfaces_serdes_tx
--        @description  show interfaces serdes tx
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
local function show_interfaces_serdes_tx(params)
    local result, values
    -- Common variables declaration
    local command_data = Command_Data() 
    
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
        result, values = show_interfaces_serdes_tx_single_port(params, command_data)
    else
        result, values = show_interfaces_serdes_tx_port_range(params, command_data)
    end
    
    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces serdes tx
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes tx", {
    func = show_interfaces_serdes_tx,
    help = "Interface(s) serdes tx",
    params = {
    {  type= "named",   {format="lane_num %lane_number_type", name="laneNum", help="Serdes lane number"}, 
              "#all_interfaces",     
              mandatory={"all_interfaces"}
    }
  }  
})
    
