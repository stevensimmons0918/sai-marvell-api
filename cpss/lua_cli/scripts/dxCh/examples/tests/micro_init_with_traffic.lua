--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* micro_init_with_traffic.lua
--*
--* DESCRIPTION:
--*       test simulate micro init load through pex and sanity traffic
--*
--*       purpose of example:
--*       1. Verify micro init load.
--*       2. Verify boot channel.Get version of micro init.
--*		  3. Verify traffic from CPU.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

local devFamily = wrlCpssDeviceFamilyGet(devNum)
local pexMode
local superImageName = "super_image_harrier.bin"
local fwImageName = "firmware_cm3_falcon.bin"
local traffigConfigFile = "cont_traf.txt"

--add here supported devices

local supportedDevices =
    {
        "CPSS_PP_FAMILY_DXCH_HARRIER_E"
    }

local testSupported = false

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

--check that device is supported

for i, supportedFamily in pairs(supportedDevices) do
    if devFamily == supportedFamily then
        testSupported=true
        break
    end
end

if testSupported==true then
 --check that device work with memory access via BAR0,BAR2
 pexMode = myGenWrapper("cpssHwDriverGenWmInPexModeGet",{})

 if pexMode ==0 then
    printLog("\n usePexLogic flag is not set.\n ")
    testSupported=false
 end
else
    printLog("\n Device family does not support this test.\n ")
end

if testSupported==true then
 if isSimulationUsed()==false or isGmUsed()==true then
  printLog("\n This test can run only on white model.\n ")
  testSupported=false
 end
end

if testSupported==true then
--check that super image exist
 if file_exists(superImageName)==false then
 printLog("\n File "..superImageName.." not found.")
 testSupported=false
 else
 printLog("\n File "..superImageName.." found.")
 end
end

if testSupported==true then
--check that fw image exist
 if file_exists(fwImageName)==false then
 printLog("\n File "..fwImageName.." not found.")
 testSupported=false
 else
 printLog("\n File "..fwImageName.." found.")
 end
end


if testSupported==false then
    setTestStateSkipped()
    return
else
    printLog("\n Test support check is done.Test can be executed.\n ")
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
     local rc, val
     local port0=16
     local port1=36

 --reset device
     printLog("1.Add token in order enable pex reset\n ")

    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","includePexInSwReset"},
        {"IN","GT_U32","value",1}
    })


    printLog("2.Remove device and reset system\n ")

    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })


    --[[printLog("3.Redirect MI log\n ")
    callWithErrorHandling("simCm3LogRedirectSet",{
            {"IN","GT_BOOL","redirectLogToStdout",1}
        })
        ]]--

    printLog("3.Load super image \n ")
    callWithErrorHandling("appDemoLoadSuperImageToDdr",{
            {"IN","GT_BOOL","runUboot",1}
        })

    printLog("4.Get MI version via boot channel.\n ")
    callWithErrorHandling("cm3BootChannelVersionGet",{
        })
    printLog("5.Configure profile 3\n ")
    callWithErrorHandling("cm3BootChannelSendRerunFilesCommand",{
            {"IN","GT_U32","shift",3}
        })



    printLog("6.Test traffic from CPU\n ")

    --check if we can configure traffic from CPU
    if file_exists(traffigConfigFile)==false then
     printLog(traffigConfigFile.." file not found.Skip traffic\n ")
    else
     printLog(traffigConfigFile.." file found.Trigering traffic via boot channel.\n ")
     callWithErrorHandling("cm3BootChannelTriggerTraffic",{
        })

    --check counters
        rc,val = callWithErrorHandling("cm3PortStatisticCounterInternalGet",{
            {"IN","GT_U32","macPort",port0},
            {"IN","GT_BOOL","rx",0},
            {"OUT","GT_U32","value"}
        })

        printLog(" Port "..port0.." sent frames "..val["value"])

        if val["value"] == 0 then
         setFailState()
         printLog("ERROR:Zero TX frames is unexpected for port "..port0)
        else
         printLog("OK: TX frames as expected for port "..port0)
        end

         rc,val = callWithErrorHandling("cm3PortStatisticCounterInternalGet",{
            {"IN","GT_U32","macPort",port0},
            {"IN","GT_BOOL","rx",1},
            {"OUT","GT_U32","value"}
        })

        printLog(" Port "..port0.." received frames "..val["value"])

            if val["value"] == 0 then
         setFailState()
         printLog("ERROR:Zero TX frames is unexpected for port "..port0)
        else
         printLog("OK: TX frames as expected for port "..port0)
        end

        rc,val = callWithErrorHandling("cm3PortStatisticCounterInternalGet",{
            {"IN","GT_U32","macPort",port1},
            {"IN","GT_BOOL","rx",0},
            {"OUT","GT_U32","value"}
        })

        printLog(" Port "..port1.." sent frames "..val["value"])

        if val["value"] == 0 then
         setFailState()
         printLog("ERROR:Zero TX frames is unexpected for port "..port1)
        else
         printLog("OK: TX frames as expected for port "..port1)
        end


    end


end

-- run the test
doTest()

--clean up
callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","includePexInSwReset"},
        {"IN","GT_U32","value",0}
    })

callWithErrorHandling("cpssReInitSystem",{})

