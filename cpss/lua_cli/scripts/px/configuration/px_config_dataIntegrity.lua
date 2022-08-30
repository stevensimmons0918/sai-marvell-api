--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_config_data_integrity.lua
--*
--* DESCRIPTION:
--*       Configure Data Integrity
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_type_dict["px_logical_table"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Logical Tables",
    enum = {
	   ["ingress-dst-port-map-table"]                = { value =   0  , help = "LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E         " },
	   ["ingress-port-filtering-table"]              = { value =   1  , help = "LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E       " },
	   ["egress-header-alteration-table"]            = { value =   2  , help = "LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E     " },
	   ["tail-drop-limits"]                          = { value =   3  , help = "LOGICAL_TABLE_TAIL_DROP_LIMITS_E                   " },
	   ["buffers-manager-multicast-counters"]        = { value =   4  , help = "LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E " },
	   ["cnc-0-counters"]                            = { value =   5  , help = "LOGICAL_TABLE_CNC_0_COUNTERS_E                     " },
	   ["cnc-1-counters"]                            = { value =   6  , help = "LOGICAL_TABLE_CNC_1_COUNTERS_E                     " },
	   ["txq-qcn-cn-sample-intervals"]               = { value =   7  , help = "LOGICAL_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E        " },
	   ["pha-shared-dmem"]                           = { value =   8  , help = "LOGICAL_TABLE_PHA_SHARED_DMEM_E                    " },
	   ["pha-fw-image"]                              = { value =   9  , help = "LOGICAL_TABLE_PHA_FW_IMAGE_E                       " },
	   ["internal-table-management"]                 = { value =   10 , help = "LOGICAL_INTERNAL_TABLE_MANAGEMENT_E                " },
	   ["internal-table-packet-data-ecc"]            = { value =   11 , help = "LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E           " },
	   ["internal-table-packet-data-parity"]         = { value =   12 , help = "LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E        " },
	   ["internal-table-cm3-ram"]                    = { value =   13 , help = "LOGICAL_INTERNAL_TABLE_CM3_RAM_E                   " },
	   ["internal-table-pha-ram"]                    = { value =   14 , help = "LOGICAL_INTERNAL_TABLE_PHA_RAM_E                   " },
	   ["internal-table-packet-data-control-ecc"]    = { value =   15 , help = "LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E   " },
	   ["internal-table-packet-data-control-parity"] = { value =   16 , help = "LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E" },
	   ["internal-table-descriptors-control"]        = { value =   17 , help = "LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E       " },
       ["all"]                                       = { value = "all", help = "all tables"    }
    }
}
CLI_type_dict["px_err_injection_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Error Injection mode",
    enum = {
		["single"]   = { value =   0  , help = "error inject mode single  " },
		["multiple"] = { value =   1  , help = "error inject mode multiple" }
	}
}

CLI_type_dict["px_table_size_range"] = {
	checker = function(param)
	return split_string_to_separated_numbers(param);
	end,
	help = "The range of numbers. Example: 2-5 means startEntry = 2, endEntry = 5"
}



cmdLuaCLI_registerCfunction("wrlCpssPxDataIntegrityEventMaskSet")
cmdLuaCLI_registerCfunction("wrlCpssPxDataIntegrityErrorCountEnableSet")
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityErrorInjectionConfigSet")
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityTableEntryFix");
cmdLuaCLI_registerCfunction("wrlCpssPxDiagDataIntegrityTableScan");
--*******************************************************************************
--/**
--pxDiagDataIntegrityEventMaskSet function
--
--
--Function sets mask/unmask for ECC/Parity event.
--Event is masked/unmasked for whole hw/logical table or for all memories of specified type.
--
--@param[in] devNum                   - PP device number
--@param[in] memEntryPtr              - (pointer to) memory (table) location
--@param[in] errorType                - type of error interrupt
--                                     relevant only for ECC protected memories
--@param[in] operation                - mask/unmask interrupt
local function pxDataIntegrityEventMaskSet(params)
	local command_data = Command_Data()
	local table, tableMin, tableMax
	local devNum, operation

	if (params["table"] == "all") then
		tableMin = 0
		tableMax = 17
	else
		tableMin = params["table"]
		tableMax = params["table"]
	end

	devNum = params["device"]
    if params["flagNo"] == true then
		operation = 1
	else
        operation = 0
    end
	print(to_string(params))
	for table = tableMin, tableMax do

		print ("table",table)
		print ("operation",operation)
		local status = wrlCpssPxDataIntegrityEventMaskSet(devNum,table,operation)
		if status ~= 0 then
			local error_string = "ERROR calling function wrlCpssPxDataIntegrityEventMaskSet"
			command_data:addErrorAndPrint(error_string)
			--print (error_string,status)
		end
	end

	return status
end

--*******************************************************************************
--/**
--pxDiagDataIntegrityErrorCountEnableSet function
--
--
--Function sets mask/unmask for ECC/Parity event.
--Event is masked/unmasked for whole hw/logical table or for all memories of specified type.
--
--@param[in] devNum                   - PP device number
--@param[in] memEntryPtr              - (pointer to) memory (table) location
--@param[in] errorType                - type of error interrupt
--                                     relevant only for ECC protected memories
--@param[in] operation                - mask/unmask interrupt
local function pxDataIntegrityErrorCountEnableSet(params)
	local command_data = Command_Data()
	local table, tableMin, tableMax
	local devNum, countEnable

	if (params["table"] == "all") then
		tableMin = 0
		tableMax = 17
	else
		tableMin = params["table"]
		tableMax = params["table"]
	end

	devNum = params["device"]
	if params["flagNo"] == true then
		countEnable = 0
	else
		countEnable = 1
	end

	print(to_string(params))
	for table = tableMin, tableMax do

		print ("table",table)

		local status = wrlCpssPxDataIntegrityErrorCountEnableSet(devNum,table,countEnable)
		if status ~= 0 then
			local error_string = "ERROR calling function wrlCpssPxDataIntegrityErrorCountEnableSet"
			command_data:addErrorAndPrint(error_string)
			--print (error_string,status)
		end
	end

	return status
end
--*******************************************************************************
--/**
--pxDiagDataIntegrityErrorCountEnableSet function
--
--
--Function sets mask/unmask for ECC/Parity event.
--Event is masked/unmasked for whole hw/logical table or for all memories of specified type.
--
--@param[in] devNum                   - PP device number
--@param[in] memEntryPtr              - (pointer to) memory (table) location
--@param[in] errorType                - type of error interrupt
--                                     relevant only for ECC protected memories
--@param[in] operation                - mask/unmask interrupt
local function pxDataIntegrityErrorInjectionConfigSet(params)
	local command_data = Command_Data()
	local table, tableMin, tableMax
	local devNum, injectionConfig

	if (params["table"] == "all") then
		tableMin = 0
		tableMax = 17
	else
		tableMin = params["table"]
		tableMax = params["table"]
	end

	devNum = params["device"]
	mode   = params["mode"]
	if params["flagNo"] == true then
		injectionConfig = 0
	else
		injectionConfig = 1
	end



	print(to_string(params))
	for table = tableMin, tableMax do

		local status = wrlCpssPxDiagDataIntegrityErrorInjectionConfigSet(devNum,table,mode,injectionConfig)
		if status ~= 0 then
			local error_string = "ERROR calling function wrlCpssPxDataIntegrityErrorinjectionConfigSet"
			command_data:addErrorAndPrint(error_string)
			--print (error_string,status)
		end
	end

	return status
end

--*******************************************************************************
--/**
--pxDiagDataIntegrityTableEntryFix function
--
--
--Function fixes the referenced entry in the referenced table.
--
--@param[in] devNum                   - PP device number
--@param[in] memEntryPtr              - (pointer to) memory (table) location
local function pxDataIntegrityTableEntryFix(params)
	local command_data = Command_Data()
	local table, tableMin, tableMax
	local devNum, index

	if (params["table"] == "all") then
		tableMin = 0
		tableMax = 17
	else
		tableMin = params["table"]
		tableMax = params["table"]
	end

	devNum = params["device"]
	index  = params["index"]

	print(to_string(params))
	for table = tableMin, tableMax do

		local status = wrlCpssPxDiagDataIntegrityTableEntryFix(devNum,table,index)
		if status ~= 0 then
			local error_string = "ERROR calling function wrlCpssPxDataIntegrityTableEntryFix"
			command_data:addErrorAndPrint(error_string)
			--print (error_string,status)
		end
	end

	return status
end


local function pxDataIntegrityScanTable(params)
	local command_data = Command_Data()
	local table, startEntry, endEntry
	local devNum


	devNum = params["device"]
	table  = params["table"]

	print(to_string(params))

	if params["start"] == nil then
		startEntry = 0
	else
		startEntry =  params["start"]
	end

	if params["number"] == nil then
		numberOfEntries = 0xFFFFFFFF
	else
		numberOfEntries =  params["number"]
	end

	local status = wrlCpssPxDiagDataIntegrityTableScan(devNum,table,startEntry,numberOfEntries)
	if status ~= 0 then
		local error_string = "ERROR calling function wrlCpssPxDiagDataIntegrityTableScan"
		command_data:addErrorAndPrint(error_string)
		--print (error_string,status)
	end
end



--******************************************************************************
-- add help: data-integrity
-- add help: data-integrity event-mask-set
--******************************************************************************
CLI_addHelp("config", "data-integrity", "Configure PIPE Data Integrity")
CLI_addHelp("config", "data-integrity event-mask-set", "Configure mask for ECC/Parity event")
--******************************************************************************
-- command registration: data-integrity event-mask-set
--******************************************************************************
CLI_addCommand("config", "data-integrity event-mask-set", {
	func = pxDataIntegrityEventMaskSet,
	help ="Configure mask for ECC/Parity event",
	params = {
		{
			type = "named",
			{format = "device %devID",name = "device",help = "The device ID"},
			{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
			mandatory = {"device", "table"},
		}
	}
})

--******************************************************************************
-- command registration: data-integrity event-mask-unset
--******************************************************************************
CLI_addCommand("config", "no data-integrity event-mask-set", {
    func=function(params)
        params["flagNo"]= true
	    return pxDataIntegrityEventMaskSet(params)
    end,
    help="Unmask Data Integrity Event",
	params = {
		{
		type = "named",
		{format = "device %devID",name = "device",help = "The device ID"},
		{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
		mandatory = {"device", "table"},
		}
}
})
CLI_addHelp("config", "data-integrity error-counter-enable", "enable error-counter")
--******************************************************************************
-- command registration: data-integrity error-counter-enable
--******************************************************************************
CLI_addCommand("config", "data-integrity error-counter-enable", {
	func = pxDataIntegrityErrorCountEnableSet,
	help ="enable error counter",
	params = {
		{
			type = "named",
			{format = "device %devID",name = "device",help = "The device ID"},
			{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
			mandatory = {"device", "table"},
		}
	}
})

CLI_addCommand("config", "no data-integrity error-counter-enable", {
    func=function(params)
        params["flagNo"]= true
	    return pxDataIntegrityErrorCountEnableSet(params)
    end,
    help="Disable Data Integrity Error Counter",
	params = {
		{
		type = "named",
		{format = "device %devID",name = "device",help = "The device ID"},
		{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
		mandatory = {"device", "table"},
		}
}
})

CLI_addHelp("config", "data-integrity error-injection-config", "Configure error injection")
--******************************************************************************
-- command registration: data-integrity error-injection-config
--******************************************************************************
CLI_addCommand("config", "data-integrity error-injection-config", {
	func = pxDataIntegrityErrorInjectionConfigSet,
	help ="Configure error injection",
	params = {
		{
			type = "named",
			{format = "device %devID",name = "device",help = "The device ID"},
			{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
			{format = "mode %px_err_injection_mode",name = "mode",help = "Data Integrity error injection mode"},

			mandatory = {"device", "table","mode"},
		}
	}
})

CLI_addCommand("config", "no data-integrity error-injection-config", {
    func=function(params)
        params["flagNo"]= true
	    return pxDataIntegrityErrorInjectionConfigSet(params)
    end,
    help="Disable Data Integrity error injection",
	params = {
		{
		type = "named",
		{format = "device %devID",name = "device",help = "The device ID"},
		{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
		{format = "mode %px_err_injection_mode",name = "mode",help = "Data Integrity error injection mode"},
		mandatory = {"device", "table","mode"},
		}
}
})

CLI_addHelp("config", "data-integrity fix-table", "Fix table entry")
--******************************************************************************
-- command registration: data-integrity fix-table
--******************************************************************************
CLI_addCommand("config", "data-integrity fix-table", {
	func = pxDataIntegrityTableEntryFix,
	help ="Fix table entry",
	params = {
		{
			type = "named",
			{format = "device %devID",name = "device",help = "The device ID"},
			{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
			{format = "index %GT_U32",name = "index",help = "index of the entry"},

			mandatory = {"device", "table","index"},
		}
	}
})

CLI_addHelp("config", "data-integrity scan-table", "Scan table entries")
--******************************************************************************
-- command registration: data-integrity fix-table
--******************************************************************************
CLI_addCommand("config", "data-integrity scan-table", {
	func = pxDataIntegrityScanTable,
	help ="Scan table entries",
	params = {
		{
			type = "named",
			{format = "device %devID",name = "device",help = "The device ID"},
			{format = "table %px_logical_table",name = "table",help = "Data Integrity table index"},
			{format = "start %GT_U32",name = "start",help = "start index to scan"},
			{format = "number %GT_U32",name = "number",help = "number of entries to scan to scan"},

			mandatory = {"device", "table"},
		}
	}
})


