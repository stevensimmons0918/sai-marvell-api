--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pip_example.lua
--*
--* DESCRIPTION:
--*       test configuration and traffic related to PIP.
--*       Pre-Ingress Prioritization (PIP)
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
--- AC3X need proper ports for this test : all ports will be taken from Aldrin
-- the call to SUPPORTED_FEATURE_DECLARE may change the value of devEnv.port[1..6] !!!
SUPPORTED_FEATURE_DECLARE(devNum,"RXDMA_PIP")

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local origPorts = {ports = {devEnv.port[1],devEnv.port[2],devEnv.port[3],devEnv.port[4]}}

local ingressPort = port2
local egressPort  = port3
local cpuPort = 63

local global_section = ""
local orig_printLog = printLog
local function local_printLog(...)
    --do nothing
end

local allow_test_TPID_vid_classifications       = true
local allow_test_non_dsa_classifications        = true
local allow_test_default_port_priority          = true
local allow_test_default_port_priority_CPU_port = true
local allow_test_dsa_classifications            = true
local allow_test_latency_sensitive              = true
local allow_test_tpid_length                    = true

-- when want to debug specific case : state : debug_specific_case = true
-- then state the specific case below ...
local debug_specific_case = false

if debug_specific_case then
    allow_test_TPID_vid_classifications       = false
    allow_test_non_dsa_classifications        = false
    allow_test_default_port_priority          = false
    allow_test_default_port_priority_CPU_port = false
    allow_test_dsa_classifications            = true
    allow_test_latency_sensitive              = false
    allow_test_tpid_length                    = false

    -- state the specific case to be tested
    allow_test_TPID_vid_classifications       = false
    
    -- state specific 'counterModesDb[]' modes to run
    allow_test_counterModesDb = { [1] = true , [2] = false , [3] = false }
end

-- get the speed of port
-- value is 'number' in units of 'Mbps'
function portSpeedGet(devNum,portNum)
    local command_data = Command_Data()
    local apiName = "cpssDxChPortSpeedGet"
    local isError , result, values =
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",    "CPSS_PORT_SPEED_ENT",    "speedPtr"}
    })

    if isError then
        myError = true
        return false
    end

    local port_speed_str,port_speed_number = speedStrGet(values["speedPtr"])
    return port_speed_number
end

local function testNotSupported(reason)
    printLog(reason)
    isDeviceNotSupported = true
    error()
end

-- the Aldrin2 (HW) not drop any packet when threshold is 0 and using single 10G port !
-- but when doing 100G port ... we see drops !
-- so we need the 'ingress port' to be 100G port
if is_sip_5_25(devNum) then
    local currSpeed = portSpeedGet(devNum,ingressPort)
    local minimalSpeed = 50000 --50G (to support falcon ports)
    local foundGoodSpeed = false
    local numPorts = 4
    if currSpeed < minimalSpeed then
        printLog("try to find 100G port for the test (because single 10G port not generate drops)")
        -- find other port to use
        for ii = 1,numPorts do
            local portNum = devEnv.port[ii]
            currSpeed = portSpeedGet(devNum,portNum)
            if currSpeed >= minimalSpeed then
                local index = ii
                ingressPort = devEnv.port[index] --current port

                index = 1 + (index % numPorts)
                egressPort  = devEnv.port[index] -- next port

                index = 1 + (index % numPorts)
                port1 = devEnv.port[index] -- next port

                index = 1 + (index % numPorts)
                port4 = devEnv.port[index] -- next port

                port2 = ingressPort
                port3 = egressPort

                devEnv.port[1] = port1
                devEnv.port[2] = port2
                devEnv.port[3] = port3
                devEnv.port[4] = port4

                foundGoodSpeed = true
                printLog("-- use 100G port [" .. port2 .. "] for Pip test -- (because single 10G port not generate drops)")
                printLog("-- egress port   [" .. port3 .. "] , other ports   [" .. port1 .. "," .. port4 .. "]")
                break
            end
        end

        if foundGoodSpeed == false then
            testNotSupported("Sip 5.25 : Failed to find 100G port for the test")
        end

    end
end

local isSimulation = isSimulationUsed()

-- devices excluding Aldrin2/BC3 drops packet when PIP FIFO threshould is zero
-- Aldrin2/BC3 does not drops
-- WM drops such packets for all devices
local isThresholdZeroDropPackets = isSimulation or not is_sip_5_20(devNum) or is_sip_6(devNum)

local prioritySet
if is_sip_5_20(devNum) then
    prioritySet = {"medium", "low", "high", "very-high"}
    lowPriorityNum = 3
else
    prioritySet = {"medium", "low", "high"}
    lowPriorityNum = 2
end

local checkExpectedValues = testCheckExpectedValues

local transmit_operation_mode = "none" --[["none" or "record" or "reply"]]--
local transmit_db = {}--[[array of records: {transmitInfo,egressInfoTable}]]
local transmit_db_reply_index = 1

--##################################
--##################################

local delayVal = 250 -- 0.25 sec by default
local disableCheckDropCounters = false
local dropOnPortTotalCounter = 0
local dropByPriorityTotalCounter = 0

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"
-- unicast macDa
local macDaUc = "000000000058" --> 00:00:00:00:00:58 --> as set in the configuration file !
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end

    return macDaUc .. macSa ..  partAfterMac .. packetPayload
end
-- build mac da - mc (multi-cast)
local function buildPacketMc(macDaMc)
    return macDaMc .. macSa ..  packetPayload
end

-- build packet with 4 bytes of vlan tag according to TPID , vid , up
local function buildPacketVlanTag(tpid,vid,up)
    local value = bit_shl(tpid,16) + bit_shl(up,13) + vid
    local vlanTagString = string.format("%8.8x",value)
    return buildPacketUc(vlanTagString)
end

-- build packet with 6 bytes of : 2 bytes mpls ethertype + 4 bytes of MPLS label according to exp
local function buildPacketMpls(mpls_exp)
    local ethertype = "8847"-- mpls unicast
    local label = 0x12345
    local ttl   = 0xaa
    local stopBit = 1
    local value = bit_shl(0x12345,12) + bit_shl(mpls_exp,9) + bit_shl(stopBit,8) + ttl
    local mplsString = string.format("%8.8x",value)
    return buildPacketUc(ethertype .. mplsString)
end

-- build packet with 22 bytes of : 2 bytes ipv4 ethertype +  20 bytes of IPv4
local function buildHeaderIpv4(tos)
    local ethertype = "0800"
    local version_and_ihl = 0x45
    local totalLength = 60
    local value = bit_shl(version_and_ihl,24) + bit_shl(tos,16) + totalLength
    local word1 = string.format("%8.8x",value)
    local word2 = string.format("%8.8x",0)

    local ttl   = 0xbb
    local protocol = 0x55
    local checksum = 0 -- unknown
    local value = bit_shl(ttl,24) + bit_shl(protocol,16) + checksum
    local word3 = string.format("%8.8x",value)

    local word4 = "01020304"--sip
    local word5 = "01020305"--dip

    return ethertype .. word1 .. word2 .. word3 .. word4 .. word5
end

local function buildPacketIpv4(tos)
    return buildPacketUc(buildHeaderIpv4(tos))
end

-- build packet with 44 bytes : 2 bytes ipv6 ethertype + 40 bytes of ip bytes of IPv6
local function buildPacketIpv6(tc)
    local ethertype = "86dd"
    local version = 0x6
    local flowLabel = 0x12345
    local value = bit_shl(version,28) + bit_shl(tc,20) + flowLabel
    local word1 = string.format("%8.8x",value)

    local hopLimit = 0xdd
    local nextHeader = 0x55
    local payloadLength = 40
    local value = bit_shl(payloadLength,16) + bit_shl(nextHeader,8) + hopLimit
    local word2 = string.format("%8.8x",value)

    local sip = "11111111" .. "11223344" .. "22334455" .. "11111111"
    local dip = "22222222" .. "aabbccdd" .. "bbccddee" .. "22222222"

    return buildPacketUc(ethertype .. word1 .. word2 .. sip .. dip)
end

--build packet with 2 bytes of UDE
local function buildPacketUde(etherType)
    local bytes_0_1 = string.format("%4.4x",etherType)
    return buildPacketUc(bytes_0_1)
end

--build packet with MC mac da
local function buildPacketMacDaMc(macDa)
    return buildPacketMc(macDa)
end

local needToRestore_e2phy_6bits_limit = false
-- set/unset port as cascade
local function generic_cscd_port_set(devNum, portNum, unset , portType)
    local command_data = Command_Data()

    if unset == true then
        portType = "CPSS_CSCD_PORT_NETWORK_E"
    elseif portType then
        -- already have portType value
    elseif is_sip_5(devNum) then
        portType = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    else
        portType = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    end

    if egressPort >= 64 and
       portType == "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E" and
       needToRestore_e2phy_6bits_limit == false
    then
        -- the extended DSA will hold only 6 bits of the target port
        -- we need to 'redirect it' using the e2phy from the limited port number to the needed physical port number
        e2phySet(command_data,devNum,egressPort % 64,egressPort)
        needToRestore_e2phy_6bits_limit = true
    end

    if portType == "CPSS_CSCD_PORT_NETWORK_E" and
       needToRestore_e2phy_6bits_limit == true
    then
        -- the extended DSA will hold only 6 bits of the target port
        -- we need to 'redirect it' using the e2phy from the limited port number to the needed physical port number
        e2phySet(command_data,devNum,egressPort % 64,egressPort % 64)
        needToRestore_e2phy_6bits_limit = false
    end

    apiName = "cpssDxChCscdPortTypeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
        { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType", portType}
    })
end

-- wrapper to call the 'per port counter'
-- in sip6 :
--      this function calls the 'global counter'
--      it is the responsibility of the test to set the 'counter mode' to count this portNum
local function internal____pip_get_info_drop_port_counter(command_data, devNum, portNum)
    if is_sip_6(devNum) then
--      this function calls the 'global counter'
--      it is the responsibility of the test to set the 'counter mode' to count this portNum
        local isErr, globalCounter = pip_get_info_drop_global_counter(command_data,devNum)
        return isErr , globalCounter.l[0] , globalCounter.l[1]
    else
        return pip_get_info_drop_port_counter(command_data, devNum, portNum)
    end
end

-- wrapper to call the 'global counter' (counter per priority)
-- in sip6 :
--      this function calls the 'global counter'
--      it is the responsibility of the test to set the 'counter mode' to count the needed priority
local function internal____pip_get_info_drop_global_counter(command_data, devNum)
    if is_sip_6(devNum) then
        local isErr, globalCounter , dummy_Counter = pip_get_info_drop_global_counter(command_data,devNum)
            -- supporting the 'expected' results
        dummy_Counter.name = pip_sip6_getPriorityName()
        return isErr, globalCounter , dummy_Counter , dummy_Counter , dummy_Counter
    else
        return pip_get_info_drop_global_counter(command_data,devNum)
    end
end


local function updateGlobalCountersByPriority(devNum)
    -- read actual values of the priority drop counters
    local isErr
    local counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow
    local vhDropCounter, hDropCounter, mDropCounter, lDropCounter


    local dropPacketsDetected = false
    local saveCounter = dropByPriorityTotalCounter


    isErr, counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow = internal____pip_get_info_drop_global_counter(command_data, devNum)
    vhDropCounter= tonumber(string.format("0x%08X%08X", counterPrioVeryhigh.l[1], counterPrioVeryhigh.l[0]))
    hDropCounter = tonumber(string.format("0x%08X%08X", counterPrioHigh.l[1], counterPrioHigh.l[0]))
    mDropCounter = tonumber(string.format("0x%08X%08X", counterPrioMedium.l[1], counterPrioMedium.l[0]))
    lDropCounter = tonumber(string.format("0x%08X%08X", counterPrioLow.l[1], counterPrioLow.l[0]))

    -- calculate total number of packets dropped by priority
    dropByPriorityTotalCounter = dropByPriorityTotalCounter + vhDropCounter + hDropCounter + mDropCounter + lDropCounter

    if isErr == true then
      local error_string = "Error reading global drop counters by priority"
      printLog ("ERROR : " .. error_string)
      testAddErrorString(error_string)
    end

    -- counters changed
    if saveCounter ~= dropByPriorityTotalCounter then
      dropPacketsDetected = true
    end

    return dropPacketsDetected
end

--##################################
--##################################

-- function to run section
local function testSection(sectionInfo)
    local sectionName       = sectionInfo.sectionName
    local purpose           = sectionInfo.purpose
    local transmitInfo      = sectionInfo.transmitInfo
    local egressInfoTable   = sectionInfo.egressInfoTable
    local configFunc        = sectionInfo.configFunc
    local deConficFunc      = sectionInfo.deConficFunc
    local thresholdChangeFunc = sectionInfo.thresholdChangeFunc

    local dropped_on_ingress = {
        -- expect ALL ports NOT to get it
        {portNum = port1  , packetCount = 0},
        {portNum = port2  , packetCount = 0},
        {portNum = port3  , packetCount = 0},
        {portNum = port4  , packetCount = 0}
    }
    local ingressPort = transmitInfo.portNum

    local index
    -- remove ingress port from dropped_on_ingress
    for ii,entry in pairs(dropped_on_ingress) do
        if entry.portNum == ingressPort then
            -- move last entry to override this entry
            index = ii
            break
        end
    end

    -- remove last index
    if index then
        dropped_on_ingress[index] = nil
    end

    printLog (delimiter)

    local egress_expected_result_string = "--> 'threshold' allow 'packet to pass'"

    printLog(global_section .."START : Section " .. sectionName .. egress_expected_result_string)
    printLog(global_section .."Purpose of section : " .. purpose)

    if configFunc then
        configFunc()
    end

    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED , when " .. egress_expected_result_string
        printLog (global_section .."ENDED : " .. pass_string .. "\n")
        testAddPassString(global_section ..pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED , when " .. egress_expected_result_string
        printLog (global_section .."ENDED : " .. error_string .. "\n")
        testAddErrorString(global_section ..error_string)
    end

    printLog (delimiter)

    if (thresholdChangeFunc and (disableCheckDropCounters == false)) then
        -- call to change the needed threshold
        thresholdChangeFunc()

        egress_expected_result_string = "--> 'threshold' NOT allow 'packet to pass' --> expected 'DROP'"
        printLog(global_section .."START : Section " .. sectionName .. egress_expected_result_string)

        local rc
        if (isThresholdZeroDropPackets == false) then
            transmitInfo.burstCount = transmit_continuous_wire_speed
            rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)

            -- the task still running

            delay(delayVal) -- delay

            transmitInfo.burstCount = stop_transmit_continuous_wire_speed
            rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
            transmitInfo.burstCount = nil

            local command_data = Command_Data()

            printLog("isThresholdZeroDropPackets",isThresholdZeroDropPackets)
            local isError , ingDropCounterLo, ingDropCounterHi = internal____pip_get_info_drop_port_counter(command_data, devNum, ingressPort)
            if isError then
                local error_string = "ERROR : FAILED to get drop counters on " .. tostring(ingressPort) .. "port"
                -- use command_data:addErrorAndPrint to be able to see also errors that came from
                -- command_data that got it inside internal____pip_get_info_drop_port_counter
                command_data:addErrorAndPrint(error_string) -- print and reset the errors string array

                --printLog ("ERROR : " .. error_string)
                testAddErrorString(error_string)
            end
            local ingressDropCounter = tonumber(string.format("0x%08X%08X", ingDropCounterHi, ingDropCounterLo))

            dropOnPortTotalCounter = dropOnPortTotalCounter + ingressDropCounter

            local isError , cpuDropCounterLo, cpuDropCounterHi = internal____pip_get_info_drop_port_counter(command_data, devNum, cpuPort)
            if isError then
                local error_string = "ERROR : FAILED to get drop counters on " .. tostring(cpuPort) .. "port"
                -- use command_data:addErrorAndPrint to be able to see also errors that came from
                -- command_data that got it inside internal____pip_get_info_drop_port_counter
                command_data:addErrorAndPrint(error_string) -- print and reset the errors string array

                --printLog ("ERROR : " .. error_string)
                testAddErrorString(error_string)
            end
            local cpuDropCounter = tonumber(string.format("0x%08X%08X", cpuDropCounterHi, cpuDropCounterLo))

            dropOnPortTotalCounter = dropOnPortTotalCounter + cpuDropCounter
            local globalCntByPrioChangesDetected = updateGlobalCountersByPriority(devNum)


            if ((cpuDropCounter == 0) and (ingressDropCounter == 0)) then
              if globalCntByPrioChangesDetected then
                printLog(" Global Priority Drop  Counters  detected! ")
              else
                printLog(" Drop  Counters  failed! ")
                rc = 1
              end
            else
                if (cpuDropCounter ~= 0) then
                    printLog("CPU port dropped " .. cpuDropCounter .. " packets")
                end
                if (ingressDropCounter ~= 0) then
                    printLog("ingress port dropped " .. ingressDropCounter .. " packets")
                end
            end
        else
            -- some of the sendings from the CPU port are dropped by the RxDma of the 'CPU port'
            -- but other sending make it to the 'ingress port' and are dropped by RxDma of the 'ingress port'
            transmitInfo.maybePacketFilteredBeforLoopback = true
            -- check that packet egress no port , when the 'threshold' NOT allow it
            rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,dropped_on_ingress)
            transmitInfo.maybePacketFilteredBeforLoopback = nil
        end

        printLog (delimiter)
        if rc == 0 then
            local pass_string = "Section " .. sectionName .. " PASSED , when " .. egress_expected_result_string
            printLog (global_section .."ENDED : " .. pass_string .. "\n")
            testAddPassString(global_section .. pass_string)
        else
            local error_string = "Section " .. sectionName .. " FAILED , when " .. egress_expected_result_string
            printLog (global_section .."ENDED : " .. error_string .. "\n")
            testAddErrorString(global_section .. error_string)
        end
        printLog (delimiter)
    end


    if deConficFunc then
        deConficFunc()
    end
end
--##################################
--##################################
local command_restoreThresholds
if is_sip_5_20(devNum) then
    command_restoreThresholds =
    [[
end
configure
pip threshold 65535 device ${dev} priority very-high
pip threshold 65535 device ${dev} priority high
pip threshold 65535 device ${dev} priority medium
pip threshold 65535 device ${dev} priority low
    ]]
else
    command_restoreThresholds =
    [[
end
configure
pip threshold 1023 device ${dev} priority high
pip threshold 1023 device ${dev} priority medium
pip threshold 1023 device ${dev} priority low
    ]]
end

local command_threshold_veryhigh_reset =
[[
end
configure
pip threshold 0 device ${dev} priority very-high
]]

local command_threshold_high_reset =
[[
end
configure
pip threshold 0 device ${dev} priority high
]]
local command_threshold_medium_reset =
[[
end
configure
pip threshold 0 device ${dev} priority medium
]]
local command_threshold_low_reset =
[[
end
configure
pip threshold 0 device ${dev} priority low
]]
-- set the egress port to keep it's TPID
local command_set_egress_port_keep_tpid =
[[
end
debug-mode
cpss-api call cpssDxChBrgVlanEgressTagTpidSelectModeSet devNum ${dev} portNum ]] .. egressPort .. [[ ethMode CPSS_VLAN_ETHERTYPE0_E mode CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E
exit
]]
-- set the egress port to get TPID according to egress port index in TPID table(default setting)
local command_set_egress_port_not_keep_tpid =
[[
end
debug-mode
cpss-api call cpssDxChBrgVlanEgressTagTpidSelectModeSet devNum ${dev} portNum ]] .. egressPort .. [[ ethMode CPSS_VLAN_ETHERTYPE0_E mode CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E
exit
]]
-- set pvid 15 on ingress port
local command_set_ingress_port_pvid_15 =
[[
end
configure
interface ethernet ${dev}/]] .. ingressPort ..
[[

switchport pvid 15
exit
]]
-- restor pvid 1 on ingress port
local command_set_ingress_port_pvid_1 =
[[
end
configure
interface ethernet ${dev}/]] .. ingressPort ..
[[

switchport pvid 1
exit
]]
-- sip6 restore parser TPIDs : 0x8100 at index 0..3
-- that modified by command : "pip vid-ethertype"
local function command_sip6_restorePortParserGlobalTpid()
    local command_data = Command_Data()
    local apiName = "cpssDxChPortParserGlobalTpidSet"
    local ethertype = 0x8100
    for index = 0,3 do
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",     "devNum",     devNum },
        { "IN",     "GT_U32",    "index",       index },
        { "IN",     "GT_U16",    "etherType",   ethertype },
        { "IN",     "CPSS_BRG_TPID_SIZE_TYPE_ENT",    "tpidSize",   "CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E" }
        })
    end
end

-- Set priority for FROM_CPU traffic
local command_from_cpu_veryhigh_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority very-high
]]

-- Set priority for FROM_CPU traffic for SIP_6 - use default priority
local command_from_cpu_veryhigh_set_sip_6 =
[[
end
configure
interface ethernet ${dev}/63
pip priority-default very-high
]]


local command_from_cpu_high_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority high
]]

local command_from_cpu_low_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority low
]]

local cpuPort_trust_default
if is_sip_6(devNum) then
    cpuPort_trust_default = "no pip trust"
else
    cpuPort_trust_default = "pip trust"
end
-- the CPU port should restore pip trust
local command_cpuPortPipTrust = [[
end
configure
interface ethernet ${dev}/]] .. cpuPort .. [[

    ]] .. cpuPort_trust_default .. [[

exit
]]

local command_allPortPipTrust = [[
end
configure
interface range ethernet all ${dev}
pip trust
exit
]]




-- set 1023 threshold for veryhigh priority
local command_set_thr_veryhigh = [[
end
configure
pip threshold 1023 device ${dev} priority very-high
expect pip threshold 1023 device ${dev} priority very-high
exit
]]

-- set dafault threshold for veryhigh priority
local command_set_dflt_thr_veryhigh = [[
end
configure
pip threshold 65535 device ${dev} priority very-high
exit
]]

-- function for sip6 to allow setting
-- input : mode { portNum = value or nil , priority = value or nil}
-- if mode.portNum = "all" and mode.priority = "all" then 'count all'
-- if mode.portNum  then 'port mode'
-- if mode.priority  then 'priority mode' set with ("very-high" or "high" or "medium" or "low")
-- if mode.priority1 then 'priority mode' added with ("very-high" or "high" or "medium" or "low")
-- else disable the counting
local function sip6_activate_pip_counter(mode)
    local currLuaCommand
    local port_string , priority_string = "" ,""

    if mode and mode.portNum == "all" and mode.priority == "all" then
        priority_string = " "
        port_string = " "
    end

    if mode and mode.portNum and mode.portNum ~= "all" then
        port_string = "port " .. to_string(mode.portNum)
        priority_string = priority_string .. " "
    end

    if mode and mode.priority and mode.priority ~= "all" then
        port_string = port_string .. " "

        local priority_string1
        if(mode.priority == "very-high") then
            priority_string = "priority-00-very-high"
        elseif (mode.priority == "high") then
            priority_string = "priority-01-high"
        elseif (mode.priority == "medium") then
            priority_string = "priority-02-medium"
        else -- low
            priority_string = "priority-03-low"
        end

        if(mode.priority1 == "very-high") then
            priority_string1 = "priority-00-very-high"
        elseif (mode.priority1 == "high") then
            priority_string1 = "priority-01-high"
        elseif (mode.priority1 == "medium") then
            priority_string1 = "priority-02-medium"
        elseif(mode.priority1 == "low") then
            priority_string1 = "priority-03-low"
        else
            priority_string1 = ""
        end

        if priority_string1 ~= "" then
            priority_string = priority_string .. " true " .. priority_string1
        end

        priority_string = priority_string .. " true "
    end

    if not mode then -- disable counting
        currLuaCommand = [[
        end
        configure
        no pip drop-counter-mode device ${dev}
        exit
        ]]
    else
        currLuaCommand = [[
        end
        configure
        pip drop-counter-mode device ${dev} ]] .. port_string .. " " .. priority_string .. [[

        exit
        ]]
    end

    executeStringCliCommands(currLuaCommand)
end

-- function to check if the value in the sip6 counter is as expected
local function sip6_check_pip_counter(expectedCounter)
    local isErr, globalCounter = pip_get_info_drop_global_counter(command_data,devNum)

    --printLog("expectedCounter",to_string(expectedCounter))
    --printLog("globalCounter.l[0]",to_string(globalCounter.l[0]))
    local isPrintAllowed = (printLog == orig_printLog)

    if not isPrintAllowed then
        -- the printLog is disabled , but we want to see errors !
        -- so open it
        printLog = orig_printLog
    end

    check_expected_value(global_section .. "global drop counter check" , expectedCounter , globalCounter.l[0])

    -- sip6 counter is not clear on read - the second read removed

    if not isPrintAllowed then
        -- restore : the printLog is disabled
        printLog = local_printLog
    end

end

local reduceExpectedFromCpu1,reduceExpectedFromCpu2,reduceExpectedFromCpu3 = 0,0,0
local function reduceExpectedFromCpuSet(numToReduce1,numToReduce2,numToReduce3)
    reduceExpectedFromCpu1 = numToReduce1
    reduceExpectedFromCpu2 = numToReduce2
    reduceExpectedFromCpu3 = numToReduce3
end

local function sip6_check_pip_counter_wa1(expectedCounter)
    printLog("expectedCounter",expectedCounter)
    printLog("reduceExpectedFromCpu1",reduceExpectedFromCpu1)
    sip6_check_pip_counter(expectedCounter - reduceExpectedFromCpu1)
    reduceExpectedFromCpu1 = 0
end
local function sip6_check_pip_counter_wa2(expectedCounter)
    printLog("expectedCounter",expectedCounter)
    printLog("reduceExpectedFromCpu2",reduceExpectedFromCpu2)
    sip6_check_pip_counter(expectedCounter - reduceExpectedFromCpu2)
    reduceExpectedFromCpu2 = 0
end
local function sip6_check_pip_counter_wa3(expectedCounter)
    printLog("expectedCounter",expectedCounter)
    printLog("reduceExpectedFromCpu3",reduceExpectedFromCpu3)
    sip6_check_pip_counter(expectedCounter - reduceExpectedFromCpu3)
    reduceExpectedFromCpu3 = 0
end


--##################################
--##################################
local ingressPacket_1 = buildPacketVlanTag(0x9100 --[[tpid]] , 5 --[[vid]] , 7 --[[up]])
local transmitInfo_1 = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket_1} }
local egressInfoTable_1 = {
    -- expected egress port .. unmodified
    {portNum = egressPort  , pktInfo = {fullPacket = ingressPacket_1}},
    -- expect other ports not to get it
    {portNum = port1  , packetCount = 0},
    --{portNum = port2  , packetCount = 0}, --> the ingress port
    --{portNum = port3  , packetCount = 0}, --> the egress port
    {portNum = port4  , packetCount = 0}
}

--[[
1.  On the 'ingress port'
a.  enable the "tpid+vid" classification
b.  set priority for "tpid+vid" classification --> medium

2. set the egress port to egress packets with the same TPID as ingress.
]]--
local function  configFunc_1()
    local command_configString =
        [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort ..
        [[

        pip vid-ethertype-classification
        pip priority-vid-ethertype-classification medium
        exit
        ]]

    executeStringCliCommands(command_configString)
end

local function deConficFunc_1()
    local command_configString =
        [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort ..
        [[

        no pip vid-ethertype-classification
        exit
        ]]

    executeStringCliCommands(command_configString)
    executeStringCliCommands(command_restoreThresholds)
end
-- function to change the proper threshold
local function thresholdChangeFunc_1()
    executeStringCliCommands(command_threshold_medium_reset)
end
local sectionInfo_1_sectionName = "TPID+vid classifications - part-1"
local orig_sectionInfo_1 = {
    sectionName = sectionInfo_1_sectionName,
    purpose = "Test need to check that packet classified as 'vlan tag' for specific 'vid' " ..
              "and get pip-priority according to 'per-port' < priority for 'tpid+vid' classification>",
    transmitInfo = transmitInfo_1,
    egressInfoTable = egressInfoTable_1,
    configFunc = configFunc_1,
    deConficFunc = deConficFunc_1,
    thresholdChangeFunc = thresholdChangeFunc_1
}
local sectionInfo_1 = deepcopy(orig_sectionInfo_1)

--TPID+vid classifications
local function test_TPID_vid_classifications()
    printLog (delimiter)
    --vid 5
    sectionInfo_1.sectionName = sectionInfo_1_sectionName .. "with tpid 0x9100 , vid = 5 , up = 7"

    -- Set FROM CPU Traffic to be high(very high) priority to avoid drop on CPU Port
    if is_sip_6(devNum) then
        executeStringCliCommands(command_from_cpu_veryhigh_set_sip_6)
    else
        if is_sip_5_20(devNum) then
            executeStringCliCommands(command_from_cpu_veryhigh_set)
        else
            executeStringCliCommands(command_from_cpu_high_set)
        end
    end
    -->> test now
    testSection(sectionInfo_1)

    printLog (delimiter)
    --vid 4094
    local ingressPacket_2 = buildPacketVlanTag(0x9100 --[[tpid]] , 4094 --[[vid]] , 7 --[[up]])
    sectionInfo_1.transmitInfo.pktInfo.fullPacket = ingressPacket_2
    sectionInfo_1.egressInfoTable[1].pktInfo.fullPacket = ingressPacket_2

    sectionInfo_1.sectionName = sectionInfo_1_sectionName .. "with tpid 0x9100 , vid = 4094 , up = 7"
    -->> test now
    testSection(sectionInfo_1)

    -- restore FROM CPU Traffic to be low priority
    executeStringCliCommands(command_from_cpu_low_set)

    printLog (delimiter)
    -- keep the priority of port
    -- not enable 'tpid+vid' classification
    sectionInfo_1.sectionName = "TPID+vid classifications - part-2" .. " not enable 'tpid+vid' classification "
    sectionInfo_1.configFunc   = nil
    -- sectionInfo_1.deConficFunc = nil
    -- function to change the proper threshold
    if is_sip_5_20(devNum) then
        sectionInfo_1.thresholdChangeFunc =
            function()
                executeStringCliCommands(command_threshold_veryhigh_reset)
            end
    else
        sectionInfo_1.thresholdChangeFunc =
            function()
                executeStringCliCommands(command_threshold_high_reset)
            end
    end

    -->> test now
    testSection(sectionInfo_1)

    -- restore needed values for next run (iteration in counterModesDb[])
    sectionInfo_1 = deepcopy(orig_sectionInfo_1)
end

--##################################
--##################################
local vlan_tag = "vlan_tag"
local mpls = "mpls"
local ipv4 = "ipv4"
local ipv6 = "ipv6"
local ude = "ude"
local mac_da = "mac_da"
local dsa_tag_qos = "dsa_tag_qos"
local dsa_tag_up = "dsa_tag_up"

local function test_classifications( packetsToSend,sectionName)
    local pip_profile
    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = nil --[[filled in runtime]]} }
    local egressInfoTable = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        -- expect other ports not to get it
        {portNum = port1  , packetCount = 0},
        --{portNum = port2  , packetCount = 0}, --> the ingress port
        --{portNum = port3  , packetCount = 0}, --> the egress port
        {portNum = port4  , packetCount = 0}
    }
    local egressInfoTable_multicast = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        {portNum = port1  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        --{portNum = port2  }, --> the ingress port
        --{portNum = port3  }, --> the egress port
        {portNum = port4  , pktInfo = {fullPacket = nil --[[filled in runtime]]}}
    }
    -- function to change the proper threshold
    local function thresholdChangeFunc()
        if is_sip_5_20(devNum) then
            if pip_profile == 2 then
                executeStringCliCommands(command_threshold_veryhigh_reset)
            else -- pip_profile = 3
                executeStringCliCommands(command_threshold_high_reset)
            end
        else
            if pip_profile == 2 then
                executeStringCliCommands(command_threshold_high_reset)
            else -- pip_profile = 3
                executeStringCliCommands(command_threshold_medium_reset)
            end

        end
    end
    -- function to set configuration for the test
    local function configFunc(new_pip_profile)
        if new_pip_profile == nil then
            new_pip_profile = pip_profile
        end
        -- set pip-profile on the ingress port
        local command_portPipProfile = [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort .. [[

            pip profile ]] .. new_pip_profile .. [[

        exit
        ]]
        executeStringCliCommands(command_portPipProfile)
    end
    -- function to de-configuration for the test
    local function deConficFunc()
        -- restore pip profile
        --[[the orig pip profile from the configuration file]]
        local new_pip_profile = 2
        configFunc(new_pip_profile)
        executeStringCliCommands(command_restoreThresholds)
    end

    local sectionInfo = {
        sectionName = sectionName,
        purpose = "Test need to check that packet classified as expected " ..
                  "and get pip-priority according to 'per-profile' per 'field' (for this packet type)",
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTable,
        configFunc = configFunc,
        deConficFunc = deConficFunc,
        thresholdChangeFunc = thresholdChangeFunc
    }

    for index,entry in pairs(packetsToSend) do
        printLog (delimiter)

        if entry.type == mac_da then
            sectionInfo.egressInfoTable = egressInfoTable_multicast
        else
            sectionInfo.egressInfoTable = egressInfoTable
        end

        -- the ingress packet
        sectionInfo.transmitInfo.pktInfo.fullPacket = entry.packet
        if entry.cascade then
            generic_cscd_port_set(devNum, ingressPort,false,entry.cascade)
            sectionInfo.egressInfoTable[1].pktInfo = nil
            sectionInfo.egressInfoTable[1].packetCount = 1 -- we not want to check the content of packet that removed the DSA tag
        else
            -- expect egress packet .. unmodified (did 'simple' bridging)
            sectionInfo.egressInfoTable[1].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
            if entry.type == mac_da then
                sectionInfo.egressInfoTable[2].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
                sectionInfo.egressInfoTable[3].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
            end
        end

        if index == 1 then
            --startSimulationLog()
        end

        for ii = 2,3 do
            pip_profile = ii

            if is_sip_5_20(devNum) then
                if pip_profile == 2 then
                    pip_priority = "very-high"
                else
                    pip_priority = "high"

                    -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
                    if is_sip_6(devNum) then
                        executeStringCliCommands(command_from_cpu_veryhigh_set_sip_6)
                    else
                        executeStringCliCommands(command_from_cpu_veryhigh_set)
                    end
                end
            else
                if pip_profile == 2 then
                    pip_priority = "high"
                else
                    pip_priority = "medium"

                    -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
                    executeStringCliCommands(command_from_cpu_high_set)
                end

            end

            sectionInfo.sectionName = sectionName .. "--> currently :" .. entry.type ..
                                    " with 'field index' = " .. entry.value ..
                                    " on profile = " .. pip_profile .. " expected pip_priority = " .. pip_priority
            -->> test now
            testSection(sectionInfo)

            -- restore FROM CPU Traffic to be low priority
            executeStringCliCommands(command_from_cpu_low_set)

            -- debug specific part
            --break
        end

        if entry.cascade then
            generic_cscd_port_set(devNum, ingressPort,true--[[unset]])
        end

        -- debug specific part
        if index == 1 then
            --stopSimulationLog()
        end
        --break
    end
end

--packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da
local function test_non_dsa_classifications()
    local test2_packets = {
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] ,   15 --[[vid]] , 6 --[[up]])},
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] , 4093 --[[vid]] , 6 --[[up]])},
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] , 4095 --[[vid]] , 6 --[[up]])},
        {type = mpls     , value = 4           , packet = buildPacketMpls(4--[[exp]])},
        {type = ipv4     , value = 0xab        , packet = buildPacketIpv4(0xab--[[TOS]])},
        {type = ipv6     , value = 0xe7        , packet = buildPacketIpv6(0xe7--[[TC]])},
        {type = ude      , value = 2--[[index]], packet = buildPacketUde(0xabcd--[[etherType]])},
        {type = mac_da   , value = 1--[[index]], packet = buildPacketMacDaMc("0180C2000004")}, --01:80:C2:00:00:04
        {type = mac_da   , value = 3--[[index]], packet = buildPacketMacDaMc("01000C005566")}, --01:00:0C:00:55:66 --> Cisco macs
    }

    local sectionName = "packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da "

    test_classifications(test2_packets,sectionName)
end

--##################################
--##################################
--packet classifications - use default port's priority
local function test_default_port_priority(test_cpu_port)
    local pip_priority
    local ingressPacket = buildPacketUc(nil) -- etherType is 0x6666 from packetPayload

    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket} }
    local egressInfoTable = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = ingressPacket}},
        -- expect other ports not to get it
        {portNum = port1  , packetCount = 0},
        --{portNum = port2  , packetCount = 0}, --> the ingress port
        --{portNum = port3  , packetCount = 0}, --> the egress port
        {portNum = port4  , packetCount = 0}
    }

    local function convertTestPipPriorityToCpuPortPriority(pip_priority)
        local cpu_pip_priority
        if is_sip_5_20(devNum) then
            if pip_priority == "low" then
                cpu_pip_priority = "medium"
            elseif pip_priority == "medium" then
                cpu_pip_priority = "high"
            elseif pip_priority == "high" then
                cpu_pip_priority = "very-high"
            else --pip_priority == "very-high"
                cpu_pip_priority = "low"
            end
        else
            if pip_priority == "low" then
                cpu_pip_priority = "medium"
            elseif pip_priority == "medium" then
                cpu_pip_priority = "high"
            else --pip_priority == "high"
                cpu_pip_priority = "low"
            end
        end
        return cpu_pip_priority
    end

    -- function to set configuration for the test
    local function configFunc(new_pip_priority)
        local param_new_pip_priority = new_pip_priority
        if new_pip_priority == nil then
            new_pip_priority = pip_priority
        end
        -- set pip-profile on the ingress port
        local command_portPipPriority = [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort .. [[

            pip priority-default ]] .. new_pip_priority .. [[

        exit
        ]]
        executeStringCliCommands(command_portPipPriority)

        local configCpuPort , cpu_pip_priority
        if param_new_pip_priority == nil and pip_priority == "low" and not test_cpu_port then
            configCpuPort = true
            cpu_pip_priority = "high"
        elseif param_new_pip_priority == nil and test_cpu_port then
            configCpuPort = true
            cpu_pip_priority = convertTestPipPriorityToCpuPortPriority(pip_priority)
        else
            configCpuPort = false
        end

        if configCpuPort then
            -- the CPU port is also in 'low' priority
            -- so changing the 'low' threshold to 'drop' the packets
            -- will cause the packets not to reach from the cpu port to out 'ingress port'
            -- so move the CPU port out from the "low" --> to "high"
            -- use 'no trust' with default's port 'high'
            printLog("Set CPU port ".. cpuPort .. " with default priority : " .. cpu_pip_priority .. " and 'not trust' pip")
            local command_cpuPortPipPriority = [[
            end
            configure
            interface ethernet ${dev}/]] .. cpuPort .. [[

                pip priority-default ]] .. cpu_pip_priority .. [[

                no pip trust
            exit
            ]]
            executeStringCliCommands(command_cpuPortPipPriority)
        end

    end
    -- function to de-configuration for the test
    local function deConficFunc()
        -- restore port default priority
        --[[the orig pip priority from the configuration file]]
        local new_pip_priority = "low"
        configFunc(new_pip_priority)
        executeStringCliCommands(command_restoreThresholds)

        if pip_priority == "low" then
            -- the CPU port should restore pip configurations
            local command_cpuPortPipPriority = [[
            end
            configure
            interface ethernet ${dev}/]] .. cpuPort .. [[

                pip priority-default ]] .. "low" .. [[

            exit
            ]]
            executeStringCliCommands(command_cpuPortPipPriority)
        end

        if test_cpu_port then
            executeStringCliCommands(command_cpuPortPipTrust)
        end

    end
    -- function to change the proper threshold
    local function thresholdChangeFunc()
        local local_priority
        if test_cpu_port then
            local_priority = convertTestPipPriorityToCpuPortPriority(pip_priority)

            -- make sure that the CPU port is the one that dropped the packet
            if (isThresholdZeroDropPackets == true) then
               -- not valid for BC3,Aldrin2 test scenario
               transmitInfo.expectPacketFilteredBeforLoopback = true
            end

        else
            local_priority = pip_priority
        end

        if local_priority == "very-high" then --The value is impossible for <SIP 5.20
            executeStringCliCommands(command_threshold_veryhigh_reset)
        elseif local_priority == "high" then
            executeStringCliCommands(command_threshold_high_reset)
        elseif local_priority == "medium" then
            executeStringCliCommands(command_threshold_medium_reset)
        else -- low
            executeStringCliCommands(command_threshold_low_reset)
        end
    end

    local sectionInfo = {
        sectionName = "packet classifications - use default port's priority",
        purpose = "Test need to check that packet use default port's priority",
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTable,
        configFunc = configFunc,
        deConficFunc = deConficFunc,
        thresholdChangeFunc = thresholdChangeFunc
    }

    local orig_sectionName = sectionInfo.sectionName
    local pip_priority_list

    for index,entry in pairs(prioritySet) do
        pip_priority = entry

        sectionInfo.sectionName = orig_sectionName .. "--> currently : port's priority = " .. pip_priority
        if test_cpu_port then
            sectionInfo.sectionName = sectionInfo.sectionName .. "--> but CPU port tested with : " .. convertTestPipPriorityToCpuPortPriority(pip_priority)
        end

        if pip_priority == "medium" then
            -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
            executeStringCliCommands(command_from_cpu_high_set)
        end

        -->> test now
        testSection(sectionInfo)

        -- restore FROM CPU Traffic to be low priority
        executeStringCliCommands(command_from_cpu_low_set)

        transmitInfo.expectPacketFilteredBeforLoopback = nil-- restore value

        -- debug specific part
        --break
    end

    -- restore CPU Port trust
    executeStringCliCommands(command_cpuPortPipTrust)

end
--##################################
--##################################

-- sip6 - reset the latency-sensitive threshold value (to 0)
local command_threshold_latency_sensitive_reset =
[[
end
configure
pip threshold-types device ${dev} type latency-sensitive threshold 0
]]
-- sip6 - reset the latency-sensitive threshold value (to 65535)
local command_threshold_latency_sensitive_restore =
[[
end
configure
pip threshold-types device ${dev} type latency-sensitive threshold 65535
]]

local pfc_ether_type_bytes = "aaaa"
-- build packet for Latency Sensitive packet
-- the ethertype need to match the pfc/qcn of the 'pip_sip6_example' config file
local function  buildPacketLatencySensitive()
    return buildPacketUc(pfc_ether_type_bytes)
end
-- test the latency sensitive threshold
-- sip 6
local function test_latency_sensitive()
    local ingressPacket = buildPacketLatencySensitive()
    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket} }
    local egressInfoTable = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = ingressPacket}},
        -- expect other ports not to get it
        {portNum = port1  , packetCount = 0},
        --{portNum = port2  , packetCount = 0}, --> the ingress port
        --{portNum = port3  , packetCount = 0}, --> the egress port
        {portNum = port4  , packetCount = 0}
    }

    local function thresholdChangeFunc()
        -- set threshold to 0 .. to check that traffic go into this threshold
        executeStringCliCommands(command_threshold_latency_sensitive_reset)

        if (isThresholdZeroDropPackets == true) then
            -- the threshold 0 will cause the test not to be able to send traffic from the CPU to the ingress port
            transmitInfo.expectPacketFilteredBeforLoopback = true
        end
    end
    local function deConficFunc()
        -- set threshold to 0 .. to check that traffic go into this threshold
        executeStringCliCommands(command_threshold_latency_sensitive_restore)

        transmitInfo.expectPacketFilteredBeforLoopback = nil-- restore value
    end

    local sectionInfo = {
        sectionName = "",--single section ... no need for name
        purpose = "Test latency sensitive threshold",
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTable,
        configFunc = nil,
        deConficFunc = deConficFunc,
        thresholdChangeFunc = thresholdChangeFunc
    }

    -->> test now
    testSection(sectionInfo)

end
--##################################
--##################################

-- sip6 - set the tpid length
local command_tpid_length_set =
[[
end
configure
tpid device ${dev} direction ingress index 1 etherType 0xabab  tag-type 6-bytes-tag
tpid device ${dev} direction ingress index 2 etherType 0xbaba  tag-type 8-bytes-tag
tpid device ${dev} direction egress  index 1 etherType 0xabab  tag-type 6-bytes-tag
tpid device ${dev} direction egress  index 2 etherType 0xbaba  tag-type 8-bytes-tag
]]
..--do not let tag 0/1 recognition
  -- NOTE: using profile 5 , because can't use profile 6 as the 'TGF' use this profile internally !
[[
tpid ingress profile 5 device ${dev} ether_type_index 0 tpid-range 0,4,5,6,7
tpid ingress profile 5 device ${dev} ether_type_index 1 tpid-range 0,4,5,6,7
]]
..-- bind the ingress port to profile that NOT classify the TPIDs 1,2,3 as TAG 0/1
[[
interface ethernet ${dev}/${port[2]}
tpid ingress profile 5 ether_type_index 0 is_default true
tpid ingress profile 5 ether_type_index 1 is_default true
exit
]]
-- sip6 - reset the tpid length
local command_tpid_length_restore =
[[
end
configure
tpid device ${dev} direction ingress index 1 etherType 0x8100  tag-type 4-bytes-tag
tpid device ${dev} direction ingress index 2 etherType 0x8100  tag-type 4-bytes-tag
tpid device ${dev} direction egress  index 1 etherType 0x8100  tag-type 4-bytes-tag
tpid device ${dev} direction egress  index 2 etherType 0x8100  tag-type 4-bytes-tag

tpid ingress profile 6 device ${dev} ether_type_index 0 tpid-range 0-7
tpid ingress profile 6 device ${dev} ether_type_index 1 tpid-range 0-7

interface ethernet ${dev}/${port[2]}
tpid ingress profile 0 ether_type_index 0 is_default true
tpid ingress profile 0 ether_type_index 1 is_default true
exit
]]

local tag1 = "893f0000" --[[4 bytes]].. "11223344" --[[4 bytes]] --[[ total 8 bytes]]
local tag2 = "abab"     --[[2 bytes]].. "55667788" --[[4 bytes]] --[[ total 6 bytes]]
local tag3 = "baba0000" --[[4 bytes]].. "99aabbcc" --[[4 bytes]] --[[ total 8 bytes]]
-- build packet for tpid length
-- the packet hold 3 tags of :8,6,8 bytes before the ipv4 header
local function  buildIpv4PacketForTpidLength(tos)
    return buildPacketUc(tag1 .. tag2 .. tag3 .. buildHeaderIpv4(tos))
end

--packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da
local function test_tpid_length()
    local test4_packets = {
        {type = ipv4     , value = 0xab        , packet = buildIpv4PacketForTpidLength(0xab--[[TOS]])},
    }

    local sectionName = "packet classifications - Test TPID length : packet hold 3 tags of :8,6,8 bytes before the ipv4 header "

    executeStringCliCommands(command_tpid_length_set)

    test_classifications(test4_packets,sectionName)

    executeStringCliCommands(command_tpid_length_restore)
end

local function cascadePortModeGet(isEdsa)
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local cascadePortType
    if not isEdsa and srcHwDevNum<31 then
        cascadePortType = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    else
        cascadePortType = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    end
    
    return cascadePortType
end 

--##################################
--##################################
local function buildPacketDsaTag_FRW(isEdsa , qosProfile)
    local vlanId = 15
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local dsaTagType
    local dsa_egressPort = egressPort

    -- indicate the DSA info to allow use the 'e2phy' or not
    -- need to allow e2phy , when port number in the extDsa limited to 6 bits
    local dsa_isTrgPhyPortValid = isEdsa or egressPort < 64

    if not isEdsa and srcHwDevNum<31 then
        dsaTagType = "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"

        -- must limit the number of 'trgPort' bits to 6
        dsa_egressPort = egressPort % 64

    else
        dsaTagType = "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end

    local trgEport = dsa_egressPort
    local srcEPort = 0
    local origSrcPhyPort = 0
    local srcId = 0

    dsaTagStc = {
      dsaInfo={
        forward={
          dstEport=trgEport,
          source={
            portNum=srcEPort
          },
          tag0TpidIndex=0,
          phySrcMcFilterEnable=false,
          srcIsTagged="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          origSrcPhy={
            portNum=origSrcPhyPort
          },
          srcIsTrunk=false,
          egrFilterRegistered=true,
          qosProfileIndex = qosProfile,
          srcId=srcId,
          isTrgPhyPortValid = dsa_isTrgPhyPortValid,
          dstInterface={
            type="CPSS_INTERFACE_PORT_E",
            devPort={
              portNum=dsa_egressPort,
              devNum=trgHwDevNum
            }
          },
          srcHwDev=srcHwDevNum,
          skipFdbSaLookup    = true
        }
      },
      commonParams={
        dsaTagType=dsaTagType,
        vpt=1,
        vid=vlanId
      },
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
    }

    local dsaString = luaTgfDsaStringFromSTC(devNum,dsaTagStc)

    return buildPacketUc(dsaString)
end

local function buildPacketDsaTag_FROM_CPU(isEdsa , up)
    local vlanId = 15
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local dsaTagType
    local dsa_egressPort = egressPort
    -- indicate the DSA info to allow use the 'e2phy' or not
    -- need to allow e2phy , when port number in the extDsa limited to 6 bits
    local dsa_isTrgPhyPortValid = isEdsa or egressPort < 64

    if not isEdsa and srcHwDevNum<31 then
        dsaTagType = "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
        -- must limit the number of 'trgPort' bits to 6
        dsa_egressPort = egressPort % 64
    else
        dsaTagType = "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end

    local trgEport = dsa_egressPort
    local srcEPort = 0
    local origSrcPhyPort = 0
    local srcId = 0

    dsaTagStc={
        dsaInfo={
            fromCpu={
                  cascadeControl=true,
                  dp="CPSS_DP_GREEN_E",
                  tag0TpidIndex=7,
                  dstEport=0,
                  egrFilterRegistered=true,
                  isTrgPhyPortValid=dsa_isTrgPhyPortValid,
                  srcId=srcId,
                  srcHwDev=srcHwDevNum,
                  tc=7,
                  egrFilterEn=true,
                  dstInterface={
                    type="CPSS_INTERFACE_PORT_E",
                    devPort={
                      portNum=dsa_egressPort,
                      devNum=trgHwDevNum
                    }
                  },
            }
        },
        commonParams={
            dsaTagType=dsaTagType,
            vpt=up,
            vid=vlanId
        },
        dsaType="CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E"
    }

    local dsaString = luaTgfDsaStringFromSTC(devNum,dsaTagStc)

    return buildPacketUc(dsaString)

end

--packet classifications - dsa-qos,dsa-up
local function test_dsa_classifications()
--[[
a.  FRW DSA , 2 words , qos-profile = 0x64 , trgPort = egress port
b.  FRW DSA , 4 words , qos-profile = 0x64 , trgPort = egress port
c.  FROM_CPU DSA , 2 words , up = 5  , trgPort = egress port
d.  FROM_CPU DSA, 4 words , up = 5 , trgPort = egress port

]]--
    local test3_packets = {
        {type = dsa_tag_qos , value = 0x64 , packet = buildPacketDsaTag_FRW(true--[[eDsa]] , 0x64--[[qosProfile]])    ,cascade = cascadePortModeGet(true--[[eDsa]])},
        {type = dsa_tag_qos , value = 0x64 , packet = buildPacketDsaTag_FRW(false--[[EXT-dsa]] , 0x64--[[qosProfile]]),cascade = cascadePortModeGet(false--[[EXT-dsa]])},
        {type = dsa_tag_up  , value = 5    , packet = buildPacketDsaTag_FROM_CPU(true--[[eDsa]] , 5--[[up]])          ,cascade = cascadePortModeGet(true--[[eDsa]]) },
        {type = dsa_tag_up  , value = 5    , packet = buildPacketDsaTag_FROM_CPU(false--[[EXT-dsa]] , 5--[[up]])      ,cascade = cascadePortModeGet(false--[[EXT-dsa]]) },
    }

    local sectionName = "packet classifications - dsa-qos,dsa-up "
    
    if test3_packets[3].cascade == test3_packets[4].cascade then
   
        -- this happens when the DSA type is 'eDSA' also when wanted 'ext-dsa' due to hwDevNum > 31
        -- the hwDevNum is implied from 'randon' devNum
        printLog("The cascade type is the same as previous and we already reach drops , so skip second")
        test3_packets[4] = nil
        reduceExpectedFromCpuSet(2,2,1)
    end

    test_classifications(test3_packets,sectionName)

end
--##################################
--##################################
--[[
    need to check configurations by using next functions :
    NOTE: the 'config' file already did 'show' --> so counters should be 0
        so we can check counters only before we call the 'deconfig' that also calls 'show'

    -- for dev,port get drop counter :
    -- function returns isError , counter
    function pip_get_info_drop_port_counter(command_data, devNum, portNum)

    -- for dev get all 3 priority drop counter :
    -- function returns isError , counterPrioHigh,counterPrioMedium,counterPrioLow
    function pip_get_info_drop_global_counter(command_data,devNum)

    -- for dev,port get info :
    -- function returns isError , portInfo
    function pip_get_info_port(command_data, devNum, portNum)

    -- for dev get info :
    -- function returns isError , devInfo
    function pip_get_info_device(command_data, devNum)
--]]

-- check drop counters of test ports and global drop counters
local function expected_checkDropCounters()
    local command_data = Command_Data()
    local note_clear_on_read = "(check clear on read)."
    local sum_the_drop_counters = { perPort = 0 , perPriority = 0}

    if is_sip_6(devNum) then
        -- SIP6 Rx DMA debug counters not clear on read
        return
    end

    local checkedPorts
    if is_sip_6(devNum) then
        -- for sip6 we use 'counterModesDb' for counters check
        -- when we get here the global counter already read so will get here ZERO (cleared)
         checkedPorts = {
             {portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
         }
    elseif is_sip_5_20(devNum) then
       if (isSimulation) then
         checkedPorts = {
             {portNum = ingressPort , expectedValue = 0x21--[[33]]} -- empiric values we got ... and are ok
            ,{portNum = cpuPort     , expectedValue = 0x04        } -- empiric values we got ... and are ok
            ,{portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}

            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
         }
       else
         checkedPorts = {
             {portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}
            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
         }
       end
    else
        checkedPorts = {
             {portNum = ingressPort , expectedValue = 0x20--[[32]]} -- empiric values we got ... and are ok
            ,{portNum = cpuPort     , expectedValue = 0x03        } -- empiric values we got ... and are ok
            ,{portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}

            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
        }
    end

    printLog("Start check drop counters - per port : ")

    -- entry is from checkedPorts[]
    local function wrapper__pip_get_info_drop_port_counter(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = { l = {[0] = entry.expectedValue , [1] = 0}}
        sum_the_drop_counters.perPort = sum_the_drop_counters.perPort + entry.expectedValue
        local description = global_section.."port " .. portNum .." drop counters (at end of test)."
        if entry.note then
            description = description .. entry.note
        end

        local isError , cpssCounterLo, cpssCounterHi = internal____pip_get_info_drop_port_counter(command_data, devNum, portNum)

        if cpssCounterLo ~= entry.expectedValue then
            printLog("portNum",portNum,"entry.expectedValue",entry.expectedValue,"cpssCounterLo",cpssCounterLo)
        end


        local cpssCounter = { l = {[0] = cpssCounterLo , [1] = cpssCounterHi}}

        return isError , cpssCounter , expectedValue , description
    end
    --check array checkedPorts
    checkExpectedValues(command_data,wrapper__pip_get_info_drop_port_counter,checkedPorts)

    printLog("Ended check drop counters - per port . ")

    printLog("Start check drop counters - per priority : ")
    local checkGlobalDrops

    if is_sip_6(devNum) then
        -- for sip6 we use 'counterModesDb' for counters check
        -- when we get here the global counter already read so will get here ZERO (cleared)
         checkGlobalDrops = {
             {priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --very-high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 3 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
         }
    elseif is_sip_5_20(devNum) then
      if (isSimulation) then
        checkGlobalDrops = {
             {priority = 0 , expectedValue =  0x10} --very-high
            ,{priority = 1 , expectedValue =  0x0f} --high
            ,{priority = 2 , expectedValue =  0x04} --medium
            ,{priority = 3 , expectedValue =  0x02} --low

            ,{priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --very-high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 3 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
      else
        checkGlobalDrops = {
             {priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --very-high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 3 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
      end
    else
        checkGlobalDrops = {
             {priority = 0 , expectedValue =  0x10} --high
            ,{priority = 1 , expectedValue =  0x11} --medium
            ,{priority = 2 , expectedValue =  0x02} --low

            ,{priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
    end

    local saved_drop_global_counter = {}
    -- entry is from checkGlobalDrops[]
    local function wrapper__pip_get_info_drop_global_counter(command_data, entry)
        local priority = entry.priority
           -- "C" code for : is_sip_6(devNum) ?    pip_sip6_getPriorityName() :  pip_getPriorityName(priority)
        local priorityName = is_sip_6(devNum) and  pip_sip6_getPriorityName() or pip_getPriorityName(priority)

        local expectedValue = { l = {[0] = entry.expectedValue , [1] = 0} , name = priorityName}
        sum_the_drop_counters.perPriority = sum_the_drop_counters.perPriority + entry.expectedValue
        local description = global_section.."priority [" .. priorityName .."] drop counters (at end of test)."
        if entry.note then
            description = description .. entry.note
        end

        if(priority == 0) then -- priority is highest
            local counterPrioHighest
            local isError
            local counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow
            isError,counterPrioVeryhigh,counterPrioHigh,counterPrioMedium,counterPrioLow = internal____pip_get_info_drop_global_counter(command_data, devNum)
            if is_sip_5_20(devNum) then
                -- save values for the next calls
                saved_drop_global_counter[1] = counterPrioHigh
                saved_drop_global_counter[2] = counterPrioMedium
                saved_drop_global_counter[3] = counterPrioLow
                counterPrioHighest = counterPrioVeryhigh
            else
                -- save values for the next calls
                saved_drop_global_counter[1] = counterPrioMedium
                saved_drop_global_counter[2] = counterPrioLow
                counterPrioHighest = counterPrioHigh
            end

            return isError, counterPrioHighest, expectedValue, description
        else
            local isError = false

            if saved_drop_global_counter[priority].notValid then
                expectedValue.notValid = true
            end

            return isError, saved_drop_global_counter[priority], expectedValue, description
        end
    end

    --check array checkGlobalDrops
    checkExpectedValues(command_data,wrapper__pip_get_info_drop_global_counter,checkGlobalDrops)

    printLog("Ended check drop counters - per priority . ")

    if (isThresholdZeroDropPackets == true) then
      if sum_the_drop_counters.perPriority ~= sum_the_drop_counters.perPort then
          -- error the sum of all ports should be the same as the sum of the global.
          local error_string = "The sum of drops 'per port' [" .. sum_the_drop_counters.perPort .. "] not equal sum of drops 'per priority' [" .. sum_the_drop_counters.perPriority .."]"
          printLog ("ERROR : " .. error_string)
          testAddErrorString(error_string)
      end
    end

end

local function expected_checkValuesAfterConfigFile_perPort(command_data)
    local checkedPorts =
        {
            -- empiric values we got ... and are ok
             {portNum = ingressPort , expectedValue = {trusted = true,profile = 2,vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
             -- in sip6 by default all ports include CPU are 'not trusted'!
             -- so if not explicitly set to 'trust' , sip6 should expect cpuPort = 'not trusted'
            ,{portNum = cpuPort     , expectedValue = {trusted = not is_sip_6(devNum),vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port1       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port3       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port4       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
        }

    -- entry is from checkedPorts[]
    local function wrapper__pip_get_info_port(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.expectedValue
        local description = "port " .. portNum .." info (after config file)."

        local isError , info = pip_get_info_port(command_data, devNum, portNum)

        return isError , info , expectedValue , description
    end
    --check array checkedPorts
    checkExpectedValues(command_data,wrapper__pip_get_info_port,checkedPorts)

end
local function expected_checkValuesAfterConfigFile_global(command_data)
    local   macDaClassification={
      [0]={macAddrValue="01:80:c1:00:00:00",macAddrMask="ff:ff:ff:ff:00:00",dbName="mac-mc-IEEE-c1"},
          {macAddrValue="01:80:c2:00:00:00",macAddrMask="ff:ff:ff:00:00:00",dbName="mac-mc-IEEE-c2"},
          {macAddrValue="01:80:c3:00:00:00",macAddrMask="ff:00:ff:00:00:00",dbName="mac-mc-IEEE-c3"},
          {macAddrValue="01:00:0c:00:00:00",macAddrMask="ff:ff:ff:ff:00:00",dbName="mac-mc-CISCO"}
    }
    local vidClassification={
      [0]={vid=1,   etherType=0x8100},
          {vid=5,   etherType=0x8100},
          {vid=5,   etherType=0x9100},
          {vid=4094,etherType=0x9100}
      }
    local thresholds
    if is_sip_5_20(devNum) then
        thresholds ={
            [0] = {value=1023},
                  {value=1023},
                  {value=1023},
                  {value=1023}
        }
    else
        thresholds ={
            [0] = {value=1023},
                  {value=1023},
                  {value=1023}
        }

    end
    local udeClassification={
      [0]={etherType=0x6789,    dbName="ude0x6789"},
          {etherType=0x789A,    dbName="ude0x789a"},
          {etherType=0xabcd,    dbName="ude0xabcd"},
          {etherType=0xfedc,    dbName="ude0xfedc"}
    }

    local profiles = { [0] = {},{},{},{} } -- fill in runtime
    local checkedDev =
        {
            -- empiric values we got ... and are ok
             {macDaClassification = macDaClassification , vidClassification = vidClassification ,thresholds = thresholds,
                udeClassification = udeClassification ,
                profiles = profiles , enabled = true
             }
        }

    -- addition in sip6
    if is_sip_6(devNum) then
        local drop_counter_config = { countPip = true , countPbFl = false,
                                portNum             = "all",
                                priority_very_high  = true,
                                priority_high       = true,
                                priority_medium     = true,
                                priority_low        = true,
                                }
        local thresholds_by_name={ -- default thresholds (according to Cider)
             [1] = {name="latency sensitive"  ,value=65535}
            ,[2] = {name="pfc fill level low" ,value=65535}
            ,[3] = {name="pfc fill level high",value=65535}
            ,[4] = {name="pfc counter traffic",value=65535}
        }

        local  pfc_tc_vector={
             [1] = {name = "priority low"        , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_LOW_E"}
            ,[2] = {name = "priority medium"     , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_MEDIUM_E"}
            ,[3] = {name = "priority high"       , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_HIGH_E"}
            ,[4] = {name = "priority very high"  , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_VERY_HIGH_E"}
            ,[5] = {name = "no congestion"       , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_NO_CONGESTION_E"}
            ,[6] = {name = "heavy congestion"    , value = 0 , id = "CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E"}
        }

        local parserTpid={
           [0]={etherType=0x8100,tpidSize="CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E"}
          ,[1]={etherType=0x8100,tpidSize="CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E"}
          ,[2]={etherType=0x9100,tpidSize="CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E"}
          ,[3]={etherType=0x893f,tpidSize="CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E"}
        }

        local latency_sensitive_Classification={
          [0]={etherType=0xaaaa,    dbName="pfc"},
              {etherType=0xbbbb,    dbName="qcn"},
              {etherType=0,    dbName="no name"},-- not set by the test
              {etherType=0,    dbName="no name"} -- not set by the test
        }


        checkedDev[1].drop_counter_config = drop_counter_config
        checkedDev[1].thresholds_by_name  = thresholds_by_name
        checkedDev[1].pfc_tc_vector       = pfc_tc_vector
        checkedDev[1].parserTpid          = parserTpid
        checkedDev[1].latency_sensitive_Classification = latency_sensitive_Classification
    end



    -- set all values with default priority ('low')
    for profile = 0,3 do
        local perProfile = profiles[profile]
        for pip_type,max_value in pairs(pip_type_max_value) do
            perProfile[pip_type] = {}
            local perType = perProfile[pip_type]
            for index = 0,max_value do
                perType[index] = lowPriorityNum
            end
        end
    end

    -- override values:
--[[
    pip-priority --> specific
    1.  On pip-profile  = 2
        a.  Priority high for 'vlan-tag' for 'up' = 6 , 7
        b.  Priority high for 'mpls' for 'EXP' = 4
        c.  Priority high for 'ipv4' for 'TOS' = 0xab
        d.  Priority high for 'ipv6' for 'TC' = 0xe7
        e.  Priority high for 'ude' for 'index' = 2
        f.  Priority high for 'mac-da' for 'index' = 1 , 3
        g.  Priority high for 'dsa-qos' for 'qos' = 0x64
        h.  Priority high for 'dsa-up' for 'up' = 5

    2.  On pip-profile  = 3
        a.  Set like for profile 2 but with pip-priority medium
]]
    local ipv6ErratumImpact = (0xe7 % 64)
    local overridePriority = {
        -- type                 fieldIndex
        [VLAN_TAG_UP]           = { 6,      7   },
        [MPLS_EXP]              = { 4           },
        [IPV4_TOS]              = {0xab         },
        [IPV6_TC]               = {0xe7         },
        [UDE_INDEX]             = {2            },
        [MAC_DA_INDEX]          = {1,       3   },
        [DSA_QOS_PROFILE]       = {0x64         },
        [DSA_UP]                = {5            }
    }

    if pip_is_device_ipv6_erratum(devNum) then
        overridePriority[IPV6_TC] = {ipv6ErratumImpact + 0 , ipv6ErratumImpact + 64 , ipv6ErratumImpact+128,ipv6ErratumImpact+192 } -- due to erratum
    end

    for profile = 2,3 do
        local perProfile = profiles[profile]
        for pip_type,fields in pairs(overridePriority) do
            local perType = perProfile[pip_type]
            for jj,field in pairs(fields) do
                if profile == 2 then
                    perType[field] = 0 -- high (very high) priority
                else
                    perType[field] = 1 -- medium (high) priority
                end
            end
        end
    end

    -- entry is from checkedDev[]
    local function wrapper__pip_get_info_device(command_data, entry)
        local expectedValue = entry
        local description = "Global info (after config file)."

        local isError , info = pip_get_info_device(command_data, devNum)

        return isError , info , expectedValue , description
    end
    --check array checkedDev
    checkExpectedValues(command_data,wrapper__pip_get_info_device,checkedDev)
end

-- called after the 'config file' so we can check it's configurations
local function expected_checkValuesAfterConfigFile()
    local command_data = Command_Data()

    expected_checkValuesAfterConfigFile_perPort(command_data)
    expected_checkValuesAfterConfigFile_global(command_data)
end

local beforeTestStartedInfo_perPort = {
     {portNum = ingressPort , expectedValue = nil}
    ,{portNum = cpuPort     , expectedValue = nil}
    ,{portNum = port1       , expectedValue = nil}
    ,{portNum = port3       , expectedValue = nil}
    ,{portNum = port4       , expectedValue = nil}
}
local function learnValuesBeforeConfigFile_perPort(command_data)
    -- entry is from beforeTestStartedInfo_perPort[]
    for index,entry in pairs(beforeTestStartedInfo_perPort) do
        local portNum = entry.portNum
        local isError , info = pip_get_info_port(command_data, devNum, portNum)
        entry.expectedValue = info
    end
end
-- global DB that hold value frmo 'before' calling the config file
-- so we can compare it to values after the deconfig file
local beforeTestStartedInfo_global = {}
-- entry is from beforeTestStartedInfo_global[]
local function learnValuesBeforeConfigFile_global(command_data)
    local isError , info = pip_get_info_device(command_data, devNum)

    beforeTestStartedInfo_global[1] = info

end

-- called BEFORE any 'config file' so we can learn info 'before test'
local function learnValuesBeforeConfigFile()
    local command_data = Command_Data()

    learnValuesBeforeConfigFile_perPort(command_data)
    learnValuesBeforeConfigFile_global(command_data)
end

local function expected_checkValuesAfterDeconfigFile_perPort(command_data)
    -- entry is from beforeTestStartedInfo_perPort[]
    local function wrapper__pip_get_info_port(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.expectedValue
        local description = "port " .. portNum .." info (after deconfig file --> end of test)."

        local isError , info = pip_get_info_port(command_data, devNum, portNum)

        return isError , info , expectedValue , description
    end
    --check array beforeTestStartedInfo_perPort
    checkExpectedValues(command_data,wrapper__pip_get_info_port,beforeTestStartedInfo_perPort)
end

local function expected_checkValuesAfterDeconfigFile_global(command_data)
    -- entry is from beforeTestStartedInfo_global[]
    local function wrapper__pip_get_info_device(command_data, entry)
        local expectedValue = entry
        local description = "Global info (after deconfig file --> end of test)."

        local isError , info = pip_get_info_device(command_data, devNum)

        return isError , info , expectedValue , description
    end

    --check array beforeTestStartedInfo_global
    checkExpectedValues(command_data,wrapper__pip_get_info_device,beforeTestStartedInfo_global)
end

-- called after the 'deconfig file' so we can check it restored configurations
local function expected_checkValuesAfterDeconfigFile()
    local command_data = Command_Data()

    expected_checkValuesAfterDeconfigFile_perPort(command_data)
    expected_checkValuesAfterDeconfigFile_global(command_data)
end


--##################################
--##################################
local function generalInit()
    if is_sip_5_20(devNum) then
        executeStringCliCommands(command_allPortPipTrust);
        executeStringCliCommands(command_restoreThresholds);
    end

    xcat3x_start(true)
    -- called BEFORE any 'config file' so we can learn info 'before test'

    learnValuesBeforeConfigFile()

    if is_sip_5_20(devNum) then
        global_test_data_env.priority0 = "very-high";
        global_test_data_env.priority1 = "high";
        global_test_data_env.defaultThreshold = 65535;
    else
        global_test_data_env.priority0 = "high";
        global_test_data_env.priority1 = "medium";
        global_test_data_env.defaultThreshold = 1023;
    end
    executeLocalConfig(luaTgfBuildConfigFileName("pip_example"))
    if is_sip_6(devNum) then
        --additional configurations
        executeLocalConfig(luaTgfBuildConfigFileName("pip_sip6_example"))
    end

    executeStringCliCommands(command_set_egress_port_keep_tpid)
    executeStringCliCommands(command_set_ingress_port_pvid_15)
    if is_sip_5_20(devNum) then
        executeStringCliCommands(command_set_thr_veryhigh)
    end

    if is_sip_6(devNum) then
        -- enable the PIP counting

        -- function for sip6 to allow setting
        -- input : mode { portNum = value or nil , priority = value or nil}
        -- if mode.portNum = "all" and mode.priority = "all" then 'count all'
        -- if mode.portNum  then 'port mode'
        -- if mode.priority then 'priority mode'
        -- else disable the counting
        sip6_activate_pip_counter({portNum = "all" , priority = "all"})-- count all
    end

    -- called after the 'config file' so we can check it's configurations
    expected_checkValuesAfterConfigFile()

end

local function generalEnd()
    xcat3x_start(false)

    -- check drop counters of test ports and global drop counters
    expected_checkDropCounters()-- need to be before 'deconfig'

    executeStringCliCommands(command_restoreThresholds)
    executeStringCliCommands(command_set_egress_port_not_keep_tpid)
    executeStringCliCommands(command_set_ingress_port_pvid_1)
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName("pip_example",true))
    if is_sip_6(devNum) then
        --additional 'unset' configurations
        executeLocalConfig(luaTgfBuildConfigFileName("pip_sip6_example",true))
    end

    if is_sip_5_20(devNum) then
        executeStringCliCommands(command_set_dflt_thr_veryhigh)
    end

    if is_sip_6(devNum) then
        -- disable the PIP counting

        -- function for sip6 to allow setting
        -- input : mode { portNum = value or nil , priority = value or nil}
        -- if mode.portNum  then 'port mode'
        -- if mode.priority then 'priority mode'
        -- else disable the counting
        sip6_activate_pip_counter(nil)-- disable pip counting

        command_sip6_restorePortParserGlobalTpid()

    end

    -- called after the 'deconfig file' so we can check it restored configurations
    expected_checkValuesAfterDeconfigFile()

    -- print results summary
    testPrintResultSummary("PIP")
end

local counterModesDb
if is_sip_6(devNum) then
    local drops = {-- we expect next drops
        ingress_port = {very_high = 0 , high = 13 , medium = 3 , low = 2},
        cpu_port     = {very_high = 15 , high = 1  , medium = 1 , low = 5}
        }
    local total_drop =
                       drops.ingress_port.very_high +
                       drops.ingress_port.high +
                       drops.ingress_port.medium +
                       drops.ingress_port.low +
                       drops.cpu_port.very_high +
                       drops.cpu_port.high +
                       drops.cpu_port.medium +
                       drops.cpu_port.low
                       
    local very_high_and_high_drop =
                       drops.ingress_port.very_high +
                       drops.ingress_port.high +
                       drops.cpu_port.very_high +
                       drops.cpu_port.high
    local ingress_port_high_and_low_drop =
                       drops.ingress_port.high +
                       drops.ingress_port.low

    -- counters : the first run checked the total number of dropped packets
    -- but we want to check 'per priority' too
    counterModesDb = {
     [1] = {skip = false,forceSilence = false , section = "first-run:counters:count 'all': ",postRunFunc = sip6_check_pip_counter_wa1 , postRunParams = total_drop}
    ,[2] = {skip = false,forceSilence = true , section = "re-run:counters:very-high and high: ",preInitFunc = sip6_activate_pip_counter , preInitParams = {priority = "very-high",priority1 = "high"} , postRunFunc = sip6_check_pip_counter_wa2 , postRunParams = very_high_and_high_drop} --very-high , high
    -- set counter only on the 'ingress port' (without the counting drops on CPU port)
    ,[3] = {skip = false,forceSilence = true , section = "re-run:counters:high and low on ingress port:",preInitFunc = sip6_activate_pip_counter , preInitParams = {portNum = ingressPort,priority = "high",priority1 = "low"}  , postRunFunc = sip6_check_pip_counter_wa3 , postRunParams = ingress_port_high_and_low_drop} --high , low
    }
else
    counterModesDb = { [1] = "dummy"    }
end

--set config
printLog("--------start generic PIP configuration : ")
generalInit()

local function run_fullIteration(entry)

    if is_sip_6(devNum) then
        local reset_command =
        [[
            end
            configure
            counters rx-dma-debug reset ports all enable true
            end
        ]]
        executeStringCliCommands(reset_command)
    end

    if entry.forceSilence then
        -- give dummy printLog function
        printLog("FORCE to silent 'printLog' ")
        printLog = local_printLog
    end

    if entry.preInitFunc then
        entry.preInitFunc(entry.preInitParams)
    end

    if entry.section then
        global_section = entry.section
    end

    if allow_test_TPID_vid_classifications then
        --TPID+vid classifications
        test_TPID_vid_classifications()
    end
    if allow_test_non_dsa_classifications then
        --packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da
        test_non_dsa_classifications()
    end

    if allow_test_default_port_priority then
        -- use default port's priority
        test_default_port_priority(false)--not testing CPU port
    end

    if allow_test_dsa_classifications then
        --packet classifications - dsa-qos,dsa-up
        if (isThresholdZeroDropPackets == false) then
          disableCheckDropCounters = true
        end
        test_dsa_classifications()
        if (isThresholdZeroDropPackets == false) then
          disableCheckDropCounters = false
        end
    end

    if allow_test_default_port_priority_CPU_port then
        -- CPU port - use default port's priority
        test_default_port_priority(true)--testing CPU port
    end

    if is_sip_6(devNum) and allow_test_latency_sensitive then
        -- test the latency sensitive threshold
        test_latency_sensitive()
    end

    if is_sip_6(devNum) and allow_test_tpid_length then
        -- test the TPID length 4/6/8 bytes
        test_tpid_length()
    end

    if entry.postRunFunc then
        entry.postRunFunc(entry.postRunParams)
    end

    global_section = ""
    --restore 'default'
    if entry.forceSilence then
        --restore the printings
        printLog = orig_printLog

        printLog("Restored the option to do 'printLog' ")
    end
end

-- run all iterations
local function main_body()
    if #counterModesDb > 1 and transmit_operation_mode == "none" then
        transmit_operation_mode = "record"
    end

    for ii = 1,#counterModesDb do
        entry = counterModesDb[ii]
        --printLog("ii",to_string(ii))
        --printLog("entry",to_string(entry))
        
        if allow_test_counterModesDb and not allow_test_counterModesDb[ii] then
            entry.skip = true
        end

        if not entry.skip then
            if entry.section then
                printLog("Start of : " .. entry.section)
            end
            run_fullIteration(entry)
            if entry.section then
                printLog("End of : " .. entry.section)
            end
        else
            printLog("SKIPPED : " .. entry.section)
        end

        transmit_operation_mode = "reply"
        transmit_db_reply_index = 1
    end
end

-- run the 'main' body
main_body();

printLog("--------restore PIP configuration : ")
generalEnd()
printLog("-------- PIP test ended --- \n")
--restore ports
devEnv.port[1] = origPorts.ports[1]
devEnv.port[2] = origPorts.ports[2]
devEnv.port[3] = origPorts.ports[3]
devEnv.port[4] = origPorts.ports[4]
