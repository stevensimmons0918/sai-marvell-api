--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* granular_locking_profiler.lua
--*
--* DESCRIPTION:
--*     granular_locking_profiler: check granular locking rules violation
--*
--*     The test consist of two parts :
--*
--*     1.First part checks that violation counters are zero. Note that this test must run as the last test.
--*     2.Second part "breaks" granular locking rules and checks that the counters are incremented.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local ret, val,counterCheckFail,toolCheckFail
local savedViolationGlobalAfterDeviceSpecificCount,savedViolationRxTxConfigurationMixCount,savedViolationZeroLevelCount,savedViolationUnprotectedDbAccessCount
local savedViolationSystemRecoveryCount,savedViolationMangersMixCount,savedViolationMangersRxTxConfigurationMixCount
counterCheckFail = false
toolCheckFail = false

--PART ONE
ret, val = myGenWrapper (
                "osMutexCountersStatistic", {
                { "OUT", "GT_U32",  "violationGlobalAfterDeviceSpecificCountPtr"},
                { "OUT", "GT_U32",  "violationRxTxConfigurationMixCountPtr"},
                { "OUT", "GT_U32",  "violationZeroLevelCountPtr"},
                { "OUT", "GT_U32",  "violationUnprotectedDbAccessCountPtr"},
                { "OUT", "GT_U32",  "violationSystemRecoveryCountPtr"},
                { "OUT", "GT_U32",  "violationMangersMixCountPtr"},
                { "OUT", "GT_U32",  "violationMangersRxTxConfigurationMixCountPtr"}
        })


-- GT_NOT_IMPLEMENTED or  GT_NOT_SUPPORTED
if(ret== 0x10  or ret== 0x11)then
print("ret is "..ret)
else
    print("Before creating errors :\n==========================\n ")
    print("violationGlobalAfterDeviceSpecificCount is "..val.violationGlobalAfterDeviceSpecificCountPtr)
    print("violationRxTxConfigurationMixCount is "..val.violationRxTxConfigurationMixCountPtr)
    print("violationZeroLevelCount is "..val.violationZeroLevelCountPtr)
    print("violationUnprotectedDbAccessCount is "..val.violationUnprotectedDbAccessCountPtr)
    print("violationSystemRecoveryCount is "..val.violationSystemRecoveryCountPtr)
    print("violationMangersMixCount is "..val.violationMangersMixCountPtr)
    print("violationMangersRxTxConfigurationMixCount is "..val.violationMangersRxTxConfigurationMixCountPtr.."\n\n\n")

    if  val.violationGlobalAfterDeviceSpecificCountPtr>0 or val.violationRxTxConfigurationMixCountPtr>0 or
        val.violationZeroLevelCountPtr>0 or val.violationUnprotectedDbAccessCountPtr>0 or
        val.violationSystemRecoveryCountPtr>0 or val.violationMangersMixCountPtr>0 or val.violationMangersRxTxConfigurationMixCountPtr>0 then
        counterCheckFail = true
    end


--PART TWO
    savedViolationGlobalAfterDeviceSpecificCount = val.violationGlobalAfterDeviceSpecificCountPtr
    savedViolationRxTxConfigurationMixCount = val.violationRxTxConfigurationMixCountPtr
    savedViolationZeroLevelCount = val.violationZeroLevelCountPtr
    savedViolationUnprotectedDbAccessCount = val.violationUnprotectedDbAccessCountPtr
    savedViolationSystemRecoveryCount = val.violationSystemRecoveryCountPtr
    savedViolationMangersMixCount = val.violationMangersMixCountPtr
    savedViolationMangersRxTxConfigurationMixCount = val.violationMangersRxTxConfigurationMixCountPtr

    --check the tool
    ret, val = myGenWrapper (
                    "osMutexErrorsCreate", {
                    { "IN", "GT_BOOL",  "violation01error",true},
                    { "IN", "GT_BOOL",  "violation02error",true},
                    { "IN", "GT_BOOL",  "violation03error",true},
                    { "IN", "GT_BOOL",  "violation04error",true},
                    { "IN", "GT_BOOL",  "violation05error",true},
                    { "IN", "GT_BOOL",  "violation06error",true},
                    { "IN", "GT_BOOL",  "violation07error",true}

            })


    ret, val = myGenWrapper (
                "osMutexCountersStatistic", {
                { "OUT", "GT_U32",  "violationGlobalAfterDeviceSpecificCountPtr"},
                { "OUT", "GT_U32",  "violationRxTxConfigurationMixCountPtr"},
                { "OUT", "GT_U32",  "violationZeroLevelCountPtr"},
                { "OUT", "GT_U32",  "violationUnprotectedDbAccessCountPtr"},
                { "OUT", "GT_U32",  "violationSystemRecoveryCountPtr"},
                { "OUT", "GT_U32",  "violationMangersMixCountPtr"},
                { "OUT", "GT_U32",  "violationMangersRxTxConfigurationMixCountPtr"}
        })

    print("\nAfter creating errors :\n==========================\n ")
    print("violationGlobalAfterDeviceSpecificCount is "..val.violationGlobalAfterDeviceSpecificCountPtr)
    print("violationRxTxConfigurationMixCount is "..val.violationRxTxConfigurationMixCountPtr)
    print("violationZeroLevelCount is "..val.violationZeroLevelCountPtr)
    print("violationUnprotectedDbAccessCount is "..val.violationUnprotectedDbAccessCountPtr)
    print("violationSystemRecoveryCount is "..val.violationSystemRecoveryCountPtr)
    print("violationMangersMixCount is "..val.violationMangersMixCountPtr)
    print("violationMangersRxTxConfigurationMixCount is "..val.violationMangersRxTxConfigurationMixCountPtr.."\n\n\n")

    if  (val.violationGlobalAfterDeviceSpecificCountPtr - savedViolationGlobalAfterDeviceSpecificCount) ~= 1    or
        (val.violationRxTxConfigurationMixCountPtr - savedViolationRxTxConfigurationMixCount) ~= 1              or
        (val.violationZeroLevelCountPtr - savedViolationZeroLevelCount) ~= 1                                    or
        (val.violationUnprotectedDbAccessCountPtr - savedViolationUnprotectedDbAccessCount) ~= 1                or
        (val.violationSystemRecoveryCountPtr - savedViolationSystemRecoveryCount) ~= 1                          or
        (val.violationMangersMixCountPtr - savedViolationMangersMixCount) ~=1                                   or
        (val.violationMangersRxTxConfigurationMixCountPtr - savedViolationMangersRxTxConfigurationMixCount) ~=1 then
        toolCheckFail = true
    end

    print("\n\nResults :\n==========================\n ")
    print("counterCheckFail is "..tostring(counterCheckFail))
    print("toolCheckFail is "..tostring(toolCheckFail))

    if(counterCheckFail or toolCheckFail)then
    setFailState()
    end
end
