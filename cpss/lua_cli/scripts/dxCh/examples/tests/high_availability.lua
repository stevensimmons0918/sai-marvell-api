--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* high_availebility.lua
--*
--* DESCRIPTION:
--*       test simulate sw crash and high availability procedure that follow
--*
--*       purpose of example:
--*       1. Check that HW is not affected by HA procedure
--*       2. Check that test run after HA
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"
local simulation_log = "dxCh/examples/configurations/simulation_log.txt"
local show_fdb = "dxCh/examples/configurations/show_fdb.txt"
local simulation_log_stop = "dxCh/examples/configurations/simulation_log_stop.txt"

local devFamily = wrlCpssDeviceFamilyGet(devNum)
dofile ("dxCh/examples/common/high_availability_utils.lua")

--add here supported devices

local supportedDevices =
    {
        "CPSS_PP_FAMILY_DXCH_FALCON_E",
        "CPSS_PP_FAMILY_DXCH_ALDRIN2_E",
        "CPSS_PP_FAMILY_DXCH_ALDRIN_E",
        "CPSS_PP_FAMILY_DXCH_AC3X_E",
        "CPSS_PP_FAMILY_DXCH_AC5P_E",
        "CPSS_PP_FAMILY_DXCH_AC5X_E",
        "CPSS_PP_FAMILY_DXCH_AC5_E",
        "CPSS_PP_FAMILY_DXCH_HARRIER_E"
    }

local testSupported = false

-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
end

for i, supportedFamily in pairs(supportedDevices) do
    if devFamily == supportedFamily then
        testSupported=true
        break
    end
end

if testSupported==false then
    setTestStateSkipped()
    return
end

local value,rc

--check if PM active
rc,value = myGenWrapper("prvWrAppDbEntryGet",{
        {"IN","string","namePtr","portMgr"},
        {"OUT","GT_U32","valuePtr"}
    })

if val and value.valuePtr == 0 then
    setTestStateSkipped()
    return
end

local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       print("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end




-- run pre-test config
local function doTest()

    --save current port configuration before crash
    preTestSaveCurrentPorts()

    printLog("\nSimulate application crash.\n ")
    callWithErrorHandling(
        "appDemoHaEmulateSwCrash", {})

    local systemRecoveryInfo = {}

    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
    }


    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_INIT_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

     printLog("\nInit port configuration ")

    --call init system with same parameters it was initialized before
    callWithErrorHandling("cpssReInitSystem",{})

    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
    }


    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    --set config
    replayPorts()

    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
    }


    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\nStart event handler tasks\n ")
    callWithErrorHandling("appDemoEventRequestDrvnModeInit", {})

    printLog("\nEnable rx sdma \n ")
    callWithErrorHandling("cpssDxChNetIfRestore",{
            {"IN","GT_U32","devNum",devNum}
    })
    printLog("\nFlush FDB.\n ")

    executeLocalConfig(flush_fdb)

    -- run the tests:
--  1. 'sanity_send_and_capture.lua'
--  2. 'sanity_learn_mac.lua'
    runTest("sanity_send_and_capture.lua")
    executeLocalConfig(flush_fdb)
    runTest("sanity_learn_mac.lua")
    executeLocalConfig(flush_fdb)
end


-- run the test
doTest()




