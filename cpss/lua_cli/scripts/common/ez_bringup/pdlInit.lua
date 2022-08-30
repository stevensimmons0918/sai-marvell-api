--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlInit.lua
--*
--* DESCRIPTION:
--*       setting of pdl commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_init")

local default_xml_prefix = "mtsplt"

local function pdl_init(params)
    local api_result=1
    local xmlPrefix = params["xml-prefix"]
    if xmlPrefix == nil then
        xmlPrefix = default_xml_prefix --default
    end
    local filename = params["file-name"]
    if (".xml" ~= string.sub(filename, -4)) then
        -- currently only XML files supported so support the option for file name that is missing the file type
        filename = filename .. ".xml"
    end
    
    api_result = wr_utils_pdl_init(filename, xmlPrefix)
    if api_result ~= 0 then
        printMsg ("pdlInit failed rc = "..api_result)
    else
        printMsg ("pdlInit successful")         
    end
    return true
       
end

--------------------------------------------
-- Command registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "pdlInit", {
    func   = pdl_init,
    help   = "Initialize Platform Driver Layer which will allow access to peripheral devices (fans, led, power, etc.) according to data provided by XML",
--          Examples:
--          pdlInit SG550X_12PMV.xml
--          pdlInit /local/store/debugger_app_v0.1/SG550X_12PMV.xml mtsplt
    params = {
        { type = "values",
            {format="%string", name="file-name", help="XML file name (relative or full path file name can be provided)"},
            --{format="%string", name="xml-prefix", help="(optional) XML prefix (prefix used in XML file for the XML tags)"},
        },
        mandatory={"file-name"--[[, "xml-prefix"]]}
    }
})
