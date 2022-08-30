--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* initCLI.lua
--*
--* DESCRIPTION:
--*       cpssInitSystem and LuaCLI configuration commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-- ************************************************************************
--  initSystem
--        @description  The cpssInitSystem function
--
--        @param params         - params[boardParams][1] - boardIdx
--                - params[boardParams][2] - boardRevId
--                - params[boardParams][3] - reloadEeprom
--                              - noPorts will not initialize any port
--
--        @return       true if there was no error otherwise false
--

cmdLuaCLI_registerCfunction("luaCLI_getDevInfo")
cli_C_functions_registered("luaIsInclude_EZ_BRINGUP")

function initSystem(params)

    if params.noPorts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSerdesDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end
    if params.pexOnly ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initRegDefaults"},
            {"IN","GT_U32","value", 1}
        })
    else
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initRegDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end

    if params.onlyPhase1and2 ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSystemOnlyPhase1and2"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.disableInerrupts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSystemWithoutInterrupts"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.doublePhysicalPorts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "doublePhysicalPorts"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.apEnable ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","apEnable"},
            {"IN","GT_U32","value",1}
        })
    end

    if params.portMgr ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","portMgr"},
            {"IN","GT_U32","value",1}
        })
    end

    if params.legacyPort ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","legacyPort"},
            {"IN","GT_U32","value",1}
        })
    end

    if params.ptpMgr ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","ptpMgr"},
            {"IN","GT_U32","value",1}
        })
    end

    if params.fastBoot then
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end


    if params.ezb_xml then
        if 1 == luaIsInclude_EZ_BRINGUP() then
            ret,val = myGenWrapper("ezbXmlLoad",{
                {"IN", "string", "fileNamePtr", params.ezb_xml}
            })
            if ret ~= 0 --[[GT_OK]] then
                print ("Error : EZ_BRINGUP FAILD to load xml file : " .. to_string(params.ezb_xml))
                return false
            end
        else
            print ("Error : The image was not compiled with the 'EZ_BRINGUP' feature")
            return false
        end
    end


    if params.haBoot == nil and params.haBoot2 == nil and params.haBoot3 == nil then
        if params.numberTable == nil then
            -- indication to trigger 'auto init system'
            ret,val = myGenWrapper("autoInitSystem",{})
        else
            ret,val = myGenWrapper("cpssInitSystem",{
                {"IN","GT_U32","boardIdx",params["numberTable"][1]},
                {"IN","GT_U32","boardRevId",params["numberTable"][2]},
                {"IN","GT_U32","reloadEeprom",params["numberTable"][3]}
            })
        end
    end

    if ret == 0 and params.fastBoot then
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end

    if params.haBoot then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif params.haBoot2 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif params.haBoot3 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    end

    if params.haBoot ~= nil or params.haBoot2 ~= nil or params.haBoot3 ~= nil then
        if params.numberTable == nil then
            -- indication to trigger 'auto init syatem'
            ret,val = myGenWrapper("autoInitSystem",{})
        else
            ret,val = myGenWrapper("cpssInitSystem",{
                {"IN","GT_U32","boardIdx",params["numberTable"][1]},
                {"IN","GT_U32","boardRevId",params["numberTable"][2]},
                {"IN","GT_U32","reloadEeprom",params["numberTable"][3]}
            })
        end
    end

    if ret == 0 and params.haBoot then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif ret == 0 and params.haBoot2 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif ret == 0 and params.haBoot3 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    end

    local dev_list = wrlDevList();

    -- ini block for dxCh family only
    if dxCh_family == true then
      for dummy,dev in pairs(dev_list) do
          system_capability_managment({devID = dev});
      end
      print("Init system returned:"..to_string(ret))

    end

    -- init device specific global parameters
    fillDeviceEnvironment()

    if fs.exists("cmdDefs.lua") then
        require_safe("cmdDefs")
    end
    if fs.exists("pxCmdDefs.lua") then
        require_safe("pxCmdDefs")
    end
    if fs.exists("dxCmdDefs.lua") then
        require_safe("dxCmdDefs")
    end

    if (ret==0) then return true
    else return false end
end

local initSystemParams_named_params =
{
    type="named",
    { format="noPorts",name="noPorts",  help="Will not initialize any port during operation"},
    { format="pexOnly",name="pexOnly",  help="Initializing only pex access during cpss init"},
    { format="onlyPhase1and2",name="onlyPhase1and2",  help="Initializing only phase 1 and phase 2 (as minimal basic init)"},
    { format="disableInerrupts",name="disableInerrupts",  help="Will not initialize and bind interrupts during operation"},
    { format="doublePhysicalPorts",name="doublePhysicalPorts",  help="Bobcat3 : Initializing with 512 physical ports"},
    { format="apEnable",name="apEnable",  help="Initializing ap engine"},
    { format="portMgr",name="portMgr",  help="Declares if system is working in Port Manager mode"},
    { format="legacyPort",name="legacyPort",  help="Declares if system is working in Legacy Port mode"},
    { format="ptpMgr",name="ptpMgr",  help="Declares if system is working in PTP Manager mode or Legacy mode"},
    { format="fastBoot",name="fastBoot",  help="Try to connect to running AP"},
    { format="haBoot",name="haBoot",  help="HA test"},
    { format="haBoot2",name="haBoot2",  help="HA test"},
    { format="haBoot3",name="haBoot3",  help="HA test"},
}

local ezSystemParams_values_params =
{
    type="values",
    { format="%filename",name="ezb_xml",  help="ez_bringup XML file name(relative or full path)"}
}

local ezSystemParamsCap_values_params =
{
    type="values",
    { format="%filename",name="ezb_xml_cap",  help="ez_bringup XML file name(relative or full path)"}
}

-------------------------------------------------------
-- command registration: cpssInitSystem
-------------------------------------------------------
local initSystemParams = {
    func=initSystem,
    help="Init system",
    params={
        { type="values",
            { format="%init_system",name="numberTable",  help="enter configuration pattern"}
        },
        initSystemParams_named_params
    }
}


-------------------------------------------------------
-- command registration: cpssSystemBaseInit
-------------------------------------------------------

function initBaseInitSystem(params)
    if params.apEnable ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","apEnable"},
            {"IN","GT_U32","value",1}
        })
    end
    if params.noPorts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSerdesDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end
    if params.portMgr ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","portMgr"},
            {"IN","GT_U32","value",1}
        })
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSerdesDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end

    if params.ptpMgr ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","ptpMgr"},
            {"IN","GT_U32","value",1}
        })

        ret,val = myGenWrapper("prvCpssDxChPtpManagerEnableSet",{
            {"IN","GT_BOOL","enable",1}
        })
    end

    ret,val = myGenWrapper("cpssSystemBaseInit",{})
end


-------------------------------------------------------
-- command registration: cpssPpInsert
-------------------------------------------------------

function initPpAddSystem(params)
-- enabling fastboot
    if params.fastBoot then
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end
    if params.parentDevNum == nil then
        params.parentDevNum = 255;
    end
    if params.insertionType == nil then
        params.insertionType = 0;
    end
        ret,val = myGenWrapper("cpssPpInsert",{
            { "IN", "CPSS_PP_INTERFACE_CHANNEL_ENT",    "busType",   params.busType },
            { "IN", "GT_U8",    "busNum",   params.busNum },
            { "IN", "GT_U8",    "busDevNum",   params.busDevNum },
            { "IN", "GT_U8",    "cpssDevNum",   params.cpssDevNum },
            { "IN", "GT_U8",    "parentDevNum",   params.parentDevNum },
            { "IN", "GT_U8",    "insertionType",   params.insertType }})
    if ret==0 and params.fastBoot then
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false,
                        ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
                        haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E" },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end
    local dev_list = wrlDevList();

    -- ini block for dxCh family only
    if dxCh_family == true then
      for dummy,dev in pairs(dev_list) do
          system_capability_managment({devID = dev});
      end
      print("PP Insert returned:"..to_string(ret))
    end

    -- init device specific global parameters
    fillDeviceEnvironment()

    if fs.exists("cmdDefs.lua") then
        require_safe("cmdDefs")
    end

    if (ret==0) then return true
    else return false end
end

-------------------------------------------------------
-- command registration: cpssPciProvision
-------------------------------------------------------
function pciProvisionSet(params)
    local pciDeviceInfoPtr={}
    pciDeviceInfoPtr.pciBus = params.pciBusNum
    pciDeviceInfoPtr.pciDev = params.pciDevNum
    pciDeviceInfoPtr.pciFunc = params.pciFunc
    pciDeviceInfoPtr.devType = params.devType
    pciDeviceInfoPtr.swDevId = params.swDevId
    pciDeviceInfoPtr.hwDevId = params.hwDevId
    pciDeviceInfoPtr.pciHeaderInfo = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

    pciDeviceInfoPtr.pciHeaderInfo[0] = 0
    if params.pciHeaderInfo0 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[0] = params.pciHeaderInfo0
    end
    pciDeviceInfoPtr.pciHeaderInfo[1] = 0
    if params.pciHeaderInfo1 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[1] = params.pciHeaderInfo1
    end
     pciDeviceInfoPtr.pciHeaderInfo[2] = 0
    if params.pciHeaderInfo2 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[2] = params.pciHeaderInfo2
    end
    pciDeviceInfoPtr.pciHeaderInfo[3] = 0
    if params.pciHeaderInfo3 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[3] = params.pciHeaderInfo3
    end
    pciDeviceInfoPtr.pciHeaderInfo[4] = 0
    if params.pciHeaderInfo4 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[4] = params.pciHeaderInfo4
    end
    pciDeviceInfoPtr.pciHeaderInfo[5] = 0
    if params.pciHeaderInfo5 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[5] = params.pciHeaderInfo5
    end
    pciDeviceInfoPtr.pciHeaderInfo[6] = 0
    if params.pciHeaderInfo6 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[6] = params.pciHeaderInfo6
    end
    pciDeviceInfoPtr.pciHeaderInfo[7] = 0
    if params.pciHeaderInfo7 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[7] = params.pciHeaderInfo7
    end
    pciDeviceInfoPtr.pciHeaderInfo[8] = 0
    if params.pciHeaderInfo8 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[8] = params.pciHeaderInfo8
    end
    pciDeviceInfoPtr.pciHeaderInfo[9] = 0
    if params.pciHeaderInfo9 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[9] = params.pciHeaderInfo9
    end
    pciDeviceInfoPtr.pciHeaderInfo[10] = 0
    if params.pciHeaderInfo10 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[10] = params.pciHeaderInfo10
    end
    pciDeviceInfoPtr.pciHeaderInfo[11] = 0
    if params.pciHeaderInfo11 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[11] = params.pciHeaderInfo11
    end
    pciDeviceInfoPtr.pciHeaderInfo[12] = 0
    if params.pciHeaderInfo12 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[12] = params.pciHeaderInfo12
    end
    pciDeviceInfoPtr.pciHeaderInfo[13] = 0
    if params.pciHeaderInfo13 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[13] = params.pciHeaderInfo13
    end
    pciDeviceInfoPtr.pciHeaderInfo[14] = 0
    if params.pciHeaderInfo14 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[14] = params.pciHeaderInfo14
    end
    pciDeviceInfoPtr.pciHeaderInfo[15] = 0
    if params.pciHeaderInfo15 ~= nil then
       pciDeviceInfoPtr.pciHeaderInfo[15] = params.pciHeaderInfo15
    end


ret,val = myGenWrapper("appDemoBusDevInfo",{
          {"IN","APP_DEMO_INIT_DEV_INFO_STC","pciDeviceInfoPtr",pciDeviceInfoPtr},
          {"IN","GT_U32","sizeOfArray",1}})
end



-------------------------------------------------------
-- command registration: ha2PhasesInitDataSet
-------------------------------------------------------
function ha2PhasesInitDataSet(params)
    local ha2PhasesPhase1ParamsPhase1Ptr={}
    ha2PhasesPhase1ParamsPhase1Ptr.pciBus = params.pciBusNum
    ha2PhasesPhase1ParamsPhase1Ptr.pciDev = params.pciDevNum
    ha2PhasesPhase1ParamsPhase1Ptr.pciFunc = params.pciFunc
    ha2PhasesPhase1ParamsPhase1Ptr.devType = params.devType

    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[0] = 0
    if params.pciHeaderInfo0 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[0] = params.pciHeaderInfo0
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[1] = 0
    if params.pciHeaderInfo1 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[1] = params.pciHeaderInfo1
    end
     ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[2] = 0
    if params.pciHeaderInfo2 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[2] = params.pciHeaderInfo2
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[3] = 0
    if params.pciHeaderInfo3 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[3] = params.pciHeaderInfo3
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[4] = 0
    if params.pciHeaderInfo4 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[4] = params.pciHeaderInfo4
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[5] = 0
    if params.pciHeaderInfo5 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[5] = params.pciHeaderInfo5
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[6] = 0
    if params.pciHeaderInfo6 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[6] = params.pciHeaderInfo6
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[7] = 0
    if params.pciHeaderInfo7 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[7] = params.pciHeaderInfo7
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[8] = 0
    if params.pciHeaderInfo8 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[8] = params.pciHeaderInfo8
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[9] = 0
    if params.pciHeaderInfo9 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[9] = params.pciHeaderInfo9
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[10] = 0
    if params.pciHeaderInfo10 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[10] = params.pciHeaderInfo10
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[11] = 0
    if params.pciHeaderInfo11 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[11] = params.pciHeaderInfo11
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[12] = 0
    if params.pciHeaderInfo12 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[12] = params.pciHeaderInfo12
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[13] = 0
    if params.pciHeaderInfo13 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[13] = params.pciHeaderInfo13
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[14] = 0
    if params.pciHeaderInfo14 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[14] = params.pciHeaderInfo14
    end
    ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[15] = 0
    if params.pciHeaderInfo15 ~= nil then
       ha2PhasesPhase1ParamsPhase1Ptr.pciHeaderInfo[15] = params.pciHeaderInfo15
    end
    ha2PhasesPhase1ParamsPhase1Ptr.deviceRevision = 3
    ha2PhasesPhase1ParamsPhase1Ptr.coreClock = 520
    ha2PhasesPhase1ParamsPhase1Ptr.maxLengthSrcIdInFdbEn = true
    ha2PhasesPhase1ParamsPhase1Ptr.tag1VidFdbEn = true
    ha2PhasesPhase1ParamsPhase1Ptr.portVlanfltTabAccessMode = 300
    ha2PhasesPhase1ParamsPhase1Ptr.portIsolationLookupPortBits = 500
    ha2PhasesPhase1ParamsPhase1Ptr.portIsolationLookupDeviceBits = 700
    ha2PhasesPhase1ParamsPhase1Ptr.portIsolationLookupTrunkBits = 400
    ha2PhasesPhase1ParamsPhase1Ptr.portIsolationLookupTrunkIndexBase = 600
    ret,val = myGenWrapper("appDemoSystemRecoveryHa2phasesInitDataSet",{
        {"IN", "GT_U8", "devNum", params.swDevId},
        {"IN","CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC","ha2PhasesPhase1ParamsPhase1",ha2PhasesPhase1ParamsPhase1Ptr}})
end


-------------------------------------------------------
-- command registration: cpssPpRemove
-------------------------------------------------------
function initPpRemoveSystem(params)
    if params.removalType == nil then
        params.removalType = 0;
    end
        ret,val = myGenWrapper("cpssPpRemove",{
            { "IN", "GT_U8",    "cpssDevNum",   params.cpssDevNum },
            { "IN", "GT_U8",  "removalType",  params.removalType }})
end



-------------------------------------------------------
-- command registration: cpssPpShowDevices
-------------------------------------------------------

function cpssShowPpDevices(params)
        ret,val = myGenWrapper("cpssPpShowDevices", {})
end


-------------------------------------------------------
-- command registration: cpssLspci
-------------------------------------------------------

function cpssLspci(params)
        ret,val = myGenWrapper("cpssLspci", {})
end


-------------------------------------------------------
-- command registration: cpssLsSmi
-------------------------------------------------------

function cpssLsSmi(params)
        ret,val = myGenWrapper("cpssLsSmi", {
            { "IN", "GT_U8",    "smiBus",   params.smiBus }})
end


-------------------------------------------------------
-- command registration: cpssPciRescan
-------------------------------------------------------

function cpssPciRescan(params)
        ret,val = myGenWrapper("cpssPciRescan", {})
end


-------------------------------------------------------
-- command registration: cpssPciRemove
-------------------------------------------------------

function cpssPciRemove(params)
        ret,val = myGenWrapper("cpssPciRemove",{
            { "IN", "GT_U8",    "pciBus",   params.busNum },
            { "IN", "GT_U8",    "pciDev",   params.devNum }})
end


-------------------------------------------------------
-- command registration: autoInitSystem
-------------------------------------------------------
local autoSystemParams = {
    func=initSystem,
    help="Auto Init system (determines init params according to PCI/PEX scanning)",
    params={
        initSystemParams_named_params
    }
}

-------------------------------------------------------
-- command registration: ezautoInitSystem
-------------------------------------------------------
local ezAutoSystemParams = {
    func=initSystem,
    help="Auto Init system with ez_bringup XML",
    params={
        ezSystemParams_values_params,
        initSystemParams_named_params
    }
}

-------------------------------------------------------
-- command registration: ezcpssInitSystem
-------------------------------------------------------
local ezInitSystemParams = {
    func=initSystem,
    help="Init system with ez_bringup XML",
    params={
        { type="values",
            { format="%init_system",name="numberTable",  help="enter configuration pattern"}
        },
        ezSystemParams_values_params,
        initSystemParams_named_params
    }
}



-- WITHOUT lower case to allow writing 'auto' and pressing 'tab' to get the command running
-- CLI_addCommand("exec", "autoinitsystem", autoSystemParams)

function CLI_addAppDemoCommands()

    ret,val = myGenWrapper("cpssEnablerIsAppPlat",{})
    if ret == 0 then
       return
    end

    CLI_addCommand("exec", "cpssInitSystem", initSystemParams)
    -- also with lower case... just to make it easier
    CLI_addCommand("exec", "cpssinitsystem", initSystemParams)

    CLI_addCommand("exec", "cpssSystemBaseInit",
    {
        func=initBaseInitSystem,
        help="Init CPSS Base system- device independent",
        params={{type="named",
                {format="apEnable",name="apEnable",  help="Initializing ap engine"},
                {format="noPorts",name="noPorts",  help="Will not initialize any port during operation"},
                {format="portMgr",name="portMgr",  help="Declares if system is working in Port Manager mode or Legacy mode"},
                {format="ptpMgr",name="ptpMgr",  help="Declares if system is working in PTP Manager mode or Legacy mode"}
                }}
    })

    CLI_addCommand("exec", "cpssPpInsert",
    {
        func=initPpAddSystem,
        help="Insert Packet Processor",
        params={{type="named",
              {format="busType %GT_U8",          name="busType",      help="Interface type connected to CPU. PCI -> 0. SMI -> 1"},
                  {format="busNum %GT_U8",           name="busNum",       help="PCI/SMI Bus Number"},
                  {format="busDevNum %GT_U8",        name="busDevNum",    help="PCI/SMI device Number"},
                  {format="cpssDevNum %hirDevNum",   name="cpssDevNum",   help="cpss logical Device Number"},
                  {format="cpssParentDevNum %GT_U8", name="parentDevNum", help="cpss Device Number of the device to which the current device is connected. 0xff if connected to CPU"},
                  {format="insertionType %GT_U8",    name="insertType",   help="PP initialization Option. full-Init -> 0."},
                  {format="fastBoot",                name="fastBoot",     help="PP Insert in correspondence to EPROM/Micro-init based Initialization"},
                  mandatory = {"busType", "busNum", "busDevNum", "cpssDevNum"}
               }}
    })

    CLI_addCommand("exec", "cpssPpRemove",
    {
        func=initPpRemoveSystem,
        help="Remove Packet Processor",
        params={{type="named",
                  {format="cpssDevNum %hirDevNum", name="cpssDevNum",  help="cpss logical Device Number of the device"},
                  {format="removalType %GT_U8",    name="removalType", help="Removal Type. Managed Removal -> 0, unManagedRemoval -> 1, managedReset -> 2"},
                  mandatory = {"cpssDevNum"}
               }}
    })

    CLI_addCommand("exec", "cpssPpShowDevices",
    {
        func=cpssShowPpDevices,
        help="Lists the Pp devices and PCI info",
        params={}
    })

    CLI_addCommand("exec", "cpssLspci",
    {
        func=cpssLspci,
        help="Lists the PCI devices",
        params={}
    })

    CLI_addCommand("exec", "cpssLsSmi",
    {
        func=cpssLsSmi,
        help="Lists the SMI devices",
        params={{type="named",
                {format="smiBus %GT_U8", name="smiBus", help="SMI bus Number"},
                mandatory = {"smiBus"}
               }}
    })

    CLI_addCommand("exec", "cpssPciRescan",
    {
        func=cpssPciRescan,
        help="rescan Pex devices",
        params={}
    })

    CLI_addCommand("exec", "cpssPciRemove",
    {
        func=cpssPciRemove,
        help="Remove Pex device from sysfs",
        params={{type="named",
                  {format="pciBus %GT_U8", name="busNum", help="PCI Bus Number"},
                  {format="pciDev %GT_U8", name="devNum", help="PCI Device Number"},
                  mandatory = {"busNum", "devNum"}
               }}
    })
    CLI_addCommand("exec", "cpssPciProvision",
    {
        func=pciProvisionSet,
        help="Insert Pci data of needed device, device number and hw device number",
        params={{type="named",
                  {format="pciBusNum %GT_U8",                  name="pciBusNum",           help="PCI Bus Number"},
                  {format="pciDevNum %GT_U8",                  name="pciDevNum",           help="PCI device Number"},
                  {format="pciFunc %GT_U8",                    name="pciFunc",             help="PCI device function number"},
                  {format="deviceType %GT_U32",                name="devType",             help="Prestera PCI device Id"},
                  {format="cpssDevNum %GT_U8",                 name="swDevId",             help="cpss logical Device Number"},
                  {format="cpssHwDevNum %hirDevNum",           name="hwDevId",             help="cpss hardware Device Number"},
                  {format="pciHeaderInfo0 %GT_U32",            name="pciHeaderInfo0",      help="pci header info 1 word"},
                  {format="pciHeaderInfo1 %GT_U32",            name="pciHeaderInfo1",      help="pci header info 2 word"},
                  {format="pciHeaderInfo2 %GT_U32",            name="pciHeaderInfo2",      help="pci header info 3 word"},
                  {format="pciHeaderInfo3 %GT_U32",            name="pciHeaderInfo3",      help="pci header info 4 word"},
                  {format="pciHeaderInfo4 %GT_U32",            name="pciHeaderInfo4",      help="pci header info 5 word"},
                  {format="pciHeaderInfo5 %GT_U32",            name="pciHeaderInfo5",      help="pci header info 6 word"},
                  {format="pciHeaderInfo6 %GT_U32",            name="pciHeaderInfo6",      help="pci header info 7 word"},
                  {format="pciHeaderInfo7 %GT_U32",            name="pciHeaderInfo7",      help="pci header info 8 word"},
                  {format="pciHeaderInfo8 %GT_U32",            name="pciHeaderInfo8",      help="pci header info 9 word"},
                  {format="pciHeaderInfo9 %GT_U32",            name="pciHeaderInfo9",      help="pci header info 10 word"},
                  {format="pciHeaderInfo10 %GT_U32",           name="pciHeaderInfo10",     help="pci header info 11 word"},
                  {format="pciHeaderInfo11 %GT_U32",           name="pciHeaderInfo11",     help="pci header info 12 word"},
                  {format="pciHeaderInfo12 %GT_U32",           name="pciHeaderInfo12",     help="pci header info 13 word"},
                  {format="pciHeaderInfo13 %GT_U32",           name="pciHeaderInfo13",     help="pci header info 14 word"},
                  {format="pciHeaderInfo14 %GT_U32",           name="pciHeaderInfo14",     help="pci header info 15 word"},
                  {format="pciHeaderInfo15 %GT_U32",           name="pciHeaderInfo15",     help="pci header info 16 word"},

                  mandatory = {"pciBusNum", "pciDevNum", "pciFunc", "devType", "swDevId", "hwDevId"}
               }}
    })

    CLI_addCommand("exec", "ha2PhasesInitDataSet",
    {
        func=ha2PhasesInitDataSet,
        help="Insert HA two phases init data for given device number",
        params={{type="named",
                  {format="pciBusNum %GT_U32",                  name="pciBusNum",           help="PCI Bus Number"},
                  {format="pciDevNum %GT_U32",                  name="pciDevNum",           help="PCI device Number"},
                  {format="pciFunc %GT_U32",                    name="pciFunc",             help="PCI device function number"},
                  {format="deviceType %GT_U32",                name="devType",             help="Prestera PCI device Id"},
                  {format="cpssDevNum %GT_U8",                 name="swDevId",             help="cpss logical Device Number"},
                  {format="pciHeaderInfo0 %GT_U32",            name="pciHeaderInfo0",      help="pci header info 1 word"},
                  {format="pciHeaderInfo1 %GT_U32",            name="pciHeaderInfo1",      help="pci header info 2 word"},
                  {format="pciHeaderInfo2 %GT_U32",            name="pciHeaderInfo2",      help="pci header info 3 word"},
                  {format="pciHeaderInfo3 %GT_U32",            name="pciHeaderInfo3",      help="pci header info 4 word"},
                  {format="pciHeaderInfo4 %GT_U32",            name="pciHeaderInfo4",      help="pci header info 5 word"},
                  {format="pciHeaderInfo5 %GT_U32",            name="pciHeaderInfo5",      help="pci header info 6 word"},
                  {format="pciHeaderInfo6 %GT_U32",            name="pciHeaderInfo6",      help="pci header info 7 word"},
                  {format="pciHeaderInfo7 %GT_U32",            name="pciHeaderInfo7",      help="pci header info 8 word"},
                  {format="pciHeaderInfo8 %GT_U32",            name="pciHeaderInfo8",      help="pci header info 9 word"},
                  {format="pciHeaderInfo9 %GT_U32",            name="pciHeaderInfo9",      help="pci header info 10 word"},
                  {format="pciHeaderInfo10 %GT_U32",           name="pciHeaderInfo10",     help="pci header info 11 word"},
                  {format="pciHeaderInfo11 %GT_U32",           name="pciHeaderInfo11",     help="pci header info 12 word"},
                  {format="pciHeaderInfo12 %GT_U32",           name="pciHeaderInfo12",     help="pci header info 13 word"},
                  {format="pciHeaderInfo13 %GT_U32",           name="pciHeaderInfo13",     help="pci header info 14 word"},
                  {format="pciHeaderInfo14 %GT_U32",           name="pciHeaderInfo14",     help="pci header info 15 word"},
                  {format="pciHeaderInfo15 %GT_U32",           name="pciHeaderInfo15",     help="pci header info 16 word"},

                  mandatory = {"pciBusNum", "pciDevNum", "pciFunc", "devType", "swDevId"}
               }}
    })


    CLI_addCommand("exec", "autoInitSystem", autoSystemParams)
    CLI_addCommand("exec", "ezautoInitSystem", ezAutoSystemParams)
    CLI_addCommand("exec", "ezcpssInitSystem", ezInitSystemParams)

end

CLI_addAppDemoCommands()

-------------------------------------------------------
-- command registration: cpssAppPlatformSystemInitMain
-------------------------------------------------------
function cpssAppPlatformSystemInitMain(params)
    if params.ezb_xml_cap then
        if 1 == luaIsInclude_EZ_BRINGUP() then
            ret,val = myGenWrapper("ezbXmlLoad",{
                {"IN", "string", "fileNamePtr", params.ezb_xml_cap}
            })
            if ret ~= 0 --[[GT_OK]] then
                print ("Error : EZ_BRINGUP FAILD to load xml file : " .. to_string(params.ezb_xml))
                return false
            end
        else
            print ("Error : The image was not compiled with the 'EZ_BRINGUP' feature")
            return false
        end
    end
    ret,val = myGenWrapper("cpssAppPlatformSysInit",{
          { "IN", "string",    "profileName",   params.profileName}})

    local dev_list = wrlDevList();
    -- ini block for dxCh family only
    if dxCh_family == true then
      for dummy,dev in pairs(dev_list) do
          system_capability_managment({devID = dev});
      end
      print("PP Insert returned:"..to_string(ret))
    end

    -- init device specific global parameters
    fillDeviceEnvironment()

    if fs.exists("cmdDefs.lua") then
        require_safe("cmdDefs")
    end

    if (ret==0) then return true
    else return false end

end

function cpssAppPlatformSystemAddPpMain(params)
        ret,val = myGenWrapper("cpssAppPlatformSysPpAdd",{
            { "IN", "string",    "profileName",   params.profileName}})
end

function cpssAppPlatformSystemRunMain(params)
        ret,val = myGenWrapper("cpssAppPlatformSysRun",{
            { "IN", "string",    "profileName",   params.profileName}})
end

function cpssAppPlatformListProfilesMain(params)
if params.testProfiles then
        ret,val = myGenWrapper("cpssAppPlatformLsTestProfile",{})
else
        ret,val = myGenWrapper("cpssAppPlatformLsProfile",{})
end
end

function cpssAppPlatformSystemPrintProfile(params)
        ret,val = myGenWrapper("cpssAppPlatformShowProfile",{
            { "IN", "string", "profileName", params.profileName}})
end

function cpssAppPlatformSystemRemovePpMain(params)
    if params.removalType == nil then
        params.removalType = 0;
    end
        ret,val = myGenWrapper("cpssAppPlatformSysPpRemove",{
            { "IN", "GT_U8",   "cpssDevNum",   params.cpssDevNum },
            { "IN", "GT_U8",   "removalType",  params.removalType }})
end

function CLI_addCpssAppPlatCommands()
   ret,val = myGenWrapper("cpssEnablerIsAppPlat",{})
   if ret ~= 0 then
      return
   end

  CLI_addCommand("exec", "cpssPpAdd",
  {
      func=cpssAppPlatformSystemAddPpMain,
      help="Initialise PP using ASK PP profile",
      params={{type="named",
                {format="profileName %string", name="profileName", help="ASK PP profile name"},
                mandatory = {"profileName"}
             }}
  })

  CLI_addCommand("exec", "cpssSysInit",
  {
      func=cpssAppPlatformSystemInitMain,
      help="Initialise full system using ASK profile",
      params={{type="named",
                {format="profileName %string", name="profileName", help="ASK System profile name"},
                {format="%filename",name="ezb_xml_cap",  help="ez_bringup XML file name(relative or full path)"},
                mandatory = {"profileName"}},
             }
  })

  CLI_addCommand("exec", "cpssRuntimeInit",
  {
      func=cpssAppPlatformSystemRunMain,
      help="Initialise Run Time using ASK runtime profile",
      params={{type="named",
                {format="profileName %string", name="profileName", help="ASK runtime profile name"},
                mandatory = {"profileName"}
             }}
  })

  CLI_addCommand("exec", "cpssProfileLs",
  {
      func=cpssAppPlatformListProfilesMain,
      help="List all ASK profiles",
      params={{type="named",
                {format="testProfiles",name="testProfiles",  help="List all Validation/Test profiles"}
             }}
  })

  CLI_addCommand("exec", "cpssProfilePrint",
  {
      func=cpssAppPlatformSystemPrintProfile,
      help="Print a ASK profile. Use cpssProfileLs to see valid list",
      params={{type="named",
                {format="profileName %string", name="profileName", help="ASK profile name"},
                mandatory = {"profileName"}
             }}
  })

 CLI_addCommand("exec", "cpssPpRemove",
 {
     func=cpssAppPlatformSystemRemovePpMain,
     help="Remove Packet Processor",
     params={{type="named",
              {format="cpssDevNum %hirDevNum", name="cpssDevNum",  help="CPSS logical device number of the device"},
              {format="removalType %GT_U8",    name="removalType", help="Removal Type. Managed Removal -> 0, unManagedRemoval -> 1, managedReset -> 2"},
                  mandatory = {"cpssDevNum"}
            }}
 })

end

CLI_addCpssAppPlatCommands()

-- ************************************************************************
---
--  updateCLItypes()
--
--        @description   - run all registered callbacks in typeCallBacksList
--
--        @param         - devEnv
--
function updateCLItypes()

    -- is cpssInitSystem done?
    local sysInfo = luaCLI_getDevInfo();
    if type(sysInfo) ~= "table" then
        -- case of invoking before cpssInitSystem
        return;
    end

    local i
    local callbackFun
    local ret,err

    -- iterate thru all callbacks
    for i = 1, #typeCallBacksList do
        callbackFun = typeCallBacksList[i]
        ret,err = pcall(callbackFun)
        if not ret then
            print("updateCLItypes() failure in callback: "..err)
        end
    end
end

-- update device depend types by calling API's added to typeCallBacksList
updateCLItypes()

