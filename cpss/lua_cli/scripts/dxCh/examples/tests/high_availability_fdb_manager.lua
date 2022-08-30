--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* high_availebility_fdb_manager.lua
--*
--* DESCRIPTION:
--*       test simulate sw crash and high availability scenario for FDB Manager
--*
--*       purpose of example:
--*       1. Check that HW is not affected by HA procedure
--*       2. Check that test run after HA
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- SIP6 devices supported
SUPPORTED_FEATURE_DECLARE(devNum, "FDB_MANAGER_HA")

local devNum  = devEnv.dev
local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"

local devFamily = wrlCpssDeviceFamilyGet(devNum)
dofile ("dxCh/examples/common/high_availability_utils.lua")

-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
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

    local result, values
    local fdbManagerId = 5
    local systemRecoveryInfo = {}

    -- Explicit assignment of Device ID
    -- In case of falcon_6_4 - device Id coming as '1' not clearing bridge
    executeStringCliCommands("do clear bridge type "..devNum.." all")

    -- Store FDB AA and TA To CPU Status
    result, values = myGenWrapper("cpssDxChBrgFdbAAandTAToCpuGet", {
        { "IN",  "GT_U8",  "devNum", devNum },
        { "OUT", "GT_BOOL", "enablePtr" }
    })

    --save current port configuration before crash
    preTestSaveCurrentPorts()

    printLog("\nFDB Manager Init before Simulate crash\n")
    myGenWrapper("prvTgfBrgFdbManagerHighAvailability_Init",{
        {"IN",  "GT_U32",  "fdbManagerId",  fdbManagerId},
        {"IN",  "GT_BOOL", "isHaSyncUp",    false}
    })

    printLog("\nSimulate application crash.\n ")
    callWithErrorHandling(
        "appDemoHaEmulateSwCrash", {})


    --Set system recovery state INIT
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

    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_INIT_STATE_E ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\nInit with port configuration ")
    --call init system with same parameters it was initialized before
    callWithErrorHandling("cpssReInitSystem",{})

    --Set system recovery state catchup
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

    --set initial port config config
    replayPorts()

    printLog("\nFDB Manager Init After Simulated crash\n")
    myGenWrapper("prvTgfBrgFdbManagerHighAvailability_Init",{
        {"IN",  "GT_U32",  "fdbManagerId",  fdbManagerId},
        {"IN",  "GT_BOOL", "isHaSyncUp",    true}
    })

    printLog("\nFDB Manager HA sync UP\n")
    callWithErrorHandling("cpssDxChBrgFdbManagerHighAvailabilityEntriesSync",{
        {"IN",  "GT_U32",  "fdbManagerId", fdbManagerId}
    })

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
    -- verify FDB Manager content
    callWithErrorHandling(
        "prvCpssDxChBrgFdbManagerBackUpInstanceVerify",{
        {"IN",  "GT_U32", "fdbManagerId",  fdbManagerId}
    })

    -- Delete FDB Manager
    printLog("\n Delete FDB Manager.\n ")
    callWithErrorHandling(
        "cpssDxChBrgFdbManagerDelete",{
        {"IN",  "GT_U32", "fdbManagerId",  fdbManagerId}
    })

    printLog("\nFlush FDB.\n ")
    executeLocalConfig(flush_fdb)

    -- Restore FDB AA and TA To Cpu Status
    result, values = myGenWrapper("cpssDxChBrgFdbAAandTAToCpuSet", {
        { "IN",  "GT_U8",  "devNum",  devNum },
        { "IN",  "GT_BOOL", "enable", values["enablePtr"] }
    })

    printLog("\nRestore FDB Size\n")
    result, values = myGenWrapper("prvTgfBrgFdbManager_restoreFdbSize",{})


end


-- run the test
doTest()

