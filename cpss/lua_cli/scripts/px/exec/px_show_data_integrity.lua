--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_data_integrity.lua
--*
--* DESCRIPTION:
--*       Show PX Data integrity Info
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("common/generic/tables")
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityEventsGet");
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityConfigStatusGet");
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityShadowSizeGet");


local function init_events_tables()
	local tblCommon = {
		{ head="evType", path="eventInfo.eventsType",
			len=12,
			rmprefix="CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_",
			rmsuffix="_E" },
		{ head="hwTbl", path="eventInfo.location.hwEntryInfo.hwTableType",
			len=30,
			rmprefix="CPSS_PX_TABLE_",
			rmsuffix="_E" },
		{ head="HW ind", path="eventInfo.location.hwEntryInfo.hwTableEntryIndex",
			len=6 },
		{ head="logical Table", path="eventInfo.location.logicalEntryInfo.logicalTableType",
			len=25,
			rmprefix="CPSS_PX_LOGICAL_TABLE_",
			rmsuffix="_E" },
		{ head="memory type", path="eventInfo.location.ramEntryInfo.memType",
			len=25,
			rmprefix="CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_",
			rmsuffix="_E" }
	}

	local tblMem = Nice_Table(tblCommon, {
		{ head="Pipe", path="eventInfo.location.ramEntryInfo.memLocation.dfxPipeId",
			len=5 },
		{ head="Client", path="eventInfo.location.ramEntryInfo.memLocation.dfxClientId",
			len=7 },
		{ head="Memory", path="eventInfo.location.ramEntryInfo.memLocation.dfxMemoryId",
			len=7 },
		{ head="row", path="failedRow",
			len=5 },
		{ head="Segment", path="failedSegment",
			len=7 },
		{ head="Syndrome", path="failedSyndrome",
			len=7 },
		{ head="errCount", path="errorCounter",
            len=7 }
	})
	local tblMppm = Nice_Table(tblCommon, {
		{ head="mppm", path="eventInfo.location.ramEntryInfo.mppmMemLocation.mppmId",
		   len=7 },
	   { head="bank", path="eventInfo.location.ramEntryInfo.mppmMemLocation.bankId",
		   len=7 },
	   { head="errCount", path="errorCounter",
		   len=7 }
    })

	return tblMem,tblMppm
end


-- ************************************************************************
---
--  px_show_data_integrity_events
--        @description  shows  all Data Integrity events
--        @param params.devID   -
--
--        @return       true on success, otherwise false and error message
--
local function px_show_data_integrity_events(params)
    local rc, tbl, i
    local command_data = Command_Data()
    -- init tables
    local tblMem,tblMppm = init_events_tables()
        local devID = params.devID
		rc, tbl = wrlCpssPxDiagDataIntegrityEventsGet(devID)
        if rc ~= 0 then
               local error_string = "ERROR calling function wrlCpssPxDiagDataIntegrityEventsGet"
               command_data:addErrorAndPrint(error_string)
        end
		for i=1,#tbl do
			if  (tbl[i].eventInfo.location.ramEntryInfo.memLocation ~= nil) then
				tblMem:addRow(tbl[i])
            end
        end

		for i=1,#tbl do
			if  (tbl[i].eventInfo.location.ramEntryInfo.mppmMemLocation ~= nil) then
				tblMppm:addRow(tbl[i])
            end
        end

        if #tblMem.rows > 0 then
            tblMem:print()

        else
            print("No events")
        end
        if #tblMppm.rows > 0 then
            tblMppm:print()

        else
            print("No mppm events")
        end

end


CLI_addCommand("exec", "show data-integrity-events", {
  func   = px_show_data_integrity_events,
  help   = "Show PX data integrity events",
  params={{type= "named",
        {format="device %devID", name="devID", help="ID of the device"},
        mandatory = {"devID"}
    }
  }
})

local function init_status_table()
    local tblGen = {
        { head="logical Table", path="table",
            len=40,
            rmprefix="CPSS_PX_LOGICAL_",
            rmsuffix="_E" },
        { head="errorCountEnable", path="errorCountEnable",
            len=16},
        { head="EventMask", path="eventMask",
            len=10,
            rmprefix="CPSS_EVENT_",
            rmsuffix="_E"  },
        { head="injectEnable", path="injectEnable",
            len=16},
        { head="injectMode", path="injectMode",
            len=10,
            rmprefix="CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_",
            rmsuffix="_E"   },
        { head="Protection", path="protectionType",
            len=7,
            rmprefix="CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_",
            rmsuffix="_E"  }
    }

    local tblStatus = Nice_Table(tblGen, {
    })

    return tblStatus
end
-- ************************************************************************
---
--  px_show_data_integrity_status
--        @description  shows  all Data Integrity configuration status
--        @param params.devID   -
--
--        @return       true on success, otherwise false and error message
--
local function px_show_data_integrity_status(params)
    local rc
    local command_data = Command_Data()
    local table, tableMin, tableMax

    if (params["table"] == "all") then
        tableMin = 0
        tableMax = 17
    else
        tableMin = params["table"]
        tableMax = params["table"]
    end


    -- init tables
    local tblStatus = init_status_table()
    local devID = params.devID
    for table = tableMin, tableMax do

        rc, tbl = wrlCpssPxDiagDataIntegrityConfigStatusGet(devID, table)
        if rc ~= 0 then
            local error_string = "ERROR calling function wrlCpssPxDiagDataIntegrityConfigStatusGet"
            command_data:addErrorAndPrint(error_string)
        end

        tblStatus:addRow(tbl)
    end
    tblStatus:print()

end


CLI_addCommand("exec", "show data-integrity-status", {
  func   = px_show_data_integrity_status,
  help   = "Show PX data-integrity status",
  params={{type= "named",
        {format="device %devID", name="devID", help="ID of the device"},
        {format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
        mandatory = {"devID", "table"},
    }
  }
})


local function init_shadow_size_table()
    local tblForm = {
        { head="logical Table", path="table",
            len=40,
            rmprefix="CPSS_PX_LOGICAL_",
            rmsuffix="_E" },
        { head="size", path="tableSize",
            len=16}
    }

    local tblShadow = Nice_Table(tblForm, {
    })

    return tblShadow
end
-- ************************************************************************
---
--  px_show_data_integrity_shadow tables size
--        @description  shows  all Data Integrity events
--        @param params.devID   -
--
--        @return       true on success, otherwise false and error message
--
local function px_show_data_integrity_shadow(params)
    local rc
    local command_data = Command_Data()
    local table
   --only tables under CPSS control that need shadow
    local array_of_tables = {0,1,2,3,7,8,9}

    -- init tables
    local tblShadow = init_shadow_size_table()
    local devID = params.devID
    for index,value in pairs(array_of_tables) do
		table = value
        rc, tbl = wrlCpssPxDiagDataIntegrityShadowSizeGet(devID, table)
        if rc ~= 0 then
            local error_string = "ERROR calling function wrlCpssPxDiagDataIntegrityShadowSizeGet"
            command_data:addErrorAndPrint(error_string)
        end

        tblShadow:addRow(tbl)
    end
    tblShadow:print()

end

CLI_addCommand("exec", "show data-integrity-shadow-sizes", {
  func   = px_show_data_integrity_shadow,
  help   = "Show PX data-integrity shadow sizes",
  params={{type= "named",
        {format="device %devID", name="devID", help="ID of the device"},
        mandatory = {"devID"},
    }
  }
})


