--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* high_availebility_vtcam_manager.lua
--*
--* DESCRIPTION:
--*       test simulate sw crash and high availability scenario for VTCAM Manager
--*
--*       purpose of example:
--*       1. Check that HW is not affected by HA procedure
--*       2. Check that test run after HA
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- void function without parameters clears all the TCAM
-- used in all Enhanced UT of VTCAM_HA to clean TCAM after tests leaving rules.
cmdLuaCLI_registerCfunction("wrlCpssClearAllTcam")

-- SIP6 devices supported
SUPPORTED_FEATURE_DECLARE(devNum, "VTCAM_MANAGER_HA")

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

    local systemRecoveryInfo = {}

    printLog("\n vTCAM Manager HA Test2 \n");

    printLog("\n vTCAM Manager HA Test2 Init before Simulation Crash \n");
    myGenWrapper("vtcamHaApp",{
        {"IN",  "GT_U8",    "devNum",           devNum},
        {"IN",  "GT_U8",    "createMngr",       1},
        {"IN",  "GT_U8",    "createVtcam",      1},
        {"IN",  "GT_U32",   "vtcamId",          0},
        {"IN",  "GT_U32",   "ruleSize",         5},
        {"IN",  "GT_U32",   "vtcamSize",        100},
        {"IN",  "GT_U32",   "numRules",         90},
        {"IN",  "GT_U8",    "vtcamIdOffset",    0},
        {"IN",  "GT_U32",   "ruleIdOffset",     0},
        {"IN",  "GT_U8",    "vtcamMode",        0},
        {"IN",  "GT_U32",   "priority",         0}
    })

    --save current port configuration before crash
    preTestSaveCurrentPorts()

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

    printLog("\n vTCAM Manager HA Test2 Config Replay after Simulation Crash \n");
    myGenWrapper("vtcamHaApp",{
        {"IN",  "GT_U8",    "devNum",           devNum},
        {"IN",  "GT_U8",    "createMngr",       1},
        {"IN",  "GT_U8",    "createVtcam",      1},
        {"IN",  "GT_U32",   "vtcamId",          0},
        {"IN",  "GT_U32",   "ruleSize",         5},
        {"IN",  "GT_U32",   "vtcamSize",        100},
        {"IN",  "GT_U32",   "numRules",         90},
        {"IN",  "GT_U8",    "vtcamIdOffset",    0},
        {"IN",  "GT_U32",   "ruleIdOffset",     0},
        {"IN",  "GT_U8",    "vtcamMode",        0},
        {"IN",  "GT_U32",   "priority",         0}
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

    --Set system recovery state completion
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

    printLog ("test add and remove of rules ")

    -- add 3 rules at the end of last tcam rule
    callWithErrorHandling(
            "vtcamHaApp",{
            {"IN", "GT_U8" , "devNum"       , devNum},
            {"IN", "GT_U8" , "createMngr"   , 0},
            {"IN", "GT_U8" , "createVtcam"  , 0},
            {"IN", "GT_U32", "vtcamId"      , 0},
            {"IN", "GT_U32", "ruleSize"     , 5},
            {"IN", "GT_U32", "vtcamSize"    , 100},
            {"IN", "GT_U32", "numRules"     , 3},
            {"IN", "GT_U8" , "vtcamIdOffset", 0},
            {"IN", "GT_U32", "ruleIdOffset" , 90},
            {"IN", "GT_U8" , "vtcamMode"    , 0},
            {"IN", "GT_U32", "priority"     , 0}
        })

    --same vTcamMngId as set during vtcamHaApp
    vTcamMngId = devNum % 31 -- CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS

    --delete old rule
    callWithErrorHandling(
            "cpssDxChVirtualTcamRuleDelete",{
            {"IN", "GT_U32" , "vTcamMngId"                  , vTcamMngId},
            {"IN", "GT_U32" , "vTcamId"                     , 0},
            {"IN", "CPSS_DXCH_VIRTUAL_TCAM_RULE_ID","ruleId", 50},
        })
    --delete new rule
    callWithErrorHandling(
            "cpssDxChVirtualTcamRuleDelete",{
            {"IN", "GT_U32" , "vTcamMngId"                  , vTcamMngId},
            {"IN", "GT_U32" , "vTcamId"                     , 0},
            {"IN", "CPSS_DXCH_VIRTUAL_TCAM_RULE_ID","ruleId", 91},
        })

    printLog("\n call prvCpssDxChVirtualTcamHaSystemRecoverySet - all config cleared from DB and TCAM\n")
    callWithErrorHandling(
        "prvCpssDxChVirtualTcamHaSystemRecoverySet",{
        {"IN", "GT_U8",                             "devNum",                devNum},
                -- vTcamHaApp(Test function) creates vTcamManager with ID = devNum, hence input for vTcamMngId should be kept as devNum
        {"IN", "GT_U32",                            "vTcamMngId",            devNum},
        {"IN", "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "systemRecoveryState",   2},
        {"IN", "CPSS_SYSTEM_RECOVERY_PROCESS_ENT",  "systemRecoveryProcess", 2},
        {"IN", "GT_U32",                            "tcamReset",             1}
    })

    printLog("\nCleanUp TCAM.\n ")
    wrlCpssClearAllTcam(devNum)
    printLog("\nFlush FDB.\n ")
    executeLocalConfig(flush_fdb)

--  sanity checks after HA:
--  1. 'sanity_send_and_capture.lua'
--  2. 'sanity_learn_mac.lua'
    runTest("sanity_send_and_capture.lua")
    executeLocalConfig(flush_fdb)
    runTest("sanity_learn_mac.lua")
    executeLocalConfig(flush_fdb)

end


-- run the test
doTest()

