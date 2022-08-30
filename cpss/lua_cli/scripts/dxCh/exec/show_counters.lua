--********************************************************************************
--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* counters.lua
--*
--* DESCRIPTION:
--*       Centralized Counters (CNC) commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("luaCLI_convertPhysicalPortsToTxq")
cmdLuaCLI_registerCfunction("wrlDxChSip6PortTxqNumOfQueuesGet")

CLI_type_dict["blockNum_no_all"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    complete = CLI_complete_param_number,
    help = "Enter CNC block number"
}

CLI_type_dict["sip6_queue_id_no_all"] = {
    checker = CLI_check_param_number,
    min=0,
        max=15,
    complete = CLI_complete_param_number,
    help="Traffic Class associated with this set of Drop Parameters"
}

--constants
local function dec2hex(IN)
    local B, K, OUT, I, D = 16, "0123456789ABCDEF", "", 0
    while IN > 0 do
        I = I + 1
        IN, D = math.floor(IN / B), math.mod(IN, B) + 1
        OUT = string.sub(K, D, D) .. OUT
    end
    return OUT
end

local function hex2dec(hex)
    local ans = 0
    local i, d
    d = 0
    for i = string.len(hex), 1, -1 do
        local char = string.sub(hex, i, i)
        local charAsNumber = tonumber(char, 16)
        ans = ans + math.pow(16, d) * charAsNumber
        d = d + 1
    end
    return ans
end

local function dec2bin(dec,length)
    dec = dec * 2
    local bin = ""
    if(length == nil) then length = 8 end
    for i = 0, (length-1) do
        bin = bin .. tostring(math.ceil(math.floor(dec / 2) % 2))
        dec = math.floor(dec / 2)
    end
    return string.reverse(bin)
end


-- ************************************************************************
--  showCountersACL
--
--  @description show CNC counters bound to PCL rules
--
--  @param params - params["devID"]     - device number.
--                  params["ruleIdx"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersACL(params)
    local ret, val
    local i, j
    local blocksList, header, body, block
    local devNum = params["devID"]
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local maxRuleId = 1023
    local ruleIdx = params["ruleIdx"]
    local allFlag = false
    local oneRuleId = false
    local retTable = {}

    header = "\n"
    header = header .. " devID   rule-id   Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"
    body = ""
    if (ruleIdx == "all") then
        allFlag = true
        local ruleIdxTmp = {}
        for i = 0, maxRuleId do
            table.insert(ruleIdxTmp, i)
        end
        ruleIdx = ruleIdxTmp
    else
        ruleIdx = {ruleIdx}
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if #ruleIdx == 1 and allFlag == false then
        oneRuleId = true
    end
    blocksList = getCNCBlock("IPCL_0")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    block = blocksList[1]

    local IPCL0_table = luaGlobalGet("IPCL_0")

    if (IPCL0_table ~= nil) then
        retTable["pass"] = {}
        for j = 1, #devices do -- main devices loop

            for i = 1, #ruleIdx do
              local counterPtr
              local index = ruleIdx[i]

              if IPCL0_table[index] ~= nil then
                ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                    { "IN", "GT_U8", "devNum", devices[j] },
                    { "IN", "GT_U32", "blockNum", block },
                    { "IN", "GT_U32", "index", index },
                    { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                    { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

                if ret ~= 0 then
                    print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters acl"
                end

                counterPtr = val["counterPtr"]
                local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                local packetsCounter, byteCounter

                -- converting the packet counter and the byte counter to 64 bit number
                packetsCounterLSB = dec2hex(packetsCounterLSB)
                packetsCounterMSB = dec2hex(packetsCounterMSB)
                packetsCounter = packetsCounterMSB .. packetsCounterLSB
                packetsCounter = hex2dec(packetsCounter)

                byteCounterLSB = dec2hex(byteCounterLSB)
                byteCounterMSB = dec2hex(byteCounterMSB)
                byteCounter = byteCounterMSB .. byteCounterLSB
                byteCounter = hex2dec(byteCounter)

                if (byteCounter ~= 0) then
                    local tuple = {
                        ["devID"] = devices[j],
                        ["rule-id"] = index,
                        ["Packet-Counter"] = packetsCounter,
                        ["Byte-count counter"] = byteCounter
                    }
                    table.insert(retTable["pass"], tuple)
                end

                if (oneRuleId == true) then
                    body = body .. "device:              " .. devices[j] .. "\n"
                    body = body .. "rule id:             " .. index .. "\n"
                    body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                    body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                else
                    if (byteCounter ~= 0 or allFlag == false) then
                        body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                            devices[j],
                            index,
                            packetsCounter,
                            byteCounter)
                    end
                end
              end  -- if
            end -- for
        end -- for

        print("IPCL counters: Enabled")
        if (body == "") then
            print("There are no Counters to show")
        else
            if oneRuleId == false then
                print(header)
            end
            print(body)
        end
    else
        print("IPCL counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters acl
--------------------------------------------

CLI_addCommand("exec", "show counters acl", {
    func = showCountersACL,
    help = "show CNC counters bound to pcl rules",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "%ruleIDorAll", name = "ruleIdx", help = "rule-id (0-16895)" },
            requirements = {
                ["ruleIdx"] = { "devID" }
            },
            mandatory = { "ruleIdx" }
        }
    }
})


-- ************************************************************************
--  showCountersEgressQueue
--
--  @description show CNC counters bound to Egress Queue client
--
--  @param params - params["devID"]     - device number.
--                  params["ruleIdx"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersEgressQueue(params)
    local ret, val
    local i, j, k, d
    local blocksList, header, body
    local devNum
    local allFlag = false
    local port, dp, tc
    local dpBits, tcBits, portBits
    local index, blockSize, block
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local listOfDP = {}
    local listOfTC = {}
    local listOfPorts = {}
    local passAndDrop = {}
    local devices
    local retTable = {}
    local mapPhysicalPortToTxqPort
    local currPort

    local numberToColor = {
        [0] = "green",
        [1] = "yellow",
        [2] = "red",
        [3] = "DROP-any dp"-- sip_5_20
    }
    blocksList = getCNCBlock("EgressQueuePassAndDrop")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    body = ""
    header = "\n"
    header = header .. " port   TC   DP    Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
        blockSize = 512
    elseif boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_AC5_E"] then
        blockSize = 2048 -- each block is 2k counters
    elseif boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_XCAT3_E"] then
        blockSize = 2048 -- each block is 2k counters
    else
        blockSize = 1024 -- each block is 1k counters
    end

    if (params["all"] == true) then
        allFlag = true
        devNum = params["devID"]

        -- listOfDP, listOfTC, listOfPorts initialyzed per device
    else
        devNum = params["dev_port"]["devId"]
        port = params["dev_port"]["portNum"]
        tc = params["tc"]
        dp = params["dp"]

        table.insert(listOfDP, dp)
        table.insert(listOfTC, tc)
        table.insert(listOfPorts, port)
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if (luaGlobalGet("EgressQueue") ~= nil) then
        table.insert(passAndDrop, "pass")
        retTable["pass"] = {}
        if (luaGlobalGet("EgressQueue") == "PassAndDrop") then
            table.insert(passAndDrop, "drop")
            retTable["drop"] = {}
        end


        for d = 1, #devices do -- main devices loop
            devNum = devices[d]

            if (params["all"] == true) then
                listOfDP = {}
                listOfTC = {}
                -- DP values causing different CNC index values
                if (is_sip_6(devNum)) then
                        table.insert(listOfDP, 0)
                elseif (is_sip_5_20(devNum)) then -- BC3 and Aldrin2
                        table.insert(listOfDP, 0)
                else -- BC2 and Aldrin
                        for i = 0, 2 do
                          table.insert(listOfDP, i)
                        end
                end

                if (is_sip_6(devNum)) then
                    -- listOfTC initialysed per port
                else
                    for i = 0, 7 do
                      table.insert(listOfTC, i)
                    end
                end

                local devInfo = luaCLI_getDevInfo(devNum)
                listOfPorts = devInfo[devNum]
            end

            if (is_sip_5(devNum)) then
                -- get TXQ iterator
                mapPhysicalPortToTxqPort = luaCLI_convertPhysicalPortsToTxq(devNum)
                --print("mapPhysicalPortToTxqPort",to_string(mapPhysicalPortToTxqPort))
                --[[ example of values in mapPhysicalPortToTxqPort -->
                    format is --> [phy_port]=txqPort
                    mapPhysicalPortToTxqPort        {
                      [0]=0,
                      [1]=1,
                      [2]=2,
                      [3]=3,
                      [65]=10,
                      [59]=8,
                      [63]=12,
                      [18]=4,
                      [80]=11,
                      [54]=6,
                      [64]=9,
                      [58]=7,
                      [36]=5
                    }
                ]]--
            end

            for j, passDrop in pairs(passAndDrop) do
                body = ""
                for i = 1, #listOfPorts do -- loop over the ports

                    if (is_sip_6(devNum)) then
                        local numOfTxqQueues;
                        currPort = listOfPorts[i]
                        numOfTxqQueues = 
                            wrlDxChSip6PortTxqNumOfQueuesGet(devNum, currPort);
                        if numOfTxqQueues == 0 then
                            print("*** error on getting SIP6 TxqNumOfQueues")
                            numOfTxqQueues = 8;
                        end
                        listOfTC = {}
                        for i = 0, (numOfTxqQueues - 1) do
                          table.insert(listOfTC, i)
                        end

                    end
                    local txPortNum = nil
                    if (is_sip_5(devNum)) then
                        currPort = listOfPorts[i]
                        txPortNum = mapPhysicalPortToTxqPort[currPort]
                        if txPortNum == nil then
                            txPortNum = 0
                        end
                    end

                    for j = 1, #listOfTC do -- loop over the traffic class
                        for k = 1, #listOfDP do -- loop over the drop precedence
                            local currTcNum = listOfTC[j]
                            local currTc = string.format("%03d", dec2bin(currTcNum))
                            local currDp
                            local drop_cnc_offset = 4

                            if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
                                currPort = string.format("%06d", dec2bin(listOfPorts[i]))
                                -- currDp = string.format("%02d", dec2bin(listOfDP[k]))

                                local dpLow = "0"
                                local dpHigh = "0"

                                if listOfDP[k]%2 > 0 then
                                  dpLow = "1"
                                end

                                if listOfDP[k]/2 > 0 then
                                  dpHigh = "1"
                                end
                                currDp = dpHigh .. dpLow

                                if (passDrop == "pass") then
                                  index = tonumber( dpHigh .. "0" .. currPort .. currTc .. dpLow, 2)
                                else
                                  index = tonumber( dpHigh .. "1" .. currPort .. currTc .. dpLow, 2)
                                end
                                block = math.floor(index / blockSize)
                                block = blocksList[block + 1]
                                index = index % blockSize
                            else
                              if (is_sip_5(devNum)) then
                                  drop_cnc_offset = 0
                                  currDp = string.format("%02d", dec2bin(listOfDP[k]))
                                  local local_dp = listOfDP[k]
                                  local sip5CurrDp;
                                  local localPassDrop;

                                  currPort = string.format("%10d", dec2bin(listOfPorts[i],10))

                                  local txPortNum_binary

                                  if (is_sip_6(devNum)) then
                                    -- support queues of ports from high tiles
                                    txPortNum = txPortNum  %  8192
                                    --  currTc used inside txPortNum_binary
                                    txPortNum_binary =
                                      string.format("%13s", dec2bin(txPortNum + currTcNum, 13))
                                  elseif (is_sip_5_20(devNum)) then -- BC3 and Aldrin2
                                      txPortNum_binary =
                                        string.format("%10s", dec2bin(txPortNum, 10))
                                  else -- BC2 and Aldrin
                                      txPortNum_binary =
                                        string.format("%08s", dec2bin(txPortNum, 8))
                                  end

                                  --print("txPortNum_binary",to_string(txPortNum_binary))

                                  localPassDrop = '0'
                                  if (passDrop ~= "pass") then
                                      localPassDrop = '1'
                                  end
                                  sip5CurrDp = string.format("%02d", dec2bin(local_dp))

                                  -- build index
                                  -- by default SIP5.20 and above uses Tail-Drop Reduced Counting Mode
                                  if (is_sip_6(devNum)) then
                                      --  currTc already used inside txPortNum_binary
                                      index = tonumber(txPortNum_binary ..  localPassDrop, 2)
                                  elseif (is_sip_5_20(devNum)) then -- BC3 and Aldrin2
                                      index = tonumber(txPortNum_binary .. currTc .. localPassDrop, 2)
                                  else -- BC2 and Aldrin
                                      index = tonumber(localPassDrop .. txPortNum_binary .. currTc .. sip5CurrDp, 2)
                                  end
                              else --older than SIP5
                                  local localPassDrop;
                                  localPassDrop = '0'
                                  if (passDrop ~= "pass") then
                                      localPassDrop = '1'
                                  end
                                  currPort = string.format("%05d", dec2bin(listOfPorts[i]))
                                  currDp = string.format("%02d", dec2bin(listOfDP[k]))
                                  index = tonumber(localPassDrop .. currPort .. currTc .. currDp, 2)
                              end

                              block = math.floor(index / blockSize) --this is the index of the block that bound to the client in the blocks table

                              if (passDrop == "pass") then
                                  block = blocksList[block + 1]
                              else
                                  block = blocksList[block + 1 + drop_cnc_offset]
                              end
                              index = index % blockSize
                            end

                            local counterPtr
                            ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_U32", "blockNum", block },
                                { "IN", "GT_U32", "index", index },
                                { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                                { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                            })

                            if ret ~= 0 then
                                --print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret] .. "     idx= " .. tostring(index) .. "  block = " .. tostring(block))
                                --print("aIdx: " .. tostring(aIdx) .. " blockSize:  " .. tostring(blockSize) .. " currPort=" .. tostring(currPort) .. " currDp=" .. tostring(currDp) .. " currTc=" .. tostring(currTc))

                                print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                                return false, "Error, cannot show counters acl"
                            end

                            counterPtr = val["counterPtr"]
                            local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                            local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                            local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                            local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                            local packetsCounter, byteCounter

                            -- converting the packet counter and the byte counter to 64 bit number
                            packetsCounterLSB = dec2hex(packetsCounterLSB)
                            packetsCounterMSB = dec2hex(packetsCounterMSB)
                            packetsCounter = packetsCounterMSB .. packetsCounterLSB
                            packetsCounter = hex2dec(packetsCounter)

                            byteCounterLSB = dec2hex(byteCounterLSB)
                            byteCounterMSB = dec2hex(byteCounterMSB)
                            byteCounter = byteCounterMSB .. byteCounterLSB
                            byteCounter = hex2dec(byteCounter)

                            local portDev = devNum .. "/" .. tonumber(currPort, 2)

                            if (byteCounter ~= 0) then
                                local tuple = {
                                    ["port"] = portDev,
                                    ["TC"] = tonumber(currTc, 2),
                                    ["DP"] = numberToColor[tonumber(currDp, 2)],
                                    ["Packet-Counter"] = packetsCounter,
                                    ["Byte-count counter"] = byteCounter
                                }

                                table.insert(retTable[passDrop], tuple)
                            end

                            if (allFlag == false) then
                                body = body .. "port:                " .. portDev .. "\n"
                                body = body .. "TC:                  " .. tonumber(currTc, 2) .. "\n"
                                body = body .. "DP:                  " .. numberToColor[tonumber(currDp, 2)] .. "\n"
                                body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                                body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                                body = body .. "CNC index:           " .. string.format("0x%X", index) .. "\n"
                            else
                                if (byteCounter ~= 0) then
                                    body = body .. string.format(" %-6s %-3s %-6s %-20s %-20s\n",
                                        portDev,
                                        tonumber(currTc, 2),
                                        numberToColor[tonumber(currDp, 2)],
                                        packetsCounter,
                                        byteCounter)
                                end
                            end
                        end
                    end
                end
                print(string.upper(passDrop) .. " counters: Enabled")
                if (body == "") then
                    print("There are no Counters to show")
                else
                    if allFlag == true then
                        print(header)
                    end
                    print(body)
                end
            end
        end
    end

    if (#passAndDrop < 2) then -- pass or drop (or both) not configured
        if (#passAndDrop == 0) then -- both
            print("PASS counters: Disabled")
        end
        print("DROP counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters egress-queue all
--------------------------------------------

CLI_addCommand("exec", "show counters egress-queue all", {
    func = function(params)
        params["all"] = true
        return showCountersEgressQueue(params)
    end,
    help = "show all CNC counters bound to Egress Queue client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            mandatory = { "devID" }
        }
    }
})

--------------------------------------------
-- command registration: show counters egress-queue ethernet
--------------------------------------------
CLI_addHelp("exec", "show counters egress-queue", "show CNC counters bound to Egress Queue client")
CLI_addCommand("exec", "show counters egress-queue ethernet", {
    func = showCountersEgressQueue,
    help = "show CNC counters for selected interface",
    params = {
        {
            type = "named",
            { format = "interface %dev_port", name = "dev_port", help = "Specific ethernet dev/port" },
            { format = "tc %sip6_queue_id_no_all", name = "tc", help = "Traffic Class associated with this set of Drop Parameters" },
            { format = "dp %drop_precedence_no_all", name = "dp", help = "Drop Profile Parameters to associate with the Traffic Class" },
            requirements = {
                ["dp"] = { "tc" },
                ["tc"] = { "dev_port" }
            },
            mandatory = { "dp" }
        }
    }
})


-- ************************************************************************
--  showCountersIngressVlan
--
--  @description show CNC counters bound to Ingress Vlan client
--
--  @param params - params["devID"]     - device number.
--                  params["vlanIdOrAll"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersIngressVlan(params)

    local ret, val
    local i, j, passDrop, d
    local blocksList, header, body
    local devNum
    local allFlag = false
    local port, dp, tc
    local dpBits, tcBits, portBits
    local index, blockSize, block, vlanId
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local listOfVlans = {}
    local passAndDrop = {}
    local devices
    local retTable = {}

    blockSize = 1024 -- each block is 1k bytes
    blocksList = getCNCBlock("IngressVlanPassAndDrop")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    body = ""
    devNum = params["devID"]
    header = "\n"
    header = header .. " devID   VlanId     Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if (params["vlanIdOrAll"] == "all") then
        allFlag = true
        for i = 1, 4095 do
            table.insert(listOfVlans, i)
        end
    else
        table.insert(listOfVlans, params["vlanIdOrAll"])
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if (luaGlobalGet("IngressVlan") ~= nil) then
        table.insert(passAndDrop, "pass")
        retTable["pass"] = {}
        if (luaGlobalGet("IngressVlan") == "PassAndDrop") then
            table.insert(passAndDrop, "drop")
            retTable["drop"] = {}
        end
        for d = 1, #devices do -- main devices loop
            devNum = devices[d]
            for j, passDrop in pairs(passAndDrop) do
                body = ""
                for i = 1, #listOfVlans do -- loop over the vlans

                    vlanId = listOfVlans[i]
                    block = math.floor(vlanId / blockSize)
                    if (passDrop == "pass") then
                        block = blocksList[block + 1]
                    else
                        block = blocksList[block + 5]
                    end
                    index = vlanId % blockSize

                    if is_sip_6_30(devNum) then
                        -- 'left shift' 1 bit the 'vlanId' ,
                        -- as the 'drop bit' is now at bit 0 and not at bit 14
                        index = (vlanId * 2) % blockSize
                    end

                    local counterPtr
                    ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_U32", "blockNum", block },
                        { "IN", "GT_U32", "index", index },
                        { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                        { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                    })

                    if ret ~= 0 then
                        print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                        return false, "Error, cannot show counters acl"
                    end
                    counterPtr = val["counterPtr"]
                    local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                    local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                    local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                    local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                    local packetsCounter, byteCounter

                    -- converting the packet counter and the byte counter to 64 bit number
                    packetsCounterLSB = dec2hex(packetsCounterLSB)
                    packetsCounterMSB = dec2hex(packetsCounterMSB)
                    packetsCounter = packetsCounterMSB .. packetsCounterLSB
                    packetsCounter = hex2dec(packetsCounter)

                    byteCounterLSB = dec2hex(byteCounterLSB)
                    byteCounterMSB = dec2hex(byteCounterMSB)
                    byteCounter = byteCounterMSB .. byteCounterLSB
                    byteCounter = hex2dec(byteCounter)

                    if (byteCounter ~= 0) then
                        local tuple = {
                            ["devID"] = devNum,
                            ["Vlan"] = vlanId,
                            ["Packet-Counter"] = packetsCounter,
                            ["Byte-count counter"] = byteCounter
                        }

                        table.insert(retTable[passDrop], tuple)
                    end
                    if (allFlag == false) then
                        body = body .. "devID:               " .. devNum .. "\n"
                        body = body .. "Vlan:                " .. vlanId .. "\n"
                        body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                        body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                    else
                        if (byteCounter ~= 0) then
                            body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                                devNum,
                                vlanId,
                                packetsCounter,
                                byteCounter)
                        end
                    end
                end
                print(string.upper(passDrop) .. " counters: Enabled")
                if (body == "") then
                    print("There are no Counters to show")
                else
                    if allFlag == true then
                        print(header)
                    end
                    print(body)
                end
            end
        end
    end
    if (#passAndDrop < 2) then -- pass or drop (or both) not configured
        if (#passAndDrop == 0) then -- both
            print("PASS counters: Disabled")
        end
        print("DROP counters: Disabled")
    end
    return true, retTable
end

--------------------------------------------
-- command registration: show counters ingress-vlan
--------------------------------------------

CLI_addCommand("exec", "show counters ingress-vlan", {
    func = showCountersIngressVlan,
    help = "show all CNC counters bound to Ingress Vlan client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "%vlanIdOrAll", name = "vlanIdOrAll", help = "The device number" },
            requirements = {
                ["vlanIdOrAll"] = { "devID" }
            },
            mandatory = { "vlanIdOrAll" }
        }
    }
})

-- ************************************************************************
--  showCountersArpNat
--
--  @description show CNC counters bound to ARP/NAT client
--
--  @param params - params["devID"]         - device number.
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************

function showCountersArpNat(params)
    local ret, val
    local i, j
    local blocksList, header, body, block
    local devNum = params["devID"]
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local maxArpNatId=1023
    local allFlag = false
    local oneArpNatId = false
    local retTable = {}
    local arpNatIdx = params["arpNatCncIdx"]
    local arpNatIdxTmp

    header = "\n"
    header = header .. " devID   arp-nat-cnc-id   Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"
    body = ""
    if (arpNatIdx == "all") then
        allFlag = true
        arpNatIdxTmp = {}
        for i = 0, maxArpNatId do
            table.insert(arpNatIdxTmp, i)
        end
        arpNatIdx = arpNatIdxTmp
    else
        arpNatIdxTmp = {}
        table.insert(arpNatIdxTmp, tonumber(arpNatIdx))
        arpNatIdx = arpNatIdxTmp
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if #arpNatIdx == 1 and allFlag == false then
        oneArpNatId = true
    end
    blocksList = getCNCBlock("ArpNatTableAccess")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    block = blocksList[1]

    if (luaGlobalGet("ArpNat") ~= nil) then

        retTable["pass"] = {}
        for j = 1, #devices do -- main devices loop

            for i = 1, #arpNatIdx do
                local counterPtr
                local index = arpNatIdx[i]
                ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                    { "IN", "GT_U8", "devNum", devices[j] },
                    { "IN", "GT_U32", "blockNum", block },
                    { "IN", "GT_U32", "index", index },
                    { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                    { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

                if ret ~= 0 then
                    print("")
                    print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters arp/nat"
                end

                counterPtr = val["counterPtr"]
                local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                local packetsCounter, byteCounter

                -- converting the packet counter and the byte counter to 64 bit number
                packetsCounterLSB = dec2hex(packetsCounterLSB)
                packetsCounterMSB = dec2hex(packetsCounterMSB)
                packetsCounter = packetsCounterMSB .. packetsCounterLSB
                packetsCounter = hex2dec(packetsCounter)

                byteCounterLSB = dec2hex(byteCounterLSB)
                byteCounterMSB = dec2hex(byteCounterMSB)
                byteCounter = byteCounterMSB .. byteCounterLSB
                byteCounter = hex2dec(byteCounter)

                if (byteCounter ~= 0) then
                    local tuple = {
                        ["devID"] = devices[j],
                        ["arp-nat-id"] = index,
                        ["Packet-Counter"] = packetsCounter,
                        ["Byte-count counter"] = byteCounter
                    }
                    table.insert(retTable["pass"], tuple)
                end
                if (oneArpNatId == true) then
                    body = body .. "device              :" .. devices[j] .. "\n"
                    body = body .. "arp/nat cnc id      :" .. index .. "\n"
                    body = body .. "Packets counter     :" .. packetsCounter .. "\n"
                    body = body .. "Byte-count counter  :" .. byteCounter .. "\n"
                else
                    if (byteCounter ~= 0 or allFlag == false) then
                        body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                            devices[j],
                            index,
                            packetsCounter,
                            byteCounter)
                    end
                end
            end
        end

        print("ARP/NAT counters: Enabled")
        if (body == "") then
            print("There are no Counters to show")
        else
            if oneArpNatId == false then
                print(header)
            end
            print(body)
        end
    else
        print("ARP/NAT counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters ARP_NAT
--------------------------------------------
CLI_addCommand("exec", "show counters ARP_NAT", {
    func = showCountersArpNat,
    help = "show all CNC counters bound to ARP/NAT client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "arpNatCncIdx %arpNatCncIDorAll", name = "arpNatCncIdx", help = "arp-nat-cnc-id (0-1023)" },
            requirements = {
                ["arpNatCncIdx"] = {"devID"}
            },
            mandatory = {"arpNatCncIdx"}
        }
    }
})

--------------------------------------------------------
-- command registration: show counters replication-types
--------------------------------------------------------
CLI_type_dict["replication_types"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Replication Types",
    enum = {
     ["qcn"] = {value="CPSS_DXCH_CFG_QCN_REPLICATION_E",
                       help="Qcn packet counter"},
     ["sniff"] = {value = "CPSS_DXCH_CFG_SNIFF_REPLICATION_E",
                       help="packet to analyzer counter"},
     ["trap"] = {value="CPSS_DXCH_CFG_TRAP_REPLICATION_E",
                       help="Trap to CPU packet counter"},
     ["mirror"] = {value = "CPSS_DXCH_CFG_MIRROR_REPLICATION_E",
                       help="Mirror to CPU packet counter"},
     ["forward"] = {value="CPSS_DXCH_CFG_OUTGOING_FORWARD_E",
                       help="forward packet counter"},
     ["all"] = {value="all", help = "Display all replication counters"}
    }
}

function showCountersReplicationType(params)
    local ret,val
    local j = 0
    local blocksList, header, body, block
    local repTypeTmp = {}
    local devNum = params["device"]
    local repType = params["replicationType"]
    local maxRepType = 4
    local allFlag
    local counterPtr
    local command_data = Command_Data()

    if repType == "CPSS_DXCH_CFG_QCN_REPLICATION_E" then
        repTypeTmp = {[0]="QCN"}
    elseif repType == "CPSS_DXCH_CFG_SNIFF_REPLICATION_E" then
        repTypeTmp = {[0]="SNIFF"}
    elseif repType == "CPSS_DXCH_CFG_TRAP_REPLICATION_E" then
        repTypeTmp = {[0]="TRAP"}
    elseif repType == "CPSS_DXCH_CFG_MIRROR_REPLICATION_E" then
        repTypeTmp = {[0]="MIRROR"}
    elseif repType == "CPSS_DXCH_CFG_OUTGOING_FORWARD_E" then
        repTypeTmp = {[0]="FORWARD"}
    elseif repType == "all" then
        repTypeTmp = {
            [0] = "QCN",
            [1] = "SNIFF",
            [2] = "TRAP",
            [3] = "MIRROR",
            [4] = "FORWARD"
        }
    end

    if not is_sip_6(devNum) then
        return
    end

    header = "\n" .. "devID     ReplicationType    Packet-Counter    \n" ..
             "---------------------------------------------------------------"
    footer = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header, footer)

    command_data:enablePausedPrinting()

    command_data:clearResultArray()

    if (repType == "all") then
        for j =0, maxRepType do
            local replicationType = j
            ret, val = myGenWrapper("cpssDxChCfgReplicationCountersGet",{
                {"IN",  "GT_U8",                                                        "devNum",                devNum    },
                {"IN",  "CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT",                  "replicationType", replicationType },
                {"OUT", "GT_32",                                                        "counterPtr"                       },
            })

            if 0 ~= ret then
                command_data:setFailLocalStatus()
                command_data:addError("Error calling cpssDxChCfgReplicationCountersGet "..returnCodes[ret])
            end

            command_data:addToResultArray(
                string.format(" %-7d| %-18s| %-16d",
                params["device"],
                repTypeTmp[j],
                val.counterPtr))
        end
    else
        local replicationType = repType
        ret, val = myGenWrapper("cpssDxChCfgReplicationCountersGet",{
            {"IN",  "GT_U8",                                                        "devNum",                devNum    },
            {"IN",  "CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT",                  "replicationType", replicationType },
            {"OUT",  "GT_32",                  "counterPtr",  },
        })

        if 0 ~= ret then
            command_data:setFailLocalStatus()
            command_data:addError("Error calling cpssDxChCfgReplicationCountersGet "..returnCodes[ret])
        end

        command_data:addToResultArray(
            string.format(" %-7d| %-18s| %-16d",
            params["device"],
            repTypeTmp[0],
            val.counterPtr))
    end

    command_data:analyzeCommandExecution()
    command_data:printAndFreeResultArray()

    return command_data:getCommandExecutionResults()
end

-------------------------------------------------------
-- command registration: show counters replication-type
-------------------------------------------------------
CLI_addCommand("exec", "show counters replication-type", {
    func = showCountersReplicationType,
    help = "show outgoing packets from replication engine",
    params = {{type = "named",
                { format = "device %devID_all", name = "device", help = "The device number" },
                { format = " repType %replication_types", name = "replicationType", help = "replication type " },
                mandatory = {"replicationType","device"}
             }}
})

-- ****************************************************************************
--  showCountersEgressPacketType
--
--  @description show CNC counters bound to Egress Packet Type Pass/Drop Client
--
--  @param params - params["devID"]     - device number.
--                  params["blockNum"]  - block number.
--                  params["cnc_format"]- counter entry format
--
--  @return  true and struct on success, otherwise false and error message
--
-- ****************************************************************************
CLI_type_dict["cnc_format"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="CNC Counter entry format mode",
    enum = {
        ["MODE_0"] = { value=0, help="29 bit packet counter 35 bit byte counter" },
        ["MODE_1"] = { value=1, help="27 bit packet counter 37 bit byte counter" },
        ["MODE_2"] = { value=2, help="37 bit packet counter 27 bit byte counter" },
        ["MODE_3"] = { value=3, help="64 bit for packet counter" },
        ["MODE_4"] = { value=4, help="64 bit for byte counter" },
        ["MODE_5"] = { value=5, help="20 bit for max value, 44 bit packet counter" }
   }
}

function showCountersEgressPacketType(params)

    local ret, val
    local d,i
    local header, body
    local devNum, blockNum, format
    local index, blockSize
    local devices
    local retTable = {}

    if not is_sip_6(devNum) then
        return
    end

    blockSize = 1024 -- each block is 1k bytes
    body = ""
    devNum = params["devID"]
    blockNum = params["blockNum"]
    format   = params["cncFormat"]

    header = "\n"
    header = header .. " devID   CNC-Index      Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if (devNum == "all") then
        devices = wrlDevList()
    else
        devices = { devNum }
    end

    for d = 1, #devices do -- main devices loop
        devNum = devices[d]
        body = ""
        for i = 1, blockSize do
            index = i-1
            local counterPtr
            ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "blockNum", blockNum },
                { "IN", "GT_U32", "index", index },
                { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "cncFormat", cncFormat },
                { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

            if ret ~= 0 then
                print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                return false, "Error, cannot show counters acl"
            end
            counterPtr = val["counterPtr"]
            local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
            local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

            local byteCounterLSB = counterPtr["byteCount"]["l"][0]
            local byteCounterMSB = counterPtr["byteCount"]["l"][1]

            local packetsCounter, byteCounter

                -- converting the packet counter and the byte counter to 64 bit number
            packetsCounterLSB = dec2hex(packetsCounterLSB)
            packetsCounterMSB = dec2hex(packetsCounterMSB)
            packetsCounter = packetsCounterMSB .. packetsCounterLSB
            packetsCounter = hex2dec(packetsCounter)

            byteCounterLSB = dec2hex(byteCounterLSB)
            byteCounterMSB = dec2hex(byteCounterMSB)
            byteCounter = byteCounterMSB .. byteCounterLSB
            byteCounter = hex2dec(byteCounter)

            if (byteCounter ~= 0) then
                    local tuple = {
                        ["devID"] = devNum,
                        ["Cnc-Index"] = index,
                        ["Packet-Counter"] = packetsCounter,
                        ["Byte-count-counter"] = byteCounter
                    }

                    table.insert(retTable, tuple)
                    body = body .. string.format("   %-6s %-15s %-20s %-20s\n",
                        devNum,
                        index,
                        packetsCounter,
                        byteCounter)
            end
        end
    end
    if (body == "") then
        print("There are no Counters to show")
    else
        print(header)
    end
    print(body)

  return true, retTable
end

---------------------------------------------------------
-- command registration: show counters egress-packet-type
---------------------------------------------------------

CLI_addCommand("exec", "show counters egress-packet-type", {
    func = showCountersEgressPacketType,
    help = "show all CNC counters bound to egress packet-type pass/drop client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "blocknum %blockNum_no_all", name = "blockNum", help = "CNC block number" },
            { format = "format %cnc_format", name = "cncFormat", help = "CNC counter entry format" },
            requirements = {
                ["cncFormat"] = { "blockNum" },
                ["blockNum"] = { "devID" }
            },
            mandatory = { "cncFormat", "devID" }
        }
    }
})

-- ************************************************************************
--  showCountersQueueStat
--
--  @description show CNC counters bound to Queue Statistics Client
--
--  @param params - params["devID"]     - device number.
--                  params["tc"]        - traffic class queue no.
--                  params["blockNum"]  - Cnc block num.
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersQueueStat(params)
    local ret, val
    local i, j, d
    local header, body
    local devNum
    local allFlag = false
    local port, tc
    local index, blockSize, block
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E = 5
    local listOfTC = {}
    local listOfPorts = {}
    local devices
    local retTable = {}
    local mapPhysicalPortToTxqPort
    local currPort

    body = ""
    header = "\n"
    header = header .. "   Port     TC     Cnc-Index      Max-Value      Packet-counter            \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if not is_sip_6(devNum) then
        return
    end

    blockSize = 1024 -- each block is 1k bytes
    block  = params["blockNum"]

    if (params["all"] == true) then
        allFlag = true
        devNum = params["devID"]

        if (is_sip_6(devNum)) then
            for i = 0, 15 do
              table.insert(listOfTC, i)
            end
        else
            for i = 0, 7 do
              table.insert(listOfTC, i)
            end
        end

        for i = 0, 127 do
            table.insert(listOfPorts, i)
        end
    else
        devNum = params["dev_port"]["devId"]
        port   = params["dev_port"]["portNum"]
        tc     = params["tc"]

        table.insert(listOfTC, tc)
        table.insert(listOfPorts, port)
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    for d = 1, #devices do -- main devices loop
        devNum = devices[d]

        -- get TXQ iterator
        mapPhysicalPortToTxqPort = luaCLI_convertPhysicalPortsToTxq(devNum)
        --print("mapPhysicalPortToTxqPort",to_string(mapPhysicalPortToTxqPort))
        --[[ example of values in mapPhysicalPortToTxqPort -->
            format is --> [phy_port]=txqPort
            mapPhysicalPortToTxqPort        {
              [0]=0,
              [1]=1,
              [2]=2,
              [3]=3,
              [65]=10,
              [59]=8,
              [63]=12,
              [18]=4,
              [80]=11,
              [54]=6,
              [64]=9,
              [58]=7,
              [36]=5
          }
      ]]--

        body = ""
        for i = 1, #listOfPorts do -- loop over the ports
            local txPortNum = nil
            currPort = listOfPorts[i]
            txPortNum = mapPhysicalPortToTxqPort[currPort]
            if txPortNum == nil then
                txPortNum = 0
            end

            for j = 1, #listOfTC do -- loop over the traffic class

                currPort = string.format("%10d", dec2bin(listOfPorts[i],10))
                local currTc = listOfTC[j]
                local txPortNum_binary

                -- support queues of ports from high tiles
                txPortNum = txPortNum  % 4096

                --  currTc used inside txPortNum_binary
                txPortNum_binary =
                         string.format("%13s", dec2bin(txPortNum + currTc, 13))
                index = tonumber(txPortNum_binary,2)

                index = index % blockSize

                local counterPtr
                ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_U32", "blockNum", block },
                    { "IN", "GT_U32", "index", index },
                    { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E },
                    { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

                if ret ~= 0 then
                    print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters acl"
                end

                counterPtr = val["counterPtr"]
                local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                local maxValueLSB = counterPtr["maxValue"]["l"][0]
                local maxValueMSB = counterPtr["maxValue"]["l"][1]

                local packetsCounter, maxValue

                -- converting the packet counter and the byte counter to 64 bit number
                packetsCounterLSB = dec2hex(packetsCounterLSB)
                packetsCounterMSB = dec2hex(packetsCounterMSB)
                packetsCounter = packetsCounterMSB .. packetsCounterLSB
                packetsCounter = hex2dec(packetsCounter)

                maxValueLSB = dec2hex(maxValueLSB)
                maxValueMSB = dec2hex(maxValueMSB)
                maxValue    = maxValueMSB .. maxValueLSB
                maxValue    = hex2dec(maxValue)

                local portDev = devNum .. "/" .. tonumber(currPort, 2)

                if (packetsCounter ~= 0) then
                    local tuple = {
                        ["port"] = portDev,
                        ["TC"] = tonumber(currTc, 2),
                        ["Cnc-Index"] = index,
                        ["max-Value"] = maxValue,
                        ["Packets-Counter"] = packetsCounter
                    }

                table.insert(retTable, tuple)
                body = body .. string.format(" %5s %6s %10s %15s %18s\n",
                           portDev,
                           currTc,
                           index,
                           maxValue,
                           packetsCounter)
                end
            end
        end
    end

    if (body == "") then
        print("There are no Counters to show")
    else
        print(header)
        print(body)
    end

    return true, retTable
end
-----------------------------------------------------------
-- command registration: show counters queue-statistics all
-----------------------------------------------------------

CLI_addCommand("exec", "show counters queue-statistics all", {
    func = function(params)
        params["all"] = true
        return showCountersQueueStat(params)
    end,
    help = "show all CNC counters bound to Queue Statistics client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "block %blockNum_no_all", name = "blockNum", help = "block number associated with the CNC Client" },
            requirements = {
                ["blockNum"] = {"devID"}
            },
            mandatory = { "blockNum" }
        }
    }
})

----------------------------------------------------------------
-- command registration: show counters queue-statistics ethernet
----------------------------------------------------------------
CLI_addHelp("exec", "show counters queue-statistics", "show CNC counters bound to Queue statistics client")
CLI_addCommand("exec", "show counters queue-statistics ethernet", {
    func = showCountersQueueStat,
    help = "show CNC counters for selected interface",
    params = {
        {
            type = "named",
            { format = "interface %dev_port", name = "dev_port", help = "Specific ethernet dev/port" },
            { format = "block %blockNum_no_all", name = "blockNum", help = "block number associated with the CNC Client" },
            { format = "tc %sip6_queue_id_no_all", name = "tc", help = "Traffic Class associated with this set of Drop Parameters" },
            requirements = {
                ["tc"] = { "blockNum" },
                ["blockNum"] = { "dev_port" }
            },
            mandatory = { "tc" }
        }
    }
})

-- ************************************************************************
--  showCountersPortStat
--
--  @description show CNC counters bound to Port Statistics Client
--
--  @param params - params["devID"]     - device number.
--                  params["blockNum"]  - Cnc block num.
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersPortStat(params)
    local ret, val
    local i, j, k, d
    local blocksList, header, body
    local devNum
    local allFlag = false
    local port, dp, tc
    local dpBits, tcBits, portBits
    local index, blockSize, block
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E = 5
    local listOfTC = {}
    local listOfPorts = {}
    local devices
    local retTable = {}
    local mapPhysicalPortToTxqPort
    local currPort

    body = ""
    header = "\n"
    header = header .. "   Port     Cnc-Index      Max-Value      Packet-counter            \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if not is_sip_6(devNum) then
        return
    end

    blockSize = 1024 -- each block is 1k bytes
    block  = params["blockNum"]

    if (params["all"] == true) then
        allFlag = true
        devNum = params["devID"]

        for i = 0, 127 do
            table.insert(listOfPorts, i)
        end
    else
        devNum = params["dev_port"]["devId"]
        port   = params["dev_port"]["portNum"]

        table.insert(listOfPorts, port)
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    for d = 1, #devices do -- main devices loop
        devNum = devices[d]

        -- get TXQ iterator
        mapPhysicalPortToTxqPort = luaCLI_convertPhysicalPortsToTxq(devNum)
        --print("mapPhysicalPortToTxqPort",to_string(mapPhysicalPortToTxqPort))
        --[[ example of values in mapPhysicalPortToTxqPort -->
            format is --> [phy_port]=txqPort
            mapPhysicalPortToTxqPort        {
              [0]=0,
              [1]=1,
              [2]=2,
              [3]=3,
              [65]=10,
              [59]=8,
              [63]=12,
              [18]=4,
              [80]=11,
              [54]=6,
              [64]=9,
              [58]=7,
              [36]=5
          }
      ]]--

        body = ""
        for i = 1, #listOfPorts do -- loop over the ports
            local txPortNum = nil
            currPort = listOfPorts[i]
            txPortNum = mapPhysicalPortToTxqPort[currPort]
            if txPortNum == nil then
                txPortNum = 0
            end

            local txPortNum_binary

            -- support queues of ports from high tiles
            txPortNum = txPortNum  % 4096

            --  currTc used inside txPortNum_binary
            txPortNum_binary =
                     string.format("%13s", dec2bin(txPortNum, 13))
            index = tonumber(txPortNum_binary)

            index = index % blockSize

            local counterPtr
            ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "blockNum", block },
                { "IN", "GT_U32", "index", index },
                { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E },
                { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
             })

            if ret ~= 0 then
                print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters acl"
            end

            counterPtr = val["counterPtr"]
            local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
            local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

            local maxValueLSB = counterPtr["maxValue"]["l"][0]
            local maxValueMSB = counterPtr["maxValue"]["l"][1]

            local packetsCounter, maxValue

            -- converting the packet counter and the byte counter to 64 bit number
            packetsCounterLSB = dec2hex(packetsCounterLSB)
            packetsCounterMSB = dec2hex(packetsCounterMSB)
            packetsCounter = packetsCounterMSB .. packetsCounterLSB
            packetsCounter = hex2dec(packetsCounter)

            maxValueLSB = dec2hex(maxValueLSB)
            maxValueMSB = dec2hex(maxValueMSB)
            maxValue    = maxValueMSB .. maxValueLSB
            maxValue    = hex2dec(maxValue)

            local portDev = devNum .. "/" .. currPort

            if (packetsCounter ~= 0) then
                local tuple = {
                    ["port"] = portDev,
                    ["Cnc-Index"] = index,
                    ["max-Value"] = maxValue,
                    ["Packets-Counter"] = packetsCounter
                }

            table.insert(retTable, tuple)
            body = body .. string.format(" %5s %10s %15s %18s\n",
                       portDev,
                       index,
                       maxValue,
                       packetsCounter)
            end
        end
    end

    if (body == "") then
        print("There are no Counters to show")
    else
        print(header)
        print(body)
    end

    return true, retTable
end

-----------------------------------------------------------
-- command registration: show counters port-statistics all
-----------------------------------------------------------

CLI_addCommand("exec", "show counters port-statistics all", {
    func = function(params)
        params["all"] = true
        return showCountersPortStat(params)
    end,
    help = "show all CNC counters bound to Port Statistics client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "block %blockNum_no_all", name = "blockNum", help = "block number associated with the CNC Client" },
            requirements = {
                ["blockNum"] = {"devID"}
            },
            mandatory = { "blockNum" }
        }
    }
})

----------------------------------------------------------------
-- command registration: show counters port-statistics ethernet
----------------------------------------------------------------
CLI_addHelp("exec", "show counters port-statistics", "show CNC counters bound to Port statistics client")
CLI_addCommand("exec", "show counters port-statistics ethernet", {
    func = showCountersPortStat,
    help = "show CNC counters for selected interface",
    params = {
        {
            type = "named",
            { format = "interface %dev_port", name = "dev_port", help = "Specific ethernet dev/port" },
            { format = "block %blockNum_no_all", name = "blockNum", help = "block number associated with the CNC Client" },
            requirements = {
                ["blockNum"] = { "dev_port" }
            },
            mandatory = { "blockNum" }
        }
    }
})


-- *******************************************************************************
--  showPfcCounters
--
--  @description show PFC counters
--               Option to show PFC counter per port and per TC
--               Option to show PFC counters for all TCs range per port
--               Option to show PFC counters for specific range of ports and TCs
--
--  @param params - params["devID"] - Device number.
--                  params["ports"] - Port number.
--                  params["tc"]    - Traffic Class number.
--
--  @return  true and struct on success, otherwise false and error message
--
-- *******************************************************************************


function showPfcCounters(params)

    local i, iterator, result, values, tcNum, pfcCounterVal
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Define the header string
    header_string = "\n" ..
        " Device/Port     TC     PFC-counter     \n" ..
        "-------------   ----   -------------    \n"
    footer_string = "\n"

    -- Enable print
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    if true == command_data["status"] then
        -- Main port handling cycle
        for dummy, devNum, portNum in command_data:getPortIterator() do
            -- Loop over Traffic Class range
            for i=1, #params.tcRange do
                -- Get TC number from the list
                tcNum = params.tcRange[i]
                -- Call CPSS API cpssDxChPortPfcCounterGet to get PFC counters
                iterator, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChPortPfcCounterGet",
                                                   { {"IN" , "GT_U8" , "devNum"  , devNum},
                                                     {"IN" , "GT_U8" , "tcQueue" , tcNum},
                                                     {"IN" , "GT_U32", "port"    , portNum},
                                                     {"OUT", "GT_U32", "pfcCounterPtr"}
                                                   })
                -- Get PFC counter value
                pfcCounterVal = values["pfcCounterPtr"]

                -- Concatenate device number and port number
                local devPort = devNum .. "/" .. portNum

                -- Set result values for print
                command_data["result"] = string.format("%-16s %-9s %-11s",
                                                   tostring("      "..devPort),
                                                   tostring(tcNum),
                                                   tostring(pfcCounterVal))
                command_data:addResultToResultArray()
                command_data:updatePorts()
            end

        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
                                         footer_string, "There are no Counters to show.\n")

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- Traffic Class type
CLI_type_dict["tc_range"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=7,
    complete = CLI_complete_param_number_range,
    help = "Traffic Classes (0..7) range. Examples: \"7\", \"0,2-4,7\", \"all\""
}

----------------------------------------------------------------
-- command registration: show counters pfc
----------------------------------------------------------------
CLI_addHelp("exec", "show counters pfc", "show PFC counter value per port and traffic class")
CLI_addCommand("exec", "show counters pfc", {
    func = showPfcCounters,
    help = "show PFC counter value per port and traffic class",
    params = {
        {
            type = "named",
            { format = "tc %tc_range ", name = "tcRange", help = "traffic classes range"},
            "#all_interfaces",
        },
    }
})
